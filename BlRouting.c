#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "BlRouting.h"

//Creates a graph with V vertices and E edges
Graph* createGraph(int V, int E)
{
    Graph* graph; 
    
    //Attempt to allocate memory for Graph structure
    if((graph = (struct Graph*) malloc(sizeof(Graph))) == NULL)
    {
        return NULL;
	}
    
    graph->V = V;
    graph->E = E;
    
    //Attempt to allocate memory for Edge structure
    if((graph->edges = (Edge*) malloc(sizeof(Edge) * E)) == NULL)
    {
        return NULL;
    }

    //Attempt to allocate memory for Router structure
	if((graph->routers = (Router*) malloc(sizeof(Router) * V)) == NULL)
	{
        return NULL;
    }
    
    return graph;
}

//free graph structure
void freeGraph(Graph *graph)
{
    int i;
    for (i = 0; i < graph->E; ++i)
    {
        free(graph->edges[i].src);
        graph->edges[i].src = NULL;
        free(graph->edges[i].dest);
        graph->edges[i].dest = NULL;
    }
    free(graph->edges);
    graph->edges = NULL;
    
    for (i = 0; i < graph->V; ++i)
    {
        free(graph->routers[i].DV);
        free(graph->routers[i].pi);
        free(graph->routers[i].ip);
        free(graph->routers[i].routerName);
    }

    free(graph->routers);
    graph->routers = NULL;

    free(graph);
    graph = NULL;
}

//find the index of a specific router
int findIndex(Graph* graph ,char* vertex)
{  
    int V = graph->V;
    int i;
    for(i = 0; i < V; i++)
    {
        if(strcmp(vertex,graph->routers[i].routerName) == 0)
        {
            return i;
        }
    }
    return ERROR;
}

//print DV table
void printDV(char* name, int* DV, int *pi, int V, Router *routers)
{
    int i;
    printf("===DV of: %s===\n", name);

    for (i = 0; i < V; ++i)
    {
        printf("%s\t", routers[i].routerName);

        if(pi[i] == -2)
            printf("%s\t", name);
        else if(pi[i] == -1)
            printf("NO-PATH\t");
        else            
            printf("%s\t", routers[pi[i]].routerName);

        if(DV[i] == INT_MAX)
            printf("Infinity\n");
        else
            printf("%d\n",DV[i]);
    }
}

//add a neighbor to neighbors list
void addToNeiArray(char* nei, Router* router)
{
  int i;
  for (i = 0; i < router->neighbors; ++i)
  {

    if (router->myNeighbors[i] == NULL)
    {
     
        router->myNeighbors[i] = malloc(strlen(nei) + 1);
        strcpy(router->myNeighbors[i],nei);

        break;
    }
  }
}

//finds a neighbor in the neighbors list
int findNeiIndex(char* nei, Router* router)
{  
  int i;
  for (i = 0; i < router->neighbors; ++i)
  {
    if (strcmp(router->myNeighbors[i], nei) == 0)
    {
        return i;
    }
  }
  return ERROR;
}
