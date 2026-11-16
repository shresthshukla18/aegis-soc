#include "http_server.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socklen_type=int;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using SOCKET=int; constexpr int INVALID_SOCKET=-1; constexpr int SOCKET_ERROR=-1; using socklen_type=socklen_t;
#endif
namespace {
void closeSock(SOCKET s){if(s==INVALID_SOCKET)return;
#ifdef _WIN32
closesocket(s);
#else
close(s);
#endif
}
std::string readFile(const std::string&p){std::ifstream f(p,std::ios::binary);if(!f)return "";std::ostringstream s;s<<f.rdbuf();return s.str();}
std::string val(const std::string& body,const std::string& key){auto p=body.find(key+"=");if(p==std::string::npos)return "";p+=key.size()+1;auto e=body.find('&',p);return body.substr(p,e==std::string::npos?e:e-p);}
std::string decode(std::string s){for(size_t p=0;(p=s.find('%',p))!=std::string::npos;){if(p+2<s.size()){try{s.replace(p,3,1,static_cast<char>(std::stoi(s.substr(p+1,2),nullptr,16)));}catch(...){++p;}}}return s;}
void reply(SOCKET c,const std::string& body,const std::string& type="application/json",int code=200){
 std::string status=code==200?"200 OK":code==404?"404 Not Found":"400 Bad Request";
 std::string h="HTTP/1.1 "+status+"\r\nContent-Type: "+type+"; charset=utf-8\r\nContent-Length: "+std::to_string(body.size())+"\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nCache-Control: no-store\r\n\r\n";
 send(c,h.c_str(),static_cast<int>(h.size()),0);send(c,body.c_str(),static_cast<int>(body.size()),0);
}
}
HttpServer::HttpServer(DetectionEngine&e,int p):engine_(e),port_(p){} HttpServer::~HttpServer(){stop();}
bool HttpServer::start(){if(running_)return true;
#ifdef _WIN32
WSADATA w; if(WSAStartup(MAKEWORD(2,2),&w)!=0)return false;
#endif
serverSocket_=static_cast<std::intptr_t>(socket(AF_INET,SOCK_STREAM,0));if(serverSocket_==INVALID_SOCKET)return false;int yes=1;setsockopt(serverSocket_,SOL_SOCKET,SO_REUSEADDR,reinterpret_cast<const char*>(&yes),sizeof(yes));sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons(static_cast<unsigned short>(port_));if(bind(serverSocket_,reinterpret_cast<sockaddr*>(&a),sizeof(a))<0||listen(serverSocket_,16)<0){closeSock(serverSocket_);serverSocket_=-1;return false;}running_=true;thread_=std::thread(&HttpServer::run,this);return true;}
void HttpServer::stop(){running_=false;if(serverSocket_!=-1){
#ifdef _WIN32
shutdown(static_cast<SOCKET>(serverSocket_),SD_BOTH);
#else
shutdown(static_cast<SOCKET>(serverSocket_),SHUT_RDWR);
#endif
closeSock(static_cast<SOCKET>(serverSocket_));serverSocket_=-1;}if(thread_.joinable())thread_.join();
#ifdef _WIN32
WSACleanup();
#endif
}
void HttpServer::run(){while(running_){sockaddr_in a{};socklen_type n=sizeof(a);SOCKET c=accept(serverSocket_,reinterpret_cast<sockaddr*>(&a),&n);if(c==INVALID_SOCKET){if(!running_)break;continue;}handleClient(c);closeSock(c);}}
void HttpServer::handleClient(std::intptr_t raw){SOCKET c=static_cast<SOCKET>(raw);char buf[16384];int n=recv(c,buf,sizeof(buf)-1,0);if(n<=0)return;buf[n]='\0';std::string req(buf,n);auto lineEnd=req.find("\r\n");std::string first=req.substr(0,lineEnd);std::istringstream ls(first);std::string method,path,ver;ls>>method>>path>>ver;auto q=path.find('?');if(q!=std::string::npos)path.resize(q);auto hdr=req.find("\r\n\r\n");std::string body=hdr==std::string::npos?"":req.substr(hdr+4);
 if(path=="/api/status")reply(c,engine_.statusJson());else if(path=="/api/events")reply(c,engine_.eventsJson());else if(path=="/api/traffic")reply(c,engine_.trafficJson());else if(path=="/api/rules")reply(c,engine_.rulesJson());
 else if(method=="POST"&&path=="/api/event/status"){auto id=val(body,"id");bool ok=false;try{ok=engine_.updateEvent(std::stoull(id),decode(val(body,"status")));}catch(...){}reply(c,ok?"{\"ok\":true}":"{\"ok\":false}","application/json",ok?200:400);}
 else if(method=="POST"&&path=="/api/rule/toggle"){bool ok=engine_.setRuleEnabled(decode(val(body,"id")),val(body,"enabled")=="true");reply(c,ok?"{\"ok\":true}":"{\"ok\":false}","application/json",ok?200:400);}
 else if(path=="/api/health")reply(c,"{\"ok\":true,\"service\":\"Aegis SOC\"}");
 else if(path=="/"||path=="/index.html"){auto page=readFile(std::string(AEGIS_WEB_DIR)+"/index.html");reply(c,page.empty()?"Dashboard file missing. Check web/index.html.":page,"text/html");}
 else if(path=="/app.js"||path=="/style.css"){auto f=readFile(std::string(AEGIS_WEB_DIR)+path);reply(c,f,path=="/app.js"?"application/javascript":"text/css");}
 else reply(c,"{\"error\":\"not found\"}","application/json",404);
}
