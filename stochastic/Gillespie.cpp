// (C) Peter Mann (2018) [University of St Andrews].
// Class to house main Gillespie simulation. 

#include <iostream>
#include <fstream>
#include <map>
#include <random>
#include <chrono>

#include "Gillespie.h"

Retval1 Gillespie::draw ( double t, std::map<std::string, double> rates, double r1, double r2) {
	// class: Gillespie
	// function: draw
	// arg(s): current time (double), rates(map, string, double), two random numbers (double)
	// definition: pick a time increment `tau` (double) and a key (a string that matches an event key)
	// according to the Gillespie method and return selected pair as a touple.
			
	// refresh variables upon each call
	double sum = 0.0;
	double ev  = 0.0;
	double tau_; 				
	std::string chosen_key_;

	// Gillespie draw for current time
	// compute sum of rates by iterating over {key:val} pair in map
	// myPair.first is key, myPair.second is val.
    for ( const auto &myPair : rates ) {
        sum += myPair.second;
    }
    // check if sum of rates is non-zero ... 
    if ( sum > 0){
    	// multiply sum of rates by random number
    	r1 *= sum;
    	// iterate over the rate values and sum until
    	// the sum exceeds the modulated r1 value, 
    	// when this occurs, the key is chosen.
    	for ( const auto &myPair : rates ) {
        	ev += myPair.second;
        	if (r1 < ev) {
        		chosen_key_ = myPair.first;
        		break;
        	}
    	}
    	// sample time interval tau
    	tau_ = -log((double)r2/INT_MAX) / sum;

    }
    // if sum of rates is zero, break while loop condition
    else {
    	propensity = false;
    }
    // return tuple of time increment and chosen key
	return { tau_, chosen_key_ };
}


void Gillespie::_do (std::map<std::string, double > params, std::ofstream &output_file ){
	// class: Gillespie
	// function: _do
	// arg(s): map (string, double), stores model params
	// definition: performs the Gillespie algorithm.

	// random number seed and generator for Mersenne Twister
	std::mt19937_64 rng;

	// initialize the random number generator with time-dependent seed
    unsigned timeSeed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(timeSeed);

    // initialize a uniform distribution between 0 and 1
   	std::uniform_real_distribution<double> uniform(0, 1);
			
  	// initialise simulation time
	t = 0.0; 

	// declare/initialise map to store current state 
	std::map<std::string, int> states = initialise_state ( );

	// start while loop
	while(propensity){

		// call user defined function to update map values with the current state
		auto retval2data = map_update (states, params);
		rates = retval2data.sd;
		updates = retval2data.ssi;

		// generate 2 random numbers
		r1 = uniform(rng);
		r2 = uniform(rng);

		// call `draw` function to select a (time,key) = (double, string) pair
		// (requires current time and for the `map_update` function to be called prior to this call)
		// std:tie is used to map the variables to the function return.
		// When sum of rates is zero, `propensity` will be set to false. The loop will break 
		// in the next iteration.
		auto retval1data = draw (t, rates, r1, r2);
		chosen_key = retval1data.str;
		tau = retval1data.d;

		// if sum of rates is not zero
		if(propensity){
			// carry out updates to state as specified by the chosen key
			states = event_action ( states, updates, chosen_key);

			// increment the time
			t += tau;
		}	
	}
	// print experiment output to file
	output(output_file, params, states);
}

std::map<std::string, int> Gillespie::event_action ( std::map<std::string, int>  states,
	std::map<std::string, std::map< std::string, int> > updates, std::string chosen_key ){
	// class: Gillespie
	// function: event_action 
	// arg(s): states (map, string, int), updates (map, string, map, string, int), `chosen_key` (string)
	// definition: performs update specified by `chosen_key` on `state`.
	// returns an updated state map
	return updates[chosen_key];
}

void Gillespie::reset ( ) {
	// class: Gillespie
	// function: reset
	// definition: called once this experiment is completed
	// (the while loop in `_do` has broken) to reset the experiment
	propensity = true;
}






