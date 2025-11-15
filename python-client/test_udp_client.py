#!/usr/bin/env python3
"""
YoloVr UDP Tracker Test Client

This script sends simulated tracker data to the YoloVr driver via UDP
using Protocol Buffers for message serialization.
"""

import socket
import time
import math
import argparse
import sys
import os

# Add the yolovr package directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'yolovr'))

try:
    import tracker_data_pb2 as yolovr
except ImportError:
    print("Error: Could not import tracker_data_pb2")
    print("Make sure you've generated the Python protobuf files:")
    print("  python python-client/scripts/generate_proto_docker.py")
    print("  OR")
    print("  python python-client/scripts/generate_proto.py")
    sys.exit(1)

class TrackerTestClient:
    def __init__(self, host='localhost', port=9999):
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.frame_id = 0
        
        # Tracker names matching the enum in our driver
        self.tracker_names = [
            "LeftLeg", "RightLeg", "LeftThigh", "RightThigh",
            "Hip", "Waist", "Chest", "LeftUpperArm", "RightUpperArm",
            "LeftForearm", "RightForearm", "Head"
        ]
        
        print(f"TrackerTestClient initialized - sending to {host}:{port}")
    
    def create_test_frame(self, timestamp_us):
        """Create a test TrackerFrame with simulated data"""
        frame = yolovr.TrackerFrame()
        frame.frame_id = self.frame_id
        frame.timestamp = timestamp_us
        frame.source_id = 1
        frame.system_name = "YoloVr Test Client"
        frame.system_fps = 30.0
        frame.is_calibrated = True
        frame.lost_tracking_count = 0
        
        # Create HMD reference pose (usually at origin for testing)
        hmd_pose = frame.hmd_pose
        hmd_pose.tracker_id = 999  # Special ID for HMD
        hmd_pose.tracker_name = "HMD"
        hmd_pose.position.x = 0.0
        hmd_pose.position.y = 0.0
        hmd_pose.position.z = 0.0
        hmd_pose.rotation.w = 1.0  # Identity quaternion
        hmd_pose.rotation.x = 0.0
        hmd_pose.rotation.y = 0.0
        hmd_pose.rotation.z = 0.0
        hmd_pose.is_tracking = True
        hmd_pose.confidence = 1.0
        hmd_pose.timestamp = timestamp_us
        
        # Create tracker poses with animation
        time_sec = timestamp_us / 1000000.0
        
        for i, name in enumerate(self.tracker_names):
            tracker = frame.trackers.add()
            tracker.tracker_id = i
            tracker.tracker_name = name
            
            # Create animated positions based on time and tracker ID
            # Each tracker moves in a small circle to test tracking
            radius = 0.1  # 10cm radius movement
            speed = 0.5   # Rotation speed
            angle = time_sec * speed + i * (math.pi * 2 / len(self.tracker_names))
            
            # Base positions (similar to our fake offsets but slightly different for testing)
            base_positions = [
                (-0.15, -1.2, 0.0),   # LeftLeg
                (0.15, -1.2, 0.0),    # RightLeg
                (-0.2, -0.6, 0.0),    # LeftThigh
                (0.2, -0.6, 0.0),     # RightThigh
                (0.0, -0.3, 0.0),     # Hip
                (0.0, -0.1, 0.0),     # Waist
                (0.0, 0.2, 0.0),      # Chest
                (-0.35, 0.1, 0.0),    # LeftUpperArm
                (0.35, 0.1, 0.0),     # RightUpperArm
                (-0.4, -0.1, -0.15),  # LeftForearm
                (0.4, -0.1, -0.15),   # RightForearm
                (0.0, 0.0, 0.0)       # Head
            ]
            
            base_x, base_y, base_z = base_positions[i]
            
            # Add circular movement
            tracker.position.x = base_x + math.cos(angle) * radius
            tracker.position.y = base_y + math.sin(angle) * radius * 0.5  # Smaller Y movement
            tracker.position.z = base_z + math.sin(angle * 2) * radius * 0.3
            
            # Simple rotation (identity for now, but could add rotation animation)
            tracker.rotation.w = 1.0
            tracker.rotation.x = 0.0
            tracker.rotation.y = 0.0
            tracker.rotation.z = 0.0
            
            # Tracking state
            tracker.is_tracking = True
            tracker.confidence = 0.95 + 0.05 * math.sin(time_sec + i)  # Varying confidence
            tracker.timestamp = timestamp_us
            
            # Simple velocity calculation (derivative of position)
            vel_scale = radius * speed
            tracker.velocity.x = -math.sin(angle) * vel_scale
            tracker.velocity.y = math.cos(angle) * vel_scale * 0.5
            tracker.velocity.z = math.cos(angle * 2) * 2 * vel_scale * 0.3
            
        self.frame_id += 1
        return frame
    
    def send_frame(self, frame):
        """Send a TrackerFrame via UDP"""
        try:
            data = frame.SerializeToString()
            self.socket.sendto(data, (self.host, self.port))
            return True
        except Exception as e:
            print(f"Error sending frame: {e}")
            return False
    
    def run_test(self, duration_seconds=30, fps=30):
        """Run the test client for specified duration"""
        frame_interval = 1.0 / fps
        start_time = time.time()
        last_frame_time = start_time
        frames_sent = 0
        
        print(f"Starting test - sending tracker data at {fps}fps for {duration_seconds}s")
        print("Press Ctrl+C to stop early")
        
        try:
            while True:
                current_time = time.time()
                elapsed = current_time - start_time
                
                if elapsed >= duration_seconds:
                    break
                
                # Send frame at specified FPS
                if current_time - last_frame_time >= frame_interval:
                    timestamp_us = int(current_time * 1000000)
                    frame = self.create_test_frame(timestamp_us)
                    
                    if self.send_frame(frame):
                        frames_sent += 1
                        if frames_sent % 30 == 0:  # Print status every second
                            print(f"Sent {frames_sent} frames, {elapsed:.1f}s elapsed")
                    
                    last_frame_time = current_time
                
                # Small sleep to prevent busy-waiting
                time.sleep(0.001)
                
        except KeyboardInterrupt:
            print("\nTest interrupted by user")
        
        print(f"Test completed: {frames_sent} frames sent in {elapsed:.1f}s")
        print(f"Average FPS: {frames_sent / elapsed:.1f}")
    
    def send_single_frame(self):
        """Send a single test frame"""
        timestamp_us = int(time.time() * 1000000)
        frame = self.create_test_frame(timestamp_us)
        
        if self.send_frame(frame):
            print(f"Sent single frame with {len(frame.trackers)} trackers")
            for tracker in frame.trackers:
                print(f"  {tracker.tracker_name}: ({tracker.position.x:.3f}, "
                      f"{tracker.position.y:.3f}, {tracker.position.z:.3f})")
        else:
            print("Failed to send frame")
    
    def close(self):
        """Close the UDP socket"""
        self.socket.close()

def main():
    parser = argparse.ArgumentParser(description='YoloVr UDP Tracker Test Client')
    parser.add_argument('--host', default='localhost', help='Target host (default: localhost)')
    parser.add_argument('--port', type=int, default=9999, help='Target port (default: 9999)')
    parser.add_argument('--duration', type=int, default=30, help='Test duration in seconds (default: 30)')
    parser.add_argument('--fps', type=int, default=30, help='Frames per second (default: 30)')
    parser.add_argument('--single', action='store_true', help='Send single frame and exit')
    
    args = parser.parse_args()
    
    client = TrackerTestClient(args.host, args.port)
    
    try:
        if args.single:
            client.send_single_frame()
        else:
            client.run_test(args.duration, args.fps)
    finally:
        client.close()

if __name__ == '__main__':
    main()