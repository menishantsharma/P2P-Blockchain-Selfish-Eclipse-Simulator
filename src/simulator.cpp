#include "simulator.h"
#include <unordered_set>
#include "event.h"
#include "node.h"
#include "network.h"
#include "transaction.h"
#include "utils.h"
#include <fstream>

#include <iostream>

using namespace std;

#define PRIVATE_CHAIN 0
#define PUBLIC_CHAIN 1

void Simulator::initialize_nodes() {
    for(int i = 0; i < malicious_nodes; i++) {
        Node* new_node = new Node(i, false, 1.0/num_nodes, genesis, true);
        nodes.push_back(new_node);
        new_node->mark_block_received(genesis, 0);
        new_node->add_block_time[genesis.id] = 0;

    }

    for(int i = malicious_nodes; i < num_nodes; i++) {
        Node* new_node = new Node(i, true, 1.0/num_nodes, genesis, false);
        nodes.push_back(new_node);
        new_node->mark_block_received(genesis, 0);
        new_node->add_block_time[genesis.id] = 0;
    }

    nodes[ringmaster]->hashing_power = malicious_nodes * 1.0 / num_nodes;
}

/***********************************
        Transaction Handling
************************************/

void Simulator::schedule_txn(double current_time, int node) {
    Transaction txn(txn_id++, node, utils::generateRandomNumber(malicious_nodes, num_nodes - 1), utils::generateRandomNumber(1, 10));
    double next_txn_time = current_time + utils::get_exponential_random_number(mean_txn_interval);
    Event new_event(node, -1, next_txn_time, EventType::TXN_SEND, txn);
    events.push(new_event);
}

void Simulator::broadcast_txn(double current_time, int node, const Transaction& txn) {
    const auto& peers = nodes[node]->is_malicious ? overlay_network.graph[node] : network.graph[node];

    for(const auto& peer: peers) {        
        double latency = network.calculate_latency(node, peer, TXN_SIZE, nodes[node]->is_slow, nodes[peer]->is_slow);
        Event new_event(node, peer, current_time + latency, EventType::TXN_RECV, txn);
        events.push(new_event);
    }
}

void Simulator::handle_txn_send(const Event& cur_event) {
    Node* cur_node = nodes[cur_event.sender];
    Transaction txn = cur_event.get_txn();

    cur_node->mark_txn_received(txn);
    broadcast_txn(cur_event.time, cur_node->id, txn);
    if(cur_event.time < T) schedule_txn(cur_event.time, cur_node->id);
}

void Simulator::handle_txn_recv(const Event& cur_event) {
    Node* cur_node = nodes[cur_event.receiver];
    Transaction txn = cur_event.get_txn();

    if(cur_node->is_txn_received(txn)) return;
    cur_node->mark_txn_received(txn);
    broadcast_txn(cur_event.time, cur_node->id, txn);
}


/***********************************
        Block Handling
************************************/

void Simulator::schedule_block(double current_time, int node) {
    Node* cur_node = nodes[node];
    int chain = cur_node->is_malicious ? PRIVATE_CHAIN : PUBLIC_CHAIN;
    unordered_set<Transaction> txns = cur_node->get_valid_txns(chain);
    Block new_block(block_id++, cur_node->id, cur_node->blockchain[cur_node->last_block[chain]], txns);
    new_block.mine_start_time = current_time;

    double next_block_time = current_time + utils::get_exponential_random_number(mean_block_interval / cur_node->hashing_power);
    Event new_event(node, -1, next_block_time, EventType::BLOCK_SEND, new_block);
    events.push(new_event);
}

void Simulator::send_cmd_to_peers(double current_time, int node, string command) {
    for(const auto& peer: overlay_network.graph[node]) {
        double latency = overlay_network.calculate_latency(node, peer, COMMAND_SIZE, nodes[node]->is_slow, nodes[peer]->is_slow);
        Event new_event(node, peer, current_time + latency, EventType::COMMAND_RECV, command);
        events.push(new_event);
    }
}

void Simulator::send_hash_to_peers(double current_time, const Block& block, int node) {
    for(const auto& peer: network.graph[node]) {
        double latency = network.calculate_latency(node, peer, HASH_SIZE, nodes[node]->is_slow, nodes[peer]->is_slow);
        Event new_event(node, peer, current_time + latency, EventType::HASH_RECV, block.block_hash());
        events.push(new_event);
    }
}

void Simulator::mal_send_hash_to_peers(double current_time, const Block& block, int node) {
    for(const auto& peer: overlay_network.graph[node]) {
        double latency = overlay_network.calculate_latency(node, peer, HASH_SIZE, nodes[node]->is_slow, nodes[peer]->is_slow);
        Event new_event(node, peer, current_time + latency, EventType::HASH_RECV, block.block_hash());
        events.push(new_event);
    }
}

void Simulator::handle_block_send(const Event& cur_event) {
    Block cur_block = cur_event.get_block();
    Node* cur_node = nodes[cur_event.sender];

    if(!cur_node->is_malicious and cur_node->is_parent_changed(cur_block, PUBLIC_CHAIN)) return;

    cur_block.mine_end_time = cur_event.time;

    cur_node->block_received_from[cur_block.id] = cur_node->id;
    cur_node->mark_block_received(cur_block, cur_event.time);
    cur_node->mined_blocks++;

    if(cur_node->id == ringmaster) {
        cur_node->add_block(cur_block, PRIVATE_CHAIN, cur_event.time);
        mal_send_hash_to_peers(cur_event.time, cur_block, cur_node->id);

        if(cur_node->is_time_to_attack()) {
            cur_node->last_block[1] = cur_node->last_block[0];
            send_cmd_to_peers(cur_event.time, cur_node->id, to_string(cur_event.time));
        }
    }
    else {
        cur_node->add_block(cur_block, PUBLIC_CHAIN, cur_event.time);
        send_hash_to_peers(cur_event.time, cur_block, cur_node->id);
    }

    if(cur_event.time < T) schedule_block(cur_event.time, cur_node->id);
}

void Simulator::process_orphan_blocks(int parent, int node, double current_time) {
    queue<int>q;
    q.push(parent);
    Node* cur_node = nodes[node];
    
    while(!q.empty()) {
        int cur_block = q.front();
        q.pop();

        for(const auto& block: cur_node->orphan_blocks) {
            if(block.parent == cur_block) {
                if(!cur_node->is_block_valid(num_nodes, block)) continue;

                if(cur_node->is_malicious) {
                    if(block.miner == ringmaster) cur_node->add_block(block, PRIVATE_CHAIN, current_time);
                    else {
                        cur_node->add_block(block, PUBLIC_CHAIN, current_time);
                        send_hash_to_peers(current_time, block, node);
                    }
                    mal_send_hash_to_peers(current_time, block, node);
                }

                else {
                    cur_node->add_block(block, PUBLIC_CHAIN, current_time);
                    send_hash_to_peers(current_time, block, node);
                }

                q.push(block.id);
            }
        }
    }
}

void Simulator::handle_block_recv(const Event& cur_event) {
    Block cur_block = cur_event.get_block();
    Node* cur_node = nodes[cur_event.receiver];

    if(cur_node->is_block_received(cur_block.block_hash())) return;
    cur_node->block_received_from[cur_block.id] = cur_event.sender;
    cur_node->mark_block_received(cur_block, cur_event.time);

    if(cur_node->is_block_orphan(cur_block)) {
        cur_node->mark_block_orphan(cur_block);
        return;
    }

    if(!cur_node->is_block_valid(num_nodes, cur_block)) return;

    if(cur_node->is_malicious) {
        if(cur_block.miner == ringmaster) {
            cur_node->add_block(cur_block, PRIVATE_CHAIN, cur_event.time);
        }

        else {
            cur_node->add_block(cur_block, PUBLIC_CHAIN, cur_event.time);
            send_hash_to_peers(cur_event.time, cur_block, cur_node->id);
        }

        mal_send_hash_to_peers(cur_event.time, cur_block, cur_node->id);
        process_orphan_blocks(cur_block.id, cur_node->id, cur_event.time);
    }

    else  {
        cur_node->add_block(cur_block, PUBLIC_CHAIN, cur_event.time);
        send_hash_to_peers(cur_event.time, cur_block, cur_node->id);
        process_orphan_blocks(cur_block.id, cur_node->id, cur_event.time);
        if(cur_event.time < T) schedule_block(cur_event.time, cur_node->id);
    }
}

/***********************************
        Timer Handling
************************************/

void Simulator::send_get_request(double current_time, const string &hash, int sender, int receiver) {
    double latency = network.calculate_latency(sender, receiver, GET_REQUEST_SIZE, nodes[sender]->is_slow, nodes[receiver]->is_slow);
    Event new_event(sender, receiver, current_time + latency, EventType::GET_REQUEST, hash);
    events.push(new_event);
}

void Simulator::start_timer(double current_time, const string& hash, int node) {
    Event timer_event(-1, node, current_time + timeout_time, EventType::TIMER_EXPIRED, hash);
    events.push(timer_event);
}

void Simulator::handle_timer_expired(const Event& cur_event) {
    Node* cur_node = nodes[cur_event.receiver];
    string hash = cur_event.get_hash();

    if(!cur_node->is_block_received(hash) and cur_node->timers[hash].size() > 0) {
        int peer = *cur_node->timers[hash].begin();

        // Remove the peer from the timers array
        cur_node->timers[hash].erase(peer);

        // Send the get request to first peer waiting in timers array
        send_get_request(cur_event.time, hash, cur_node->id, peer);

        // Start the timer again
        start_timer(cur_event.time, hash, cur_node->id);
    }

    // Remove the timer for the hash
    else {
        cur_node->remove_timer(hash);
    }
}

void Simulator::handle_hash_recv(const Event& cur_event) {
    Node* cur_node = nodes[cur_event.receiver];
    string hash = cur_event.get_hash();
    // Check if the hash is already received before
    if(cur_node->is_hash_received(hash)) {
        if(cur_node->is_block_received(hash)) return;
        else {
            // Timeout running for the hash
            if(cur_node->is_timer_set(hash)) cur_node->set_timer(hash, cur_event.sender);

            // If no timer running for the hash
            else {
                // Send get request to the sender
                send_get_request(cur_event.time, hash, cur_node->id, cur_event.sender);

                // Start the timer
                nodes[cur_node->id]->set_timer(hash, cur_event.sender);     
                start_timer(cur_event.time, hash, cur_node->id);
            }
        }
    }

    else {
        cur_node->mark_hash_received(hash);

        // Send get request to the sender
        send_get_request(cur_event.time, hash, cur_node->id, cur_event.sender);

        // Start the timer
        nodes[cur_node->id]->set_timer(hash, cur_event.sender);     
        start_timer(cur_event.time, hash, cur_node->id);
    }
}

void Simulator::send_block(int sender, int receiver, const Block& block, double current_time) {
    double latency = network.calculate_latency(sender, receiver, block.txns.size() + 1, nodes[sender]->is_slow, nodes[receiver]->is_slow);
    Event new_event(sender, receiver, current_time + latency, EventType::BLOCK_RECV, block);
    events.push(new_event);
}

void Simulator::mal_send_block(int sender, int receiver, const Block& block, double current_time) {
    double latency = overlay_network.calculate_latency(sender, receiver, block.txns.size() + 1, nodes[sender]->is_slow, nodes[receiver]->is_slow);
    Event new_event(sender, receiver, current_time + latency, EventType::BLOCK_RECV, block);
    events.push(new_event);
}

void Simulator::handle_get_request(const Event& cur_event) {
    Node* cur_node = nodes[cur_event.receiver];
    Node* other_node = nodes[cur_event.sender];
    string hash = cur_event.get_hash();

    if(!cur_node->is_block_received(hash)) return;

    Block block = cur_node->received_blocks[hash];

    if(cur_node->is_malicious) {
        if(other_node->is_malicious) mal_send_block(cur_node->id, other_node->id, block, cur_event.time);
        else {
            if(block.miner == ringmaster) send_block(cur_node->id, other_node->id, block, cur_event.time);
            else if(!with_eclipse) send_block(cur_node->id, other_node->id, block, cur_event.time);
        }
    }

    else {
        send_block(cur_node->id, other_node->id, block, cur_event.time);
    }
}

void Simulator::handle_cmd_recv(const Event& cur_event) {
    Node* cur_node = nodes[cur_event.receiver];
    string command = cur_event.get_hash();

    if(cur_node->is_command_received(command)) return;
    cur_node->mark_command_received(command);
    
    for(int cur = cur_node->last_block[0]; cur != cur_node->last_block_sent; cur = cur_node->blockchain[cur].parent) {
        send_hash_to_peers(cur_event.time, cur_node->blockchain[cur], cur_node->id);
    }

    cur_node->last_block[1] = cur_node->last_block[0];

    cur_node->last_block_sent = cur_node->last_block[0];
    send_cmd_to_peers(cur_event.time, cur_node->id, command);
}

void Simulator::run() {
    for(int i = malicious_nodes; i < num_nodes; i++) {
        schedule_txn(0, i);
        schedule_block(0, i);
    }

    schedule_block(0, ringmaster);
    
    double current_time = 0;

    while(!events.empty()) {
        Event cur_event = events.top();
        events.pop();
        current_time = cur_event.time;

        // Show progress
        // utils::printProgressBar(current_time, max(T, current_time), 50);

        switch(cur_event.event_type) {
            case EventType::TXN_SEND:
                handle_txn_send(cur_event);
                break;
            case EventType::TXN_RECV:
                handle_txn_recv(cur_event);
                break;
            case EventType::BLOCK_SEND:
                handle_block_send(cur_event);
                break;
            case EventType::BLOCK_RECV:
                handle_block_recv(cur_event);
                break;
            case EventType::TIMER_EXPIRED:
                handle_timer_expired(cur_event);
                break;
            case EventType::HASH_RECV:
                handle_hash_recv(cur_event);
                break;
            case EventType::GET_REQUEST:
                handle_get_request(cur_event);
                break;
            case EventType::COMMAND_RECV:
                handle_cmd_recv(cur_event);
                break;
        }
    }
}

void Simulator::save_stats() {
    int mined_blocks_in_chain = 0;
    for(int cur = nodes[ringmaster]->last_block[1]; cur != -1; cur = nodes[ringmaster]->blockchain[cur].parent) if(nodes[ringmaster]->blockchain[cur].miner == ringmaster) mined_blocks_in_chain++;
    int longest_chain_length = nodes[ringmaster]->blockchain[nodes[ringmaster]->last_block[1]].height;

    cout << endl << "Malicious Blocks in Longest Chain at Ringmaster: " << mined_blocks_in_chain << endl;
    cout << "Total Malicious Blocks Created: " << nodes[ringmaster]->mined_blocks << endl;
    cout << "Total Blocks in the Longest Chain at Ringmaster: " << longest_chain_length << endl;
    cout << "Fraction of Blocks Mined by Ringmaster included in Main Chain to Total Blocks in Longest Chain " << mined_blocks_in_chain * 1.0 / longest_chain_length << endl;
    cout << "Fraction of Blocks Mined by Ringmaster included in Main Chain to Total Blocks Mined by Ringmaster: " << mined_blocks_in_chain * 1.0 / nodes[ringmaster]->mined_blocks << endl;
    cout << "Hashing Power of Ringmaster: " << nodes[ringmaster]->hashing_power << endl;

    nodes[ringmaster]->draw_blockchain(with_eclipse);

    // for(const auto& node: nodes) node->draw_blockchain(with_eclipse);
}

void Simulator::save_csv() {
    string filename;
    if(with_eclipse) filename = "results_eclipse.csv";
    else filename = "results_no_eclipse.csv";

    ofstream file(filename, ios::app);

    if(!file) {
        cout << "Error opening file" << endl;
        return;
    }

    int mined_blocks_in_chain = 0;
    for(int cur = nodes[ringmaster]->last_block[1]; cur != -1; cur = nodes[ringmaster]->blockchain[cur].parent) if(nodes[ringmaster]->blockchain[cur].miner == ringmaster) mined_blocks_in_chain++;
    int longest_chain_length = nodes[ringmaster]->blockchain[nodes[ringmaster]->last_block[1]].height;

    file << mined_blocks_in_chain << "," << nodes[ringmaster]->mined_blocks << "," << longest_chain_length << "," << mined_blocks_in_chain * 1.0 / longest_chain_length << "," << mined_blocks_in_chain * 1.0 / nodes[ringmaster]->mined_blocks << endl;
    file.close();

}