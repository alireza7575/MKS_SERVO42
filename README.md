# MKS_SERVO42 Library

The MKS_SERVO42 library controls one or more Nema17 stepper motors using an
MKS SERVO42 driver from an Arduino-compatible board. It provides a small serial
API for checking motor status, reading the current encoder position, and moving
a motor by direction, speed, and pulse count.

## Features

- Control Nema17 stepper motors with MKS SERVO42 drivers
- Address one or more drivers by stepper ID
- Read the current encoder position
- Move a motor by direction, speed, and number of pulses

## Supported Boards

This library uses the Arduino `HardwareSerial` API, so it is intended to work
with Arduino-compatible boards that provide a hardware serial port, including
ESP32-based boards and boards such as the Arduino Giga.

The library has mainly been used with ESP32 so far. Other boards should work,
but may need board-specific serial port selection and wiring.

## Installation

Download this repository and place it in your Arduino `libraries` folder, or use
the Arduino Library Manager to search for `MKS_SERVO42` and install it directly.

## Usage

Include the library at the top of your Arduino sketch:

```cpp
#include <MKS_SERVO42.h>
```

Refer to the example in `examples/Example1_BasicStepper` to get started.

```cpp
#include "MKS_SERVO42.h"

byte const stepperId = 1;
uint8_t const speed = 16;
uint32_t const numberOfPulses = 5000;

MKS_SERVO42 stepper;

void setup() {
  Serial.begin(115200);
  Serial1.begin(38400);
  stepper.initialize(&Serial1);
  stepper.ping(stepperId);
}

byte direction = 1;

void loop() {
  Serial.println(stepper.getCurrentPosition(stepperId));
  direction = direction == 0 ? 1 : 0;
  stepper.setTargetPosition(stepperId, direction, speed, numberOfPulses);
  delay(5000);
}
```

## Multiple Controllers

Yes, one UART or RS232 serial port can control multiple MKS SERVO42C controllers
when each controller is configured with a unique stepper ID. Pass the target ID
as the `stepperId` argument when calling methods such as `ping()`,
`getCurrentPosition()`, or `setTargetPosition()`.

Use the correct electrical interface for your hardware. TTL UART and RS232 are
not the same signal levels, so use an appropriate transceiver or adapter when
connecting an Arduino UART to an RS232 bus.

```cpp
byte const xAxisId = 1;
byte const yAxisId = 2;

stepper.ping(xAxisId);
stepper.ping(yAxisId);

stepper.setTargetPosition(xAxisId, 0, 16, 5000);
stepper.setTargetPosition(yAxisId, 1, 16, 5000);
```

## Contributing

Pull requests and issues are welcome.

## Author

Alireza Ahmadi
