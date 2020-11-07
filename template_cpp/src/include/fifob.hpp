#pragma once

#include "urb.hpp"
#include <map>
#include <mutex>
class FIFOBroadcast{
    public:
        FIFOBroadcast(){};
        FIFOBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> fifoDeliver);
        
        void broadcast(Packet pkt);
    private:
        void urbDeliver(Packet pkt);
        void addToPending(Packet pkt);
        std::vector<Packet> pendingOrdered(int processID);
        void removePending(Packet pkt);
        bool receivedPrevious(Packet pkt);
        int nextSeqNumber(size_t processID);
        void increaseSeqNumber(size_t processID);
        UniformBroadcast* urb;
        //buffered packets of a
        std::map<unsigned long, std::vector<Packet>> pendingSorted;
        //array next, which contains an entry for every process p with the 
        //sequence number of the next message to be frb-delivered from sender p.
        std::map<unsigned long, int> nextToDeliver;
        std::function<void(Packet)> fifoDeliver;
        std::mutex lock;
};