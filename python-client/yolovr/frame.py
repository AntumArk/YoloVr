"""
TrackerFrameBuilder - Helper for constructing tracker data frames
"""

import time
from typing import Optional, Tuple, List, Dict

try:
    from . import tracker_data_pb2 as pb
except ImportError:
    try:
        import tracker_data_pb2 as pb
    except ImportError:
        raise ImportError("tracker_data_pb2 not found. Run scripts/generate_proto.py first.")


class TrackerFrameBuilder:
    """Builder class for constructing TrackerFrame messages"""
    
    def __init__(self, frame_id: int = 0, source_id: int = 1, system_name: str = "YoloVr"):
        """Initialize frame builder
        
        Args:
            frame_id: Unique frame identifier
            source_id: Source system identifier
            system_name: Name of the tracking system
        """
        self.frame_id = frame_id
        self.source_id = source_id
        self.system_name = system_name
        self.trackers: Dict[int, dict] = {}
        
    def add_tracker(self, 
                   tracker_id: int,
                   position: Tuple[float, float, float],
                   rotation: Tuple[float, float, float, float] = (0, 0, 0, 1),
                   velocity: Optional[Tuple[float, float, float]] = None,
                   angular_velocity: Optional[Tuple[float, float, float]] = None,
                   confidence: float = 1.0,
                   is_tracking: bool = True) -> 'TrackerFrameBuilder':
        """Add a tracker to the frame
        
        Args:
            tracker_id: Tracker identifier (0-11)
            position: (x, y, z) position in meters
            rotation: (x, y, z, w) quaternion rotation
            velocity: (x, y, z) linear velocity in m/s (optional)
            angular_velocity: (x, y, z) angular velocity in rad/s (optional)
            confidence: Tracking confidence [0.0, 1.0]
            is_tracking: Whether tracker is actively tracking
            
        Returns:
            Self for method chaining
        """
        self.trackers[tracker_id] = {
            'position': position,
            'rotation': rotation,
            'velocity': velocity,
            'angular_velocity': angular_velocity,
            'confidence': confidence,
            'is_tracking': is_tracking
        }
        return self
        
    def add_tracker_simple(self, 
                          tracker_id: int,
                          x: float, y: float, z: float,
                          qx: float = 0, qy: float = 0, qz: float = 0, qw: float = 1) -> 'TrackerFrameBuilder':
        """Add a tracker with simple position/rotation parameters
        
        Args:
            tracker_id: Tracker identifier (0-11)
            x, y, z: Position coordinates in meters
            qx, qy, qz, qw: Quaternion rotation components
            
        Returns:
            Self for method chaining
        """
        return self.add_tracker(tracker_id, (x, y, z), (qx, qy, qz, qw))
    
    def remove_tracker(self, tracker_id: int) -> 'TrackerFrameBuilder':
        """Remove a tracker from the frame
        
        Args:
            tracker_id: Tracker identifier to remove
            
        Returns:
            Self for method chaining
        """
        self.trackers.pop(tracker_id, None)
        return self
    
    def clear_trackers(self) -> 'TrackerFrameBuilder':
        """Remove all trackers from the frame
        
        Returns:
            Self for method chaining
        """
        self.trackers.clear()
        return self
        
    def build(self) -> 'pb.TrackerFrame':
        """Build the protobuf TrackerFrame message
        
        Returns:
            Serializable TrackerFrame protobuf message
        """
        frame = pb.TrackerFrame()
        frame.frame_id = self.frame_id
        frame.timestamp_ns = int(time.time() * 1_000_000_000)
        frame.source_id = self.source_id
        frame.system_name = self.system_name
        
        for tracker_id, data in self.trackers.items():
            pose = pb.TrackerPose()
            pose.tracker_id = tracker_id
            
            # Position
            pose.position.x = data['position'][0]
            pose.position.y = data['position'][1]
            pose.position.z = data['position'][2]
            
            # Rotation
            pose.rotation.x = data['rotation'][0]
            pose.rotation.y = data['rotation'][1]
            pose.rotation.z = data['rotation'][2]
            pose.rotation.w = data['rotation'][3]
            
            # Velocity (optional)
            if data['velocity'] is not None:
                pose.velocity.x = data['velocity'][0]
                pose.velocity.y = data['velocity'][1]
                pose.velocity.z = data['velocity'][2]
            
            # Angular velocity (optional)
            if data['angular_velocity'] is not None:
                pose.angular_velocity.x = data['angular_velocity'][0]
                pose.angular_velocity.y = data['angular_velocity'][1]
                pose.angular_velocity.z = data['angular_velocity'][2]
            
            # Tracking state
            pose.confidence = data['confidence']
            pose.is_tracking = data['is_tracking']
            
            frame.trackers.append(pose)
        
        return frame
    
    def get_tracker_count(self) -> int:
        """Get the number of trackers in the frame
        
        Returns:
            Number of trackers currently added
        """
        return len(self.trackers)
    
    def get_tracker_ids(self) -> List[int]:
        """Get list of tracker IDs in the frame
        
        Returns:
            List of tracker identifiers
        """
        return list(self.trackers.keys())