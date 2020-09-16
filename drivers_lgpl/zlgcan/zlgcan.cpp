//------------------------------//
// This is a driver for zlgcan. //
//------------------------------//

#include <string.h>
#include <string>

#include "message.h"
#include "logger.h"
#include "zlgcan.h"

/// CAN message type
using kaco::Message;

/// This struct contains C-strings for
/// busname and baudrate
struct CANBoard {

	/// Bus name
	const char * busname;

	/// Baudrate
	const char * baudrate;

};

struct ZlgCanHandle
{
	DEVICE_HANDLE devHandle;
	CHANNEL_HANDLE channelHandle;
};

#define CAN_BAUD_1M      1000000
#define CAN_BAUD_500K    500000
#define CAN_BAUD_250K    250000
#define CAN_BAUD_125K    125000
#define CAN_BAUD_100K    100000
#define CAN_BAUD_50K     50000
#define CAN_BAUD_20K     20000
#define CAN_BAUD_10K     10000
#define CAN_BAUD_5K      5000

static int TranslateBaudRate (const char *optarg)
{
	if(!strcmp( optarg, "1M")) return CAN_BAUD_1M;
	if(!strcmp( optarg, "500K")) return CAN_BAUD_500K;
	if(!strcmp( optarg, "250K")) return CAN_BAUD_250K;
	if(!strcmp( optarg, "125K")) return CAN_BAUD_125K;
	if(!strcmp( optarg, "100K")) return CAN_BAUD_100K;
	if(!strcmp( optarg, "50K")) return CAN_BAUD_50K;
	if(!strcmp( optarg, "20K")) return CAN_BAUD_20K;
	if(!strcmp( optarg, "10K")) return CAN_BAUD_10K;
	if(!strcmp( optarg, "5K")) return CAN_BAUD_5K;
	if(!strcmp( optarg, "none")) return 0;
	return 0x0000;
}


/// Handle type which should represent the driver instance.
/// You can just some constant != 0 if only one instance is supported.
/// 0 is interpreted as failed initialization.
/// KaCanOpen uses just one instance.
using CANHandle = void*;

/// Initialize the driver and return some handle.
/// The board argument can be used for configuration.
extern "C" CANHandle canOpen_driver(CANBoard* board) {
	PRINT("canOpen_driver");

	// Open device
	UINT index = std::stoul(board->busname);
	DUMP(index);
	
	DEVICE_HANDLE devHandle = ZCAN_OpenDevice(ZCAN_USBCAN2, index, 0);
	DUMP_HEX(devHandle);
	
	if (devHandle == INVALID_DEVICE_HANDLE) {
		PRINT("Open failed: invalid device handle!")
		return (CANHandle) 0;
	}

	// Set baudrate
	int baudrate = TranslateBaudRate(board->baudrate);
	std::string path = std::to_string(index) + "/baud_rate";
	std::string value = std::to_string(baudrate);
	IProperty* property = GetIProperty(devHandle);
	if (property == NULL) {
		PRINT("Set baudrate failed: property pointer is null!");
		return (CANHandle) 0;
	}

	if (property->SetValue(path.c_str(), value.c_str()) != STATUS_OK) {
		PRINT("Set baudrate failed!");
		return (CANHandle) 0;
	} else {
		PRINT("Set baudrate successful!");
	}
	
	// Init CAN
	ZCAN_CHANNEL_INIT_CONFIG config;
	memset(&config, 0, sizeof(config));

	config.can_type = TYPE_CAN;
	config.can.acc_code = 0;
	config.can.acc_mask = 0xFFFFFFFF;
	config.can.filter = 0;
	config.can.mode = 0;

	CHANNEL_HANDLE channelHandle = ZCAN_InitCAN(devHandle, index, &config);
	DUMP_HEX(channelHandle);

	if (channelHandle == INVALID_CHANNEL_HANDLE) {
		PRINT("Open failed: invalid channel handle!");
		return (CANHandle) 0;
	}

	if (ZCAN_StartCAN(channelHandle) != STATUS_OK) {
		PRINT("Start CAN failed!");
		return (CANHandle) 0;
	} else {
		PRINT("Start CAN successful!");
	}

	ZlgCanHandle *handle = new ZlgCanHandle;
	handle->devHandle = devHandle;
	handle->channelHandle = devHandle;

	return (CANHandle) handle;
}

/// Destruct the driver.
/// Return 0 on success.
extern "C" int32_t canClose_driver(CANHandle handle_) {
	PRINT("canClose_driver");
	if (handle_) {
		ZlgCanHandle *handle = (ZlgCanHandle*) handle_;
		UINT status = ZCAN_CloseDevice(handle->devHandle);
		delete handle;
		if (status == STATUS_OK) {
			PRINT("Close successful!");
			return 0;
		} else {
			PRINT("Close failed!");
			return -1;
		}
	}
	return -1;
}

/// Receive a message.
/// This should be a blocking call and wait for any message.
/// Return 0 on success.
extern "C" uint8_t canReceive_driver(CANHandle handle_, Message* message) {
	PRINT("canReceive_driver");
	ZlgCanHandle *handle = (ZlgCanHandle*) handle_;
	UINT num = 0;

	// wait message
	while (num == 0) {
		num = ZCAN_GetReceiveNum(handle->channelHandle, TYPE_CAN);
	}

	ZCAN_Receive_Data data;
	UINT len = ZCAN_Receive(handle->channelHandle, &data, 1);
	if (len < 1) {
		PRINT("Receive failed!");
		return 1;
	}

	can_frame frame = data.frame;

	message->cob_id = frame.can_id & CAN_EFF_MASK;
	message->len = frame.can_dlc;
	if (frame.can_id & CAN_RTR_FLAG) {
		message->rtr = 1;
	} else {
		message->rtr = 0;
	}

	memcpy(message->data, frame.data, 8);

	return 0;
}

/// Send a message
/// Return 0 on success.
extern "C" uint8_t canSend_driver(CANHandle handle_, Message const* message) {
	PRINT("canSend_driver");
	ZlgCanHandle *handle = (ZlgCanHandle*) handle_;

	ZCAN_Transmit_Data data;
	memset(&data, 0, sizeof(data));
	data.transmit_type = 0;
	data.frame.can_id = message->cob_id;
	if (data.frame.can_id >= 0x800) {
		data.frame.can_id |= CAN_EFF_FLAG;
	}
	data.frame.can_dlc = message->len;
	if (message->rtr) {
		data.frame.can_id |= CAN_RTR_FLAG;
	} else {
		memcpy(data.frame.data, message->data, 8);
	}

	UINT len = ZCAN_Transmit(handle->channelHandle, &data, 1);
	if (len < 1) {
		PRINT("Send failed!");
		return 1;
	}

	return 0;
}

/// Change the bus baudrate.
/// The baudrate is given as a C-string.
/// Supported values are 1M, 500K, 250K, 125K, 100K, 50K, 20K, 10K, 5K, and none.
/// Return 0 on success.
extern "C" uint8_t canChangeBaudRate_driver(CANHandle handle, char* baudrate) {
	PRINT("canChangeBaudRate_driver");
	(void) handle;
	(void) baudrate;
	PRINT("canChangeBaudRate not supported by zlgcan");
	return 0;
}