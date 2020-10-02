#pragma once 
#include <chrono>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "parser.hpp"
#include "hello.h"
#include <signal.h>
//added includes for perfect links
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

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

        int send(const void *msg, Parser::Host peer){
            
            sockaddr_in destSocket;
            std::memset(&destSocket, 0, sizeof(destSocket));

            destSocket.sin_family = AF_INET;
            destSocket.sin_addr.s_addr = peer.ip;
            destSocket.sin_port = peer.port;
            if(sendto(fd, msg, sizeof(msg), 0,reinterpret_cast<const sockaddr*>(&destSocket), sizeof(destSocket))){
                //std::cout << "Sent message" << std::endl;
                return 1;
            }else{
                std::cout << "Couldn't send message" << std::endl;
                return 0;
            }
        }

        int deliver(void *msg){
            if (recv(fd, msg, 1, 0) < 0) {
                throw std::runtime_error("Could not read from the barrier socket: " +
                                        std::string(std::strerror(errno)));
            }else {
                return 1;
            }
        }
    private:
        int fd;
        struct sockaddr_in server;


};