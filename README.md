# Stochastic

A collection of algorithms that integrate a coupled system of ODEs using stochastic kinetic Monte Carlo techniques. To showcase the usage of these implementations, the SIR system is solved for range of infectivities. In order to increase performance, the exploration of the models parameter space has been made parallel using the MPI library. Therefore, when using these scripts, compile using `mpic++ -std=c++11 filename.cpp` and run using `mpirun -np n ./a.out` where $n$ is the number of cores on your machine. The load on each core is allocated evenly. 

## Gillespie

`Gillespie` contains an implementation of the direct stochastic simulation algorithm by Gillespie [1,2]. To use this class, simply import the `Gillespie.h` header file into your script and subclass it to override the virtual methods as has been done for the SIR system in `Gillespie_SIR.cpp` and run the experiment in a `main()` function (see `multi_run.cpp`). For this experiment, we track the final epidemic size as a function of the infectivity of the disease. The output can be plotted to generate: 

![picture1](https://user-images.githubusercontent.com/29250174/46246444-904a6a80-c3f5-11e8-8ca6-138b137d0bb8.png)

subject to stochastic fluctuations. 

[1] Daniel Gillespie. A general method for numerically simulating the stochastic time evolution of coupled chemical reactions. Journal of Computational Physics 22, pages 403-–434. 1976.

[2] Daniel Gillespie. Exact stochastic simulation of coupled chemical reactions. Journal of Physical Chemistry 81(25), pages 2340-–2361. 1977.
