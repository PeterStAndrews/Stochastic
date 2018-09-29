// (C) Peter Mann (2018) [University of St Andrews].
// `main` function to run the Gillespie simulation algorithm for the SIR model.
// This simulates multiple runs of the stochastic SIR experiment and prints the 
// final size of the epidemic to a file. The for loop through the infectivities 
// is partitioned onto different cores in an attempt to speed up the computation.
// Each Gillespie algorithm runs as a complete serial unit and is not affected by 
// the parallelization. 
//
// Compile withing the same directory as `Gillespie_SIR.cpp_SIR.cpp` and `Gillespie.h/.cpp`
//
//  mpic++ -std=c++11 *.cpp
//  mpirun -np 2 ./a.out

#include <iostream>
#include <fstream>
#include <map>
#include <random>
#include <chrono>
#include <mpi.h>

#include "Gillespie_SIR.cpp"	  // import SIR-type experiment

int main(int argc, char *argv[]){

	// number of trajectories
	int N = 100;

	// declare model params map
	std::map<std::string, double > params;

	// populate params map 
	params["gamma"] = 0.25;

	// open an output data file
	std::ofstream output_file;
	output_file.open("multi_run.txt", std::ios_base::app);

	// Record start time
	auto start = std::chrono::high_resolution_clock::now();

	// instance class
	User_Definied ud;

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

		// update beta
		params["beta"] = (double) i/100;

		// run algorithm
		ud._do(params, output_file);

		// reset the algorithm for the next run (?)
		ud.reset();
		
	}

	// Record end time
	auto finish = std::chrono::high_resolution_clock::now();

	// compute elapsed time
	std::chrono::duration<double> elapsed = finish - start;

	// print to console
	std::cout << "Elapsed time: " << elapsed.count() << "s for process: " << procid << "\n";

	// close the output file
	output_file.close();

	ierr = MPI_Finalize();

}






