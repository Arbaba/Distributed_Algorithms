#include <chrono>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "parser.hpp"
#include "packet.hpp"
#include "perfectlink.hpp"
#include "beb.hpp"
#include "urb.hpp"
#include "fifob.hpp"
#include "fifoController.hpp"
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
FIFOController* fifoController = NULL;
std::ofstream outputFile;
std::mutex receivedMutex;

static void signalHandler(int signum) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

   std::cout << "Interrupt signal (" << signum << ") received.\n";

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";
  if(fifoController != NULL){
    fifoController->stop();
  }
  // write/flush output file if necessary
  std::cout << "Writing output.\n";
   // cleanup and close up stuff here  
   // terminate program  
  for(auto && pkt: broadcasts){
    outputFile << "b " << pkt.payload << std::endl;
  }
  receivedMutex.lock();
  for(auto && pkt: receivedPackets){
    outputFile << "d " << pkt.peerID << " " << pkt.payload << std::endl;
  }
  receivedMutex.unlock();
  outputFile.close();
  exit(0);  
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

int main(int argc, char **argv) {
  signal(SIGTERM, signalHandler);
  signal(SIGINT, signalHandler);
  
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
  unsigned long nMessages = parser.parseNMessages();
  outputFile.open(parser.outputPath());
  Parser::Host localhost = parser.getLocalhost();
  struct sockaddr_in server;
  std::memset(&server, 0, sizeof(server));
  enum BROADCASTTYPE{
    PERFECTLINKTYPE, BEBTYPE, URBTYPE, FIFOTYPE
  };
  BROADCASTTYPE btype = BROADCASTTYPE::FIFOTYPE;
  Coordinator coordinator(parser.id(), barrier, signal);
  fifoController = new FIFOController(localhost,
                                                 parser.getPeers(),
                                                 [localhost](Packet p){ if(p.payload % 10 ==0) std::cerr << "Received " << p.payload << "from process" << p.peerID << ";" << std::endl; receivePacket(p, localhost.id);},
                                                 [localhost](Packet p){broadcasts.push_back(p);},
                                                 &coordinator
                                                 );

  std::cout << "Waiting for all processes to finish initialization\n\n";
  coordinator.waitOnBarrier();
  //Finished broadcasting will be called by the fifoController once all packets have been broadcasted.
  std::cout << "Broadcasting messages...\n\n";
  switch (btype){
    /*
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
*/
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
        std::cout << "Signaling end of broadcasting messages\n\n";
        coordinator.finishedBroadcasting();
        std::cout << "Broadcasting done" << std::endl;
      }
      break;
      
    case FIFOTYPE:
      {

        std::cout << "Broadcasting fifo" << std::endl;
            fifoController->broadcast(nMessages);    
      }
      break;
    default:
      break;
  }

  while(true){
  	std::this_thread::sleep_for(std::chrono::seconds(60));	  
  }

  return 0;
}
