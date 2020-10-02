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

class PerfectLink{
    public:
        PerfectLink(in_addr_t ip, in_port_t port){
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0) {
                throw std::runtime_error("Could not create the UDP socket: " +
                                        std::string(std::strerror(errno)));
            }
            std::memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = ip;
            server.sin_port = port;
            int bindAttempt = bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server));
            if(bindAttempt != 0){
                throw std::runtime_error("Could not bind the socket local address: " +
                                        std::string(std::strerror(errno)));            
            }
        }

        int send(const Packet *msg, Parser::Host peer){
            
            sockaddr_in destSocket;
            std::memset(&destSocket, 0, sizeof(destSocket));

            destSocket.sin_family = AF_INET;
            destSocket.sin_addr.s_addr = peer.ip;
            destSocket.sin_port = peer.port;
            if(sendto(fd, msg, sizeof(Packet), 0,reinterpret_cast<const sockaddr*>(&destSocket), sizeof(destSocket))){
                return 1;
            }else{
                std::cout << "Couldn't send message" << std::endl;
                return 0;
            }
        }

        int deliver(Packet *pkt){
            bool deliveryReady = false;
            while(!deliveryReady){
                if (recv(fd, pkt, sizeof(Packet), 0) < 0) {
                                throw std::runtime_error("Could not read from the barrier socket: " +
                                                        std::string(std::strerror(errno)));
                }else {
                    auto iter = std::find_if(delivered.begin(), delivered.end(), 
                                [&](const Packet& p){return p.equals(*pkt);});
                    deliveryReady = (delivered.size() == 0) || iter == delivered.end();

                    if(deliveryReady){
                        delivered.push_back(*pkt);
                    }
                }
            }
            return 1;
        }
    private:
        std::vector<Packet> delivered;
        int fd;
        struct sockaddr_in server;


};