#include "simlib.h"		/* Required for use of simlib.c. */

#define STREAM_INTER_ARV_1		1	 /* Random-number stream for interarrivals at terminal 1. */
#define STREAM_INTER_ARV_2		2	 /* Random-number stream for interarrivals at terminal 2. */
#define STREAM_INTER_ARV_3		3	 /* Random-number stream for interarrivals at car rental. */
#define STREAM_UNLOADING		4	 /* Random-number stream for unloading time. */
#define STREAM_LOADING			5	 /* Random-number stream for loading time. */
#define STREAM_DESTINATION_ARV	6	 /* Random-number stream for selecting destination from car rental. */

#define EVENT_ARV_1				1	 /* Event type for arrival of a job to terminal 1. */
#define EVENT_ARV_2				2	 /* Event type for arrival of a job to terminal 2. */
#define EVENT_ARV_3				3	 /* Event type for arrival of a job to car rental. */
#define EVENT_DEPARTURE_BUS		4	 /* Event type for departure of a bus. */
#define EVENT_ARV_BUS			5	 /* Event type for arrival of a bus. */
#define EVENT_LOAD				6	 /* Event type for loading passengers from a particular station. */
#define EVENT_UNLOAD		    7	 /* Event type for unloading passengers to a particular station. */
#define EVENT_END_SIMULATION	8	 /* Event type for end of the simulation. */

#define MAX_NUM_STATIONS		3	 /* Maximum number of stations. */
#define MAX_NUM_BUS				1	 /* Maximum number of bus. */
#define MAX_NUM_SEATS			20 /* Maximum number of seats. */
#define VAR_QUEUE_STATION       0  /* Zero index of statistic variable for queue in station 1/2/3 */
#define VAR_BUS_AT_STATION      3  /* Zero index of statistic variable for bus stop at station 1/2/3 */
#define VAR_PERSON_FROM_STATION 6  /* Zero index of statistic variable for person arrive at station 1/2/3 */
#define VAR_BUS                 10 /* Statistic variable for bus */

int bus_position, bus_moving, capacity, num_stations, i, j, bus_idle, looping;
double waiting_time, arrive_time_bus, mean_interarrival[MAX_NUM_STATIONS + 1], simulation_duration, prob_distrib_dest[3], dist[MAX_NUM_STATIONS+1][MAX_NUM_STATIONS+1], loop_ori, loop_final, speed;
FILE *input_file, *output_file;

void move_bus(){
    int init = bus_position;
    int dest;
    bus_moving = 1;
    bus_idle = 0;

    if(bus_position != 3){
        dest = bus_position+1;
    }else{
        dest = 3;
    }

    sampst(sim_time - arrive_time_bus, VAR_BUS_AT_STATION+bus_position);
    event_schedule(sim_time+(dist[init][dest]), EVENT_ARV_BUS);
}

void load(){
    int destination;
    int terminal = bus_position;
    double time_at_position = sim_time - arrive_time_bus, arrival_time;

    bus_idle = 0;
    if (list_size[terminal] > 0 && capacity>0){
        list_remove(FIRST, terminal);
        arrival_time = transfer[1];
        destination= transfer[3]; //destination yang ditentukan saat arrive

        list_file(LAST, MAX_NUM_STATIONS+destination);

        capacity -= 1;
        timest(MAX_NUM_SEATS-capacity, VAR_BUS);

        sampst(sim_time-arrival_time, VAR_QUEUE_STATION+terminal);

        event_schedule(sim_time+uniform(0.0041667, 0.0069444, STREAM_LOADING), EVENT_LOAD);
    }else{
        if(time_at_position >= waiting_time){
            move_bus();
        }else{
            event_schedule(sim_time+(waiting_time-time_at_position), EVENT_DEPARTURE_BUS);
            bus_idle = 1;
        }
    }
}

void unload(){
    int destination = bus_position;
    int origin;
    double arrive_time;

    if(list_size[MAX_NUM_STATIONS+destination]> 0){
        list_remove(FIRST, MAX_NUM_STATIONS+destination);
        arrive_time = transfer[1];
        origin = transfer[2];

        capacity += 1;
        timest(MAX_NUM_SEATS - capacity, VAR_BUS);

        sampst(sim_time - arrive_time, VAR_PERSON_FROM_STATION+origin);

        if(list_size[MAX_NUM_STATIONS+destination]>0){
            event_schedule(sim_time+uniform(0.00444, 0.00677, STREAM_UNLOADING), EVENT_UNLOAD);
        }else{
            event_schedule(sim_time+uniform(0.00444, 0.00677, STREAM_UNLOADING), EVENT_LOAD);
        }
    }else {
        load();
    }
}

void arrive (int new_job, int station) 
{
	int dest;
	if (station == 1)
	{
		event_schedule(sim_time + expon (mean_interarrival[1], STREAM_INTER_ARV_1), EVENT_ARV_1);
        dest = 3;
	}
	else if (station == 2)
	{
		event_schedule(sim_time + expon (mean_interarrival[2], STREAM_INTER_ARV_2), EVENT_ARV_2);
        dest = 3;
	}
	else if (station == 3)
	{
		event_schedule(sim_time + expon (mean_interarrival[3], STREAM_INTER_ARV_3), EVENT_ARV_3);
        dest = random_integer (prob_distrib_dest, STREAM_DESTINATION_ARV);
    }

    transfer[1] = sim_time;
    transfer[2] = station;
    transfer[3] = dest;
    list_file(LAST, station);
    
    if ((bus_position == station) && (bus_idle == 1))
    {
        event_cancel(EVENT_DEPARTURE_BUS);
        load();
    }
    
}

void arrive_bus(){
    int init = bus_position;
    bus_moving = 0;

    if(bus_position != 3){
        bus_position = bus_position+1;
    }
    else{
        bus_position = 1;
        looping = 1;
    }

    if(bus_position == 3 && looping){
        loop_final = sim_time - dist[init][bus_position];
        sampst(loop_final - loop_ori, VAR_BUS);
        loop_ori = loop_final;
    }
    
    arrive_time_bus = sim_time;
    unload();
}

void report_output(void){
    fprintf (output_file, "\n\nReport Bus Route Model\n");

    fprintf (output_file, "A.\n");
    for (int i = 1; i <= MAX_NUM_STATIONS; i++){
        filest(i);
        fprintf (output_file, "Average number location queue %d : %0.3f\n", i, transfer[1]);
        fprintf (output_file, "Maximum number location queue %d : %0.3f\n", i, transfer[2]);
    }
    
    fprintf (output_file, "\nB.\n");
    for (int i = 1; i <= MAX_NUM_STATIONS; i++){
        sampst(0.0, -i);
        fprintf (output_file, "Average delay location queue %d : %0.3f\n", i, transfer[1]);
        fprintf (output_file, "Maximum delay location queue %d : %0.3f\n", i, transfer[3]);
    }
    
    fprintf (output_file, "\nC.\n");
    timest(0.0, -VAR_BUS);
    fprintf (output_file, "Average number on the bus: %0.3f\n", transfer[1]);
    fprintf (output_file, "Maximum number on the bus: %0.3f\n", transfer[2]);
    
    fprintf (output_file, "\nD.\n");
    for (int i = 1; i <= MAX_NUM_STATIONS; i++){
    sampst(0.0, -i - VAR_BUS_AT_STATION);
        fprintf (output_file, "Average time stop in location %d: %0.3f\n", i, transfer[1]);
        fprintf (output_file, "Maximum time stop in location %d: %0.3f\n", i, transfer[3]);
        fprintf (output_file, "Minimum time stop in location %d: %0.3f\n", i, transfer[4]);
    }
    
    fprintf (output_file, "\nE.\n");
    sampst(0.0, -VAR_BUS);
        fprintf (output_file, "Average time to make a loop:	%0.3f\n", transfer[1]);
        fprintf (output_file, "Maximum time to make a loop:	%0.3f\n", transfer[3]);
        fprintf (output_file, "Minimum time to make a loop:	%0.3f\n", transfer[4]);
        
    fprintf (output_file, "\n\nf.\n");
    for (int i = 1; i <= MAX_NUM_STATIONS; i++){
    sampst(0.0, -i - VAR_PERSON_FROM_STATION);
        fprintf (output_file, "Average time person in system from location %d: %0.3f\n", i, transfer[1]);
        fprintf (output_file, "Maximum time person in system from location %d: %0.3f\n", i, transfer[3]);
        fprintf (output_file, "Minimum time person in system from location %d: %0.3f\n", i, transfer[4]);
    }
}

int main (){
    // Read file
    input_file = fopen("carrental.in", "r");
    output_file = fopen("carrental.out", "w");
    
    // Read parameters
    fscanf (input_file, "%d %lg", &num_stations, &simulation_duration);
    fscanf (input_file, "%lg %lg", &speed, &waiting_time);

    for (i = 1; i <= num_stations; ++i) {
        for (j = 1; j <=num_stations; ++j) {
            fscanf (input_file, "%lg", &dist[i][j]);
            dist[i][j] = dist[i][j]/speed; 
        }
    }

    for (j = 1; j <= num_stations; ++j)
        fscanf (input_file, "%lg", &mean_interarrival[j]);

    for (i = 1; i <= num_stations-1; ++i)
        fscanf (input_file, "%lg", &prob_distrib_dest[i]);

    // Report 
    fprintf (output_file, "Bus Route model\n\n");
    fprintf (output_file, "Number of stations%21d", num_stations);
    fprintf (output_file, "\nDestination  distribution");
    for (i = 1; i <= num_stations-1; ++i)
        fprintf (output_file, "%8.3f", prob_distrib_dest[i]);
    fprintf (output_file, "\nMean interarrival time  ");
    for (i = 1; i <= num_stations; ++i)
        fprintf (output_file, "%8.3f", mean_interarrival[i]);
    fprintf (output_file, "\nSimulation duration%20.1f hours", simulation_duration);
    fprintf (output_file, "\nSpeed%32.1f miles per hour", speed);
    fprintf (output_file, "\nIdle time%20.1f minutes\n\n", waiting_time);

    // Initialize bus position
    bus_position = 2;
    bus_idle = 1;
    waiting_time = waiting_time/60.0f;
    capacity = MAX_NUM_SEATS;

    // Init simlib
    init_simlib ();

    // Set maxatr = max(maximum number of attributes per record, 4) 
    maxatr = 4;			

    // Init job bus
    event_schedule (0.0, EVENT_ARV_BUS);

    // init job customer
    event_schedule (expon (mean_interarrival[1], STREAM_INTER_ARV_1), EVENT_ARV_1);
    event_schedule (expon (mean_interarrival[2], STREAM_INTER_ARV_2), EVENT_ARV_2);
    event_schedule (expon (mean_interarrival[3], STREAM_INTER_ARV_3), EVENT_ARV_3);
  
    // schedule ending
    event_schedule (simulation_duration, EVENT_END_SIMULATION);

    // loop until end
    while (next_event_type != EVENT_END_SIMULATION){
        /* Determine the next event. */
        timing ();
        switch (next_event_type){
            case EVENT_ARV_1:
                arrive (1,1);
                break;
            case EVENT_ARV_2:
                arrive (1,2);
                break;  	
            case EVENT_ARV_3:
                arrive (1,3);
                break;
            case EVENT_LOAD:
                load ();
                break;
            case EVENT_UNLOAD:
                unload ();
                break;
            case EVENT_ARV_BUS:
                arrive_bus ();
                break;
            case EVENT_DEPARTURE_BUS:
                move_bus ();
                break;   
            case EVENT_END_SIMULATION:
                report_output ();
                break;
        }  
    }
}