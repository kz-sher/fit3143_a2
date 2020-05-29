// Student Name: Ho Kong Zheng
// Student ID: 28279174

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "mpi.h"

/* Constant declaration */
#define MIN 1
#define MAX 10
#define ROW_SIZE 4
#define COL_SIZE 5
#define NUM_OF_NEIGHBOUR 4
#define TRUE 1
#define FALSE 0
#define NEIGHBOUR_TAG 666
#define EVENT_TAG 777
#define EXIT_TAG 444
#define NUM_OF_ITERATION 3

/* Function definition declaration */
void master_io(MPI_Comm master_comm, int rank);
void slave_io(MPI_Comm master_comm, int rank);
int equals(int a, int b, int c, int d);
int getNumberOfNeighbours(int rank);

/* The main program which is the first part where the flow starts from */
int main(int argc, char **argv)
{
	/* The ranking of each process*/
    int rank;
    /* The communication scope within slaves */
    MPI_Comm slave_comm;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_split(MPI_COMM_WORLD, rank == 20, rank, &slave_comm);
    
    	if (rank == 20)
   			master_io(MPI_COMM_WORLD, rank);
    	else
    	{
    		for (int i = 0; i < NUM_OF_ITERATION; i++)
    		{
   				slave_io(MPI_COMM_WORLD, rank);
   				MPI_Barrier(slave_comm);
   				sleep(1); // let each iteration take 1 second (1000 milliseconds) after the completion of process
    		}
    	}
    MPI_Finalize();
    return 0;
}

/* This function is used for comparing whether the set of numbers are equal*/
/* The if-else statement below uses the logic gate method to examine whether they are the same */
/* d might be a flag if this function is performed on three numbers only */
int equals(int a, int b, int c, int d)
{
	if ((( a | b | c | d ) == ( a & b & c & d )))
		return TRUE;
	else if (d == -1 && (( a | b | c ) == ( a & b & c )) ) 
    	return TRUE;
    else
    	return FALSE;
}

/* This function is used to calculate how many valid neighbours a node has */
/* In this case, it is used for the calculation of messages passed throughout the network */
int getNumberOfNeighbours(int rank)
{
	/* Number of messages a node passes */
	int num_of_msg = 4,
		x = rank / COL_SIZE,
    	y = rank % COL_SIZE;
    if (x == 0)
   		num_of_msg --;
    if (x == (ROW_SIZE - 1))
   	 	num_of_msg --;
    if (y == 0)
   	 	num_of_msg --;	 
    if (y == (COL_SIZE - 1))
   	 	num_of_msg --;
   	return num_of_msg;
}

/* This is the master */
void master_io(MPI_Comm master_comm, int rank)
{
	/* Iteration counter */
    int i = 1,
    	/* Number of slaves which are 20 as specified in the assignment specification*/
    	num_of_slave = 20,
    	/* Total number of slaves */
    	total_num_of_slave = num_of_slave * NUM_OF_ITERATION, 
    	/* Number of events */
    	num_of_event = 0, 
    	/* Total number of events */
    	total_num_of_event = 0,
    	/* Number of messages */
    	num_of_msg = 0,
    	/* Total number of messages */
    	total_num_of_msg = 0;
    /* Output buffer for storing the information given by slaves */
    char buf[256];
    /* The start and end time of the simulation */
    double simulation_start_time, simulation_end_time;
    /* flag for checking whether the receive action is successfully complete */    
    MPI_Status status;
    
    // Write to an output file
    FILE *file;
    file = fopen("output.txt", "w"); // "w" means the mode where the data will be written to the file

    simulation_start_time = MPI_Wtime(); // Record the start time
    fprintf(file, "<< Iteration %d >>\n", i ++);

    while (total_num_of_slave > 0)
    {
    	MPI_Recv(buf, 256, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, master_comm, &status);
    	switch(status.MPI_TAG)
    	{
    		case EXIT_TAG: 
    			break;
    		case EVENT_TAG:
    			num_of_event ++;
    			fprintf(file, "%s\n", buf);
    			break;		
    	}
    	total_num_of_slave --;
    	num_of_msg += getNumberOfNeighbours(status.MPI_SOURCE); 

    	if (total_num_of_slave % 20 == 0)
	    {
	    	num_of_msg += num_of_event;
	    	fprintf(file, "Number of events: %d\n", num_of_event);
	    	fprintf(file, "Number of messages: %d\n\n", num_of_msg);
	    	if (i != 4)
	    		fprintf(file, "<< Iteration %d >>\n", i ++);

	    	// Adding up to the total number of messages and events
	    	total_num_of_msg += num_of_msg;
	    	total_num_of_event += num_of_event;

	    	// Reset the current number of messages and events for the current iteration
	    	num_of_msg = 0;
	    	num_of_event = 0;
	    }
    }

    simulation_end_time = MPI_Wtime(); // Record the end time
    fprintf(file, "<< The end of simulation >>\n");
    fprintf(file, "Total number of events: %d\n", total_num_of_event);
    fprintf(file, "Total number of messages: %d\n", total_num_of_msg);
    fprintf(file, "Simulation_time: %1.5f\n\n", simulation_end_time - simulation_start_time);

   	// Close the file to indicate the end of writing
    fclose(file);
}

/* This is the slave */
void slave_io(MPI_Comm master_comm, int rank)
{    
    
    /* Request list used for non-blocking communication */
    // Its size is 8 because the maximum send and receive actions a node will do are both 4
    MPI_Request reqs[8];    
    /* Status list used for non-blocking communication */
    MPI_Status statuses[8];   	 
    /* Random number generated for reference node */
    int random_num,
    	/* Number of requests including both non-blocking send and receive */
    	num_of_req = 0,
    	/* List for storing neighbours' random number */
    	// Index 0 = north neighbour
    	// Index 1 = west neighbour
    	// Index 2 = south neighbour
    	// Index 3 = east neighbour
    	neighbour_num_list[NUM_OF_NEIGHBOUR] = { [0 ... (NUM_OF_NEIGHBOUR - 1)] = -1}, 
    	/* X- and Y-coordinates of reference node */
    	x = rank / COL_SIZE,
    	y = rank % COL_SIZE,
    	/* All neighbour nodes' rank */
    	north_node_rank = ((x - 1) * COL_SIZE) + y,
    	south_node_rank = ((x + 1) * COL_SIZE) + y,
    	west_node_rank = (x * COL_SIZE) + (y - 1),
    	east_node_rank = (x * COL_SIZE) + (y + 1),
    	/* Flag used for location validity and same neighbours*/
    	up = TRUE,
    	down = TRUE,
    	left = TRUE,
    	right = TRUE,
    	AT_LEAST_THREE = TRUE;
    /* Output for write file for each slave */
    char buf[256];
    
    srandom(time(NULL) | rank); // different seed given
    random_num = (random() % (MAX - MIN + 1)) + MIN; // generate a random number

    // Turn the flag according to the coordinate of node
    // The flag indicate the possible neighbour
    if (x == 0)
   	 	up = FALSE;
    if (x == (ROW_SIZE - 1))
   	 	down = FALSE;
    if (y == 0)
   	 	left = FALSE;   	 
    if (y == (COL_SIZE - 1))
    	right = FALSE;

   	// Interact with neighbours if they are valid
    if (up == TRUE)
    {
   	 	MPI_Isend(&random_num, 1, MPI_INT, north_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
   	 	MPI_Irecv(&neighbour_num_list[0], 1, MPI_INT, north_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
    }
    if (down == TRUE)
    {
   	 	MPI_Isend(&random_num, 1, MPI_INT, south_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
   	 	MPI_Irecv(&neighbour_num_list[2], 1, MPI_INT, south_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
    }
    if (left == TRUE)
    {
   	 	MPI_Isend(&random_num, 1, MPI_INT, west_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
   	 	MPI_Irecv(&neighbour_num_list[1], 1, MPI_INT, west_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
    }    
    if (right == TRUE)
    {
   	 	MPI_Isend(&random_num, 1, MPI_INT, east_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
   	 	MPI_Irecv(&neighbour_num_list[3], 1, MPI_INT, east_node_rank, NEIGHBOUR_TAG, master_comm, &reqs[num_of_req++]);
    }
    MPI_Waitall(num_of_req, reqs, statuses);

    // Evaluate whether there exists at least 3 neighbours holding the same number
    // When all four neighbours are the same
    if (equals(neighbour_num_list[0], neighbour_num_list[1], neighbour_num_list[2], neighbour_num_list[3]) == TRUE)
    	sprintf(buf, "Event occured at slave/reference node %d\nTimestamp of the occurred events at reference node (in seconds): %1.5f\nNumber of neighbours which activated this event: 4\nNeighbours: %d %d %d %d\n", rank, MPI_Wtime(), north_node_rank, west_node_rank, south_node_rank, east_node_rank);
    // When all neighbours except the east one are the same
    else if (equals(neighbour_num_list[0], neighbour_num_list[1], neighbour_num_list[2], -1) == TRUE)
    	sprintf(buf, "Event occured at slave/reference node %d\nTimestamp of the occurred events at reference node (in seconds): %1.5f\nNumber of neighbours which activated this event: 3\nNeighbours: %d %d %d\n", rank, MPI_Wtime(), north_node_rank, west_node_rank, south_node_rank);
    // When all neighbours except the south one are the same
    else if (equals(neighbour_num_list[0], neighbour_num_list[1], neighbour_num_list[3], -1) == TRUE)
    	sprintf(buf, "Event occured at slave/reference node %d\nTimestamp of the occurred events at reference node (in seconds): %1.5f\nNumber of neighbours which activated this event: 3\nNeighbours: %d %d %d\n", rank, MPI_Wtime(), north_node_rank, west_node_rank, east_node_rank);
    // When all neighbours except the west one are the same
    else if (equals(neighbour_num_list[0], neighbour_num_list[2], neighbour_num_list[3], -1) == TRUE)
    	sprintf(buf, "Event occured at slave/reference node %d\nTimestamp of the occurred events at reference node (in seconds): %1.5f\nNumber of neighbours which activated this event: 3\nNeighbours: %d %d %d\n", rank, MPI_Wtime(), north_node_rank, south_node_rank, east_node_rank);
    // When all neighbours except the north one are the same
    else if (equals(neighbour_num_list[1], neighbour_num_list[2], neighbour_num_list[3], -1) == TRUE)
    	sprintf(buf, "Event occured at slave/reference node %d\nTimestamp of the occurred events at reference node (in seconds): %1.5f\nNumber of neighbours which activated this event: 3\nNeighbours: %d %d %d\n", rank, MPI_Wtime(), west_node_rank, south_node_rank, east_node_rank);
    // When less than 3 neighbours are the same
    else 
    	AT_LEAST_THREE = FALSE;

    // If there are at least 3 neighbours holding the same number, then send information with event tag
    // Otherwise, send it with exit tag
    if (AT_LEAST_THREE == TRUE)
    	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 20, EVENT_TAG, master_comm);    
    else
    	MPI_Send(buf, strlen(buf) + 1, MPI_CHAR, 20, EXIT_TAG, master_comm);

}