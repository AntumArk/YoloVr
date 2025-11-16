"""
YoloVr Python Client Package

Provides utilities for communicating with the YoloVr tracking system
via UDP and Protocol Buffers.
"""

__version__ = "1.0.0"
__author__ = "YoloVr Team"

try:
    # Main classes
    from .client import TrackerClient
    from .frame import TrackerFrameBuilder
    
    # Expose main interface
    __all__ = ['TrackerClient', 'TrackerFrameBuilder']
    
except ImportError:
    import warnings
    warnings.warn(
        "Protobuf bindings not found. Run scripts/generate_proto.py first to generate them.",
        ImportWarning
    )
    
    # Provide stub classes so imports don't fail completely
    class TrackerClient:
        def __init__(self, *args, **kwargs):
            raise ImportError("Protobuf bindings not generated. Run scripts/generate_proto.py")
    
    class TrackerFrameBuilder:
        def __init__(self, *args, **kwargs):
            raise ImportError("Protobuf bindings not generated. Run scripts/generate_proto.py")
    
    __all__ = ['TrackerClient', 'TrackerFrameBuilder']