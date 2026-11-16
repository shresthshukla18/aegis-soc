#pragma once
#include "engine.hpp"
#include <atomic>
#include <string>
#include <thread>
class HttpServer {
public:
    HttpServer(DetectionEngine& engine, int port=8787);
    ~HttpServer();
    bool start();
    void stop();
private:
    void run();
    void handleClient(std::intptr_t socket);
    DetectionEngine& engine_;
    int port_;
    std::atomic<bool> running_{false};
    std::thread thread_;
    std::intptr_t serverSocket_{-1};
};
