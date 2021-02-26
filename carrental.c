int bus_position, bus_moving, capacity, num_stations, i, j, bus_idle, looping;
double waiting_time, arrive_time_b, mean_interarrival[MAX_NUM_STATIONS + 1], length_simulation, prob_distrib_dest[3], dist[MAX_NUM_STATIONS+1][MAX_NUM_STATIONS+1], loop_ori, loop_final, speed;
FILE *input_file, *output_file;

int main (){
    // Read file
    input_file = fopen("carrental.in", "r");
    output_file = fopen("carrental.out", "w");



}