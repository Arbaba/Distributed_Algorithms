#include "beb.hpp"
#include "parser.hpp"

BeBroadcast::BeBroadcast(Parser::Host localhost, std::vector<Parser::Host> peers,  std::function<void(Packet)> bebDeliver){
    this->localhost = localhost;
    this->peers = std::vector<Parser::Host>(peers);
    this->bebDeliver = bebDeliver;
    std::function<void(Packet)> callback = [this](Packet pkt){this->pp2pDeliver(pkt);};
    perfectLink =  new PerfectLink(localhost.ip, localhost.port, callback);
}


void BeBroadcast::bebBroadcast(Packet pkt){
	//std::cout << "broadcast pkt " << pkt.peerID << "  from " << pkt.senderID << "  seq " << pkt.payload << std::endl;
    for(Parser::Host peer: peers){
        pkt.senderID = localhost.id;
        perfectLink->send(&pkt, peer);
    }
}

void BeBroadcast::pp2pDeliver(Packet pkt){
    bebDeliver(pkt);
}

