#pragma once
#include <iostream>
#include <stdio.h>
#include "packet.hpp"
#include "fifob.hpp"
#include "parser.hpp"
#include <mutex>
class FIFOController{
    public:
        FIFOController(){}
        FIFOController(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> fifoDeliver,std::function<void(Packet)> broadcastCB, Coordinator* coordinator);
        void broadcast(unsigned long n);

        void deliver(Packet pkt);
        void stop();
        std::vector<int> vectorClock;

    private:
        void groupedBroadcast(unsigned long n);
        unsigned long nPacketsPerBroadcast;
        Parser::Host localhost;
        unsigned long maxPackets;
        unsigned long id;
        FIFOBroadcast* fifo;
        std::function<void(Packet)> fifoDeliver;
        std::function<void(Packet)> broadcastCB;
        Coordinator* coordinator;
        unsigned long nLocalDeliveries;
        unsigned long counter;
        std::mutex mtx;
};