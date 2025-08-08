#include "node.h"
#include <fstream>
#include <filesystem>

// Add the received block to the blockchain
void Node::add_block(const Block& block, int chain_type, double time) {
    add_block_time[block.id] = time;
    blockchain[block.id] = block;
    if(block.height > blockchain[last_block[chain_type]].height) last_block[chain_type] = block.id;        
}

bool Node::is_block_valid(int num_nodes, const Block& block) {
    vector<int> balances = blockchain[block.parent].balances;

    for(auto txn: block.txns) {
        if(balances[txn.sender] < txn.amount) return false;
        balances[txn.sender] -= txn.amount;
    }

    return true;
}

unordered_set<Transaction> Node::get_valid_txns(int chain_type) {
    vector<int> balances = blockchain[last_block[chain_type]].balances;
    unordered_set<int> included_txn_ids;
    for(int cur = last_block[chain_type]; cur != -1; cur = blockchain[cur].parent) {
        for(auto txn: blockchain[cur].txns) included_txn_ids.insert(txn.id);
    }

    unordered_set<Transaction> valid_txns = {};

    for(const auto& txn: received_txns) {
        if(included_txn_ids.find(txn.id) == included_txn_ids.end()) {
            if(balances[txn.sender] >= txn.amount) {
                balances[txn.sender] -= txn.amount;
                valid_txns.insert(txn);
            }
        }

        if(valid_txns.size() == MAX_TXNS) break;
    }

    return valid_txns;
}

void Node::mark_txn_received(const Transaction& txn) {
    received_txns.insert(txn);
}

bool Node::is_txn_received(const Transaction& txn) {
    return (received_txns.find(txn) != received_txns.end());
}

void Node::mark_block_received(const Block& block, double time) {
    received_block_time[block.id] = time;
    received_blocks[block.block_hash()] = block;
}

bool Node::is_block_received(const string& hash) {
    return (received_blocks.find(hash) != received_blocks.end());
}

bool Node::is_block_orphan(const Block& block) {
    return (blockchain.find(block.parent) == blockchain.end());
}

void Node::mark_block_orphan(const Block& block) {
    orphan_blocks.insert(block);
}

void Node::unmark_block_orphan(const Block& block) {
    orphan_blocks.erase(block);
}

void Node::set_timer(const string& hash, int node) {
    timers[hash].insert(node);
}

void Node::remove_timer(const string& hash) {
    timers.erase(hash);
}

bool Node::is_timer_set(const string& hash) {
    return (timers.find(hash) != timers.end());
}

void Node::mark_command_received(const string& command) {
    received_commands.insert(command);
}

bool Node::is_command_received(const string& command) {
    return (received_commands.find(command) != received_commands.end());
}

bool Node::is_parent_changed(const Block& block, int chain_type) {
    return (last_block[chain_type] != block.parent);
}

void Node::mark_hash_received(const string& hash) {
    received_hashes.insert(hash);
}

bool Node::is_hash_received(const string& hash) {
    return (received_hashes.find(hash) != received_hashes.end());
}

void Node::draw_blockchain(bool with_eclipse) {
    filesystem::create_directories("vis");
    string filename = with_eclipse ? "vis/" + to_string(id) + "_with_eclipse" : "vis/" + to_string(id) + "_without_eclipse";
    
    ofstream file(filename + ".dot");

    if(!file) {
        cout << "Error opening file" << endl;
        return;
    }

    file << "digraph Blockchain {" << endl;

    // Make border thick with green color
    for(int cur = last_block[1]; cur != -1; cur = blockchain[cur].parent) file << " " << cur << " [fillcolor=green, style=filled];" << endl;
    
    for(const auto& block: blockchain) {
        if(block.second.miner == 0) {
            file << " " << block.second.id << " [fontcolor=white, fillcolor=red, style=filled];" << endl;
        }
        if(block.second.parent != -1) {
            file << " " << block.second.parent << " -> " << block.second.id  << ";" << endl;
        }

        file << " " << block.second.id << " [label=\"" << block.second.id << "\\n" << "MineSTime: " << block.second.mine_start_time << "\\n" << "MineETime: " << block.second.mine_end_time << "\\n" << "RecvTime: " << received_block_time[block.second.id] << "\\n" << "AddTime: " << add_block_time[block.second.id] << "\\n" << "RecvFrom: " << block_received_from[block.second.id] << "\\n" << "Miner: " << block.second.miner << "\\n" << "Block Hash: " << block.second.block_hash() << "\\n" << "\"];" << endl;
    }

    file << "}" << endl;

    file.close();

    string command = with_eclipse ? "dot -Tpng " + filename + ".dot -o " + filename + ".png" : "dot -Tpng " + filename + ".dot -o " + filename + ".png";
    system(command.c_str());
    command = "rm " + filename + ".dot";
    system(command.c_str());
}

void Node::print_balances() {
    vector<int> balances = blockchain[last_block[1]].balances;
    
    cout << "+-----------+-----------+" << endl;
    cout << "|  Node     |  Balance  |" << endl;
    cout << "+-----------+-----------+" << endl;

    for(int i = 0; i < balances.size(); i++) {
        cout << "| " << setw(9) << i
        << " | " << setw(9) << balances[i] << " |" << endl;
    }

    cout << "+-----------+-----------+" << endl;
}

int Node::get_my_blocks_in_main_chain() {
    int count = 0;
    for(int cur = last_block[1]; cur != -1; cur = blockchain[cur].parent) {
        if(blockchain[cur].miner == id) count++;
    }

    return count;
}

bool Node::is_time_to_attack() {
    return (blockchain[last_block[0]].height > blockchain[last_block[1]].height);
}