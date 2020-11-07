#include <chrono>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "parser.hpp"
#include "perfectlink.hpp"
#include "beb.hpp"
#include "urb.hpp"
#include "fifob.hpp"
#include "hello.h"
#include <signal.h>
//added includes for perfect links
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>

std::vector<Packet> broadcasts;
std::vector<Packet> receivedPackets;
unsigned long nlocalDeliveries = 0;

std::ofstream outputFile;
std::mutex receivedMutex;

static void signalHandler( int signum ) {
   std::cout << "Interrupt signal (" << signum << ") received.\n";

   // cleanup and close up stuff here  
   // terminate program  
  for(auto && pkt: broadcasts){
    outputFile << "b " << pkt.payload << std::endl;
  }
  receivedMutex.lock();
  for(auto && pkt: receivedPackets){
    outputFile << "d " << pkt.senderID << " " << pkt.payload << std::endl;
  }
  receivedMutex.unlock();
  outputFile.close();
  exit(signum);  
}

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

  // exit directly from signal handler
  exit(0);
}


static void receivePacket(Packet p, unsigned long localID){
  receivedMutex.lock();
  receivedPackets.push_back(p);
  if(p.peerID == localID){
    nlocalDeliveries++;
  }
  receivedMutex.unlock();
}
static void waitPackets(PerfectLink link){
  
  Packet dummy;
  std::cout << "Waiting for packets" << std::endl;

    
 


}
int main(int argc, char **argv) {
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
  std::vector<Packet> dummy;
  for(unsigned long i = 0; i < 20; i++){
    Packet pkt(i, i, 1, PacketType::FIFO, true);
    std::cout <<  dummy.capacity() << std::endl;
  dummy.insert(dummy.begin() , pkt);

  }
  for(auto x: dummy){
        //std::cerr <<x.peerID << " "<< x.  << std::endl;  

  }
  for(size_t i = 0; i < 20; i++){
  
    std::cerr << i << " " << dummy.at(i).peerID << " " <<dummy.at(i).senderID   << std::endl;  
  }
 // return 0;
  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;
  std::cout << "parse" << std::endl;
  Parser parser(argc, argv, requireConfig);
  std::cout << "parse done" << std::endl;

  hello();
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "Use `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

  std::cout << "Path to hosts:\n";
  std::cout << "==============\n";
  std::cout << parser.hostsPath() << "\n\n";

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  auto hosts = parser.hosts();
  for (auto &host : hosts) {
    std::cout << host.id << "\n";
    std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
    std::cout << "Machine-readable IP: " << host.ip << "\n";
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
    std::cout << "\n";
  }
  std::cout << "\n";

  std::cout << "Barrier:\n";
  std::cout << "========\n";
  auto barrier = parser.barrier();
  std::cout << "Human-readable IP: " << barrier.ipReadable() << "\n";
  std::cout << "Machine-readable IP: " << barrier.ip << "\n";
  std::cout << "Human-readbale Port: " << barrier.portReadable() << "\n";
  std::cout << "Machine-readbale Port: " << barrier.port << "\n";
  std::cout << "\n";

  std::cout << "Signal:\n";
  std::cout << "========\n";
  auto signal = parser.signal();
  std::cout << "Human-readable IP: " << signal.ipReadable() << "\n";
  std::cout << "Machine-readable IP: " << signal.ip << "\n";
  std::cout << "Human-readbale Port: " << signal.portReadable() << "\n";
  std::cout << "Machine-readbale Port: " << signal.port << "\n";
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";

  if (requireConfig) {
    std::cout << "Path to config:\n";
    std::cout << "===============\n";
    std::cout << parser.configPath() << "\n\n";
  }


  std::cout << "Doing some initialization...\n\n";

  Coordinator coordinator(parser.id(), barrier, signal);

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();
  //coordinator.finishedBroadcasting();

  std::cout << "Broadcasting messages...\n\n";


  unsigned long nMessages = parser.parseNMessages();
  outputFile.open(parser.outputPath());
  Parser::Host localhost = parser.getLocalhost();
  struct sockaddr_in server;
  std::memset(&server, 0, sizeof(server));
  enum BROADCASTTYPE{
    PERFECTLINKTYPE, BEBTYPE, URBTYPE, FIFOTYPE
  };
  BROADCASTTYPE btype = BROADCASTTYPE::FIFOTYPE;
  
  switch (btype){
    case PERFECTLINKTYPE:
      {
        PerfectLink perfectLink(localhost.ip, localhost.port, [](Packet p){ std::cout << "Received " << p.payload << "from process" << p.peerID << std::endl; });
          while(true){
            for(Parser::Host peer: parser.getPeers()){
              Packet pkt(peer.id, peer.id, 380, PacketType::FIFO, true);
              perfectLink.send(&pkt, peer);
            }
          } 
        break;
      }
    
    case BEBTYPE:
      {
        BeBroadcast bebroadcast = BeBroadcast(localhost, parser.getPeers(), [](Packet p){ std::cout << "Received " << p.payload << "from process" << p.peerID << std::endl; });
        std::cout << "Broadcasting done" << std::endl;
        while(true){

              for(Parser::Host peer: parser.getPeers()){
                Packet pkt(peer.id, peer.id, 380, PacketType::FIFO, true);
                bebroadcast.bebBroadcast(pkt);
              }
       
        }
          std::cout << "Broadcasting done" << std::endl;
      }
      break;

    case URBTYPE:
      {
        UniformBroadcast urb = UniformBroadcast(localhost, parser.getPeers(), [](Packet p){ std::cout << "Received " << p.payload << "from process " << p.peerID <<";"<< std::endl; });
        std::cout << "Broadcasting done" << std::endl;
        for(int j = 0; j < 20; j++){
            for(int i = 0; i < 20; i++){          
              for(Parser::Host peer: parser.getPeers()){
                Packet pkt(localhost.id, localhost.id, i, PacketType::FIFO, true);
                urb.broadcast(pkt);
              }
            }
        }
        std::cout << "Broadcasting done" << std::endl;
      }
      break;
      
    case FIFOTYPE:
      {
        FIFOBroadcast fifo = FIFOBroadcast(localhost, parser.getPeers(), [localhost](Packet p){ std::cout << "Received " << p.payload << "from process" << p.peerID << ";" << std::endl; receivePacket(p, localhost.id);});
        std::cout << "Broadcasting fifo" << std::endl;
            for(int i = 1; static_cast<unsigned long>(i) <= nMessages; i++){          
                Packet pkt(localhost.id, localhost.id, i, PacketType::FIFO, true);
                fifo.broadcast(pkt);
                broadcasts.push_back(pkt);
            }        
      }
      break;
    default:
      break;
  }
  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();
  while(true){
    receivedMutex.lock();
    bool shouldFlush = nlocalDeliveries == nMessages;
    receivedMutex.unlock();
    if(shouldFlush){
        usleep(30000);
        signalHandler(0);
    }
    usleep(2000);
  }
  //usleep(3000000);
  //signalHandler(0);
  return 0;
}
