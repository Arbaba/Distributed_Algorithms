#pragma once
#include <iostream>
#include <stdio.h>
#include "packet.hpp"
#include "fifoController.hpp"
#include "parser.hpp"
class LCBroadcast{
    public:
        LCBroadcast(){}
        LCBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> lcbDeliver,std::function<void(Packet)> broadcastCB, Coordinator* coordinator);
        void broadcast(unsigned long n);
        void deliver(Packet pkt);
        void stop();
    private:
        void groupedBroadcast(unsigned long n);
        Parser::Host localhost;
        FIFOController* fifoController;
        std::function<void(Packet)> lcbDeliver;
        std::function<void(Packet)> broadcastCB;
        std::map<unsigned long, std::vector<Packet>> pending;
        size_t nProcesses;
};