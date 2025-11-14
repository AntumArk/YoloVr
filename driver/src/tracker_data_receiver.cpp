//============ Copyright (c) YoloVr Project, All rights reserved. ============
#include "tracker_data_receiver.h"
#include "driverlog.h"
#include <cstring>

#ifndef _WIN32
#include <fcntl.h>
#include <errno.h>
#endif

namespace yolovr {

#ifdef _WIN32
std::atomic<int> TrackerDataReceiver::winsock_ref_count_(0);

bool TrackerDataReceiver::InitializeWinsock() {
    if (winsock_ref_count_.fetch_add(1) == 0) {
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            winsock_ref_count_.fetch_sub(1);
            return false;
        }
    }
    return true;
}

void TrackerDataReceiver::CleanupWinsock() {
    if (winsock_ref_count_.fetch_sub(1) == 1) {
        WSACleanup();
    }
}
#endif

TrackerDataReceiver::TrackerDataReceiver(const std::string& bind_address, uint16_t port)
    : bind_address_(bind_address)
    , port_(port)
    , socket_(INVALID_SOCKET_VALUE)
    , running_(false)
    , last_update_time_(std::chrono::steady_clock::now())
    , timeout_ms_(std::chrono::milliseconds(50))
    , max_frame_size_(64 * 1024) // 64KB max frame size
{
    // Initialize statistics
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = {};
    stats_.last_frame_time = std::chrono::steady_clock::now();
    
#ifdef _WIN32
    InitializeWinsock();
#endif
    
    DriverLog("TrackerDataReceiver created: %s:%d", bind_address_.c_str(), port_);
}

TrackerDataReceiver::~TrackerDataReceiver() {
    Stop();
    
#ifdef _WIN32
    CleanupWinsock();
#endif
    
    DriverLog("TrackerDataReceiver destroyed");
}

bool TrackerDataReceiver::Start() {
    if (running_.load()) {
        DriverLog("TrackerDataReceiver already running");
        return true;
    }
    
    if (!InitializeSocket()) {
        DriverLog("Failed to initialize UDP socket");
        return false;
    }
    
    running_.store(true);
    receiver_thread_ = std::thread(&TrackerDataReceiver::ReceiverThreadFunction, this);
    
    DriverLog("TrackerDataReceiver started successfully");
    return true;
}

void TrackerDataReceiver::Stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    
    // Close socket to unblock receiver thread
    CleanupSocket();
    
    if (receiver_thread_.joinable()) {
        receiver_thread_.join();
    }
    
    DriverLog("TrackerDataReceiver stopped");
}

bool TrackerDataReceiver::GetLatestFrame(yolovr::TrackerFrame& frame) {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    
    if (!HasRecentData()) {
        return false;
    }
    
    frame = latest_frame_;
    return true;
}

bool TrackerDataReceiver::HasRecentData(std::chrono::milliseconds max_age) {
    auto now = std::chrono::steady_clock::now();
    auto age = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update_time_);
    return age <= max_age;
}

TrackerDataReceiver::Stats TrackerDataReceiver::GetStats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

bool TrackerDataReceiver::InitializeSocket() {
    socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_ == INVALID_SOCKET_VALUE) {
        DriverLog("Failed to create UDP socket");
        return false;
    }
    
    // Set socket to non-blocking mode
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(socket_, FIONBIO, &mode) != 0) {
        DriverLog("Failed to set socket to non-blocking mode");
        CleanupSocket();
        return false;
    }
#else
    int flags = fcntl(socket_, F_GETFL, 0);
    if (flags == -1 || fcntl(socket_, F_SETFL, flags | O_NONBLOCK) == -1) {
        DriverLog("Failed to set socket to non-blocking mode");
        CleanupSocket();
        return false;
    }
#endif
    
    // Set receive timeout
#ifdef _WIN32
    DWORD timeout = static_cast<DWORD>(timeout_ms_.count());
    if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR_VALUE) {
        DriverLog("Failed to set socket receive timeout");
    }
#else
    struct timeval timeout;
    timeout.tv_sec = timeout_ms_.count() / 1000;
    timeout.tv_usec = (timeout_ms_.count() % 1000) * 1000;
    if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == SOCKET_ERROR_VALUE) {
        DriverLog("Failed to set socket receive timeout");
    }
#endif
    
    // Bind socket
    struct sockaddr_in bind_addr;
    std::memset(&bind_addr, 0, sizeof(bind_addr));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(port_);
    
    if (bind_address_ == "0.0.0.0") {
        bind_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, bind_address_.c_str(), &bind_addr.sin_addr) != 1) {
            DriverLog("Invalid bind address: %s", bind_address_.c_str());
            CleanupSocket();
            return false;
        }
    }
    
    if (bind(socket_, reinterpret_cast<struct sockaddr*>(&bind_addr), sizeof(bind_addr)) == SOCKET_ERROR_VALUE) {
        DriverLog("Failed to bind UDP socket to %s:%d", bind_address_.c_str(), port_);
        CleanupSocket();
        return false;
    }
    
    DriverLog("UDP socket bound successfully to %s:%d", bind_address_.c_str(), port_);
    return true;
}

void TrackerDataReceiver::CleanupSocket() {
    if (socket_ != INVALID_SOCKET_VALUE) {
        closesocket(socket_);
        socket_ = INVALID_SOCKET_VALUE;
    }
}

void TrackerDataReceiver::ReceiverThreadFunction() {
    DriverLog("TrackerDataReceiver thread started");
    
    std::vector<uint8_t> buffer(max_frame_size_);
    
    while (running_.load()) {
        if (ReceiveFrame()) {
            // Frame received and processed successfully
            continue;
        }
        
        // Small delay to prevent busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    DriverLog("TrackerDataReceiver thread stopped");
}

bool TrackerDataReceiver::ReceiveFrame() {
    std::vector<uint8_t> buffer(max_frame_size_);
    struct sockaddr_in sender_addr;
    socklen_t sender_addr_len = sizeof(sender_addr);
    
    ssize_t bytes_received = recvfrom(socket_, 
                                     reinterpret_cast<char*>(buffer.data()), 
                                     buffer.size(), 
                                     0,
                                     reinterpret_cast<struct sockaddr*>(&sender_addr), 
                                     &sender_addr_len);
    
    if (bytes_received == SOCKET_ERROR_VALUE) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK && error != WSAETIMEDOUT) {
            UpdateStats(false);
            DriverLog("UDP receive error: %d", error);
        }
#else
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            UpdateStats(false);
            DriverLog("UDP receive error: %s", strerror(errno));
        }
#endif
        return false;
    }
    
    if (bytes_received == 0) {
        return false;
    }
    
    // Parse protobuf message
    yolovr::TrackerFrame frame;
    if (!frame.ParseFromArray(buffer.data(), bytes_received)) {
        UpdateStats(false, true);
        DriverLog("Failed to parse protobuf message of %zd bytes", bytes_received);
        return false;
    }
    
    // Validate frame
    if (frame.trackers().size() > 32) { // Sanity check
        UpdateStats(false, true);
        DriverLog("Received frame with too many trackers: %d", frame.trackers().size());
        return false;
    }
    
    // Update latest frame
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        latest_frame_ = std::move(frame);
        last_update_time_ = std::chrono::steady_clock::now();
    }
    
    UpdateStats(true);
    return true;
}

void TrackerDataReceiver::UpdateStats(bool success, bool parse_error) {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (success) {
        stats_.frames_received++;
        stats_.last_frame_time = std::chrono::steady_clock::now();
    } else {
        if (parse_error) {
            stats_.parse_errors++;
        } else {
            stats_.network_errors++;
        }
        stats_.frames_dropped++;
    }
}

} // namespace yolovr