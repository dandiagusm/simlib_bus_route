#include "simlib.h"
#include <stdio.h>

// contants
#define MAX_NUM_STATIONS		3	 // Max number of stations
#define MAX_NUM_BUS				1	 // Max number of bus
#define MAX_NUM_SEATS			20   // Max number of seats in a bus

#define STREAM_INTER_ARV_1		1	 // stream random number for interarrival time at terminal 1
#define STREAM_INTER_ARV_2		2	 // stream random number for interarrival time at terminal 1
#define STREAM_INTER_ARV_3		3	 // stream random number for interarrival time at car rental
#define STREAM_UNLOADING		4	 // stream random number for unloading time
#define STREAM_LOADING			5	 // stream random number for loading time
#define STREAM_DESTINATION_ARV	6	 // stream random number for determine destination from car rental

#define EVENT_ARV_1				1	 // event arrival person in terminal 1
#define EVENT_ARV_2				2	 // event arrival person in terminal 2
#define EVENT_ARV_3				3	 // event arrival person in car rental
#define EVENT_DEPARTURE_BUS		4	 // event type for departure of bus
#define EVENT_ARV_BUS			5	 // event type for arrival of bus
#define EVENT_LOAD				6	 // event loading passenger
#define EVENT_UNLOAD		    7	 // event unloading passenger
#define EVENT_END_SIMULATION	8	 // event end of the simulation

#define VAR_QUEUE_STATION       0    // zero index of statistic variable for queue in station 1/2/3
#define VAR_BUS_AT_STATION      3    // zero index of statistic variable for bus stop at station 1/2/3
#define VAR_PERSON_FROM_STATION 6    // zero index of statistic variable for person arrive at station 1/2/3
#define VAR_BUS                 10   // index statistic variable for bus

int pos_bus, bus_moving, capacity, num_stations, i, j, bus_idle, looping;
double waiting_time, arrive_time_bus, mean_interarrival[MAX_NUM_STATIONS + 1], simulation_duration, prob_distrib_dest[3], dist_time[MAX_NUM_STATIONS+1][MAX_NUM_STATIONS+1], loop_ori, loop_final, speed;
FILE *input_file, *output_file;
int three_two, three_one;

void move_bus(){
    int init = pos_bus;
    int dest;
    bus_moving = 1;
    bus_idle = 0;

    if(pos_bus == 3){
        dest = 1;
    }else{
        dest = pos_bus+1;
    }

    sampst(sim_time - arrive_time_bus, VAR_BUS_AT_STATION+pos_bus);
    event_schedule(sim_time+(dist_time[init][dest]), EVENT_ARV_BUS);
}

void load(){
    int dest;
    int terminal = pos_bus;
    double time_at_position = sim_time - arrive_time_bus, arrival_time;

    bus_idle = 0;
    if (list_size[terminal] > 0 && capacity>0){
        list_remove(FIRST, terminal);
        arrival_time = transfer[1];
        dest= transfer[3]; //destination yang ditentukan saat arrive

        list_file(LAST, MAX_NUM_STATIONS+dest);

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
    int dest = pos_bus;
    int origin;
    double arrive_time;

    if(list_size[MAX_NUM_STATIONS+dest]> 0){
        list_remove(FIRST, MAX_NUM_STATIONS+dest);
        arrive_time = transfer[1];
        origin = transfer[2];

        capacity += 1;
        timest(MAX_NUM_SEATS - capacity, VAR_BUS);

        sampst(sim_time - arrive_time, VAR_PERSON_FROM_STATION+origin);

        if(list_size[MAX_NUM_STATIONS+dest]>0){
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
    
    if ((pos_bus == station) && (bus_idle == 1))
    {
        event_cancel(EVENT_DEPARTURE_BUS);
        load();
    }
    
}

void arrive_bus(){
    int init = pos_bus;
    bus_moving = 0;

    if(pos_bus != 3){
        pos_bus = pos_bus+1;
    }
    else{
        pos_bus = 1;
        looping = 1;
    }

    if(pos_bus == 3 && looping){
        loop_final = sim_time - dist_time[init][pos_bus];
        sampst(loop_final - loop_ori, VAR_BUS);
        loop_ori = loop_final;
    }
    
    arrive_time_bus = sim_time;
    unload();
}

void report_output(void){
    fprintf (output_file, "\n\nReport Output\n");

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
    
    fprintf (output_file, "\nC.Average and maximum number on the bus\n");
    timest(0.0, -VAR_BUS);
    fprintf (output_file, "Average : %0.3f people\n", transfer[1]);
    fprintf (output_file, "Maximum : %0.3f People\n", transfer[2]);
    
    fprintf (output_file, "\nD.Average, maximum, and minimum time the bus is stopped at each location\n");
    for (int i = 1; i <= MAX_NUM_STATIONS; i++){
        sampst(0.0, -i - VAR_BUS_AT_STATION);
        fprintf (output_file, "LOCATION %d\n", i);
        fprintf (output_file, "Average : %0.3f hour\n", transfer[1]);
        fprintf (output_file, "Maximum : %0.3f hour\n", transfer[3]);
        fprintf (output_file, "Minimum : %0.3f hour\n", transfer[4]);
    }
    
    fprintf (output_file, "\nE.Average, maximum, and minimum time for the bus to make a loop\n");
    sampst(0.0, -VAR_BUS);
    fprintf (output_file, "Average : %0.3f hour\n", transfer[1]);
    fprintf (output_file, "Maximum : %0.3f hour\n", transfer[3]);
    fprintf (output_file, "Minimum : %0.3f hour\n", transfer[4]);
        
    fprintf (output_file, "\n\nF.Average, maximum, and minimum time a person is in the system by arrival location\n");
    for (int i = 1; i <= MAX_NUM_STATIONS; i++){
        sampst(0.0, -i - VAR_PERSON_FROM_STATION);
        fprintf (output_file, "LOCATION %d\n", i);
        fprintf (output_file, "Average : %0.3f hour\n", transfer[1]);
        fprintf (output_file, "Maximum : %0.3f hour\n", transfer[3]);
        fprintf (output_file, "Minimum : %0.3f hour\n", transfer[4]);
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
            fscanf (input_file, "%lg", &dist_time[i][j]);
            dist_time[i][j] = dist_time[i][j]/speed; 
        }
    }

    for (j = 1; j <= num_stations; ++j)
        fscanf (input_file, "%lg", &mean_interarrival[j]);

    for (i = 1; i <= num_stations-1; ++i)
        fscanf (input_file, "%lg", &prob_distrib_dest[i]);

    // Report 
    fprintf (output_file, "Bus Route model\n\n");
    fprintf (output_file, "Number of stations : %d", num_stations);
    fprintf (output_file, "\nDestination  distribution : ");
    for (i = 1; i <= num_stations-1; ++i)
        fprintf (output_file, "%.3f ", prob_distrib_dest[i]);
    fprintf (output_file, "\nMean interarrival time : ");
    for (i = 1; i <= num_stations; ++i)
        fprintf (output_file, "%.3f ", mean_interarrival[i]);
    fprintf (output_file, "\nSimulation duration : %.1f hours", simulation_duration);
    fprintf (output_file, "\nSpeed : %.1f miles per hour", speed);
    fprintf (output_file, "\nIdle time : %.1f minutes\n\n", waiting_time);

    // Initialize bus position
    pos_bus = 2;
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