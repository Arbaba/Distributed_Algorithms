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
    bool equals(const Packet p) const{
        return peerID == p.peerID 
            && payload == p.payload
            && senderID == p.senderID
            && type == p.type
            && ack == p.ack;
    }
};