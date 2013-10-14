program has three command line arguments

1. file name
2. bidirectional flag
3. number of processors

to use nqmq.dat as the source of information about routes between cities
assuming routes are bidirectional (i can get to toledo from cleveland and to cleveland from toledo)
and i want to use 4 processors i initiate the program with the command

on windows
	floyd nqmq.dat 1 4

on unix
	./floyd nqmq.dat 1 4