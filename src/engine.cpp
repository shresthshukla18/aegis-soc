#include "engine.hpp"
#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <random>
#include <sstream>

namespace {
std::string nowText() { auto t=std::time(nullptr); std::tm tm{};
#ifdef _WIN32
 localtime_s(&tm,&t);
#else
 localtime_r(&t,&tm);
#endif
 std::ostringstream o; o<<std::put_time(&tm,"%H:%M:%S"); return o.str(); }
std::string esc(const std::string& s) { std::string o; for(char c:s) { switch(c) { case '"':o+="\\\"";break; case '\\':o+="\\\\";break; case '\n':o+="\\n";break; case '\r':o+="\\r";break; case '\t':o+="\\t";break; default: if(static_cast<unsigned char>(c)>=32)o+=c; } } return o; }
}
DetectionEngine::DetectionEngine() {
 rules_={{"AEG-001","SSH exposure","Service Exposure","MEDIUM",22,"TCP",true},
 {"AEG-002","RDP exposure","Service Exposure","HIGH",3389,"TCP",true},
 {"AEG-003","SMB exposure","Service Exposure","HIGH",445,"TCP",true},
 {"AEG-004","Telnet exposure","Weak Service","HIGH",23,"TCP",true},
 {"AEG-005","Database exposure","Service Exposure","MEDIUM",3306,"TCP",true},
 {"AEG-006","DNS traffic","Protocol","LOW",53,"UDP",false}};
 traffic_.push_back({nowText(),0,0,0});
}
DetectionEngine::~DetectionEngine(){ stop(); }
void DetectionEngine::startSimulation(){ if(running_.exchange(true))return; mode_="SIMULATION"; simulationThread_=std::thread(&DetectionEngine::simulationLoop,this); }
void DetectionEngine::stop(){ running_=false; if(simulationThread_.joinable())simulationThread_.join(); }
void DetectionEngine::setMode(const std::string& m){ std::lock_guard l(mutex_); mode_=m; }
std::string DetectionEngine::mode() const { std::lock_guard l(mutex_); return mode_; }
void DetectionEngine::trimLocked(){ while(events_.size()>500)events_.pop_back(); while(traffic_.size()>60)traffic_.pop_front(); }
void DetectionEngine::addEvent(std::string sev,std::string cat,std::string src,std::string dst,std::string proto,std::string title,std::string details,int sp,int dp){
 std::lock_guard l(mutex_); events_.push_front({nextId_++,nowText(),std::move(sev),std::move(cat),std::move(src),std::move(dst),std::move(proto),std::move(title),std::move(details),"Open",sp,dp}); ++totalAlerts_; if(events_.front().severity=="CRITICAL")++blocked_; if(!traffic_.empty())++traffic_.back().alerts; trimLocked();
}
void DetectionEngine::ingestPacket(const std::string& src,const std::string& dst,int sp,int dp,const std::string& proto,std::size_t bytes,bool syn){
 std::vector<Rule> rules; bool scan=false,flood=false;
 { std::lock_guard l(mutex_); ++totalPackets_; totalBytes_+=bytes; if(traffic_.empty()||std::chrono::steady_clock::now()-windowStart_>=std::chrono::seconds(1)){ traffic_.push_back({nowText(),0,0,0}); windowStart_=std::chrono::steady_clock::now(); }
 auto& p=traffic_.back(); ++p.packets;p.bytes+=bytes; portsBySource_[src].insert(dp); if(portsBySource_[src].size()>=10){scan=true;portsBySource_[src].clear();} if(syn&&++synBySource_[src]>=35){flood=true;synBySource_[src]=0;} rules=rules_; trimLocked(); }
 if(scan)addEvent("HIGH","Reconnaissance",src,dst,proto,"Possible port scan detected","Source contacted 10 or more distinct destination ports in the observation window",sp,dp);
 if(flood)addEvent("CRITICAL","DoS / Flood",src,dst,proto,"SYN flood threshold exceeded","At least 35 SYN packets observed from this source in the current counter window",sp,dp);
 for(const auto& r:rules) if(r.enabled&&r.destinationPort==dp&&(r.protocol==proto||r.protocol=="ANY")) addEvent(r.severity,r.category,src,dst,proto,r.name,"Traffic matched rule "+r.id,sp,dp);
}
void DetectionEngine::simulationLoop(){ std::mt19937 rng(std::random_device{}()); std::uniform_int_distribution<int> port(1024,65000),oct(2,250),pick(0,99),size(60,1400); int tick=0;
 while(running_){ std::string src="192.168.1."+std::to_string(oct(rng)),dst="10.0.0."+std::to_string(oct(rng)); int p=pick(rng); if(p<5){src="203.0.113."+std::to_string(oct(rng));dst="192.168.1.10";int d[]={22,23,445,3389,3306};int dp=d[p%5];ingestPacket(src,dst,port(rng),dp,"TCP",size(rng),true);} else if(p<8){src="198.51.100.77";dst="192.168.1.10";ingestPacket(src,dst,port(rng),port(rng),"TCP",size(rng),true);} else ingestPacket(src,dst,port(rng),p%2?443:53,p%2?"TCP":"UDP",size(rng),p%2);
 if(++tick%45==0)addEvent("INFO","System","127.0.0.1","localhost","SYSTEM","Sensor heartbeat","Monitoring pipeline is operational");
 std::this_thread::sleep_for(std::chrono::milliseconds(120)); }
}
std::string DetectionEngine::statusJson() const { std::lock_guard l(mutex_); std::ostringstream o; o<<"{\"version\":\""<<AEGIS_VERSION<<"\",\"mode\":\""<<esc(mode_)<<"\",\"running\":"<<(running_?"true":"false")<<",\"packets\":"<<totalPackets_<<",\"bytes\":"<<totalBytes_<<",\"alerts\":"<<totalAlerts_<<",\"critical\":"<<std::count_if(events_.begin(),events_.end(),[](auto&e){return e.severity=="CRITICAL";})<<",\"open\":"<<std::count_if(events_.begin(),events_.end(),[](auto&e){return e.status=="Open";})<<",\"sensors\":1,\"uptime\":\"ACTIVE\"}";return o.str(); }
std::string DetectionEngine::eventsJson() const { std::lock_guard l(mutex_);std::ostringstream o;o<<"[";bool first=true;for(auto&e:events_){if(!first)o<<",";first=false;o<<"{\"id\":"<<e.id<<",\"time\":\""<<esc(e.time)<<"\",\"severity\":\""<<esc(e.severity)<<"\",\"category\":\""<<esc(e.category)<<"\",\"source\":\""<<esc(e.source)<<"\",\"destination\":\""<<esc(e.destination)<<"\",\"protocol\":\""<<esc(e.protocol)<<"\",\"title\":\""<<esc(e.title)<<"\",\"details\":\""<<esc(e.details)<<"\",\"status\":\""<<esc(e.status)<<"\",\"srcPort\":"<<e.sourcePort<<",\"dstPort\":"<<e.destinationPort<<"}";}o<<"]";return o.str(); }
std::string DetectionEngine::trafficJson() const { std::lock_guard l(mutex_);std::ostringstream o;o<<"[";bool f=true;for(auto&p:traffic_){if(!f)o<<",";f=false;o<<"{\"time\":\""<<p.time<<"\",\"packets\":"<<p.packets<<",\"bytes\":"<<p.bytes<<",\"alerts\":"<<p.alerts<<"}";}o<<"]";return o.str(); }
std::string DetectionEngine::rulesJson() const { std::lock_guard l(mutex_);std::ostringstream o;o<<"[";bool f=true;for(auto&r:rules_){if(!f)o<<",";f=false;o<<"{\"id\":\""<<esc(r.id)<<"\",\"name\":\""<<esc(r.name)<<"\",\"category\":\""<<esc(r.category)<<"\",\"severity\":\""<<r.severity<<"\",\"port\":"<<r.destinationPort<<",\"protocol\":\""<<r.protocol<<"\",\"enabled\":"<<(r.enabled?"true":"false")<<"}";}o<<"]";return o.str(); }
bool DetectionEngine::updateEvent(std::uint64_t id,const std::string& status){if(status!="Open"&&status!="Investigating"&&status!="Resolved")return false;std::lock_guard l(mutex_);for(auto&e:events_)if(e.id==id){e.status=status;return true;}return false;}
bool DetectionEngine::setRuleEnabled(const std::string&id,bool enabled){std::lock_guard l(mutex_);for(auto&r:rules_)if(r.id==id){r.enabled=enabled;return true;}return false;}
bool DetectionEngine::loadRules(const std::string& path){std::ifstream in(path);if(!in)return false;std::string line;while(std::getline(in,line)){if(line.empty()||line[0]=='#')continue;std::stringstream s(line);Rule r;std::string port,enabled;std::getline(s,r.id,'|');std::getline(s,r.name,'|');std::getline(s,r.category,'|');std::getline(s,r.severity,'|');std::getline(s,port,'|');std::getline(s,r.protocol,'|');std::getline(s,enabled,'|');try{r.destinationPort=std::stoi(port);}catch(...){continue;}r.enabled=enabled!="false";if(!r.id.empty()){std::lock_guard l(mutex_);auto it=std::find_if(rules_.begin(),rules_.end(),[&](auto&x){return x.id==r.id;});if(it==rules_.end())rules_.push_back(r);else *it=r;}}return true;}
