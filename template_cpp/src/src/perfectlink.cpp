#include "perfectlink.hpp"
#include "packet.hpp"
#include <thread>

PerfectLink::PerfectLink(Parser::Host localhost,  std::function<void(Packet)> pp2pDeliver, std::function<void(unsigned long)> onCrash, std::map<unsigned long, Parser::Host> idToPeer){
    std::cout << "Create perfect link " << std::endl;
    this->pp2pDeliver = pp2pDeliver; 
    this->onCrash = onCrash;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        throw std::runtime_error("Could not create the UDP socket: " +
                                std::string(std::strerror(errno)));
    }
    this->idToPeer = idToPeer;
    std::memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = localhost.ip;
    server.sin_port = localhost.port;
    int bindAttempt = bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server));
    if(bindAttempt != 0){
        throw std::runtime_error("Could not bind the socket local address: " +
                                std::string(std::strerror(errno)));            
    }
    std::thread t1(&PerfectLink::listen, this, localhost.id);
    std::thread t2(&PerfectLink::resendMessages, this, localhost.id);
    t1.detach();
    t2.detach();

}

void PerfectLink::resendMessages(unsigned long localhostID){
    while(true){
        usleep(1000000000);
        lock.lock();
        std::map<std::string, Packet> toResend = waitingAcks;
        lock.unlock();
        for(auto& [key, pkt]: toResend){
            std::cout << "Resend " << pkt.toString() << std::endl;
            send(&pkt, idToPeer[pkt.destinationID]);
        }
    }
}
int PerfectLink::send(const Packet *msg, Parser::Host peer){    

    sockaddr_in destSocket;
    std::memset(&destSocket, 0, sizeof(destSocket));

    destSocket.sin_family = AF_INET;
    destSocket.sin_addr.s_addr = peer.ip;
    destSocket.sin_port = peer.port;
    if(sendto(fd, msg, sizeof(Packet), 0,reinterpret_cast<const sockaddr*>(&destSocket), sizeof(destSocket))){
        //sent.push_back(*msg);
        lock.lock();
        if(!msg->ack){
            std::string key = ackKey(*msg);
            waitingAcks.insert({key, *msg});
        }
        lock.unlock();
        return 1;
    }else{
        std::cout << "Couldn't send message" << std::endl;
        return 0;
    }
}

std::string PerfectLink::ackKey(Packet pkt){
    std::ostringstream stream;
    stream << pkt.peerID << " " << pkt.senderID << " " << pkt.destinationID << " " << pkt.payload;
    return stream.str();
}


void PerfectLink::listen(unsigned long localID){
    bool deliveryReady = false;
    while(true){
        //std::cout << "Listening"<< std::endl;
        while(!deliveryReady){
            Packet pkt;
            if (recv(fd, &pkt, sizeof(Packet), 0) < 0) {
                            throw std::runtime_error("Could not read from the perfect link socket: " +std::string(std::strerror(errno)));
            }else {
		//std::cout << "received pkt " << pkt.peerID << "  from " << pkt.senderID << "  seq " << pkt.payload << std::endl;
                auto iter = std::find_if(delivered.begin(), delivered.end(), 
                            [&](const Packet& p){return p.equals(pkt);});
                deliveryReady = (delivered.size() == 0) || iter == delivered.end();
                if(deliveryReady){
                    //be careful with the unlock

                    delivered.push_back(pkt);
                    if(pkt.ack){
                        std::string key = ackKey(pkt);
                        lock.lock();
                        std::cout << pkt.toString() << std::endl;
                        std::cout << "Ack size " << waitingAcks.size() << std::endl;
                        std::map<std::string, Packet>::iterator rmIt =  waitingAcks.find(key);
                        
                        if(rmIt !=waitingAcks.end()){
                            waitingAcks.erase(rmIt);
                        }
                        std::cout << "Ack size " << waitingAcks.size() << std::endl;
                        lock.unlock();
                    }else{
                        Packet ack(pkt.peerID, pkt.senderID, pkt.payload, PacketType::ACK, true);
                        lock.lock();
                        Parser::Host peer =  idToPeer[pkt.senderID];
                        lock.unlock();
                        send(&ack, peer);
                        pp2pDeliver(pkt);
                    }
                }
            }
        }
        deliveryReady = false;
    }
    std::cout << "out"<< std::endl;
}
