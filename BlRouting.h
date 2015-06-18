#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ERROR -1
#define INT_MAX 99999


//Router structure
typedef struct Router
{
	char* routerName;
	int* DV;
	int* pi;
	int port;
	char* ip;
	int neighbors;
	char** myNeighbors;
}Router;

typedef struct Edge
{
    char *src, *dest;
    int weight;
}Edge;
 
// a structure to represent a connected, directed and weighted graph
typedef struct Graph
{
	// V: Number of vertices, E: Number of edges
    int V, E;

	//All the vertices
	Router* routers;

    // graph is represented as an array of edges
    Edge *edges; 
    

}Graph;

//create graph structure
Graph* createGraph(int V, int E);

//free graph structure
void freeGraph(Graph* graph);

//find the index of specific router
int findIndex(Graph* graph ,char* vertex);

//print DV table
void printDV(char* name, int* DV, int *pi, int V, Router *routers);

//add neighbor to neighbors list
void addToNeiArray(char* nei, Router* router);

//find a neifgbor in the neighbors list
int findNeiIndex(char* nei, Router* router);
