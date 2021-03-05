int bus_position, bus_moving, capacity, num_stations, i, j, bus_idle, looping;
double waiting_time, arrive_time_bus, mean_interarrival[MAX_NUM_STATIONS + 1], simulation_duration, prob_distrib_dest[3], dist[MAX_NUM_STATIONS+1][MAX_NUM_STATIONS+1], loop_ori, loop_final, speed;
FILE *input_file, *output_file;

void arrive (int new_job, int station) 
{
	int dest;
	if (station == 1)
	{
		event_schedule(sim_time + expon (mean_interarrival[1], STREAM_INTERARRIVAL_1), EVENT_ARRIVAL_1);
        dest = 3;
	}
	else if (station == 2)
	{
		event_schedule(sim_time + expon (mean_interarrival[2], STREAM_INTERARRIVAL_2), EVENT_ARRIVAL_2);
        dest = 3;
	}
	else if (station == 3)
	{
		event_schedule(sim_time + expon (mean_interarrival[3], STREAM_INTERARRIVAL_3), EVENT_ARRIVAL_3);
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

    fprintf (outfile, "A.\n");
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
    event_schedule (0.0, EVENT_ARRIVE_BUS);

    // init job customer
    event_schedule (expon (mean_interarrival[1], STREAM_INTERARRIVAL_1), EVENT_ARRIVAL_1);
    event_schedule (expon (mean_interarrival[2], STREAM_INTERARRIVAL_2), EVENT_ARRIVAL_2);
    event_schedule (expon (mean_interarrival[3], STREAM_INTERARRIVAL_3), EVENT_ARRIVAL_3);
  
    // schedule ending
    event_schedule (length_simulation, EVENT_END_SIMULATION);

    // loop until end
    while (next_event_type != EVENT_END_SIMULATION){
        /* Determine the next event. */
        timing ();
        switch (next_event_type){
            case EVENT_ARRIVAL_1:
                arrive (1,1);
                break;
            case EVENT_ARRIVAL_2:
                arrive (1,2);
                break;  	
            case EVENT_ARRIVAL_3:
                arrive (1,3);
                break;
            case EVENT_LOAD:
                load ();
                break;
            case EVENT_UNLOAD:
                unload ();
                break;
            case EVENT_ARRIVE_BUS:
                arrive_bus ();
                break;
            case EVENT_DEPARTURE_BUS:
                move_b ();
                break;   
            case EVENT_END_SIMULATION:
                report_output ();
                break;
        }  
    }

}