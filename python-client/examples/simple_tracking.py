#!/usr/bin/env python3
"""
Simple example showing how to send tracker data to YoloVr
"""

import sys
import time
import math
from pathlib import Path

# Add the yolovr package to Python path
# Go up to python-client directory, then into yolovr
sys.path.insert(0, str(Path(__file__).parent.parent))

try:
    from yolovr import TrackerClient
    print("✓ YoloVr package imported successfully")
except ImportError as e:
    print(f"Error importing YoloVr package: {e}")
    print("Generate protobuf bindings first:")
    print("  python python-client/scripts/generate_proto_docker.py")
    print("  OR")
    print("  python python-client/scripts/generate_proto.py")
    sys.exit(1)


def main():
    """Send animated tracker data to YoloVr"""
    print("Starting YoloVr tracker example...")
    
    # Create client
    client = TrackerClient(host='localhost', port=9999)
    
    try:
        # Send animated data for 10 seconds
        start_time = time.time()
        frame_count = 0
        
        while time.time() - start_time < 10.0:
            current_time = time.time() - start_time
            
            # Create animated positions for 3 trackers
            tracker_data = {}
            
            # Tracker 0: Circle motion
            angle = current_time * 2.0  # 2 rad/s
            tracker_data[0] = (
                math.cos(angle) * 0.5,  # x
                math.sin(angle) * 0.5,  # y
                1.5,                    # z
                0, 0, 0, 1             # rotation quaternion
            )
            
            # Tracker 1: Figure-8 motion
            tracker_data[1] = (
                math.sin(current_time) * 0.3,
                math.sin(current_time * 2) * 0.2,
                1.2,
                0, 0, 0, 1
            )
            
            # Tracker 2: Up-down motion
            tracker_data[2] = (
                0.2,
                0.0,
                1.0 + math.sin(current_time * 3) * 0.3,
                0, 0, 0, 1
            )
            
            # Send the data
            success = client.send_tracker_data(tracker_data)
            if success:
                frame_count += 1
                if frame_count % 60 == 0:  # Print every 60 frames
                    print(f"Sent {frame_count} frames ({frame_count/current_time:.1f} fps)")
            else:
                print("Failed to send frame")
            
            # Sleep to maintain ~60fps
            time.sleep(1.0 / 60.0)
            
    except KeyboardInterrupt:
        print("\nStopping...")
    
    finally:
        client.close()
        print(f"Sent {frame_count} total frames")


if __name__ == '__main__':
    main()