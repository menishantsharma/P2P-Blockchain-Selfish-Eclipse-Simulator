#ifndef BLOCK_H
#define BLOCK_H

#include <unordered_set>
#include <iostream>
#include <string>
#include <functional>
#include "utils.h"
#include "transaction.h"

using namespace std;

#define MAX_TXNS 1023
class Block {
    public:
        int id;
        int miner;
        int parent;
        double mine_start_time;
        double mine_end_time;

        unordered_set<Transaction> txns;
        vector<int> balances;
        int height;

        Block() : id(-1), miner(-1), parent(-1), mine_start_time(0), mine_end_time(0) {}

        Block(int id, int num_nodes) : id(id), miner(-1), parent(-1), mine_start_time(0), mine_end_time(0) {
            balances.resize(num_nodes, 1000);
            height = 0;
        }

        Block(int id, int miner, Block parent, unordered_set<Transaction> txns) : id(id), miner(miner), parent(parent.id), txns(txns){
            height = parent.height + 1;     
            calculate_balances(parent);
        }

        string block_hash() const {
            string header = to_string(id) + to_string(miner) + to_string(parent);
            hash<string> hasher;
            size_t hashValue = hasher(header);
            return to_string(hashValue);
        }

        void calculate_balances(Block parent) {
            if(parent.id != -1) balances = parent.balances;

            for(const auto& txn: txns) {
                balances[txn.sender] -= txn.amount;
                balances[txn.receiver] += txn.amount;
            }

            balances[miner] += 50;
        }

        bool operator==(const Block& b) const {
            return id == b.id;
        }

        bool operator<(const Block& b) const {
            return id < b.id;
        }

        friend ostream& operator<<(ostream& os, const Block& b) {
            os << "Block: " << b.id << "\tParent: " << b.parent << "\tMiner: " << b.miner << "\tTxns: " << b.txns.size();
            return os;
        }
};

namespace std {
    template <>
    struct hash<Block> {
        size_t operator()(const Block& b) const {
            return hash<int>()(b.id);
        }
    };
}

#endif