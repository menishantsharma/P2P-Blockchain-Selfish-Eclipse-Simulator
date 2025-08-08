#ifndef EVENT_H
#define EVENT_H

#include <iostream>
#include "node.h"
#include "transaction.h"
#include "block.h"

enum class EventType {
    TXN_SEND = 0,
    TXN_RECV = 1,
    BLOCK_SEND = 2,
    BLOCK_RECV = 3,
    TIMER_EXPIRED = 4,
    HASH_RECV = 5,
    GET_REQUEST = 6,
    COMMAND_RECV = 7,
};

class Event {
    public:
        int id;
        double time;
        variant<monostate, Transaction, Block, string> data;
        EventType event_type;
        int sender;
        int receiver;
        
        Event(int sender, int receiver, double time, EventType event_type, const Transaction& txn) : sender(sender), receiver(receiver), time(time), data(txn), event_type(event_type) {}
        Event(int sender, int receiver, double time, EventType event_type, const Block& block) : sender(sender), receiver(receiver), time(time), data(block), event_type(event_type) {}
        Event(int sender, int receiver, double time, EventType event_type, const string& hash) : sender(sender), receiver(receiver), time(time), data(hash), event_type(event_type) {}

        inline const Transaction& get_txn() const {
            return get<Transaction>(data);
        }

        inline const Block& get_block() const {
            return get<Block>(data);
        }

        inline const string& get_hash() const {
            return get<string>(data);
        }

        bool operator<(const Event& e) const {
            return time > e.time;
        }

        friend ostream& operator<<(ostream& os, const Event& e) {
            os << "Time: " << e.time << " Sender: " << e.sender << " Receiver: " << e.receiver << " Event Type: " << static_cast<int>(e.event_type);
            return os;
        }
};

#endif