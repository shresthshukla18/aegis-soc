#pragma once
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Event {
    std::uint64_t id{};
    std::string time, severity, category, source, destination, protocol, title, details, status{"Open"};
    int sourcePort{}, destinationPort{};
};
struct TrafficPoint { std::string time; std::uint64_t packets{}, bytes{}, alerts{}; };
struct Rule { std::string id, name, category, severity; int destinationPort{-1}; std::string protocol; bool enabled{true}; };

class DetectionEngine {
public:
    DetectionEngine();
    ~DetectionEngine();
    void startSimulation();
    void stop();
    void ingestPacket(const std::string& src, const std::string& dst, int srcPort, int dstPort,
                      const std::string& protocol, std::size_t bytes, bool syn=false);
    void addEvent(std::string severity, std::string category, std::string source,
                  std::string destination, std::string protocol, std::string title,
                  std::string details, int srcPort=0, int dstPort=0);
    std::string statusJson() const;
    std::string eventsJson() const;
    std::string trafficJson() const;
    std::string rulesJson() const;
    bool updateEvent(std::uint64_t id, const std::string& status);
    bool setRuleEnabled(const std::string& id, bool enabled);
    bool loadRules(const std::string& path);
    void setMode(const std::string& mode);
    std::string mode() const;
private:
    void simulationLoop();
    void trimLocked();
    mutable std::mutex mutex_;
    std::deque<Event> events_;
    std::deque<TrafficPoint> traffic_;
    std::vector<Rule> rules_;
    std::unordered_map<std::string, std::unordered_set<int>> portsBySource_;
    std::unordered_map<std::string, std::uint64_t> synBySource_;
    std::atomic<bool> running_{false};
    std::thread simulationThread_;
    std::uint64_t nextId_{1}, totalPackets_{0}, totalBytes_{0}, totalAlerts_{0}, blocked_{0};
    std::string mode_{"SIMULATION"};
    std::chrono::steady_clock::time_point windowStart_{std::chrono::steady_clock::now()};
};
