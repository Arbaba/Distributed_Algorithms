#include "perfectlink.hpp"
#include "packet.hpp"
#include <thread>

PerfectLink::PerfectLink(in_addr_t ip, in_port_t port, std::function<void(Packet)> pp2pDeliver){
    std::cout << "Create perfect link " << std::endl;
    this->pp2pDeliver = pp2pDeliver; 
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
    std::thread t1(&PerfectLink::listen, this);
    t1.detach();

}

int PerfectLink::send(const Packet *msg, Parser::Host peer){
            
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

void PerfectLink::listen(){
    bool deliveryReady = false;
    while(true){
        //std::cout << "Listening"<< std::endl;
        while(!deliveryReady){
            Packet pkt;
            if (recv(fd, &pkt, sizeof(Packet), 0) < 0) {
                            throw std::runtime_error("Could not read from the perfect link socket: " +std::string(std::strerror(errno)));
            }else {
                auto iter = std::find_if(delivered.begin(), delivered.end(), 
                            [&](const Packet& p){return p.equals(pkt);});
                deliveryReady = (delivered.size() == 0) || iter == delivered.end();
                if(deliveryReady){
                    pp2pDeliver(pkt);
                    delivered.push_back(pkt);
                }
            }
        }
        deliveryReady = false;
    }
    std::cout << "out"<< std::endl;
}
