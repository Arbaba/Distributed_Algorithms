#include "fifoController.hpp"
#include "packet.hpp"
FIFOController::FIFOController(Parser::Host localhost, std::vector<Parser::Host> peers, std::function<void(Packet)> fifoDeliver,  std::function<void(Packet)> broadcastCB){
    this->fifoDeliver = fifoDeliver;
     std::function<void(Packet)> fifoCB = [this](Packet pkt){this->deliver(pkt);};
    this->localhost = localhost;
    this->fifo = new FIFOBroadcast(localhost, peers, fifoCB);
    this->broadcastCB = broadcastCB;
    nLocalDeliveries = 1;
    nPacketsPerBroadcast = 1;
}
void FIFOController::broadcast(unsigned long n){
    maxPackets = n;
    counter = 1;
    groupedBroadcast(nPacketsPerBroadcast);
}

void FIFOController::groupedBroadcast(unsigned long n){
    for(unsigned long i = counter; i <= maxPackets && i < counter + n; i++){
        std::cout << "i " << i << " counter + n" << counter + n << std::endl;
        //counter += 1;
        Packet pkt(localhost.id, localhost.id,  static_cast<int>(i), PacketType::FIFO, false);
        std::cout << "Send" << pkt.toString() << std::endl;

        broadcastCB(pkt);
        fifo->broadcast(pkt);
    }
    counter += n ;
}
void FIFOController::deliver(Packet pkt){
    std::cout << "Deliver" << pkt.toString() << std::endl;
    fifoDeliver(pkt);
    if(pkt.peerID == localhost.id){
        nLocalDeliveries += 1;
        std::cout << "Counter " << counter << " nLocalDeliveries " << nLocalDeliveries << std::endl;
        /*if(counter <= maxPackets){
            Packet next(localhost.id, localhost.id, static_cast<int>(counter), PacketType::FIFO, false);
            broadcastCB(next);
            fifo->broadcast(next);
        }*/
        if(nLocalDeliveries == counter){
            groupedBroadcast(nPacketsPerBroadcast);
        }
    }
    
}