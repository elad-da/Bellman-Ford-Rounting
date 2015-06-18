#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>//defines data types used in system source code such as: key_t, pid_t, pthread_t and more.
#include <sys/socket.h>//Core socket functions and data structures.
#include <netdb.h>//For network database operations.
#include <netinet/in.h>//For Internet address.
#include <unistd.h>
#include <pthread.h>//for creating threads
#include <arpa/inet.h>//for inet_addr
#include <signal.h>//for signal
#include <fcntl.h>

#include "BlRouting.h"

int dvIsChaneged = 1;//flag for dv changing
int calc = 0;//check if calculator is finish

//lock and cond.variables
pthread_cond_t senderCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t calcCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutexLock = PTHREAD_MUTEX_INITIALIZER;

#define MAX_CONNECTION 10 //for listening

//argumants structure for receiver threads
typedef struct argsOfThreadRecieve
{
	int port;
	char* name;
	int dvCount;
	int neighbors;
	char* ip;
	Router* mySelf;
	int** neighborsDv;

}argsOfThreadRecieve;

//argumants structure for sender threads
typedef struct argsOfThreadSender
{
	int port;
	char* name;
	int dvCount;
	int conLoop;
	char* ip;
	int* routerDV;
	int neighbors;
	int** neighborsDv;

}argsOfThreadSender;

//argumants structure for calculator threads
typedef struct argsOfThreadClac
{
	Router* routers;
	int dvCount;
	int* routerDV;
	int index;
	int neighbors;
	int* pi;
	char* name;
	int** neighborsDv;

}argsOfThreadClac;


int calcPort(char* name, int port);//calc port with ascii value

void createReciever(argsOfThreadRecieve *args);//reciever function

void createSender(argsOfThreadSender *args);//sender function

void createCalc(argsOfThreadClac *args);//calculator function


//free functions
void freeThreads(pthread_t* tR, pthread_t* tS, argsOfThreadSender* argsS, argsOfThreadRecieve* argsR);

void freeGlobalDv(int** globalDv, int neighbors);


int main(int argc, char* argv[])
{
	FILE *fp;
	Graph *graph = NULL;
	int V = 0,E = 0;
	int lines = 0;
	char ch[256];
	char temp;
	char* buf;
	int i, j, index;
	
	//Variables for reading file
	char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char* inputRouter = argv[2];



	if(argc != 4)
	{
		printf("Error, there are missing arguments or to much arguments.\n");
		printf("You should enter the command in the following format: ./<Executable name> <File name> <source vertex> <num of connect loops>\n");
		return 0;
	}
	else
	{
		fp = fopen(argv[1],"r"); // read mode
 	
 		//If the file does not open properly
	   	if( fp == NULL )
	   	{
	   		perror("Error while opening the file.\n");
	      		exit(EXIT_FAILURE);
	   	}
	}

   	//Initializes the number of vertices in the graph
	if(fgets(ch,sizeof(ch),fp) != NULL)
		V = atoi(ch);

	//Counts the number of lines in the file
	while ((temp = fgetc(fp)) != EOF)
	{
		if (temp == '\n')
			lines++;
	}

   	E = (lines - V) * 2;//Initialize num og edges

   	fseek(fp,2L,SEEK_SET);//Go to beginning of file

   	//Create Graph
   	graph = createGraph(V, E);
   	if(graph == NULL)
   	{
   		printf("Something went wrong - Graph is NULL\n");
   		exit(EXIT_FAILURE);
   	}

   	//Read file, intialized data structure
   	for (i = 0; i < V; ++i)
   	{
   		//Allocated memory for DV
   		if((graph->routers[i].DV = malloc(sizeof(int) * V)) == NULL)
   			exit(EXIT_FAILURE);
   		for (j = 0; j < V; ++j)
   		{
   			graph->routers[i].DV[j] = INT_MAX;
   		}
   		//Allocated memory for pi
   		if((graph->routers[i].pi = malloc(sizeof(int) * V)) == NULL)
   			exit(EXIT_FAILURE);
   		for (j = 0; j < V; ++j)
   		{
   			graph->routers[i].pi[j] = -1;
   		}
   		graph->routers[i].neighbors = 0;//initialize num of neighbors

	    if((read = getline(&line,&len,fp)) != -1)
    	{
    		buf = strtok(line," ");
    		graph->routers[i].routerName = malloc(strlen(buf) + 1);
    		strcpy(graph->routers[i].routerName,buf);
    		buf = strtok(NULL," ");
    		graph->routers[i].ip = malloc(strlen(buf) + 1);
    		strcpy(graph->routers[i].ip,buf);
    		buf = strtok(NULL," ");
    		graph->routers[i].port = atoi(buf);
    	}
   	}

   	//Read from file and initialize data
   	i = 0;
   	while((read = getline(&line,&len,fp)) != -1)
   	{
   		buf = strtok(line," ");
   		graph->edges[i].src = malloc(strlen(buf) + 1);
    	graph->edges[i+1].dest = malloc(strlen(buf) + 1);
    	strcpy(graph->edges[i].src, buf);
    	strcpy(graph->edges[i+1].dest, buf);

   		buf = strtok(NULL," ");
   		graph->edges[i].dest = malloc(strlen(buf) + 1);
   		graph->edges[i+1].src = malloc(strlen(buf) + 1);
   		strcpy(graph->edges[i].dest, buf);
   		strcpy(graph->edges[i+1].src, buf);

   		buf = strtok(NULL," ");
   		graph->edges[i].weight = atoi(buf);
   		graph->edges[i+1].weight = atoi(buf);
   		
   		i = i+2;
   	}

   	//Initialize DV and pi

   	for(i = 0; i < V; i++)
   	{	
   		for(j = 0; j < E; ++j)
   		{
   			if(strcmp(graph->edges[j].src, graph->routers[i].routerName) == 0)
   			{

   				index = findIndex(graph, graph->edges[j].dest);//Find index in routeres
   				
   				graph->routers[i].DV[index] = graph->edges[j].weight;//intialize DV
   				graph->routers[i].pi[index] = i;//initialize pi
	   			graph->routers[i].neighbors++;	
   			}
   		}
   		graph->routers[i].DV[i] = 0;//my dist to my self
   		graph->routers[i].pi[i] = -2;//if found my self
   	}	

	index = findIndex(graph,inputRouter); //index of the router name argument

	//if no connection
	if(graph->routers[index].neighbors == 0)
	{
		printDV(graph->routers[index].routerName, graph->routers[index].DV, graph->routers[index].pi,graph->V, graph->routers);
		if(line)
    		free(line);
		freeGraph(graph);
    	fclose(fp);

		return 0;
	}

	//Initialize neighbors array 
	graph->routers[index].myNeighbors = malloc(sizeof(char*)*graph->routers[index].neighbors);
  	for (i = 0 ; i < graph->routers[index].neighbors ; ++i)
    	graph->routers[index].myNeighbors[i] = NULL;


    //add neighbors to neighbors-list
 	for (i = 0 ; i < graph->E  ; ++i)
 	{
    	if (strcmp(inputRouter,graph->edges[i].src) == 0)
    	{
      		addToNeiArray(graph->edges[i].dest, &graph->routers[index]);
    	}
 
 	}

  	int** neighborsDv;
	//Allocating for neighborsDv
  	neighborsDv = malloc(sizeof(int*) * graph->routers[index].neighbors);
	for (i = 0; i < graph->routers[index].neighbors; ++i)
	{
		neighborsDv[i] = malloc(sizeof(int)*(graph->V + 1));
		for (j = 0; j < graph->V + 1; ++j)
		{
			neighborsDv[i][j] = -1;
		}
	}

	//==========Creates Threads==========
	pthread_mutex_init(&mutexLock, NULL);

	pthread_t* threadsS = malloc(sizeof(pthread_t) * graph->V); 
	pthread_t* threadsR = malloc(sizeof(pthread_t) * graph->V);
	pthread_t calculator;
	argsOfThreadSender* argsS = malloc(sizeof(argsOfThreadSender) * graph->V);
	argsOfThreadRecieve* argsR = malloc(sizeof(argsOfThreadRecieve) * graph->V);
	argsOfThreadClac argsC;

	for (i = 0; i < graph->V; ++i)
	{

		if(graph->routers[index].pi[i] == index)
		{
			//SENDER
			argsS[i].port = graph->routers[i].port;
			argsS[i].name = graph->routers[index].routerName;
			argsS[i].dvCount = graph->V;
			argsS[i].conLoop = atoi(argv[3]);
			argsS[i].ip = graph->routers[index].ip;
			argsS[i].routerDV = graph->routers[index].DV;
			argsS[i].neighbors = graph->routers[index].neighbors;
			argsS[i].neighborsDv = neighborsDv;
			pthread_create(&threadsS[i],NULL,(void *)&createSender, &argsS[i]);
					
			//RECIEVER
			argsR[i].port = graph->routers[index].port;
			argsR[i].name = graph->routers[i].routerName;
			argsR[i].dvCount = graph->V;
			argsR[i].neighbors = graph->routers[index].neighbors;
			argsR[i].ip = graph->routers[index].ip;
			argsR[i].mySelf = &graph->routers[index];
			argsR[i].neighborsDv = neighborsDv;
			pthread_create(&threadsR[i],NULL,(void *)&createReciever, &argsR[i]);
		}					
	}

	//CALCULATOR
	argsC.routers = graph->routers;
	argsC.dvCount = graph->V;
	argsC.routerDV = graph->routers[index].DV;
	argsC.index = index;
	argsC.neighbors = graph->routers[index].neighbors;
	argsC.pi = graph->routers[index].pi;
	argsC.name = graph->routers[index].routerName;
	argsC.neighborsDv = neighborsDv;
	pthread_create(&calculator, NULL, (void *)&createCalc, &argsC);


	//Join threads
	for (i = 0; i < graph->V; ++i)
	{
		if(graph->routers[index].pi[i] == index)
		{
			pthread_join(threadsS[i], NULL);
			pthread_join(threadsR[i], NULL);
		}
	}
	pthread_join(calculator, NULL);

	pthread_mutex_destroy(&mutexLock);


	//=========== Free allocated memory ==========
	fclose(fp);

	freeGlobalDv(neighborsDv, graph->routers[index].neighbors);

   	if(line)
    	free(line);

    for (i = 0; i < graph->routers[index].neighbors; ++i)
    {
    	free(graph->routers[index].myNeighbors[i]);
    }
    free(graph->routers[index].myNeighbors);
   
	freeThreads(threadsR, threadsS, argsS, argsR);

    freeGraph(graph);


	
	return 0;
}


//for calculate (PORT NUMBER)+(ASCII VALUE NAME) 
int calcPort(char* name, int port)
{
	int sum = 0, i;
	for (i = 0; i < strlen(name); ++i)
	{
		sum += (int)name[i];
	}
	
	return (sum+port);
}



//Client - sender
void createSender(argsOfThreadSender *args)
{
	int m_socket_fd = 0, m_conn_fd, res, j, i;
	int fixPort = calcPort(args->name, args->port);//calculate the correct port
	char* localHost = args->ip;
	
	int len = (args->dvCount + 1);//correct length for buffer = DV+1
   	int* m_buffer = malloc(sizeof(int)*len); 
   	if(m_buffer == NULL)
   	{
   		printf("Error Allocating\n");
   		pthread_exit(NULL);
   	}

    struct sockaddr_in m_serv_addr;

    //creates an endpoint for communication and returns a descriptor
    m_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket_fd < 0 ) 
    {
    	perror("Error creating socket\n");
    	free(m_buffer);
    	pthread_exit(NULL);
   	}
    
    bzero(&m_serv_addr, sizeof(m_serv_addr));
    m_serv_addr.sin_family = AF_INET;
    m_serv_addr.sin_port = htons(fixPort);
    m_serv_addr.sin_addr.s_addr = inet_addr(localHost);


	//initiate a connection on a socket
    i = 0;
    while(i < (args->conLoop))//attempts to connect
    {	
    	m_conn_fd = connect(m_socket_fd,(struct sockaddr *)&m_serv_addr, sizeof(m_serv_addr));
    	if (m_conn_fd < 0) 
    	{
        	sleep(1);
    	}
    	else
    	{
    		break;
    	}
    	i++;
    }
    sleep(1);
    if(m_conn_fd < 0)
    {
    	printf("ERROR in connecting - server\n");
    	close(m_socket_fd);
    	free(m_buffer);
    	pthread_exit(NULL);
    }

	while(1)
	{
		pthread_mutex_lock(&mutexLock);

		if(dvIsChaneged == 1)//own dv changed
		{
			m_buffer[0] = 1;
		}
		else
		{
			m_buffer[0] = 0;
			dvIsChaneged = 0;
		}
		
		//copy DV
		for (j = 0; j < (len - 1); ++j)
		{
			m_buffer[j+1] = args->routerDV[j];
		}

	    pthread_mutex_unlock(&mutexLock);

	    //send the DV
	    if((res = write(m_socket_fd, m_buffer, len*sizeof(int))) < 0)
	    {
	    	free(m_buffer);
	    	close(m_socket_fd);
			pthread_exit(NULL);
	    }
		

		pthread_mutex_lock(&mutexLock);
		if(calc == 1)//sender done
		{
    		break;
    	}		

		pthread_cond_wait(&senderCond, &mutexLock);//wait until calc wakes me up
		pthread_mutex_unlock(&mutexLock);
	}

	pthread_mutex_unlock(&mutexLock);

	free(m_buffer);

	// if (close(m_socket_fd) < 0)
	// 	printf("Error closing socketFD\n");
	close(m_socket_fd);

	pthread_exit(NULL);
}



//Server - reciever
void createReciever(argsOfThreadRecieve *args)
{
	int i, index = 0;
	int len = (args->dvCount + 1);//correct length for buffer = DV+1 
   	int* m_buffer = malloc(sizeof(int)*len); 
   	if(m_buffer == NULL)
   	{
   		printf("Error Allocating\n");
   		pthread_exit(NULL);
   	}

	int fixPort = calcPort(args->name, args->port);//calculate the correct port
	
	int m_socket_fd = 0 , m_conn_fd = 0;
    struct sockaddr_in m_socket_struct;
    struct sockaddr_in client_addr;
  	socklen_t size = sizeof (client_addr);

    //creates an endpoint for communication and returns a descriptor
    m_socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket_fd < 0 ) 
    {
    	perror("Error creating socket\n");
    	free(m_buffer);
    	pthread_exit(NULL);
   	}

   	memset(&m_socket_struct, 0, sizeof(m_socket_struct)); //initialize the sockaddr_in memory block to 0
	
	m_socket_struct.sin_family = AF_INET;
	m_socket_struct.sin_addr.s_addr = htonl(INADDR_ANY);
	m_socket_struct.sin_port = htons(fixPort);


	//assigning a name to a socket
	if ((bind(m_socket_fd, (struct sockaddr*) &m_socket_struct, sizeof(m_socket_struct))) < 0 ) 
	{
    	perror("Error in Bind\n");
    	close(m_socket_fd);
    	free(m_buffer);
    	pthread_exit(NULL);
	}

	//listen for connections on a socket
	if (listen(m_socket_fd, 10) < 0 )
	{
		perror("Error listening to socket\n");
		close(m_socket_fd);
    	free(m_buffer);
   		pthread_exit(NULL);
   	}


   	//accept/reject jobs sent to a destination
	m_conn_fd = accept(m_socket_fd, (struct sockaddr *) &client_addr, &size);
	if (m_conn_fd < 0) 
	{
	   	perror("Error on accepting\n");
	   	free(m_buffer);
	   	pthread_exit(NULL);
	}

    while(1) 
    { 
    	
    	pthread_mutex_lock(&mutexLock);

    	if(calc == 1)//reciever done
    	{
    		break;
    	}
		int res;
		pthread_mutex_unlock(&mutexLock);
		
		//read from a file descriptor(m_buffer in our case)
		sleep(1);
		if ((res = read(m_conn_fd, m_buffer, len*sizeof(int)))  < 0)
		{
			perror("Error in read\n");
			close(m_conn_fd);
    		free(m_buffer);
			pthread_exit(NULL);
		}
		else 
		{ //If read was successful
			
			//findes the index and update the global array when done wake up calc
			index = findNeiIndex(args->name, args->mySelf);

			pthread_mutex_lock(&mutexLock);

			//copy data to neighborsDv
			for (i = 0; i < len; ++i)
			{
				args->neighborsDv[index][i] = m_buffer[i];				
			}

			//check if got all neighbors DV
			int check = 1;
			for (i = 0; i < args->neighbors; ++i)
			{
				if(args->neighborsDv[i][0] == -1)
				{
					check = 0;
				}
			}
			if(check)//got all neighbors DV
			{
				pthread_cond_signal(&calcCond);
			}
			
			pthread_mutex_unlock(&mutexLock);
		}
				
	}
	
	pthread_mutex_unlock(&mutexLock);

	free(m_buffer);	

    close(m_conn_fd);

	pthread_exit(NULL);
}

//thread calculator
void createCalc(argsOfThreadClac *args)
{	
	int routersCount = args->dvCount;//for checking that all routers send and gets their DV
	printf("Wait while calculate own DV\nIt may take a few seconds...\n");
	while(1)
	{
		pthread_mutex_lock(&mutexLock);		
		pthread_cond_wait(&calcCond, &mutexLock);//wait until reciever wakes me up

		int i,j,tmpIndex = 0, k;

		//check if dv did not changed - got '0' from all
		int finish = 1;
		for (i = 0; i < args->neighbors; ++i)
		{
			if(args->neighborsDv[i][0] != 0)
				finish = 0;
		}
		dvIsChaneged = 0;
		if(finish == 1)//if got '0' from all - calc finish
		{
			calc = 1;
			for(i = 0; i < args->neighbors; ++i)
			{
				pthread_cond_signal(&senderCond);//waking up senders
			}
			pthread_mutex_unlock(&mutexLock);
			
			//prints the DV table and exit
			printDV(args->name, args->routerDV, args->pi, args->dvCount, args->routers);
			pthread_exit(NULL);
		}
		else
		{
			//Relax
			for (i = 0; i < args->dvCount; ++i)
			{
				if(i != args->index)
				{
					for (j = 0; j < args->neighbors; ++j)
					{
						//finds neighbor index
						for (k = 1; k < args->dvCount + 1; ++k)
						{
							if(args->neighborsDv[j][k] == 0)
							{
								tmpIndex = k;
								break;
							}
						}
						if(args->routerDV[i] > args->neighborsDv[j][i+1] + args->routerDV[tmpIndex-1])
						{
							args->routerDV[i] = args->neighborsDv[j][i+1] + args->routerDV[tmpIndex-1];
							args->pi[i] = tmpIndex-1;//who found me
							dvIsChaneged = 1;//tells that DV is changed
							routersCount = args->dvCount;
						}							
					}
				}
			}
			//check if all calculations are done 
			if(dvIsChaneged == 0)
			{	
				routersCount--;
				if(routersCount > 0)
					dvIsChaneged = 1;
			}	
			for(i = 0; i < args->neighbors; ++i)
			{
				pthread_cond_signal(&senderCond);//waking up senders
			}
			pthread_mutex_unlock(&mutexLock);
		}
	}
}


//free all thread arrays
void freeThreads(pthread_t* tR, pthread_t* tS, argsOfThreadSender* argsS, argsOfThreadRecieve* argsR)
{
	free(tR);
	free(tS);
	free(argsS);
	free(argsR);
}

//free global DV array
void freeGlobalDv(int** globalDv, int neighbors)
{
	int i;
	for (i = 0; i < neighbors; ++i)
	{
		free(globalDv[i]);
	}
	free(globalDv);
}