#pragma once
#include "beb.hpp"
#include <set>
#include <mutex>
class UniformBroadcast {
    public:
    
        UniformBroadcast(){};

        UniformBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers,  std::function<void(Packet)> urbDeliver);

        void broadcast(Packet msg);

    private:

        void storeAck(Packet pkt);
        bool isForwarded(Packet pkt);
        void addToDelivered(Packet pkt);
        void addToForwarded(Packet pkt);
        bool isDelivered(Packet pkt);
        void tryDelivery(Packet pkt);
        bool receivedAllAcks(Packet pkt);
        void crash(size_t processID);
        void bebDeliver(Packet pkt);
        
        Parser::Host localhost;
        BeBroadcast* beb;
        std::set<size_t> correctProcesses;
        //origin -> senderid -> sendr
        std::function<void(Packet)> urbDeliver;
        std::vector<Packet> forwarded;
        std::vector<Packet> acks;
        std::vector<Packet> delivered;
        std::vector<Packet> received;
        std::mutex lock;

};