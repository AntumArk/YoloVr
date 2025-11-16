"""
TrackerClient - High-level interface for sending tracker data to YoloVr
"""

import socket
import time
from typing import Optional, Tuple

try:
    from . import tracker_data_pb2 as pb
except ImportError:
    try:
        import tracker_data_pb2 as pb
    except ImportError:
        raise ImportError("tracker_data_pb2 not found. Run scripts/generate_proto.py first.")

from .frame import TrackerFrameBuilder


class TrackerClient:
    """High-level client for sending tracker data to YoloVr via UDP"""
    
    def __init__(self, host: str = 'localhost', port: int = 9999):
        """Initialize tracker client
        
        Args:
            host: Target hostname or IP address
            port: Target UDP port
        """
        self.host = host
        self.port = port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.frame_id = 0
        self.source_id = 1
        self.system_name = "YoloVr Python Client"
        
    def connect(self, host: Optional[str] = None, port: Optional[int] = None):
        """Update connection parameters
        
        Args:
            host: New target host (optional)
            port: New target port (optional)
        """
        if host is not None:
            self.host = host
        if port is not None:
            self.port = port
    
    def create_frame(self) -> TrackerFrameBuilder:
        """Create a new tracker frame builder
        
        Returns:
            TrackerFrameBuilder instance for constructing frame data
        """
        return TrackerFrameBuilder(self.frame_id, self.source_id, self.system_name)
    
    def send_frame(self, frame_builder: TrackerFrameBuilder) -> bool:
        """Send a tracker frame to the YoloVr driver
        
        Args:
            frame_builder: TrackerFrameBuilder with tracker data
            
        Returns:
            True if sent successfully, False on error
        """
        try:
            frame = frame_builder.build()
            data = frame.SerializeToString()
            self.socket.sendto(data, (self.host, self.port))
            self.frame_id += 1
            return True
        except Exception as e:
            print(f"Error sending frame: {e}")
            return False
    
    def send_tracker_data(self, tracker_positions: dict) -> bool:
        """Send tracker positions directly
        
        Args:
            tracker_positions: Dict mapping tracker_id to (x, y, z, qx, qy, qz, qw)
                             Position values in meters, rotation as quaternion
        
        Returns:
            True if sent successfully
        """
        frame_builder = self.create_frame()
        
        for tracker_id, data in tracker_positions.items():
            if len(data) >= 3:
                position = data[:3]
                rotation = data[3:7] if len(data) >= 7 else (0, 0, 0, 1)
                frame_builder.add_tracker(
                    tracker_id, 
                    position=position, 
                    rotation=rotation
                )
        
        return self.send_frame(frame_builder)
    
    def close(self):
        """Close the UDP socket"""
        self.socket.close()
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()