#pragma once
#include "perfectlink.hpp"
#include "parser.hpp"

class BeBroadcast {
    public:
        BeBroadcast(){};
        BeBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> bebDeliver);
        void bebBroadcast(Packet pkt);
        void stop();
    private:
        void pp2pDeliver(Packet);
        Parser::Host localhost;
        std::vector<Parser::Host> peers;
        PerfectLink* perfectLink;
        std::function<void(Packet)> bebDeliver;
        std::function<void(unsigned long)> onCrash;
};