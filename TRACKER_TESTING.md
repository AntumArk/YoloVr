# YoloVr Tracker Testing Documentation

## Overview

This document outlines how to test the YoloVr 12-tracker full-body tracking system to ensure all trackers are properly detected and functioning in SteamVR.

## System Architecture

Our tracker system consists of:
- **12 Individual Trackers**: Each representing a specific body part
- **Position-Only Tracking**: No input devices, pure pose tracking
- **HMD-Relative Positioning**: All trackers positioned relative to the HMD
- **Real-Time Updates**: 200Hz pose update rate (5ms intervals)

## Testing Prerequisites

### 1. Hardware Requirements
- VR Headset (any OpenVR-compatible HMD)
- Windows PC or Linux system with SteamVR installed
- Sufficient USB ports for headset connection

### 2. Software Requirements
- SteamVR installed and configured
- Our compiled driver: `libdriver_zincyolotrackers.so` (Linux) or `driver_zincyolotrackers.dll` (Windows)
- SteamVR Developer tools enabled

### 3. Driver Installation
```bash
# Linux Installation
sudo cp -r output/zincyolotrackers /usr/local/share/steamvr/drivers/
# Or register with vrpathreg (Windows/Linux)
vrpathreg adddriver /path/to/YoloVr/output/zincyolotrackers
```

## Testing Procedures

### Phase 1: Driver Registration Test

#### 1.1 Check Driver Loading
**Objective**: Verify SteamVR recognizes our driver

**Steps**:
1. Start SteamVR
2. Open SteamVR Settings → Developer → Web Console
3. Look for log entries containing "zincyolotrackers"
4. Search for "Created X tracker devices successfully" message

**Expected Results**:
```
[zincyolotrackers] Created 12 tracker devices successfully
[zincyolotrackers] Tracker LeftLeg Model Number: YoloVr Full Body Tracker
[zincyolotrackers] Tracker LeftLeg Serial Number: YoloVr_LeftLeg_0
... (repeated for all 12 trackers)
```

#### 1.2 Device Enumeration Test
**Objective**: Verify all 12 trackers are registered

**Steps**:
1. In Web Console, search for "TrackedDeviceAdded"
2. Count the number of tracker devices added
3. Note the device indices assigned to each tracker

**Expected Results**:
- 12 tracker devices successfully added
- Each with unique serial numbers
- All classified as `TrackedDeviceClass_GenericTracker`

### Phase 2: Pose Tracking Test

#### 2.1 Visual Verification in SteamVR
**Objective**: Confirm trackers appear in VR space

**Steps**:
1. Put on VR headset
2. Open SteamVR Dashboard
3. Look around for tracker representations
4. Note tracker positions relative to HMD

**Expected Results**:
- 12 tracker objects visible in VR space
- Positioned according to our offset configuration:
  - Head tracker: At HMD position
  - Leg trackers: 1.2m below HMD
  - Arm trackers: At shoulder/forearm level
  - Torso trackers: At chest/waist/hip level

#### 2.2 Movement Response Test
**Objective**: Verify trackers follow HMD movement

**Steps**:
1. Move head left/right while in VR
2. Rotate head up/down
3. Walk around (if room-scale available)
4. Observe tracker movement

**Expected Results**:
- All trackers maintain relative positions to HMD
- Smooth movement without jitter
- No tracking dropouts

#### 2.3 Pose Update Rate Test
**Objective**: Confirm 200Hz update rate

**Steps**:
1. Monitor SteamVR logs for pose updates
2. Use SteamVR frame timing tools
3. Check for consistent update intervals

**Expected Results**:
- Pose updates every ~5ms
- No missed updates or delays
- Consistent timing across all trackers

### Phase 3: Individual Tracker Validation

#### 3.1 Tracker Identification Test
**Objective**: Verify each tracker has correct identity

**Test Matrix**:
| Tracker ID | Name | Expected Position | Serial Number Pattern |
|------------|------|-------------------|----------------------|
| 0 | LeftLeg | (-0.15, -1.2, 0.0) | YoloVr_LeftLeg_0 |
| 1 | RightLeg | (0.15, -1.2, 0.0) | YoloVr_RightLeg_1 |
| 2 | LeftThigh | (-0.2, -0.6, 0.0) | YoloVr_LeftThigh_2 |
| 3 | RightThigh | (0.2, -0.6, 0.0) | YoloVr_RightThigh_3 |
| 4 | Hip | (0.0, -0.3, 0.0) | YoloVr_Hip_4 |
| 5 | Waist | (0.0, -0.1, 0.0) | YoloVr_Waist_5 |
| 6 | Chest | (0.0, 0.2, 0.0) | YoloVr_Chest_6 |
| 7 | LeftUpperArm | (-0.35, 0.1, 0.0) | YoloVr_LeftUpperArm_7 |
| 8 | RightUpperArm | (0.35, 0.1, 0.0) | YoloVr_RightUpperArm_8 |
| 9 | LeftForearm | (-0.4, -0.1, -0.15) | YoloVr_LeftForearm_9 |
| 10 | RightForearm | (0.4, -0.1, -0.15) | YoloVr_RightForearm_10 |
| 11 | HeadTracker | (0.0, 0.0, 0.0) | YoloVr_Head_11 |

#### 3.2 Resource File Validation
**Objective**: Ensure all tracker profiles load correctly

**Steps**:
1. Check SteamVR logs for profile loading messages
2. Verify no "profile not found" errors
3. Confirm input profile associations

**Expected Results**:
- All 12 tracker profiles loaded successfully
- No JSON parsing errors
- Correct driver name associations

### Phase 4: Integration Testing

#### 4.1 SteamVR Application Test
**Objective**: Test tracker visibility in VR applications

**Recommended Test Apps**:
- **SteamVR Room Setup**: Check tracker visibility during setup
- **VRChat**: Test full-body tracking integration
- **Beat Saber**: Verify trackers don't interfere with gameplay
- **Custom OpenVR Test App**: Direct API testing

#### 4.2 Performance Impact Test
**Objective**: Measure system performance with 12 trackers

**Metrics to Monitor**:
- CPU usage during tracking
- Memory consumption
- SteamVR frame rate impact
- Tracking latency

**Expected Results**:
- Minimal CPU overhead (<1% additional)
- Memory usage <50MB for all trackers
- No frame rate degradation
- Tracking latency <20ms

### Phase 5: Error Handling Tests

#### 5.1 Driver Reload Test
**Objective**: Test driver stability during SteamVR restart

**Steps**:
1. Start SteamVR with trackers active
2. Restart SteamVR
3. Verify all trackers reconnect properly
4. Check for memory leaks or crashes

#### 5.2 Tracker Disconnect Simulation
**Objective**: Test graceful handling of tracker "disconnection"

**Steps**:
1. Modify driver to simulate tracker disconnection
2. Verify SteamVR handles missing trackers gracefully
3. Test reconnection scenarios

## Debugging Tools

### SteamVR Web Console
- **URL**: `http://localhost:8998/console/index.html`
- **Usage**: Real-time log monitoring and device inspection

### OpenVR Input Emulator
- **Purpose**: Manual tracker control and testing
- **Download**: GitHub - matzman666/OpenVR-InputEmulator

### Custom Test Scripts
```python
# Example OpenVR Python test script
import openvr

def test_tracker_detection():
    vr_system = openvr.init(openvr.VRApplication_Background)
    
    for i in range(openvr.k_unMaxTrackedDeviceCount):
        if vr_system.isTrackedDeviceConnected(i):
            device_class = vr_system.getTrackedDeviceClass(i)
            if device_class == openvr.TrackedDeviceClass_GenericTracker:
                serial = vr_system.getStringTrackedDeviceProperty(
                    i, openvr.Prop_SerialNumber_String
                )
                print(f"Found tracker: {serial}")
    
    openvr.shutdown()

if __name__ == "__main__":
    test_tracker_detection()
```

## Troubleshooting Common Issues

### Issue 1: No Trackers Detected
**Symptoms**: SteamVR shows no tracker devices
**Solutions**:
1. Check driver registration path
2. Verify file permissions
3. Review SteamVR logs for loading errors
4. Confirm manifest file syntax

### Issue 2: Partial Tracker Detection
**Symptoms**: Only some trackers appear
**Solutions**:
1. Check device provider loop logic
2. Verify all tracker profiles exist
3. Monitor memory allocation issues
4. Check for initialization failures

### Issue 3: Tracking Jitter
**Symptoms**: Trackers move erratically
**Solutions**:
1. Verify pose update thread stability
2. Check for numerical precision issues
3. Monitor system performance
4. Review quaternion calculations

### Issue 4: Performance Degradation
**Symptoms**: Low frame rates with trackers enabled
**Solutions**:
1. Optimize pose update frequency
2. Check for memory leaks
3. Profile CPU usage
4. Review thread synchronization

## Success Criteria

### ✅ Complete Test Pass Requirements:
1. All 12 trackers detected and registered
2. Correct positioning relative to HMD
3. Smooth pose updates at 200Hz
4. No SteamVR crashes or errors
5. Minimal performance impact
6. Proper cleanup on driver shutdown
7. All tracker profiles loaded correctly
8. Unique serial numbers for each tracker
9. Correct device classification
10. Stable operation for >30 minutes

## Automation Scripts

### Automated Test Suite
```bash
#!/bin/bash
# automated_tracker_test.sh

echo "Starting YoloVr Tracker Test Suite..."

# Test 1: Driver Registration
echo "Testing driver registration..."
if grep -q "zincyolotrackers" ~/.steam/logs/vrserver.txt; then
    echo "✅ Driver registered successfully"
else
    echo "❌ Driver registration failed"
    exit 1
fi

# Test 2: Tracker Count
echo "Testing tracker count..."
tracker_count=$(grep -c "YoloVr_.*_[0-9]" ~/.steam/logs/vrserver.txt)
if [ "$tracker_count" -eq 12 ]; then
    echo "✅ All 12 trackers detected"
else
    echo "❌ Expected 12 trackers, found $tracker_count"
    exit 1
fi

# Test 3: Pose Updates
echo "Testing pose updates..."
if grep -q "TrackedDevicePoseUpdated" ~/.steam/logs/vrserver.txt; then
    echo "✅ Pose updates active"
else
    echo "❌ No pose updates detected"
    exit 1
fi

echo "🎉 All tests passed! YoloVr tracker system is working correctly."
```

This comprehensive testing documentation ensures that our 12-tracker system is thoroughly validated and ready for production use.