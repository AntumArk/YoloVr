//============ Copyright (c) YoloVr Project, All rights reserved. ============
#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    using socket_t = SOCKET;
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
    #define INVALID_SOCKET_VALUE -1
    #define SOCKET_ERROR_VALUE -1
    #define closesocket close
#endif

#include "tracker_data.pb.h"

namespace yolovr {

class TrackerDataReceiver {
public:
    TrackerDataReceiver(const std::string& bind_address = "0.0.0.0", uint16_t port = 9999);
    ~TrackerDataReceiver();

    // Start/stop the UDP receiver thread
    bool Start();
    void Stop();
    
    // Get the latest received tracker frame
    bool GetLatestFrame(yolovr::TrackerFrame& frame);
    
    // Check if we have recent data
    bool HasRecentData(std::chrono::milliseconds max_age = std::chrono::milliseconds(100));
    
    // Get receiver statistics
    struct Stats {
        uint64_t frames_received;
        uint64_t frames_dropped;
        uint64_t parse_errors;
        uint64_t network_errors;
        std::chrono::steady_clock::time_point last_frame_time;
    };
    
    Stats GetStats() const;
    
    // Configuration
    void SetTimeout(std::chrono::milliseconds timeout) { timeout_ms_ = timeout; }
    void SetMaxFrameSize(size_t max_size) { max_frame_size_ = max_size; }

private:
    // Network configuration
    std::string bind_address_;
    uint16_t port_;
    socket_t socket_;
    
    // Threading
    std::atomic<bool> running_;
    std::thread receiver_thread_;
    
    // Data storage
    mutable std::mutex frame_mutex_;
    yolovr::TrackerFrame latest_frame_;
    std::chrono::steady_clock::time_point last_update_time_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    Stats stats_;
    
    // Configuration
    std::chrono::milliseconds timeout_ms_;
    size_t max_frame_size_;
    
    // Internal methods
    void ReceiverThreadFunction();
    bool InitializeSocket();
    void CleanupSocket();
    bool ReceiveFrame();
    void UpdateStats(bool success, bool parse_error = false);
    
#ifdef _WIN32
    // Windows-specific initialization
    static bool InitializeWinsock();
    static void CleanupWinsock();
    static std::atomic<int> winsock_ref_count_;
#endif
};

} // namespace yolovr