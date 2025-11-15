# YoloVr Python Client

This package provides Python tools for interfacing with the YoloVr tracking system via UDP and Protocol Buffers.

## Components

- **`test_udp_client.py`**: Test client for sending simulated tracker data
- **`yolovr/`**: Python package with tracker communication utilities
- **`examples/`**: Example scripts showing different use cases
- **`requirements.txt`**: Python dependencies

## Quick Start

### Install Dependencies
```bash
pip install -r requirements.txt
```

### Generate Protobuf Files

**Option 1: Using Docker (Recommended - no local protoc needed)**
```bash
# Uses the same protoc version as the C++ driver build
python python-client/scripts/generate_proto_docker.py
```

**Option 2: Using local protoc**
```bash
# Requires protoc installed on your system
python python-client/scripts/generate_proto.py
```

To install protoc locally:
- **Ubuntu/Debian**: `sudo apt install protobuf-compiler`
- **macOS**: `brew install protobuf`
- **Windows**: Download from [protobuf releases](https://github.com/protocolbuffers/protobuf/releases)

### Run Test Client
```bash
# Send single test frame
python test_udp_client.py --single

# Run continuous test at 30fps
python test_udp_client.py --duration 30 --fps 30
```

## Usage Examples

### Basic Tracker Data Sending
```python
from yolovr import TrackerClient

client = TrackerClient()
client.connect('localhost', 9999)

# Send tracker data
frame = client.create_frame()
frame.add_tracker(0, position=(0.1, -1.2, 0.0), rotation=(0, 0, 0, 1))
client.send_frame(frame)
```

### Integration with YOLO
```python
from yolovr import TrackerClient
import cv2

client = TrackerClient()
# Your YOLO detection code here
# Convert detections to tracker positions
# Send to YoloVr driver
```

## Network Protocol

- **Default Port**: 9999 (UDP)
- **Protocol**: Protocol Buffers v3
- **Message Format**: TrackerFrame with up to 12 tracker positions
- **Frequency**: Up to 200Hz supported

## Tracker IDs

| ID | Body Part | Description |
|----|-----------|-------------|
| 0  | LeftLeg | Left ankle/foot |
| 1  | RightLeg | Right ankle/foot |
| 2  | LeftThigh | Left upper leg |
| 3  | RightThigh | Right upper leg |
| 4  | Hip | Center hip |
| 5  | Waist | Waist level |
| 6  | Chest | Chest/torso |
| 7  | LeftUpperArm | Left shoulder-elbow |
| 8  | RightUpperArm | Right shoulder-elbow |
| 9  | LeftForearm | Left elbow-wrist |
| 10 | RightForearm | Right elbow-wrist |
| 11 | Head | Head tracker |

## Development

See `examples/` directory for integration patterns with:
- OpenCV pose detection
- MediaPipe integration  
- Motion capture systems
- Custom tracking algorithms