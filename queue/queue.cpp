// (C) Peter Mann (2018) [University of St Andrews].
// ======================================================================
// Strategy:
// ======================================================================
// A network SIR experiment based on a priority queue of events. 

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iterator>
#include <random>
#include <algorithm>
#include <chrono>
#include <map>
#include <queue>
#include <mpi.h>

// ======================================================================
// Typedef
// ======================================================================
// Variable types:

struct EDGE {
    // type: edge struct
    // definition: creates a edge data between two node IDs
    // and overloads << to print.

    int i;                              // focal node
    int j;                              // neighbour node

    EDGE( int i, int j) : i(i), j(j) {} // constructor

                                        // overload stream operator
    friend std::ostream& operator<<(std::ostream& s, const EDGE & e) {
        return s << e.i << " " << e.j;
    }
};          

struct NODE{
    // type: node struct
    // definition: creates a node object.

    unsigned int ID;                    // unique node id 
    std::vector<EDGE> EDGES;            // list of nodes neighbours
    int STATUS;                         // infection status {-1,0,1}

    NODE( unsigned int ID, std::vector<EDGE> EDGES, 
        int STATUS) : ID(ID), EDGES(EDGES), STATUS(STATUS) {} 
};

struct EVENT {
    // type: event struct
    // definition: creates an event object

    double time;                        // time of event
    std::string name;                   // name of event
    unsigned int location;              // event location (node ID)
    EVENT( double time, std::string name, unsigned int location) : time(time), name(name), location(location) {}
};

struct COMPARISON {
    // type: comparison struct
    // definition: auxillary struct for priority queue. Compares 
    // two event times and returns true if the left hand 
    // side is after the right hand side. Using greater than `>` 
    // gives the queue a `lowest first` ordering.

    bool operator() (const EVENT& l, const EVENT& r) const {
            return (l.time > r.time);
        }
};

typedef std::vector<EDGE> EDGES;        // type: list of edges
typedef std::vector<NODE> NODES;        // type: list of nodes
typedef std::vector<EDGES> CONTACTS;    // type: list of list of edges
typedef std::vector<EVENT> EVENTS;      // type: list of events

                                        // create priority queue object
std::priority_queue < EVENT, EVENTS, COMPARISON > queue;

// ======================================================================
// Main:
// ======================================================================

int main ( int argc, char *argv[] ) {


    unsigned int ID = 0;
    CONTACTS contacts;                  // initialise list of edge lists

    std::ifstream theStream("data.txt");
    if (!theStream)
        std::cerr << "data.txt\n";

    while (true) {                      // while lines in file   
        std::string line;
        std::getline(theStream, line);  // grab a line as a string
        if (line.empty())
            break;

        std::istringstream myStream(line); 
        std::istream_iterator<int> begin(myStream), eof; 
        std::vector<int> numbers(begin, eof); 

        EDGES neighbours;               // vector of edges
        
        for (int i =0; i< numbers.size(); i++){
            EDGE e(ID,numbers[i]);      // EDGE object     
            neighbours.push_back(e);    // append edge to nodes neighbour list
        }
        contacts.push_back(neighbours);
        ++ID;                           // increment node ID counter
    }

    NODES nodes;                        // instance nodes list
    for (std::vector<int>::size_type n = 0; n != contacts.size(); ++n) {
        NODE node{static_cast<unsigned int>(n), contacts[n], -1};
        nodes.push_back(node);
    }

    NODES _nodes = nodes;               // variable to store untouched node list
    CONTACTS _contacts = contacts;      // variable to store untouched contacts list

    // random number seed and generator for Mersenne Twister
    std::mt19937 rng;

    // initialize the random number generator with time-dependent seed
    unsigned timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(timeSeed);

    // initialize a uniform distribution between 0 and 1
    std::uniform_real_distribution<double> uniform(0, 1);

    // generate a uniform distribuition betwen 1.0 and the network order
    std::uniform_real_distribution<double> distribuition(1, nodes.size()-1);

    unsigned int N = 100;                // number of points to take

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // initialise the MPI 
    int ierr = MPI_Init(&argc, &argv);
    int procid, numprocs;

    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &procid);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &numprocs);

    // partition = job over processors. 
    unsigned int partition = N / numprocs;

    // if N not divisible by numprocs:
    if (N % numprocs > 0) {
        partition++;
        N = partition * numprocs;
    }

    // iterate over partitions on processors
    for (unsigned int i = procid*partition; i<= partition * (procid + 1); i++){

        double beta = (double) i/N;         // compute infectivity
        double gamma = 0.1;                 // recovery rate per site
        nodes = _nodes;                     // grab a copy of the network topology

        double sum;                         // sum of rates
        bool propensity = true;             // declare bool type for sum of rates

        double r1, r2, r3;                  // declare random numbers
        double tau_t, tau_r;                // declare time interval(s)

        unsigned int I = 1;                 // counter for number of infected nodes
        unsigned int R = 0;                 // counter for number of recovered nodes

       // ======================================================================
       // Simuation
       // ======================================================================

        double t = 0.0;                     // start time
                                            
        unsigned int root = 24;             // select index for root of infected tree         
        nodes[root].STATUS = 0;             // set status to infected (0)
                                  
        r1 = uniform(rng);                  // sample time interval until transmission/recovery
        r2 = uniform(rng);

        tau_t = -log((double)r1/INT_MAX) / beta;
        tau_r = -log((double)r2/INT_MAX) / gamma;

        while ( tau_t < tau_r ){            // while transmission time < recovery time
            queue.push( EVENT(tau_t, "infection", nodes[root].ID));
            tau_t += -log((double)r1/INT_MAX) / beta;
        }
                                            // post recovery event
        queue.push( EVENT(tau_r, "recovery", nodes[root].ID));

        while(propensity and I != 0){

            if (queue.empty()){
                propensity = false;
            }
            else{
                EVENT e = queue.top();      // grab the next event
                t += e.time;                // increment the time

                // play it out
                if(e.name == "infection"){
                    ++I;
                    // pick a neighbour of `nodes[e.location]` at random. 
                    // To do this, we shuffle the vector containing the nodes
                    // edges (Fisher-Yates algorithm) and pick the first 
                    // element as our edge of choice.
                    std::shuffle(nodes[e.location].EDGES.begin(), nodes[e.location].EDGES.end(), rng);
                    unsigned int j = nodes[e.location].EDGES[0].j;

                    if(nodes[j].STATUS < 0){
                                            // infect the neighbour
                        nodes[j].STATUS = 0; 

                        r1 = uniform(rng);  // sample time interval until transmission/recovery
                        r2 = uniform(rng);

                        tau_t = -log((double)r1/INT_MAX) / beta;
                        tau_r = -log((double)r2/INT_MAX) / gamma;

                                            // while transmission time < recovery time
                                            // post contacts.
                        while ( tau_t < tau_r ){            
                            queue.push( EVENT(tau_t, "infection", nodes[j].ID));
                            tau_t += -log((double)r1/INT_MAX) / beta;
                        }
                                            // post recovery event
                        queue.push( EVENT(tau_r, "recovery", nodes[j].ID));

                    }
                }

                if(e.name == "recovery"){
                    nodes[e.location].STATUS = 1;
                    ++R;
                    --I;
                }
                queue.pop();                // the event has been carried out so remove from queue
            }
        }

        double epidemic = (double) R / (double) nodes.size();
        std::cout << beta << " " << epidemic  << " a" << "\n";

    }

    ierr = MPI_Finalize();

    // Record end time
    auto finish = std::chrono::high_resolution_clock::now();

    // compute elapsed time
    std::chrono::duration<double> elapsed = finish - start;

    // print to console
    std::cout << "Elapsed time: " << elapsed.count() << "\n";

    return 0;
}





