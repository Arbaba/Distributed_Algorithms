#include "beb.hpp"
#include "parser.hpp"

BeBroadcast::BeBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers,  std::function<void(Packet)> bebDeliver,  std::function<void(unsigned long)> onCrash){
    this->localhost = localhost;
    this->peers = std::vector<Parser::Host>(peers);
    this->bebDeliver = bebDeliver;
    this->onCrash = onCrash;
    std::function<void(Packet)> callback = [this](Packet pkt){this->pp2pDeliver(pkt);};
    std::map<unsigned long, Parser::Host> idToPeer;
    for(auto &&peer: peers){
        idToPeer.insert({peer.id, peer});
    }
    perfectLink =  new PerfectLink(localhost, this->bebDeliver, this->onCrash, idToPeer);
}
void BeBroadcast::bebBroadcast(Packet pkt){
	//std::cout << "broadcast pkt " << pkt.peerID << "  from " << pkt.senderID << "  seq " << pkt.payload << std::endl;
    for(Parser::Host peer: peers){

        pkt.senderID = localhost.id;
        pkt.destinationID = peer.id;
        perfectLink->send(&pkt, peer);
    }
}

void BeBroadcast::pp2pDeliver(Packet pkt){
    bebDeliver(pkt);
}

