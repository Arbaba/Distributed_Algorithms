#include "perfectlink.hpp"
#include "packet.hpp"
#include <thread>
#include <set>
PerfectLink::PerfectLink(Parser::Host localhost,  std::function<void(Packet)> pp2pDeliver, std::map<unsigned long, Parser::Host> idToPeer){
    std::cout << "Create perfect link " << std::endl;
    this->pp2pDeliver = pp2pDeliver; 
    this->localhost = localhost; 
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
    for(auto && [id, foo]: idToPeer){
        correctProcesses.insert(id);
    }
    stopFlag = false;
    std::thread t1(&PerfectLink::listen, this, localhost.id);
    std::thread t2(&PerfectLink::resendMessages, this, localhost.id);
    //std::thread t3(&PerfectLink::pingPeers, this);

    t1.detach();
    t2.detach();
    //t3.detach();
}

void PerfectLink::stop(){
    stopLock.lock();
    stopFlag = true;
    stopLock.unlock();
}
bool PerfectLink::shouldStop(){
    stopLock.lock();
    bool b = stopFlag;
    stopLock.unlock();
    return b;
}
void PerfectLink::resendMessages(unsigned long localhostID){
    while(true){
     	std::this_thread::sleep_for(std::chrono::seconds(1));

        if(shouldStop()){
            return;
        }

        lock.lock();
        std::map<std::string, Packet> toResend = waitingAcks;

        lock.unlock();

        //std::cout << "Resend " << toResend.size() << "\n";
        unsigned long count = 0;
        for(auto& [key, pkt]: toResend){
            //std::cout << key <<" Resend " << pkt.toString() << std::endl;
            count++;
            if((count % 10) == 0){
     	        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                 count = 0;
            }
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
        if(!msg->ack && !(msg->peerID == localhost.id && msg->senderID == localhost.id && msg->destinationID == localhost.id) ){
            lock.lock();
            std::string key = ackKey(*msg);
            
            waitingAcks.insert({key, *msg});
            lock.unlock();
        }
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

void PerfectLink::handleAck(Packet pkt){
    std::string key = ackKey(pkt);
    lock.lock();
    //std::cout << "Received ack for packet: " << pkt.toString() << std::endl;
    //std::cout << "Ack size " << waitingAcks.size() << std::endl;
    std::map<std::string, Packet>::iterator rmIt =  waitingAcks.find(key);
    
    if(rmIt !=waitingAcks.end()){
        waitingAcks.erase(rmIt);
    }
    //std::cout << "Ack size " << waitingAcks.size() << std::endl;
    lock.unlock();
}
void PerfectLink::handlePacket(Packet pkt){
      //std::cout << "Received PKT " << pkt.toString() << std::endl; 

    //We send the ack packet to the source by only changing the packet type and ack boolean
    Packet ack(pkt.peerID, pkt.senderID, pkt.payload, PacketType::ACK, true);
    ack.destinationID = pkt.destinationID;
    //std::cout << "SEND ACK PKT " << ack.toString() << std::endl; 

    //lock.lock();
    Parser::Host peer =  idToPeer[pkt.senderID];
    //lock.unlock();
    send(&ack, peer);
}


void PerfectLink::listen(unsigned long localID){
    while(true){
        //std::cout << "Listening"<< std::endl;
            if(shouldStop()){
                return;
            }
            Packet pkt;
            if (recv(fd, &pkt, sizeof(Packet), 0) < 0) {
                            throw std::runtime_error("Could not read from the perfect link socket: " +std::string(std::strerror(errno)));
            }else if ((pkt.type == PacketType::FIFO) || (pkt.type == PacketType::ACK)){
                auto iter = std::find_if(delivered.begin(), delivered.end(), 
                            [&](const Packet& p){return p.equals(pkt);});
                bool deliveryReady = (delivered.size() == 0) || iter == delivered.end();

                //First time we receive the message. We store and handle it.
                if(deliveryReady){
                    delivered.push_back(pkt);
                    if(pkt.ack){
                        handleAck(pkt);
                    }else{
		                //std::cout << "received pkt " << pkt.peerID << "  from " << pkt.senderID << "  seq " << pkt.payload << std::endl;
                        if(!(pkt.peerID == localhost.id && pkt.senderID == localhost.id)){
                            handlePacket(pkt);
                        }
                        pp2pDeliver(pkt);
                    }
                }else{ 
                    //We already received the message. The previous reply probably got lost on the network.
                    //Hence we send the reply again.
                    if(pkt.ack){
                        handleAck(pkt);
                    }else{
                        handlePacket(pkt);
                    }
                }
            }

    }
    std::cout << "out"<< std::endl;
}
