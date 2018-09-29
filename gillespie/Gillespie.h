// (C) Peter Mann (2018) [University of St Andrews].
// Class to house main Gillespie simulation. 

#include <iostream>
#include <fstream>
#include <map>


struct Retval1{
	// struct that provides return type for tuple overload in `draw`
	double d;
	std::string str;
};

struct Retval2{
	// struct that provides return type for tuple overload in `map_update`
	std::map<std::string, double > sd;
	std::map<std::string, std::map< std::string, int> > ssi;
};

class Gillespie {
	// class: Gillespie
	// methods: _do, event_action, draw, initialise_state, map_update.
	// definition: houses main Gillespie algorithm.
	// map objects have been chosen as the back bone of 
	// this implementation simply due to their ease of use
	// for the User_Defined subclass. This section includes the
	// declarations for the class.

	public:
		// constructor for class
		Gillespie( ) { }

		std::ofstream output_file;

		// declarations for time, time increment and random numbers
		double t, tau, r1, r2;
		
		// declaration for chosen event key string
		std::string chosen_key;

		// 'while loop' condition
		bool propensity = true;
		
		// declare maps to store: event rates, model params, update calls
		std::map<std::string, double> rates;
		std::map<std::string, double > params;
		std::map<std::string, std::map< std::string, int> > updates;

		void _do (std::map<std::string, double > params, std::ofstream& output_file );

		std::map<std::string, int>  event_action ( std::map<std::string, int>  states,
			std::map<std::string, std::map< std::string, int> > updates, std::string chosen_key );

		// struct return type
		Retval1 draw ( double t, std::map<std::string, double> rates, double r1, double r2); 
		
		void reset ( );
		
		// pure virtual functions for later overloading
		virtual void output ( std::ofstream &output_file, std::map<std::string, double > params, std::map<std::string, int>  states ) = 0;

		virtual std::map<std::string, int> initialise_state ( ) = 0;

		virtual Retval2 map_update ( std::map<std::string, int> states, std::map<std::string, double > params ) = 0;
};







