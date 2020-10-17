#pragma once 
enum PacketType{
    ACK,
    FIFO,
    URB
};
struct Packet{
    Packet(){}
    Packet(unsigned long peerID, unsigned long senderID, int payload, PacketType type, bool ack)
        :peerID(peerID), senderID(senderID), payload(payload), type(type), ack(ack){}
    unsigned long  peerID;
    unsigned long senderID;
    int payload;
    PacketType type;
    bool ack;
    bool equals(const Packet p) const{
        return peerID == p.peerID 
            && payload == p.payload
            && senderID == p.senderID
            && type == p.type
            && ack == p.ack;
    }
};