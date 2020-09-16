//------------------------------//
// This is a driver for zlgcan. //
//------------------------------//

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

/// Handle type which should represent the driver instance.
/// You can just some constant != 0 if only one instance is supported.
/// 0 is interpreted as failed initialization.
/// KaCanOpen uses just one instance.
using CANHandle = void*;

/// Initialize the driver and return some handle.
/// The board argument can be used for configuration.
extern "C" CANHandle canOpen_driver(CANBoard* board) {
	PRINT("canOpen_driver");
	(void) board;
	return (CANHandle) 1;
}

/// Destruct the driver.
/// Return 0 on success.
extern "C" int32_t canClose_driver(CANHandle handle) {
	PRINT("canClose_driver");
	(void) handle;
	return 0;
}

/// Receive a message.
/// This should be a blocking call and wait for any message.
/// Return 0 on success.
extern "C" uint8_t canReceive_driver(CANHandle handle, Message* message) {
	PRINT("canReceive_driver");
	(void) handle;
	(void) message;
	return 0;
}

/// Send a message
/// Return 0 on success.
extern "C" uint8_t canSend_driver(CANHandle handle, Message const* message) {
	PRINT("canSend_driver");
	(void) handle;
	(void) message;
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
	return 0;
}