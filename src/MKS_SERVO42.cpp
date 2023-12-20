#include "MKS_SERVO42.h"

void MKS_SERVO42::initialize(HardwareSerial *serialPort, long const &baudRate)
{
	port_ = serialPort;
	port_->begin(baudRate);
}

bool MKS_SERVO42::ping(byte const &stepperId)
{
	// Flush
	while (port_->read() != -1)
		;
	int send = sendMessage(stepperId, instruction::STEPPER_PING);
	if (send != 3)
	{
		Serial.println("Failed to send");
		// TODO: throw
		return false;
	}

	return reciveStepperStatus();
}

int MKS_SERVO42::sendMessage(byte stepperId, byte const &commandId)
{
	stepperId += 0xe0;
	byte message[3];
	byte checksum = stepperId + commandId;
	message[0] = stepperId;
	message[1] = commandId;
	message[2] = checksum & 0xFF;
	/*for (size_t i = 0; i < 3; i++)
	{
		Serial.print(" ");
		Serial.print(message[i], HEX);
	}
	Serial.println();*/
	return port_->write(message, 3);
}

int MKS_SERVO42::reciveStepperStatus()
{
	int messageSize = 2 + sizeof(uint8_t);
	byte receivedBytes[messageSize];
	size_t rd = port_->readBytes(receivedBytes, messageSize);
	/*for (size_t i = 0; i < 2 + sizeof(uint8_t); i++)
	{
		Serial.print(" ");
		Serial.print(receivedBytes[i], HEX);
	}
	Serial.println();*/
	return receivedBytes[1] == 1;
}

int MKS_SERVO42::getCurrentPosition(byte const &stepperId)
{
	// Flush
	while (port_->read() != -1)
		;
	int send = sendMessage(stepperId, instruction::GET_ENCODER_POS);
	if (send != 3)
	{
		// TODO: throw
		Serial.println("Failed to send");
		return -1;
	}
	return recieveEncoderPosition();
}

int MKS_SERVO42::recieveEncoderPosition()
{
	byte receivedBytes[8] = {0xe0, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20};
	size_t rd = port_->readBytes(receivedBytes, 8);
	int32_t carry = 0;
	for (int i = 1; i < 5; i++)
		carry = (carry << 8) | receivedBytes[i];
	uint16_t value = 0;
	for (int i = 5; i < 7; i++)
		value = (value << 8) | receivedBytes[i];
	return carry * 360 + value * 22 / 1000;
}

// TODO: fix
bool MKS_SERVO42::setTargetPosition(byte const &stepperId, byte const &dir, int const &speed, byte const &acc, uint32_t const &pulses)
{
	// Ensure parameters are within their valid ranges
	if (speed < 0 || speed > 1600 || acc > 32)
		return false;
	byte message[11];
	message[0] = HEADER;	// Header
	message[1] = stepperId; // Slave address
	message[2] = 0xFD;		// Function code for position mode
	// Speed and direction encoding
	message[3] = (dir << 7) | ((speed >> 4) & 0x0F);
	message[4] = speed & 0x0F;
	message[5] = acc; // Acceleration
	// Pulses (4 bytes)
	message[6] = (pulses >> 24) & 0xFF;
	message[7] = (pulses >> 16) & 0xFF;
	message[8] = (pulses >> 8) & 0xFF;
	message[9] = pulses & 0xFF;
	message[10] = calculateChecksum(message, 10); // Checksum
	port_->write(message, sizeof(message)) == sizeof(message);
	int messageSize = 4 + sizeof(uint8_t);
	byte receivedBytes[messageSize];
	size_t rd = port_->readBytes(receivedBytes, messageSize);
	return receivedBytes[3] == 1;
	// Send the message
}

byte MKS_SERVO42::calculateChecksum(const byte *message, int length)
{
	byte checksum = 0;
	for (int i = 0; i < length; i++)
		checksum += message[i];
	return checksum & 0xFF;
}