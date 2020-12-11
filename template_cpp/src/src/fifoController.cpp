#include "fifoController.hpp"
#include "packet.hpp"
FIFOController::FIFOController(Parser::Host localhost,
                                std::vector<Parser::Host> peers, 
                                std::function<void(Packet)> fifoDeliver,
                                std::function<void(Packet)> broadcastCB,
                                Coordinator* coordinator){
    this->fifoDeliver = fifoDeliver;
    std::function<void(Packet)> fifoCB = [this](Packet pkt){this->deliver(pkt);};
    this->localhost = localhost;
    this->fifo = new FIFOBroadcast(localhost, peers, fifoCB);
    this->broadcastCB = broadcastCB;
    nLocalDeliveries = 1;
    nPacketsPerBroadcast =10;
    this->coordinator = coordinator;
}
void FIFOController::broadcast(unsigned long n){
    maxPackets = n;
    counter = 1;
    groupedBroadcast(nPacketsPerBroadcast);
}

void FIFOController::groupedBroadcast(unsigned long n){
    for(unsigned long i = counter; i <= maxPackets && i < counter + n; i++){
        mtx.lock();
        int iCasted = static_cast<int>(i);
        Packet pkt(localhost.id, localhost.id,  iCasted, PacketType::FIFO, false);
        if(this->vectorClock.size() > 0){
            for(auto pid: this->localhost.dependencies){
                pkt.vectorClock[pid - 1] =  vectorClock[pid - 1];
            }
        }
        broadcastCB(pkt);
        mtx.unlock();
        fifo->broadcast(pkt);
    }
    counter += n ;
}
void FIFOController::deliver(Packet pkt){
    mtx.lock();
    fifoDeliver(pkt);
    mtx.unlock();
    if(pkt.peerID == localhost.id){
        nLocalDeliveries += 1;
        if(nLocalDeliveries == (maxPackets + 1)){
            std::cout << "Signaling end of broadcasting messages\n\n";
            coordinator->finishedBroadcasting();
        }else if(nLocalDeliveries == counter){
            groupedBroadcast(nPacketsPerBroadcast);
        }
    }
    
}

void FIFOController::stop(){
    fifo->stop();
}