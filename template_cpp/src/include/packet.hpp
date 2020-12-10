#pragma once 
#include <iostream>
#include <sstream>
enum PacketType{
    ACK,
    FIFO,
    URB,
    PING
};



struct Packet{
    Packet(){}
    Packet(unsigned long peerID, unsigned long senderID, int payload, PacketType type, bool ack)
        :peerID(peerID), senderID(senderID), payload(payload), type(type), ack(ack){}
    /*Packet(Packet &&pkt )noexcept
        :peerID(pkt.peerID), senderID(pkt.senderID), payload(pkt.payload), type(pkt.type), ack(pkt.ack) {}
    Packet(const Packet& pkt ) noexcept
        :peerID(pkt.peerID), senderID(pkt.senderID), payload(pkt.payload), type(pkt.type), ack(pkt.ack){}
      Packet operator=(const Packet& b) {  
          Packet pkt(b);
          return pkt;
      }  */
    unsigned long   peerID;
    unsigned long senderID;
    int payload;
    PacketType type;
    bool ack;
    unsigned long destinationID;
    int vectorClock[128];
    bool equals(const Packet p) const{
        return peerID == p.peerID 
            && payload == p.payload
            && senderID == p.senderID
            && type == p.type
            && ack == p.ack
            && destinationID == p.destinationID;
    }

    std::string typeToString(PacketType type){
        std::ostringstream string;
        switch(type) {
            case PacketType::ACK:
                string << "ACK";
                break;
            case PacketType::FIFO:
                string << "FIFO";
                break;
            case PacketType::URB:
                string << "URB";
                break;
            case PacketType::PING:
                string << "PING";
            default:
                break;
        }
        return string.str();
    }

    std::string toString()  {
        std::ostringstream str;
        str << "Packet(peerID:" << this->peerID 
                                << " senderID: " << this->senderID
                                << " destinationID: " << this->destinationID
                                << " payload: " << this->payload
                                << " type: " << typeToString(this->type)
                                << " ack: " << this->ack
                                << " )";
        return str.str();
    }
};