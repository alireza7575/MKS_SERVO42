#include "MKS_SERVO42.h"
#include <limits.h>

void MKS_SERVO42::initialize(HardwareSerial *serialPort, long const &baudRate)
{
	port_ = serialPort;
	if (port_ == nullptr)
	{
		Serial.println("Serial port is not initialized");
		return;
	}
	port_->begin(baudRate);
}

bool MKS_SERVO42::ping(byte const &stepperId)
{
	if (!isInitialized())
		return false;
	flushInput();
	int send = sendMessage(stepperId, instruction::STEPPER_PING);
	if (send != 3)
	{
		Serial.println("Failed to send");
		return false;
	}
	return receiveStepperStatus(stepperId);
}

int MKS_SERVO42::sendMessage(byte stepperId, byte const &commandId)
{
	stepperId += 0xe0;
	byte message[3];
	byte checksum = stepperId + commandId;
	message[0] = stepperId;
	message[1] = commandId;
	message[2] = checksum & 0xFF;
	for (size_t i = 0; i < 3; i++)
	{
		Serial.print(" ");
		Serial.print(message[i], HEX);
	}
	Serial.println();
	return port_->write(message, 3);
}

bool MKS_SERVO42::isInitialized()
{
	if (port_ != nullptr)
		return true;

	Serial.println("Serial port is not initialized");
	return false;
}

void MKS_SERVO42::flushInput()
{
	while (port_->read() != -1)
		;
}

bool MKS_SERVO42::receiveStepperStatus(byte const &stepperId)
{
	const size_t messageSize = 3;
	byte receivedBytes[messageSize];
	size_t rd = port_->readBytes(receivedBytes, messageSize);
	if (rd != messageSize)
	{
		Serial.println("Invalid response from motor controller");
		return false;
	}
	for (size_t i = 0; i < 2 + sizeof(uint8_t); i++)
	{
		Serial.print(" ");
		Serial.print(receivedBytes[i], HEX);
	}
	Serial.println();
	if (receivedBytes[0] != stepperId + 0xE0 || receivedBytes[2] != calculateChecksum(receivedBytes, 2))
	{
		Serial.println("Invalid response from motor controller");
		return false;
	}
	return receivedBytes[1] == 1;
}

long MKS_SERVO42::getCurrentPosition(byte const &stepperId)
{
	if (!isInitialized())
		return LONG_MIN;
	flushInput();
	int send = sendMessage(stepperId, instruction::GET_ENCODER_POS);
	if (send != 3)
	{
		Serial.println("Failed to send");
		return LONG_MIN;
	}
	return recieveEncoderPosition(stepperId);
}

long MKS_SERVO42::recieveEncoderPosition(byte const &stepperId)
{
	byte receivedBytes[8] = {0xe0, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20};
	size_t bytesRead = port_->readBytes(receivedBytes, 8);
	if (bytesRead == 8 && receivedBytes[0] == (stepperId + 0xE0) && receivedBytes[7] == calculateChecksum(receivedBytes, 7))
	{
		int32_t carry = (int32_t)receivedBytes[1] << 24 | (int32_t)receivedBytes[2] << 16 | (int32_t)receivedBytes[3] << 8 | (int32_t)receivedBytes[4];
		uint16_t value = (uint16_t)receivedBytes[5] << 8 | (uint16_t)receivedBytes[6];
		return (carry * 0xffff) + value;
	}
	else
	{
		Serial.println("Invalid response from motor controller");
		return LONG_MIN;
	}
}

bool MKS_SERVO42::setTargetPosition(byte stepperId, byte direction, uint8_t speed, uint32_t pulses)
{
	if (!isInitialized())
		return false;
	if (speed > 127 || direction > 1)
	{
		Serial.println("Speed out of range or invalid direction");
		return false;
	}
	flushInput();
	stepperId += 0xE0;
	byte message[8];
	message[0] = stepperId;							// Slave address
	message[1] = instruction::MOVE_SPEED_PULSES;	// Function code for running the motor
	message[2] = (direction << 7) | speed;			// VAL byte, with direction and speed
	message[3] = (pulses >> 24) & 0xFF;
	message[4] = (pulses >> 16) & 0xFF;
	message[5] = (pulses >> 8) & 0xFF;
	message[6] = pulses & 0xFF;
	message[7] = calculateChecksum(message, 7);
	if (port_->write(message, sizeof(message)) != sizeof(message))
	{
		Serial.println("Failed to send");
		return false;
	}
	byte response[3];
	size_t bytesRead = port_->readBytes(response, 3);
	if (bytesRead == 3 && response[0] == stepperId && response[2] == calculateChecksum(response, 2))
	{
		if (response[1] == 0)
		{
			Serial.println("Run command failed");
			return false;
		}
		else if (response[1] == 1)
		{
			Serial.println("Run command starting...");
			return true;
		}
		else if (response[1] == 2)
		{
			Serial.println("Run command completed");
			return true;
		}
		Serial.println("Unknown response from motor controller");
		return false;
	}
	else
	{
		Serial.println("Invalid response from motor controller");
		return false;
	}
}

byte MKS_SERVO42::calculateChecksum(const byte *message, int length)
{
	byte checksum = 0;
	for (int i = 0; i < length; i++)
		checksum += message[i];
	return checksum & 0xFF;
}
