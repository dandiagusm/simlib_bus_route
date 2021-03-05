#include "simlib.h"		/* Required for use of simlib.c. */


int bus_position, bus_moving, capacity, num_stations, i, j, bus_idle, looping;
double waiting_time, arrive_time_b, mean_interarrival[MAX_NUM_STATIONS + 1], length_simulation, prob_distrib_dest[3], dist[MAX_NUM_STATIONS+1][MAX_NUM_STATIONS+1], loop_ori, loop_final, speed;
FILE *input_file, *output_file;

int main (){
    // Read file
    input_file = fopen("carrental.in", "r");
    output_file = fopen("carrental.out", "w");
    

}

void move_b(){
    int init = bus_position;
    int dest;
    bus_moving = 1;
    bus_idle = 0;

    if(bus_position != 3){
        dest = bus_position+1;
    }else{
        dest = 3;
    }

    sampst(sim_time - arrive_time_b, VAR_BUS_AT_STATION+bus_position);
    event_schedule(sim_time+(dist[init][dest]), EVENT_ARRIVE_BUS)
}

void load(){
    int origin, destination;
    int terminal = bus_position;
    double time_at_position = sim_time - arrive_time_b, arrival_time;

    bus_idle = 0;
    if (list_size[terminal] > 0 && capacity>0){
        list_remove(FIRST, terminal);
        arrival_time = transfer[1];
        destination= transfer[3]; //destination yang ditentukan saat arrive

        last_file(LAST, MAX_NUM_STATIONS+destination);

        --capacity;
        timest(MAX_NUM_SEATS-capacity, VAR_BUS);

        sampst(sim_time-arrival_time, VAR_QUEUE_STATION+terminal);

        event_schedule(sim_time+uniform(0.0041667, 0.0069444, STREAM_LOADING), EVENT_LOAD);
    }else{
        if(time_at_position >= waiting_time){
            move_b();
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

    if(list_size(MAX_NUM_STATIONS+destination)> 0){
        list_remove(FIRST, MAX_NUM_STATIONS+destination);
        arrival_time = transfer[1];
        origin = transfer[2];

        ++capacity;
        timest(MAX_NUM_SEATS - capacity, VAR_BUS);

        sampst(sim_time - arrival_time, VAR_PERSON_FROM_STATION+origin);

        if(list_size[MAX_NUM_STATIONS+destination]>0){
            event_schedule(sim_time+uniform(0.00444, 0.00677, STREAM_UNLOADING), EVENT_UNLOAD);
        }else{
            event_schedule(sim_time+uniform(0.00444, 0.00677, STREAM_UNLOADING), EVENT_LOAD))
        }
    }else {
        load();
    }
}