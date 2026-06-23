#ifndef MKS_SERVO_D_h
#define MKS_SERVO_D_h

#include <Arduino.h>

class MKS_SERVO_D
{
public:
	enum AckStatus : uint8_t
	{
		ACK_FAILED = 0,
		ACK_STARTED = 1,
		ACK_COMPLETED = 2
	};

	void initialize(HardwareSerial *serialPort = nullptr, long const &baudRate = 38400);

	bool readPosition(uint8_t slaveAddress, int32_t &position);
	bool readSpeed(uint8_t slaveAddress, int16_t &speedRpm);
	AckStatus runSpeedMode(uint8_t slaveAddress, uint8_t direction, uint16_t speedRpm, uint8_t acceleration);
	AckStatus runPositionMode1(uint8_t slaveAddress, uint8_t direction, uint16_t speedRpm, uint8_t acceleration, uint32_t pulses, uint32_t timeoutMs = 3000);
	AckStatus waitForAck(uint32_t timeoutMs = 3000);

private:
	static const uint8_t REQUEST_HEADER = 0xFA;
	static const uint8_t RESPONSE_HEADER = 0xFB;
	static const uint8_t READ_POSITION = 0x31;
	static const uint8_t READ_SPEED = 0x32;
	static const uint8_t SPEED_MODE = 0xF6;
	static const uint8_t POSITION_MODE_1 = 0xFD;

	HardwareSerial *port_ = nullptr;

	bool isInitialized();
	void flushInput();
	uint8_t checksum(const uint8_t *buffer, uint8_t length);
	bool writeFrame(const uint8_t *frame, size_t length);
	bool readFrame(uint8_t *buffer, uint8_t length, uint32_t timeoutMs = 3000);
};

#endif

