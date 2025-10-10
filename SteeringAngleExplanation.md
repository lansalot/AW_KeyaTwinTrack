# Steering Angle Control in AW KeyaTwinTrack System

## Overview

The AW KeyaTwinTrack system controls steering for twin-track tractors through a combination of UDP commands from AgOpenGPS and CAN bus communication with Keya steering motors. The system is designed to smooth steering movements for agricultural applications, but some issues have been identified regarding the "hold position" behavior of the steering motor.

## Steering Angle Control Flow

### 1. UDP Command Reception

The desired steering angle originates from AgOpenGPS and is sent to the system via UDP packets:

1. **UDP Packet Reception**: The system receives steering commands via `ReceiveUdp()` function in [Autosteer.ino](file:///c%3A/Users/andre/OneDrive/Documents/github/_AW_KeyaTwinTrack/Autosteer.ino#L408-L597)
2. **Steering Angle Extraction**: The desired steering angle is extracted from bytes 8-9 of the UDP packet:
   ```cpp
   steerAngleSetPoint = ((float)(autoSteerUdpData[8] | ((int8_t)autoSteerUdpData[9]) << 8)) * 0.01;
   ```
3. **Watchdog Management**: The watchdog timer is reset when valid steering commands are received

### 2. CAN Bus Command Transmission

The steering angle is then transmitted to the Keya steering motor via CAN bus:

1. **Position Conversion**: The steering angle is converted to motor position units:
   ```cpp
   intendedSteerAngle = steerAngleSetPoint * -1;  // left is right in position-mode
   intendedAngleConverted = degreesToPosition(intendedSteerAngle);
   ```
2. **CAN Commands**: Multiple CAN commands are sent to the motor:
   - Position command with the desired angle
   - Enable command to activate steering

Note: When sending a position command, the motor uses its internal acceleration parameter rather than any speed command.

### 3. Keya Motor Behavior

The Keya steering motor has two distinct modes of operation:

1. **Position Hold Mode**: When a position is set, the motor actively holds that position
2. **Free Return Mode**: When disabled, the motor releases and allows the steering wheel to return to center naturally

## Identified Issues

Based on the Issues.md file and code analysis, three main problems have been identified:

### 1. Steering Holds Position When Stopped

**Problem**: When the vehicle stops, the steering motor continues to hold its last position instead of releasing.

**Root Cause**: The system doesn't properly disable the motor when GPS speed drops below the threshold.

### 2. Steering Doesn't Disable When Stopped

**Problem**: The motor doesn't receive a disable command when movement stops.

**Root Cause**: The condition check for disabling steering only considers GPS speed but doesn't send the explicit disable command.

### 3. Voltage State Not Being Catered For

**Problem**: The system doesn't monitor or respond to voltage state changes that might affect steering operation.

**Root Cause**: No voltage monitoring or response logic is implemented.

## Suggested Fixes

### Fix 1: Proper Motor Disable on Stop

Modify the speed check in [Autosteer.ino](file:///c%3A/Users/andre/OneDrive/Documents/github/_AW_KeyaTwinTrack/Autosteer.ino#L408-L597) to explicitly disable the motor:

```cpp
// Current code around line 320:
if (gpsSpeed < 1) { 
    keyaIntendToSteer = false;
    Serial.println("Stopping as not moving!");
    sendHardwareMessage("Not moving, so not steering!",5);
}
else {
    keyaIntendToSteer = true;
    SteerKeya(keyaIntendToSteer);
}

// Improved version:
if (gpsSpeed < 1) { 
    keyaIntendToSteer = false;
    SteerKeya(false);  // Explicitly send disable command
    Serial.println("Stopping as not moving!");
    sendHardwareMessage("Not moving, so not steering!",5);
}
else {
    keyaIntendToSteer = true;
    SteerKeya(keyaIntendToSteer);
}
```

### Fix 2: Enhanced Watchdog Handling

Improve the watchdog behavior to ensure proper motor disable:

```
// Current code around line 340:
if (watchdogTimer < WATCHDOG_THRESHOLD)
{
}
else
{
    keyaIntendToSteer = false;
    pulseCount = 0;
    // Autosteer Led goes back to RED when autosteering is stopped
    digitalWrite(AUTOSTEER_STANDBY_LED, 1);
    digitalWrite(AUTOSTEER_ACTIVE_LED, 0);
}

// Improved version:
if (watchdogTimer < WATCHDOG_THRESHOLD)
{
}
else
{
    keyaIntendToSteer = false;
    SteerKeya(false);  // Explicitly disable motor
    pulseCount = 0;
    // Autosteer Led goes back to RED when autosteering is stopped
    digitalWrite(AUTOSTEER_STANDBY_LED, 1);
    digitalWrite(AUTOSTEER_ACTIVE_LED, 0);
}
```

### Fix 3: Dead-Band Implementation

Add a dead-band to prevent small steering adjustments and reduce unnecessary CAN bus traffic:

```
// In autosteerLoop function in Autosteer.ino, add dead-band check
static int16_t lastSteerAngleSetPoint = 0;
const int16_t deadBand = 10; // 0.1 degrees

// Replace the direct call to SteerKeya with:
if (abs(steerAngleSetPoint - lastSteerAngleSetPoint) > deadBand) {
    lastSteerAngleSetPoint = steerAngleSetPoint;
    SteerKeya(keyaIntendToSteer);
}
// Otherwise, don't send new command to reduce CAN bus traffic
```

## Twin-Track Steering Optimization

For smoother operation in twin-track tractors:

### 1. Acceleration Control

The system already implements startup commands to control acceleration. The Keya motor uses its internal acceleration parameter when executing position commands, which provides smooth movement without requiring separate speed commands.

### 2. Steering Scaling Adjustment

Use the `steerSettings.steerSensorCounts` parameter to fine-tune steering response:

- Lower values (e.g., 15) increase oversteer sensitivity
- Higher values (e.g., 60) increase understeer (more conservative)
- Default value is 30 for 1:1 mapping

### 3. Proper Position Hold Management

When the vehicle is stationary:
1. Send explicit disable commands to release motor hold
2. Allow natural return to center position
3. Re-enable when movement resumes

## Summary

The current system has a solid foundation for twin-track steering control but needs improvements in:

1. Proper motor disable behavior when stopped
2. Dead-band implementation to reduce unnecessary commands
3. Better position hold management

These improvements will make the steering system safer and more responsive for agricultural applications while maintaining the smooth operation required for twin-track tractors. Voltage and current monitoring for stall detection should be implemented using appropriate methods specific to the Keya motor protocol.
