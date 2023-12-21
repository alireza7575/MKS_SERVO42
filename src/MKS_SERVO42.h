/// \file MKS_SERVO42.h
/// \brief Provides control over MKS SERVO42 stepper motors via serial communication.
///
/// \details This class is designed to interface with MKS SERVO42 stepper motors, allowing for
///  		 initialization and control of the motors through a series of commands sent over a
///  		 serial connection. It provides methods to ping motors to check their status, retrieve
///  		 current position, and set target positions with specified speed and direction.
///  		 It encapsulates the communication protocol needed to interact with the motors, providing
///  		 a higher-level interface for ease of use in applications.
#ifndef MKS_SERVO42_h
#define MKS_SERVO42_h

#include <Arduino.h>

namespace instruction
{
	byte const GET_ENCODER_POS = 0x30;
	byte const STEPPER_PING = 0x3A;
	byte const MOVE_SPEED_PULSES = 0xFD;
};

/// @brief Driver for MKS SERVO42 stepper motors via serial communication
class MKS_SERVO42
{
public:
	/// @brief Initializes the MKS_SERVO42 object with a serial port and a baud rate.
	/// @param serialPort Pointer to the HardwareSerial object that corresponds to the serial port.
	/// @param baudRate The baud rate for serial communication. If not specified, defaults to 38400.
	void initialize(HardwareSerial *serialPort = nullptr, long const &baudRate = 38400);

	/// @brief Sends a ping command to the stepper motor to check its presence and connectivity.
	/// @param stepperId The ID of the stepper motor to ping.
	/// @return Returns true if the stepper motor is successfully pinged, false otherwise.
	bool ping(byte const &stepperId);

	/// @brief Retrieves the current position of the stepper motor in pulses.
	/// @param stepperId The ID of the stepper motor whose position is to be retrieved.
	/// @return Returns the current position of the stepper motor as a long integer value.
	long getCurrentPosition(byte const &stepperId);

	/// @brief Sets the target position for the stepper motor by specifying the direction, speed, and number of pulses.
	///        The speed parameter should not exceed 2000 RPM and the direction should be 0 for one way or 1 for the opposite.
	/// @param stepperId The ID of the stepper motor to which the command will be sent.
	/// @param direction The direction in which the motor should run (0 or 1).
	/// @param speed The speed at which the motor should run, up to a maximum of 2000 RPM.
	/// @param pulses The number of pulses to move, which translates to the target position.
	/// @return Returns true if the command is successfully sent and the motor starts running, false if there is an error.
	bool setTargetPosition(byte stepperId, byte direction, uint8_t speed, uint32_t pulses);

private:
	HardwareSerial *port_;

	int sendMessage(byte stepperId, byte const &commandID);
	int reciveStepperStatus();
	long recieveEncoderPosition(byte const &stepperId);
	byte calculateChecksum(const byte *message, int length);
};

#endif
