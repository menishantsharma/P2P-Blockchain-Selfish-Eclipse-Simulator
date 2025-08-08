#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <iostream>
#include "utils.h"

#define TXN_SIZE 1  // 1 KB

class Transaction {
    public:
        int id;
        int sender;
        int receiver;
        double amount;
        Transaction(int id, int sender, int receiver, double amount) : id(id), sender(sender), receiver(receiver), amount(amount) {}

        bool operator==(const Transaction& txn) const {
            return id == txn.id;
        }

        friend ostream& operator<<(ostream& os, const Transaction& txn) {
            os << "Sender: " << txn.sender << " Receiver: " << txn.receiver << " Amount: " << txn.amount;
            return os;
        }

};

namespace std {
    template <>
    struct hash<Transaction> {
        size_t operator()(const Transaction& txn) const noexcept {
            return std::hash<int>()(txn.id);
        }
    };
}

#endif