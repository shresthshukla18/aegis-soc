#include "engine.hpp"
#include "http_server.hpp"
#include "packet_capture.hpp"
#include <atomic>
#include <chrono>
#include <csignal>
#include <iostream>
#include <thread>
std::atomic<bool> quit{false}; void onSignal(int){quit=true;}
int main(int argc,char**argv){int port=8787;std::string liveDevice;bool simulation=true;for(int i=1;i<argc;++i){std::string a=argv[i];if(a=="--help"){std::cout<<"Aegis SOC v"<<AEGIS_VERSION<<"\n  --port N       Dashboard port (default 8787)\n  --live DEVICE  Capture packets from a PCAP/Npcap interface\n  --no-sim       Do not start the simulator\n  --help         Show help\n";return 0;}else if(a=="--port"&&i+1<argc)port=std::stoi(argv[++i]);else if(a=="--live"&&i+1<argc){liveDevice=argv[++i];simulation=false;}else if(a=="--no-sim")simulation=false;}
 DetectionEngine engine;engine.loadRules(std::string(AEGIS_CONFIG_DIR)+"/rules.conf");PacketCapture capture(engine);if(!liveDevice.empty()){if(!capture.available()){std::cerr<<"PCAP support is not compiled in. Install Npcap/libpcap and rebuild.\n";return 2;}engine.setMode("LIVE / PCAP");if(!capture.start(liveDevice)){std::cerr<<"Could not start packet capture. Check interface name and administrator/root privileges.\n";return 3;}}
 else if(simulation)engine.startSimulation();else engine.setMode("IDLE");
 HttpServer server(engine,port);if(!server.start()){std::cerr<<"Could not bind dashboard port "<<port<<". Is it already in use?\n";capture.stop();engine.stop();return 4;}
 std::cout<<"\n  AEGIS SOC // NETWORK DEFENCE CONSOLE\n  Dashboard: http://127.0.0.1:"<<port<<"\n  Mode: "<<engine.mode()<<"\n  Press Ctrl+C to stop.\n\n";std::signal(SIGINT,onSignal);std::signal(SIGTERM,onSignal);while(!quit)std::this_thread::sleep_for(std::chrono::milliseconds(250));server.stop();capture.stop();engine.stop();std::cout<<"Aegis SOC stopped cleanly.\n";return 0;}
