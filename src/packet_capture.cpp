#include "packet_capture.hpp"
#include <sstream>
#ifdef AEGIS_HAS_PCAP
#ifdef _WIN32
#include <pcap.h>
#else
#include <pcap/pcap.h>
#endif
#endif
PacketCapture::PacketCapture(DetectionEngine&e):engine_(e){} PacketCapture::~PacketCapture(){stop();}
bool PacketCapture::available()const{
#ifdef AEGIS_HAS_PCAP
return true;
#else
return false;
#endif
}
std::string PacketCapture::devices()const{
#ifdef AEGIS_HAS_PCAP
pcap_if_t* all=nullptr;char err[PCAP_ERRBUF_SIZE]{};std::ostringstream o;if(pcap_findalldevs(&all,err)!=0)return "Unable to enumerate capture devices";for(auto*d=all;d;d=d->next)o<<d->name<<"  "<<(d->description?d->description:"(no description)")<<"\n";pcap_freealldevs(all);return o.str();
#else
return "Live capture unavailable: install Npcap (Windows) or libpcap (Linux), then rebuild.";
#endif
}
bool PacketCapture::start(const std::string& device){
#ifdef AEGIS_HAS_PCAP
if(running_)return false;running_=true;thread_=std::thread(&PacketCapture::run,this,device);return true;
#else
(void)device;return false;
#endif
}
void PacketCapture::stop(){running_=false;if(thread_.joinable())thread_.join();}
void PacketCapture::run(std::string device){
#ifdef AEGIS_HAS_PCAP
char err[PCAP_ERRBUF_SIZE]{};pcap_t* p=pcap_open_live(device.c_str(),65536,1,500,err);if(!p){engine_.addEvent("HIGH","Sensor","localhost","localhost","SYSTEM","Packet capture failed",err);running_=false;return;}struct pcap_pkthdr* h=nullptr;const u_char* data=nullptr;while(running_){int r=pcap_next_ex(p,&h,&data);if(r<=0)continue;if(h->caplen<34)continue;const unsigned char* b=data;int off=14;if((b[12]!=0x08)||(b[13]!=0x00))continue;int ihl=(b[off]&0x0f)*4;if(ihl<20||h->caplen<static_cast<unsigned>(off+ihl+4))continue;int proto=b[off+9];if(proto!=6&&proto!=17)continue;char src[32],dst[32];snprintf(src,sizeof(src),"%u.%u.%u.%u",b[off+12],b[off+13],b[off+14],b[off+15]);snprintf(dst,sizeof(dst),"%u.%u.%u.%u",b[off+16],b[off+17],b[off+18],b[off+19]);int t=off+ihl;int sp=(b[t]<<8)|b[t+1],dp=(b[t+2]<<8)|b[t+3];bool syn=proto==6&&(b[t+13]&0x02);engine_.ingestPacket(src,dst,sp,dp,proto==6?"TCP":"UDP",h->len,syn);}pcap_close(p);
#else
(void)device;
#endif
}
