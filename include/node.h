#ifndef NODE_H
#define NODE_H

#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <iomanip>
#include "transaction.h"
#include "block.h"

using namespace std;

#define MAL_FOLDER_PATH "vis/mal/"
#define HON_FOLDER_PATH "vis/hon/"

class Node {
    public:
        int id;
        bool is_slow;
        double hashing_power;
        bool is_malicious;
        int mined_blocks = 0;

        unordered_set<string> received_commands;
        unordered_set<string> received_hashes;
        unordered_map<string, unordered_set<int>> timers;
        unordered_map<string, Block> received_blocks;
        unordered_map<int, Block> blockchain;
        unordered_set<Transaction> received_txns;
        unordered_set<Block> orphan_blocks;
        vector<int> last_block;
        unordered_map<int, double> received_block_time;
        unordered_map<int, double> add_block_time;
        unordered_map<int, int> block_received_from;
        int last_block_sent = 0;

        Node(int id, bool is_slow, double hashing_power, Block genesis, bool is_malicious) : id(id), is_slow(is_slow), hashing_power(hashing_power), is_malicious(is_malicious) {
            blockchain[genesis.id] = genesis;
            // 0 - Private Chain, 1 - Public Chain
            last_block.resize(2, genesis.id);
        }

        // Transaction Handling

        unordered_set<Transaction> get_valid_txns(int chain_type);
        void mark_txn_received(const Transaction& txn);
        bool is_txn_received(const Transaction& txn);

        // Block Handling
        void add_block(const Block& block, int chain_type, double time);
        bool is_block_valid(int num_nodes, const Block& block);
        void mark_block_received(const Block& block, double time);
        bool is_block_received(const string& hash);
        bool is_block_orphan(const Block& block);
        void mark_block_orphan(const Block& block);
        void unmark_block_orphan(const Block& block);

        // Timer Handling

        void set_timer(const string& hash, int node);
        void remove_timer(const string& hash);
        bool is_timer_set(const string& hash);

        // Command Handling

        void mark_command_received(const string& command);
        bool is_command_received(const string& command);
        bool is_parent_changed(const Block& block, int chain_type);

        // Hash Handling

        void mark_hash_received(const string& hash);
        bool is_hash_received(const string& hash);

        // Print the blockchain

        void draw_blockchain(bool with_eclipse);
        void print_balances();
        int get_my_blocks_in_main_chain();

        bool is_time_to_attack();
        
        friend ostream& operator<<(ostream& os, const Node& n) {
            os << "Node: " << n.id;
            // os << " Txns: " << n.txn_pool.size() << " Is Node Slow: " << n.is_slow << " Is Node Low CPU: " << n.is_low_cpu << " Hashing Power: " << n.hashing_power;
            return os;
        }
};
#endif