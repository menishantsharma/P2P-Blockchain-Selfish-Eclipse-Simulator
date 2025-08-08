#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <queue>
#include "node.h"
#include "event.h"
#include "network.h"

using namespace std;

#define GET_REQUEST_SIZE 0.0625
#define HASH_SIZE 0.0625
#define COMMAND_SIZE 0.0625

class Simulator {    
    public:
        priority_queue<Event> events;
        vector<Node*> nodes;
        int num_nodes;
        double T;
        double mean_txn_interval;
        double mean_block_interval;
        vector<vector<double>>rho;
        int block_id = 0;
        Block genesis = Block(block_id++, num_nodes);
        double timeout_time;
        int malicious_nodes;
        Network network;
        Network overlay_network;
        int ringmaster;
        bool with_eclipse;
        int txn_id = 0;

        Simulator(int num_nodes, double T, double mean_txn_interval, double mean_block_interval, double timeout_time, int malicious_nodes, Network network, Network overlay_network, bool with_eclipse) : num_nodes(num_nodes), T(T), mean_txn_interval(mean_txn_interval), mean_block_interval(mean_block_interval), timeout_time(timeout_time), malicious_nodes(malicious_nodes), overlay_network(overlay_network), network(network), with_eclipse(with_eclipse) {            
            ringmaster = 0;
            initialize_nodes();
        }

        void initialize_nodes();
        void schedule_txn(double current_time, int node);
        void broadcast_txn(double current_time, int node, const Transaction& txn);
        void handle_txn_send(const Event& cur_event);
        void handle_txn_recv(const Event& cur_event);
        void handle_block_send(const Event& cur_event);
        void handle_block_recv(const Event& cur_event);
        void schedule_block(double current_time, int node);
        void process_orphan_blocks(int parent, int node, double time);
        void handle_timer_expired(const Event& cur_event);
        void handle_hash_recv(const Event& cur_event);
        void handle_get_request(const Event& cur_event);
        void send_get_request(double current_time, const string &hash, int sender, int receiver);
        void start_timer(double current_time, const string& hash, int node);
        void send_block(int sender, int receiver, const Block& block, double current_time);
        void mal_send_block(int sender, int receiver, const Block& block, double current_time);
        // Malicious Node Functions
        void send_hash_to_peers(double current_time, const Block& block, int node);
        void mal_send_hash_to_peers(double current_time, const Block& block, int node);
        void send_cmd_to_peers(double current_time, int node, string command);
        void handle_cmd_recv(const Event& cur_event);
        void run();
        void save_stats();
        void save_csv();

};

#endif