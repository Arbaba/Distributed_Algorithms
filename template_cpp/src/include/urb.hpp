#include "perfectlink.hpp"
#include <set>
class UniformBroadcast {
    public:

        UniformBroadcast(in_addr_t ip, in_port_t port);

        void crash(int processID);

        int broadcast(const Packet *msg);

        int bebDeliver(Packet *pck);

        bool receivedAllAcks(int originID, int messageID);
        bool tryDelivery(int originID, int messageID);

        int deliver(Packet *pck);
    private:
        std::set<int> correctProcesses;
        //origin -> senderid -> sendr
};