#pragma once 
enum PacketType{
    ACK,
    FIFO,
    URB
};
struct Packet{
    Packet(){}
    Packet(unsigned long peerID, int payload, PacketType type, bool ack )
        :peerID(peerID), payload(payload), type(type), ack(ack){}
    unsigned long  peerID;
    int payload;
    PacketType type;
    bool ack;
    bool equals(const Packet p) const{
        return peerID == p.peerID 
            && payload == p.payload
            && type == p.type
            && ack == p.ack;
    }
};