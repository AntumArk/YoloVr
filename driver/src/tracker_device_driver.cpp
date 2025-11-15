//============ Copyright (c) Valve Corporation, All rights reserved. ============
#include "tracker_device_driver.h"

#include <chrono>
#include "driverlog.h"
#include "vrmath.h"

// Let's create some variables for strings used in getting settings.
// This is the section where all of the settings we want are stored. A section name can be anything,
// but if you want to store driver specific settings, it's best to namespace the section with the driver identifier
// ie "<my_driver>_<section>" to avoid collisions
static const char *my_tracker_main_settings_section = "driver_simpletrackers";

// These are the keys we want to retrieve the values for in the settings
static const char *my_tracker_settings_key_model_number = "mytracker_model_number";

// Tracker names for identification
static const char* tracker_names[] = {
    "LeftLeg", "RightLeg", "LeftThigh", "RightThigh", 
    "Hip", "Waist", "Chest", "LeftUpperArm", "RightUpperArm", 
    "LeftForearm", "RightForearm", "Head"
};

// Default positions relative to HMD (in meters)
struct TrackerOffset {
    float x, y, z;
};

static const TrackerOffset tracker_offsets[] = {
    {-0.15f, -1.2f, 0.0f},  // LeftLegTracker
    {0.15f, -1.2f, 0.0f},   // RightLegTracker  
    {-0.2f, -0.6f, 0.0f},   // LeftThighTracker
    {0.2f, -0.6f, 0.0f},    // RightThighTracker
    {0.0f, -0.3f, 0.0f},    // HipTracker
    {0.0f, -0.1f, 0.0f},    // WaistTracker
    {0.0f, 0.2f, 0.0f},     // ChestTracker
    {-0.35f, 0.1f, 0.0f},   // LeftHandTracker (upper arm - shoulder to elbow)
    {0.35f, 0.1f, 0.0f},    // RightHandTracker (upper arm - shoulder to elbow)
    {-0.4f, -0.1f, -0.15f}, // LeftElbowTracker (forearm - elbow to wrist)
    {0.4f, -0.1f, -0.15f},  // RightElbowTracker (forearm - elbow to wrist)
    {0.0f, 0.0f, 0.0f}      // HeadTracker (same as HMD)
};

// Tracker roles for SteamVR
static const vr::ETrackedControllerRole tracker_roles[] = {
    vr::TrackedControllerRole_Invalid, // LeftLegTracker
    vr::TrackedControllerRole_Invalid, // RightLegTracker
    vr::TrackedControllerRole_Invalid, // LeftThighTracker
    vr::TrackedControllerRole_Invalid, // RightThighTracker
    vr::TrackedControllerRole_Invalid, // HipTracker
    vr::TrackedControllerRole_Invalid, // WaistTracker
    vr::TrackedControllerRole_Invalid, // ChestTracker
    vr::TrackedControllerRole_Invalid, // LeftHandTracker (upper arm tracker, not controller)
    vr::TrackedControllerRole_Invalid, // RightHandTracker (upper arm tracker, not controller)
    vr::TrackedControllerRole_Invalid, // LeftElbowTracker (forearm tracker)
    vr::TrackedControllerRole_Invalid, // RightElbowTracker (forearm tracker)
    vr::TrackedControllerRole_Invalid  // HeadTracker
};

MyTrackerDeviceDriver::MyTrackerDeviceDriver( unsigned int my_tracker_id )
{
	// Set a member to keep track of whether we've activated yet or not
	is_active_ = false;
	has_udp_data_ = false;

	my_tracker_id_ = my_tracker_id;

	// We have our model number and serial number stored in SteamVR settings. We need to get them and do so here.
	// Other IVRSettings methods (to get int32, floats, bools) return the data, instead of modifying, but strings are
	// different.
	char model_number[ 1024 ];
	vr::VRSettings()->GetString(
		my_tracker_main_settings_section, my_tracker_settings_key_model_number, model_number, sizeof( model_number ) );
	my_device_model_number_ = model_number;

	// Create a unique serial number for each tracker type
	my_device_serial_number_ = std::string("YoloVr_") + tracker_names[my_tracker_id] + "_" + std::to_string( my_tracker_id );

	// Here's an example of how to use our logging wrapper around IVRDriverLog
	// In SteamVR logs (SteamVR Hamburger Menu > Developer Settings > Web console) drivers have a prefix of
	// "<driver_name>:". You can search this in the top search bar to find the info that you've logged.
	DriverLog( "Tracker %s Model Number: %s", tracker_names[my_tracker_id], my_device_model_number_.c_str() );
	DriverLog( "Tracker %s Serial Number: %s", tracker_names[my_tracker_id], my_device_serial_number_.c_str() );
}

//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver after our
//  IServerTrackedDeviceProvider calls IVRServerDriverHost::TrackedDeviceAdded.
//-----------------------------------------------------------------------------
vr::EVRInitError MyTrackerDeviceDriver::Activate( uint32_t unObjectId )
{
	// Set an member to keep track of whether we've activated yet or not
	is_active_ = true;

	// Let's keep track of our device index. It'll be useful later.
	my_device_index_ = unObjectId;

	// Properties are stored in containers, usually one container per device index. We need to get this container to set
	// The properties we want, so we call this to retrieve a handle to it.
	vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer( my_device_index_ );

	// Let's begin setting up the properties now we've got our container.
	// A list of properties available is contained in vr::ETrackedDeviceProperty.

	// Set up this device as a tracker
	vr::VRProperties()->SetInt32Property( container, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_GenericTracker );

	// First, let's set the model number.
	vr::VRProperties()->SetStringProperty( container, vr::Prop_ModelNumber_String, my_device_model_number_.c_str() );

	// Set the serial number
	vr::VRProperties()->SetStringProperty( container, vr::Prop_SerialNumber_String, my_device_serial_number_.c_str() );

	// Set the tracker role if applicable (for hand trackers)
	if (tracker_roles[my_tracker_id_] != vr::TrackedControllerRole_Invalid) {
		vr::VRProperties()->SetInt32Property( container, vr::Prop_ControllerRoleHint_Int32, tracker_roles[my_tracker_id_] );
	}

	// Set some other useful properties for trackers
	vr::VRProperties()->SetStringProperty( container, vr::Prop_ManufacturerName_String, "YoloVr" );
	vr::VRProperties()->SetStringProperty( container, vr::Prop_TrackingFirmwareVersion_String, "1.0" );
	vr::VRProperties()->SetStringProperty( container, vr::Prop_HardwareRevision_String, "1.0" );

	// Set tracker-specific name
	std::string display_name = std::string("YoloVr ") + tracker_names[my_tracker_id_] + " Tracker";
	vr::VRProperties()->SetStringProperty( container, vr::Prop_RenderModelName_String, display_name.c_str() );
	
	// CRITICAL: Set the controller type to help VRChat identify the tracker role
	// Use "vive_tracker" as the input profile to ensure compatibility
	vr::VRProperties()->SetStringProperty( container, vr::Prop_ControllerType_String, "vive_tracker" );
	
	// Set this property to make the tracker available for body tracking
	vr::VRProperties()->SetBoolProperty( container, vr::Prop_NeverTracked_Bool, false );
	vr::VRProperties()->SetBoolProperty( container, vr::Prop_WillDriftInYaw_Bool, true );
	vr::VRProperties()->SetBoolProperty( container, vr::Prop_DeviceProvidesBatteryStatus_Bool, false );
	
	// Set input profile path - even though we have no inputs, this helps with compatibility
	vr::VRProperties()->SetStringProperty( container, vr::Prop_InputProfilePath_String, "{htc}/input/vive_tracker_profile.json" );

	// Trackers don't have inputs, so we skip all the input setup

	my_pose_update_thread_ = std::thread( &MyTrackerDeviceDriver::MyPoseUpdateThread, this );

	// We've activated everything successfully!
	// Let's tell SteamVR that by saying we don't have any errors.
	return vr::VRInitError_None;
}

//-----------------------------------------------------------------------------
// Purpose: If you're an HMD, this is where you would return an implementation
// of vr::IVRDisplayComponent, vr::IVRVirtualDisplay or vr::IVRDirectModeComponent.
//
// But this a simple example to demo for a controller, so we'll just return nullptr here.
//-----------------------------------------------------------------------------
void *MyTrackerDeviceDriver::GetComponent( const char *pchComponentNameAndVersion )
{
	return nullptr;
}

//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver when a debug request has been made from an application to the driver.
// What is in the response and request is up to the application and driver to figure out themselves.
//-----------------------------------------------------------------------------
void MyTrackerDeviceDriver::DebugRequest(
	const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize )
{
	if ( unResponseBufferSize >= 1 )
		pchResponseBuffer[ 0 ] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: This is never called by vrserver in recent OpenVR versions,
// but is useful for giving data to vr::VRServerDriverHost::TrackedDevicePoseUpdated.
//-----------------------------------------------------------------------------
vr::DriverPose_t MyTrackerDeviceDriver::GetPose()
{
	// First, initialize the struct that we'll be submitting to the runtime to tell it we've updated our pose.
	vr::DriverPose_t pose = { 0 };

	// These need to be set to be valid quaternions. The device won't appear otherwise.
	pose.qWorldFromDriverRotation.w = 1.f;
	pose.qDriverFromHeadRotation.w = 1.f;

	// Check if we have UDP data for this tracker
	bool use_udp = has_udp_data_.load();
	
	if (use_udp) {
		// Use UDP tracking data
		std::lock_guard<std::mutex> lock(udp_data_mutex_);
		
		// Set position from UDP data
		pose.vecPosition[0] = udp_pose_.position().x();
		pose.vecPosition[1] = udp_pose_.position().y();
		pose.vecPosition[2] = udp_pose_.position().z();
		
		// Set rotation from UDP data
		pose.qRotation.x = udp_pose_.rotation().x();
		pose.qRotation.y = udp_pose_.rotation().y();
		pose.qRotation.z = udp_pose_.rotation().z();
		pose.qRotation.w = udp_pose_.rotation().w();
		
		// Set velocities if available
		if (udp_pose_.has_velocity()) {
			pose.vecVelocity[0] = udp_pose_.velocity().x();
			pose.vecVelocity[1] = udp_pose_.velocity().y();
			pose.vecVelocity[2] = udp_pose_.velocity().z();
			pose.vecWorldFromDriverTranslation[0] = pose.vecVelocity[0];
			pose.vecWorldFromDriverTranslation[1] = pose.vecVelocity[1];
			pose.vecWorldFromDriverTranslation[2] = pose.vecVelocity[2];
		}
		
		// Set tracking confidence
		pose.poseIsValid = udp_pose_.is_tracking();
		pose.deviceIsConnected = true;
		pose.result = udp_pose_.is_tracking() ? vr::TrackingResult_Running_OK : vr::TrackingResult_Running_OutOfRange;
		
		DriverLog("Tracker %s using UDP data: pos(%.3f,%.3f,%.3f) tracking=%s", 
			tracker_names[my_tracker_id_], 
			udp_pose_.position().x(), udp_pose_.position().y(), udp_pose_.position().z(),
			udp_pose_.is_tracking() ? "true" : "false");
		
	} else {
		// Fallback to fake data when no UDP data available
		vr::TrackedDevicePose_t hmd_pose{};
		vr::VRServerDriverHost()->GetRawTrackedDevicePoses( 0.f, &hmd_pose, 1 );

		// Get the position of the hmd from the 3x4 matrix GetRawTrackedDevicePoses returns
		const vr::HmdVector3_t hmd_position = HmdVector3_From34Matrix( hmd_pose.mDeviceToAbsoluteTracking );
		// Get the orientation of the hmd from the 3x4 matrix GetRawTrackedDevicePoses returns
		const vr::HmdQuaternion_t hmd_orientation = HmdQuaternion_FromMatrix( hmd_pose.mDeviceToAbsoluteTracking );

		// For HeadTracker, attach directly to HMD position
		if (my_tracker_id_ == HeadTracker) {
			pose.qRotation = hmd_orientation;
			pose.vecPosition[0] = hmd_position.v[0];
			pose.vecPosition[1] = hmd_position.v[1];
			pose.vecPosition[2] = hmd_position.v[2];
		} else {
			// For other trackers, use the predefined offset
			const TrackerOffset& offset = tracker_offsets[my_tracker_id_];
			
			// Set the pose orientation to match HMD orientation for body trackers
			pose.qRotation = hmd_orientation;

			const vr::HmdVector3_t offset_position = {
				offset.x,
				offset.y,
				offset.z
			};

			// Rotate our offset by the hmd quaternion (so the trackers maintain relative position to user), 
			// and then add the position of the hmd to put it into position.
			const vr::HmdVector3_t position = hmd_position + (offset_position * hmd_orientation);

			// copy our position to our pose
			pose.vecPosition[0] = position.v[0];
			pose.vecPosition[1] = position.v[1];
			pose.vecPosition[2] = position.v[2];
		}

		// The pose we provided is valid.
		pose.poseIsValid = true;
		pose.deviceIsConnected = true;
		pose.result = vr::TrackingResult_Running_OK;
	}

	return pose;
}

void MyTrackerDeviceDriver::MyPoseUpdateThread()
{
	while ( is_active_ )
	{
		// Inform the vrserver that our tracked device's pose has updated, giving it the pose returned by our GetPose().
		vr::VRServerDriverHost()->TrackedDevicePoseUpdated( my_device_index_, GetPose(), sizeof( vr::DriverPose_t ) );

		// Update our pose every five milliseconds.
		// In reality, you should update the pose whenever you have new data from your device.
		std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver when the device should enter standby mode.
// The device should be put into whatever low power mode it has.
// We don't really have anything to do here, so let's just log something.
//-----------------------------------------------------------------------------
void MyTrackerDeviceDriver::EnterStandby()
{
	DriverLog( "Tracker %s has been put into standby", tracker_names[my_tracker_id_] );
}

//-----------------------------------------------------------------------------
// Purpose: This is called by vrserver when the device should deactivate.
// This is typically at the end of a session
// The device should free any resources it has allocated here.
//-----------------------------------------------------------------------------
void MyTrackerDeviceDriver::Deactivate()
{
	// Let's join our pose thread that's running
	// by first checking then setting is_active_ to false to break out
	// of the while loop, if it's running, then call .join() on the thread
	if ( is_active_.exchange( false ) )
	{
		my_pose_update_thread_.join();
	}

	// unassign our controller index (we don't want to be calling vrserver anymore after Deactivate() has been called
	my_device_index_ = vr::k_unTrackedDeviceIndexInvalid;
}


//-----------------------------------------------------------------------------
// Purpose: Update tracker with data from UDP
//-----------------------------------------------------------------------------
void MyTrackerDeviceDriver::MyUpdateFromUDP( const yolovr::TrackerFrame &frame )
{
	// Find our tracker in the UDP frame
	for (const auto& tracker_pose : frame.trackers()) {
		if (tracker_pose.tracker_id() == my_tracker_id_) {
			std::lock_guard<std::mutex> lock(udp_data_mutex_);
			udp_pose_ = tracker_pose;
			has_udp_data_.store(tracker_pose.is_tracking());
			return;
		}
	}
	
	// If we didn't find our tracker in the frame, mark as not tracking
	has_udp_data_.store(false);
}

//-----------------------------------------------------------------------------
// Purpose: This is called by our IServerTrackedDeviceProvider when its RunFrame() method gets called.
// It's not part of the ITrackedDeviceServerDriver interface, we created it ourselves.
//-----------------------------------------------------------------------------
void MyTrackerDeviceDriver::MyRunFrame()
{
	// Trackers don't have inputs, so this function is mostly empty.
	// In a real implementation, you might update tracker state here
	// or handle any tracker-specific logic that needs to run each frame.
}


//-----------------------------------------------------------------------------
// Purpose: This is called by our IServerTrackedDeviceProvider when it pops an event off the event queue.
// It's not part of the ITrackedDeviceServerDriver interface, we created it ourselves.
//-----------------------------------------------------------------------------
void MyTrackerDeviceDriver::MyProcessEvent( const vr::VREvent_t &vrevent )
{
	// Our tracker doesn't have any events it wants to process.
}

//-----------------------------------------------------------------------------
// Purpose: Our IServerTrackedDeviceProvider needs our serial number to add us to vrserver.
// It's not part of the ITrackedDeviceServerDriver interface, we created it ourselves.
//-----------------------------------------------------------------------------
const std::string &MyTrackerDeviceDriver::MyGetSerialNumber()
{
	return my_device_serial_number_;
}