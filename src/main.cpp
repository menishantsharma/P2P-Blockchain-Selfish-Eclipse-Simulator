#include <iostream>
#include <unistd.h>
#include <vector>
#include <random>
#include <queue>
#include <iomanip>
#include <fstream>
#include "node.h"
#include "event.h"
#include "simulator.h"

using namespace std;

void run_simulation(int num_nodes, double T, double mean_txn_interval, double mean_block_interval, double timeout_time, int malicious_nodes, Network network, Network overlay_network, bool with_eclipse) {
    cout << endl << "Running Simulation with" << (with_eclipse ? " " : "out ") << "Eclipse Attack" << endl;
    Simulator sim(num_nodes, T, mean_txn_interval, mean_block_interval, timeout_time, malicious_nodes, network, overlay_network, with_eclipse);
    sim.run();
    sim.save_stats();
    // sim.save_csv();
}

Network create_network(int num_nodes, int min_degree, int max_degree, double min_time, double max_time, string filename, int mal_nodes) {
    bool flag = true;
    cout << "Creating " << filename << " network" << endl;
    Network network = Network(num_nodes, min_degree, max_degree, min_time, max_time);
    network.visualize_network(filename, mal_nodes);
    
    // while(flag) {
    //     Network network = Network(num_nodes, min_degree, max_degree, min_time, max_time);
    //     network.visualize_network(filename, mal_nodes);

    //     cout << "Are you satisfied with the network? (y/n): ";
    //     char c;
    //     cin >> c;
    //     if(c == 'y') {
    //         // network.save_network(filename);
    //         flag = false;
    //         return network;
    //     }
    // }
    
    return network;

}

void analyze(int num_nodes, int T, int mean_txn_interval, int mean_block_interval, int malicious_percentage, int timeout_time) {
    int malicious_nodes = num_nodes * malicious_percentage/100.0;
    Network network = create_network(num_nodes, 3, 6, 0.01, 0.5, "normal", malicious_nodes);
    Network overlay_network = create_network(malicious_nodes, 3, 6, 0.001, 0.01, "overlay", malicious_nodes);
    run_simulation(num_nodes, T, mean_txn_interval, mean_block_interval, timeout_time, malicious_nodes, network, overlay_network, true);
    run_simulation(num_nodes, T, mean_txn_interval, mean_block_interval, timeout_time, malicious_nodes, network, overlay_network, false);
}

int main(int argc, char *argv[]) {
    int opt;
    int num_nodes = 0;
    int T = 0;
    int mean_txn_interval;
    int mean_block_interval;
    int timeout_time;
    int malicious_percentage;
    
    if(argc < 13) {
        cerr << "Usage: " << argv[0] << " -n <num_nodes> -T <time> -t <txn_interarrival> -b <block_interarrival> -w <timeout> -m <malicious>" << endl;
        exit(EXIT_FAILURE);
    }

    while((opt = getopt(argc, argv, "n:T:t:b:w:m:")) != -1) {
        switch(opt) {
            case 'n':
                num_nodes = atoi(optarg);
                break;
            
            case 'T':
                T = atoi(optarg);
                break;

            case 't':
                mean_txn_interval = atoi(optarg);
                break;
            
            case 'b':
                mean_block_interval = atoi(optarg);
                break;
            
            case 'w':
                timeout_time = atoi(optarg);
                break;

            case 'm':
                malicious_percentage = atoi(optarg);
                break;

            case '?':
                exit(EXIT_FAILURE);

            default:
                cerr << "Usage: " << argv[0] << " -n <num_nodes> -T <time> -s <> -l <>" << endl;
                exit(EXIT_FAILURE);
        }
    }
    
    // Running Simulation
    analyze(num_nodes, T, mean_txn_interval, mean_block_interval, malicious_percentage, timeout_time);
    
    return 0;
}