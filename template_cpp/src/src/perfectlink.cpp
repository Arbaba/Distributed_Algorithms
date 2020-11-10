#include "perfectlink.hpp"
#include "packet.hpp"
#include <thread>
#include <set>
PerfectLink::PerfectLink(Parser::Host localhost,  std::function<void(Packet)> pp2pDeliver, std::function<void(unsigned long)> onCrash, std::map<unsigned long, Parser::Host> idToPeer){
    std::cout << "Create perfect link " << std::endl;
    this->pp2pDeliver = pp2pDeliver; 
    this->onCrash = onCrash;
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
        countPerProcess.insert({id, 0});
    }
    std::thread t1(&PerfectLink::listen, this, localhost.id);
    //std::thread t2(&PerfectLink::resendMessages, this, localhost.id);
    std::thread t3(&PerfectLink::pingPeers, this);

    t1.detach();
    //t2.detach();
    t3.detach();
}

void PerfectLink::crashed(unsigned long processID){
    //Clean up expected acks
    lock.lock();
    

    lock.unlock();
    onCrash(processID);
}
void PerfectLink::pingPeers(){
    while(correctProcesses.size() > 0){
	    std::this_thread::sleep_for(std::chrono::seconds(1));
        
        for(auto &&id: correctProcesses){
            lock.lock();
            int count = countPerProcess[id] + 1;
            countPerProcess[id] = count;
            lock.unlock();
            if(count >= 10){
                std::cout << "Oh no! process " << id << " crashed! My life is ruined!" << std::endl;
                onCrash(id);
            }else{
                Packet ping(localhost.id, localhost.id, 0, PacketType::PING, false);
                ping.destinationID = id;
                std::cout << "Ping " << id << std::endl; 
                send(&ping, idToPeer[id]);
            }
        }
    }
}
void PerfectLink::resendMessages(unsigned long localhostID){
    while(true){
	    std::this_thread::sleep_for(std::chrono::seconds(1));
        lock.lock();
        std::map<std::string, Packet> toResend = waitingAcks;
        lock.unlock();
        for(auto& [key, pkt]: toResend){
            //std::cout << "Resend " << pkt.toString() << std::endl;
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
        if(!msg->ack && !(msg->peerID == localhost.id && msg->senderID == localhost.id && msg->destinationID == localhost.id) ){
            lock.lock();
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
            }else if ((pkt.type == PacketType::FIFO) || (pkt.type == PacketType::ACK)){
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
			//std::cout << "Received ack for packet: " << pkt.toString() << std::endl;
                        //std::cout << "Ack size " << waitingAcks.size() << std::endl;
                        std::map<std::string, Packet>::iterator rmIt =  waitingAcks.find(key);
                        
                        if(rmIt !=waitingAcks.end()){
                            waitingAcks.erase(rmIt);
                        }
                       //std::cout << "Ack size " << waitingAcks.size() << std::endl;
                        lock.unlock();
                    }else{
                        if(!(pkt.peerID == localhost.id && pkt.senderID == localhost.id)){
                            //We send the ack packet to the source by only changing the packet type and ack boolean
                            Packet ack(pkt.peerID, pkt.senderID, pkt.payload, PacketType::ACK, true);
                            ack.destinationID = pkt.destinationID;
                            lock.lock();
                            Parser::Host peer =  idToPeer[pkt.senderID];
                            lock.unlock();
                            send(&ack, peer);
                        }
                        pp2pDeliver(pkt);
                    }
                }
            }else if(pkt.type == PacketType::PING){
                std::cout << "Received ping from" << pkt.senderID << std::endl; 

                if(pkt.ack){
                    lock.lock();
                    countPerProcess[pkt.destinationID] = 0;
                    lock.unlock();
                    std::cout << "Received ping reply from" << pkt.destinationID << std::endl; 
                }else{
                    Packet pingReply(pkt.peerID, pkt.senderID, 0, PacketType::PING, true);
                    pingReply.destinationID = localhost.id;
                    std::cout << "Send ping reply to" << pkt.senderID << std::endl; 

                    send(&pingReply, idToPeer[pkt.senderID]);
                }
            }
        }
        deliveryReady = false;
    }
    std::cout << "out"<< std::endl;
}
