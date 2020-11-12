#pragma once
#include <iostream>
#include <stdio.h>
#include "packet.hpp"
#include "fifob.hpp"
#include "parser.hpp"
class FIFOController{
    public:
        FIFOController(){}
        FIFOController(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> fifoDeliver,std::function<void(Packet)> broadcastCB);
        void broadcast(unsigned long n);

        void deliver(Packet pkt);

    private:
        void groupedBroadcast(unsigned long n);
        unsigned long nPacketsPerBroadcast;
        Parser::Host localhost;
        unsigned long maxPackets;
        unsigned long id;
        FIFOBroadcast* fifo;
        std::function<void(Packet)> fifoDeliver;
        std::function<void(Packet)> broadcastCB;
        unsigned long nLocalDeliveries;
        unsigned long counter;
};