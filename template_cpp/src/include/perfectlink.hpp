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
class PerfectLink{
    public:
        PerfectLink(){};
        PerfectLink(in_addr_t ip, in_port_t port, std::function<void(Packet)> pp2pDeliver);

        int send(const Packet *msg, Parser::Host peer);

        
        void listen();
    private:
        std::vector<Packet> delivered;
        int fd;
        struct sockaddr_in server;
        std::function<void(Packet)>pp2pDeliver;


};