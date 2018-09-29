// (C) Peter Mann (2018) [University of St Andrews].
// Class to house the user sub-class for `Gillespie.cpp` stochastic simulation. 
//
// This example integrates an SIR system of ODEs.

#include <iostream>
#include <fstream>
#include <map>

#include "Gillespie.h"

class User_Definied : public Gillespie {
	// class: User_Definied 
	// methods: initialise_state, infection_event_rate, recovery_event_rate,
	// infection_event_consequence, recovery_event_consequence, map_update.
	// definition: Class houses the model details for the simulation. The user 
	// must declare the initial state as a map object, un-pack the model 
	// parameters, write functions that return the rate of each event (as double)
	// and define the consequences of these events. Lastly, the user should add their
	// functions to the `rates` and `updates` map in `map_update`. While this method
	// seems contrived, each step is simple to implement and allows the user to 
	// build their model with ease.  

	double beta, gamma;

	public:
		// constructor for class 
		User_Definied() { }

		std::map<std::string, int> initialise_state (  ) {
			// function: initialise_state
			// definition: returns the state map with {key:val} pairs

			// map to store current state 
			std::map<std::string, int> states;

			// populate map with {key:val} pairs
 			states["S"] = 10000;
			states["I"] = 500;
			states["R"] = 0;

			return states;
		}

		double infection_event_rate ( std::map<std::string, int> states, double beta) { 
			// function: infection_event_rate 
			// arg(s): map( string, int ), beta (double)
			// definition: returns the infection event rate for a given state
			return beta * states["S"] * states["I"] / (states["S"] + states["I"] + states["R"]); 
		}

		double recovery_event_rate ( std::map<std::string, int> states, double gamma) { 
			// function: recovery_event_rate 
			// definition: returns the recovery event rate for a given state
			return gamma * states["I"];
		}

		std::map<std::string, int> infection_event_consequence ( std::map<std::string, int> states ){ 
			// function: infection_event_consequence
			// arg(s): map ( string, int )
			// definition: plays out the infection event on the current state
			states["S"] -= 1;
			states["I"] += 1;

			return states;
		}

		std::map<std::string, int> recovery_event_consequence ( std::map<std::string, int> states ){ 
			// function: recovery_event_consequence
			// arg(s): `states` (map, string, int )
			// definition: plays out the recovery event on the current state.
			states["I"] -= 1;
			states["R"] += 1;

			return states;
		}

		Retval2 map_update ( std::map<std::string, int> states,
					std::map<std::string, double > params  ){
			// function: map_update
			// arg(s): `states` (map, string, int), params (map, string, double)
			// definition: refreshes the `rates` and `updates` maps upon each call
			// returning a tuple of map objects.

			// Declare new updates and rates objects for local scope
			std::map<std::string, std::map< std::string, int> > updates;
			std::map<std::string, double > rates;
			
			// unpack the model parameters
			beta = params["beta"];
			gamma = params["gamma"];

			// populate rates map
			rates["infection"] = infection_event_rate (states, beta);
			rates["recovery"] = recovery_event_rate (states, gamma);

			// populate updates map
			updates["infection"] = infection_event_consequence(states);
			updates["recovery"] = recovery_event_consequence(states);

			return { rates, updates };
		}

		void output ( std::ofstream &output_file, std::map<std::string, double > params, 
		std::map<std::string, int> states ){
		// class: Gillespie
		// function: output
		// arg(s): output_file (std::ofstream&), params (map, string, double),
		//		   std::map<std::string, int>  states
		// definition: prints output to file
			output_file << params["beta"] << ", " << states["R"]<< "\n";
		}
};

