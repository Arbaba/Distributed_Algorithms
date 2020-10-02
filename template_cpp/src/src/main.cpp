#include <chrono>
#include <iostream>
#include <thread>

#include "barrier.hpp"
#include "parser.hpp"
#include "perfectlink.hpp"

#include "hello.h"
#include <signal.h>
//added includes for perfect links
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
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


static void waitPackets(PerfectLink link){
  
  char dummy;
  std::cout << "Waiting for packets" << std::endl;
  while(link.deliver(&dummy)){
    
      std::cout << "Received : " << dummy << std::endl;
    
  }
 


}
int main(int argc, char **argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

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

  std::cout << "Broadcasting messages...\n\n";

  std::cout << "Signaling end of broadcasting messages\n\n";
  coordinator.finishedBroadcasting();


  Parser::Host localhost = parser.getLocalhost();
  struct sockaddr_in server;
  std::memset(&server, 0, sizeof(server));

  PerfectLink perfectLink(localhost.ip, localhost.port);
  std::thread t1(waitPackets, perfectLink);

  while(true){
    for(Parser::Host peer: parser.getPeers()){
      perfectLink.send("h", peer);
    }
  }
  /*int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0) {
    throw std::runtime_error("Could not create the UDP socket: " +
                             std::string(std::strerror(errno)));
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = localhost.ip;
  server.sin_port = localhost.port;
  int bindAttempt = bind(fd, reinterpret_cast<sockaddr*>(&server), sizeof(server));
  if(bindAttempt != 0){
    stop(bindAttempt);
  }
  std::thread t1(waitPackets, fd);
  usleep(1000000);
  std::cout << "setup" << std::endl;
  while(true){
    sockaddr_in destSocket;
    std::memset(&destSocket, 0, sizeof(destSocket));

    for(Parser::Host peer: parser.getPeers()){
      destSocket.sin_family = AF_INET;
      destSocket.sin_addr.s_addr = peer.ip;
      destSocket.sin_port = peer.port;
      if(sendto(fd, "h",1, 0,reinterpret_cast<const sockaddr*>(&destSocket), sizeof(destSocket))){
        std::cout << "Sent message" << std::endl;
      }else{
        std::cout << "Couldn't send message" << std::endl;

        stop(0);

      }
    }
  }*/
  return 0;
}
