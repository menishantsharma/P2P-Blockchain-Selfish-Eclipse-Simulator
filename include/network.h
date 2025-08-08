#ifndef NETWORK_H
#define NETWORK_H

#include <vector>
using namespace std;
#include "utils.h"

enum class NetworkType {
    NORMAL,
    MALICIOUS
};

class Network {
    public:
        int n;
        vector<vector<int>> graph;
        int min_degree;
        int max_degree;
        vector<vector<double>>rho;
        double min_prop_delay;
        double max_prop_delay;

        Network() {}

        Network(int n, int min_degree, int max_degree, double min_prop_delay, double max_prop_delay) : n(n), min_degree(min_degree), max_degree(max_degree), min_prop_delay(min_prop_delay), max_prop_delay(max_prop_delay) {
            graph.resize(n, vector<int>());
            create_network();
            rho = utils::generateRandomMatrix(n, min_prop_delay, max_prop_delay);
        }

    void create_network();
    void print_network();
    double calculate_latency(int sender, int receiver, double message_size, bool is_sender_slow, bool is_receiver_slow);
    void visualize_network(string filename, int malicious_nodes);
    void save_network(string filename);
};

#endif