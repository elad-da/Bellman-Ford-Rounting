#Bellman-Ford Routing
- BlRouting.h - Header file of routing Bellman-Ford algorithm implementation.
- BlRouting.c - Source file of routing Bellman-Ford algorithm implementation.
- main.c - Main file creates threads and socket connections, execute routing with bellman ford algorithm.
- Makefile - Execute and runs the program: 

make - Creation of an executable Bellman-Ford with sockets connections.
make clean - erases without any further intervention the above executable.
Run Program - ./bf <file name> <Router name> <Attempts to connect>.
Notes:
- Program takes a few seconds to calculate the distance vectors, due to the check that all routers connected.
