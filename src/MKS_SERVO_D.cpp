#include "MKS_SERVO_D.h"

void MKS_SERVO_D::initialize(HardwareSerial *serialPort, long const &baudRate)
{
	port_ = serialPort;
	if (port_ == nullptr)
	{
		Serial.println("Serial port is not initialized");
		return;
	}
	port_->begin(baudRate);
}

bool MKS_SERVO_D::readPosition(uint8_t slaveAddress, int32_t &position)
{
	uint8_t frame[4] = {REQUEST_HEADER, slaveAddress, READ_POSITION, 0};
	uint8_t response[10];

	frame[3] = checksum(frame, 3);
	if (!writeFrame(frame, sizeof(frame)) || !readFrame(response, sizeof(response)))
		return false;
	if (response[1] != slaveAddress || response[2] != READ_POSITION)
		return false;

	position = (int32_t)(((uint32_t)response[5] << 24) |
						 ((uint32_t)response[6] << 16) |
						 ((uint32_t)response[7] << 8) |
						 ((uint32_t)response[8]));
	return true;
}

bool MKS_SERVO_D::readSpeed(uint8_t slaveAddress, int16_t &speedRpm)
{
	uint8_t frame[4] = {REQUEST_HEADER, slaveAddress, READ_SPEED, 0};
	uint8_t response[6];

	frame[3] = checksum(frame, 3);
	if (!writeFrame(frame, sizeof(frame)) || !readFrame(response, sizeof(response)))
		return false;
	if (response[1] != slaveAddress || response[2] != READ_SPEED)
		return false;

	speedRpm = (int16_t)(((uint16_t)response[3] << 8) | response[4]);
	return true;
}

MKS_SERVO_D::AckStatus MKS_SERVO_D::runSpeedMode(uint8_t slaveAddress, uint8_t direction, uint16_t speedRpm, uint8_t acceleration)
{
	if (direction > 1 || speedRpm > 0x0FFF)
		return ACK_FAILED;

	uint8_t frame[7] = {
		REQUEST_HEADER,
		slaveAddress,
		SPEED_MODE,
		(uint8_t)((direction << 7) | ((speedRpm >> 8) & 0x0F)),
		(uint8_t)(speedRpm & 0xFF),
		acceleration,
		0};

	frame[6] = checksum(frame, 6);
	if (!writeFrame(frame, sizeof(frame)))
		return ACK_FAILED;

	return waitForAck();
}

MKS_SERVO_D::AckStatus MKS_SERVO_D::runPositionMode1(uint8_t slaveAddress, uint8_t direction, uint16_t speedRpm, uint8_t acceleration, uint32_t pulses, uint32_t timeoutMs)
{
	if (direction > 1 || speedRpm > 0x0FFF)
		return ACK_FAILED;

	uint8_t frame[11] = {
		REQUEST_HEADER,
		slaveAddress,
		POSITION_MODE_1,
		(uint8_t)((direction << 7) | ((speedRpm >> 8) & 0x0F)),
		(uint8_t)(speedRpm & 0xFF),
		acceleration,
		(uint8_t)((pulses >> 24) & 0xFF),
		(uint8_t)((pulses >> 16) & 0xFF),
		(uint8_t)((pulses >> 8) & 0xFF),
		(uint8_t)(pulses & 0xFF),
		0};

	frame[10] = checksum(frame, 10);
	if (!writeFrame(frame, sizeof(frame)))
		return ACK_FAILED;

	return waitForAck(timeoutMs);
}

MKS_SERVO_D::AckStatus MKS_SERVO_D::waitForAck(uint32_t timeoutMs)
{
	uint8_t response[5];
	if (!readFrame(response, sizeof(response), timeoutMs))
		return ACK_FAILED;
	if (response[2] != SPEED_MODE && response[2] != POSITION_MODE_1)
		return ACK_FAILED;

	return (AckStatus)response[3];
}

bool MKS_SERVO_D::isInitialized()
{
	if (port_ != nullptr)
		return true;

	Serial.println("Serial port is not initialized");
	return false;
}

void MKS_SERVO_D::flushInput()
{
	while (port_->read() != -1)
		;
}

uint8_t MKS_SERVO_D::checksum(const uint8_t *buffer, uint8_t length)
{
	uint16_t sum = 0;
	for (uint8_t i = 0; i < length; i++)
		sum += buffer[i];

	return (uint8_t)(sum & 0xFF);
}

bool MKS_SERVO_D::writeFrame(const uint8_t *frame, size_t length)
{
	if (!isInitialized())
		return false;

	flushInput();
	return port_->write(frame, length) == length;
}

bool MKS_SERVO_D::readFrame(uint8_t *buffer, uint8_t length, uint32_t timeoutMs)
{
	if (!isInitialized() || length == 0)
		return false;

	uint8_t count = 0;
	const unsigned long start = millis();
	while (timeoutMs == 0 || millis() - start <= timeoutMs)
	{
		if (port_->available() <= 0)
			continue;

		const int value = port_->read();
		if (value < 0)
			continue;

		const uint8_t byteValue = (uint8_t)value;
		if (count != 0)
		{
			buffer[count++] = byteValue;
		}
		else if (byteValue == RESPONSE_HEADER)
		{
			buffer[count++] = byteValue;
		}

		if (count == length)
			return buffer[length - 1] == checksum(buffer, length - 1);
	}

	return false;
}

