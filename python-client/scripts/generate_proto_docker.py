#!/usr/bin/env python3
"""
Generate protobuf Python bindings using Docker container.
Uses the same protoc version as the C++ driver build.
"""

import os
import sys
import subprocess
from pathlib import Path


def main():
    """Generate protobuf bindings using Docker"""
    # Get paths
    script_dir = Path(__file__).parent
    project_root = script_dir.parent.parent
    proto_file = project_root / 'proto' / 'tracker_data.proto'
    output_dir = script_dir.parent / 'yolovr'
    
    print(f"Project root: {project_root}")
    print(f"Proto file: {proto_file}")
    print(f"Output directory: {output_dir}")
    
    # Check if proto file exists
    if not proto_file.exists():
        print(f"Error: Proto file not found at {proto_file}")
        return 1
    
    # Create output directory
    output_dir.mkdir(exist_ok=True)
    
    # Use Docker to run protoc
    # We'll use the windows-builder image which already has protoc installed
    cmd = [
        'docker', 'compose', 'run', '--rm', 'windows-builder',
        'protoc',
        '--proto_path=/workspace/proto',
        '--python_out=/workspace/python-client/yolovr',
        'tracker_data.proto'
    ]
    
    print(f"Running: {' '.join(cmd)}")
    print("Using protoc from Docker container...")
    
    try:
        # Change to project root for docker compose
        result = subprocess.run(
            cmd,
            cwd=project_root,
            check=True,
            capture_output=True,
            text=True
        )
        
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)
        
        print("✓ Protobuf Python bindings generated successfully!")
        
        # Check if the output file was created
        expected_output = output_dir / 'tracker_data_pb2.py'
        if expected_output.exists():
            print(f"✓ Generated: {expected_output}")
        else:
            print(f"Warning: Expected output file not found: {expected_output}")
            return 1
            
    except subprocess.CalledProcessError as e:
        print(f"Error running protoc in Docker: {e}")
        if e.stdout:
            print(f"stdout: {e.stdout}")
        if e.stderr:
            print(f"stderr: {e.stderr}")
        return 1
    except FileNotFoundError:
        print("Error: Docker not found!")
        print("Please install Docker Desktop or use generate_proto.py with local protoc")
        return 1
    
    print("\nProtobuf generation complete!")
    print("You can now import the generated bindings with:")
    print("  from yolovr import tracker_data_pb2")
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
