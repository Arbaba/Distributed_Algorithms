#pragma once 
#include <chrono>
#include <iostream>
#include <thread>
#include "barrier.hpp"
#include "parser.hpp"
#include "hello.h"
#include "packet.hpp"
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <mutex>
class PerfectLink{
    public:
        PerfectLink(){};
        PerfectLink(Parser::Host localhost,  std::function<void(Packet)> pp2pDeliver,  std::function<void(unsigned long)> onCrash, std::map<unsigned long, Parser::Host> idToPeer);

        int send(const Packet *msg, Parser::Host peer);
        
        void listen(unsigned long localID);
        void resendMessages(unsigned long localID);
        void pingPeers();
    private:
        void crashed(unsigned long processID);
        std::string ackKey(Packet pkt);
        std::mutex lock;
        std::mutex counterLock;
        std::vector<Packet> sent;
        std::vector<Packet> delivered;
        std::map<std::string, Packet> waitingAcks;
        std::map<unsigned long, unsigned int> pingCounter;
        Parser::Host localhost;
        //mapping string of "peerID senderID payload" to boolean
        std::map<std::string, bool> acked;
        int fd;
        struct sockaddr_in server;
        std::function<void(Packet)>pp2pDeliver;
        std::map<unsigned long, Parser::Host> idToPeer;
        std::function<void(unsigned long)> onCrash;
        std::set<unsigned long> correctProcesses;
        std::map<unsigned long, int> countPerProcess;
};