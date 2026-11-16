#pragma once
#include "engine.hpp"
#include <atomic>
#include <string>
#include <thread>
class PacketCapture {
public:
    explicit PacketCapture(DetectionEngine& engine);
    ~PacketCapture();
    bool available() const;
    std::string devices() const;
    bool start(const std::string& device);
    void stop();
private:
    void run(std::string device);
    DetectionEngine& engine_;
    std::atomic<bool> running_{false};
    std::thread thread_;
};
