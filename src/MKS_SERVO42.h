#ifndef MKS_SERVO42_h
#define MKS_SERVO42_h

#include <Arduino.h>


namespace instruction
{
	byte const GET_ENCODER_POS = 0x30;
	byte const STEPPER_PING = 0x3A;
};

class MKS_SERVO42
{
public:

	void initialize(HardwareSerial *serialPort = nullptr, long const &baudRate = 38400);
	bool ping(byte const &stepperId);
	int getCurrentPosition(byte const &stepperId);
	bool isMoving(byte const &stepperId);
	bool setTargetPosition(byte const &stepperId, byte const &dir, int const &speed, byte const &acc, uint32_t const &pulses);

private:

	HardwareSerial *port_;
	byte const HEADER = 0xFA;

	int sendMessage(byte stepperId, byte const &commandID);
	int reciveStepperStatus();
	int recieveEncoderPosition();
	byte calculateChecksum(const byte *message, int length);
};




#endif
