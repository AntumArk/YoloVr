# YoloVr Quick Testing Guide

## 🚀 Quick Start Testing

### 1. Install & Register Driver
```bash
# Linux
sudo cp -r output/zincyolotrackers /usr/local/share/steamvr/drivers/

# Windows  
vrpathreg adddriver C:\path\to\YoloVr\output\zincyolotrackers
```

### 2. Start SteamVR & Check Logs
1. Start SteamVR
2. Open Web Console: `http://localhost:8998/console/index.html`
3. Search for: `zincyolotrackers`

**Expected Log Output:**
```
[zincyolotrackers] Created 12 tracker devices successfully
[zincyolotrackers] Tracker LeftLeg Model Number: YoloVr Full Body Tracker
[zincyolotrackers] Tracker LeftLeg Serial Number: YoloVr_LeftLeg_0
... (x12 trackers)
```

### 3. Visual Verification
1. Put on VR headset
2. Look around for 12 tracker objects
3. Verify positioning:
   - **Legs**: 1.2m below your head
   - **Arms**: At shoulder/forearm level  
   - **Torso**: At chest/waist/hip level
   - **Head**: Exactly at HMD position

## ✅ Quick Validation Checklist

- [ ] **Driver Loads**: No errors in SteamVR logs
- [ ] **12 Trackers**: All devices detected with unique serials
- [ ] **Visual Presence**: All trackers visible in VR space
- [ ] **Smooth Movement**: Trackers follow HMD without jitter
- [ ] **Correct Positioning**: Anatomically correct relative positions
- [ ] **Performance**: No frame rate drops or crashes
- [ ] **Stability**: Works for >10 minutes without issues

## 🐛 Quick Troubleshooting

### No Trackers Detected
```bash
# Check driver registration
ls -la /usr/local/share/steamvr/drivers/ | grep zinc

# Check SteamVR logs for errors
grep -i error ~/.steam/logs/vrserver.txt | tail -10
```

### Partial Detection (< 12 trackers)
```bash
# Check device creation logs
grep "YoloVr_.*_[0-9]" ~/.steam/logs/vrserver.txt | wc -l
# Should return: 12
```

### Performance Issues
```bash
# Monitor CPU usage during tracking
top -p $(pgrep vrserver)

# Check memory usage
grep -i memory ~/.steam/logs/vrserver.txt
```

## 📊 Testing Commands

### Automated Test
```bash
#!/bin/bash
echo "Testing YoloVr Tracker System..."

# Test 1: Count detected trackers
TRACKER_COUNT=$(grep -c "YoloVr_.*_[0-9]" ~/.steam/logs/vrserver.txt)
echo "Detected trackers: $TRACKER_COUNT/12"

# Test 2: Check for errors  
ERROR_COUNT=$(grep -ci error ~/.steam/logs/vrserver.txt | tail -1)
echo "Errors in log: $ERROR_COUNT"

# Test 3: Verify pose updates
POSE_UPDATES=$(grep -c "TrackedDevicePoseUpdated" ~/.steam/logs/vrserver.txt)
echo "Pose updates: $POSE_UPDATES"

if [ "$TRACKER_COUNT" -eq 12 ] && [ "$ERROR_COUNT" -eq 0 ]; then
    echo "✅ YoloVr tracker system working correctly!"
else
    echo "❌ Issues detected. Check SteamVR logs."
fi
```

### Manual Verification
```python
# test_trackers.py - Simple Python test
import openvr

def check_trackers():
    openvr.init(openvr.VRApplication_Background)
    vr_system = openvr.VRSystem()
    
    yolo_trackers = []
    for i in range(openvr.k_unMaxTrackedDeviceCount):
        if vr_system.isTrackedDeviceConnected(i):
            device_class = vr_system.getTrackedDeviceClass(i)
            if device_class == openvr.TrackedDeviceClass_GenericTracker:
                serial = vr_system.getStringTrackedDeviceProperty(
                    i, openvr.Prop_SerialNumber_String
                ).decode('utf-8')
                if serial.startswith('YoloVr_'):
                    yolo_trackers.append(serial)
    
    print(f"Found {len(yolo_trackers)} YoloVr trackers:")
    for tracker in sorted(yolo_trackers):
        print(f"  - {tracker}")
    
    openvr.shutdown()
    return len(yolo_trackers) == 12

if __name__ == "__main__":
    success = check_trackers()
    print(f"Test {'PASSED' if success else 'FAILED'}")
```

## 📈 Expected Performance Metrics

| Metric | Expected Value | Test Method |
|--------|----------------|-------------|
| Tracker Count | 12 | Log analysis |
| Update Rate | 200 Hz (5ms) | Frame timing |
| CPU Usage | <1% additional | System monitor |
| Memory Usage | <50MB total | Process monitor |
| Latency | <20ms | Timing analysis |
| Uptime | >30min stable | Extended test |

## 🔍 Diagram References

The `ARCHITECTURE_DIAGRAMS.puml` file contains:

1. **System Architecture**: Overall component relationships
2. **Data Flow**: How pose data moves through the system  
3. **Component Interaction**: Class relationships and interfaces
4. **Testing Flow**: Step-by-step testing procedure
5. **State Machine**: Tracker lifecycle states

To view diagrams:
```bash
# Install PlantUML
sudo apt install plantuml

# Generate diagrams
plantuml ARCHITECTURE_DIAGRAMS.puml

# This creates PNG files for each diagram
```

## 📝 Test Report Template

```
# YoloVr Tracker Test Report

**Date**: ___________
**Tester**: ___________
**SteamVR Version**: ___________
**Driver Version**: ___________

## Results
- [ ] Driver Registration: PASS/FAIL
- [ ] Tracker Detection (12/12): PASS/FAIL  
- [ ] Visual Verification: PASS/FAIL
- [ ] Movement Tracking: PASS/FAIL
- [ ] Performance: PASS/FAIL
- [ ] Stability Test: PASS/FAIL

## Notes
_____________________________________
_____________________________________

## Conclusion
Overall Status: PASS/FAIL
Ready for Production: YES/NO
```

This testing documentation provides everything needed to validate that all 12 trackers are working correctly in the YoloVr system!