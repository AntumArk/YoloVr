//============ Copyright (c) Valve Corporation, All rights reserved. ============
#pragma once

#include <array>
#include <string>

#include "openvr_driver.h"
#include <atomic>
#include <thread>

enum MyTrackers
{
	LeftLegTracker = 0,
	RightLegTracker = 1,
	LeftThighTracker = 2,
	RightThighTracker = 3,
	HipTracker = 4,
	WaistTracker = 5,
	ChestTracker = 6,
	LeftHandTracker = 7,
	RightHandTracker = 8,
	LeftElbowTracker = 9,
	RightElbowTracker = 10,
	HeadTracker = 11, // only for ground-truth tracking setups
};
//-----------------------------------------------------------------------------
// Purpose: Represents a single tracked device in the system.
// What this device actually is (controller, hmd) depends on the
// properties you set within the device (see implementation of Activate)
//-----------------------------------------------------------------------------
class MyTrackerDeviceDriver : public vr::ITrackedDeviceServerDriver
{
public:
	MyTrackerDeviceDriver( unsigned int my_tracker_id );

	vr::EVRInitError Activate( uint32_t unObjectId ) override;

	void EnterStandby() override;

	void *GetComponent( const char *pchComponentNameAndVersion ) override;

	void DebugRequest( const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize ) override;

	vr::DriverPose_t GetPose() override;

	void Deactivate() override;

	// ----- Functions we declare ourselves below -----

	const std::string &MyGetSerialNumber();

	void MyRunFrame();
	void MyProcessEvent( const vr::VREvent_t &vrevent );

	void MyPoseUpdateThread();

private:
	unsigned int my_tracker_id_;

	std::atomic< vr::TrackedDeviceIndex_t > my_device_index_;

	std::string my_device_model_number_;
	std::string my_device_serial_number_;

	std::atomic< bool > is_active_;
	std::thread my_pose_update_thread_;
};