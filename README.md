# ManualThrottle

**Simple example of controlling a DC motor with a kickstart pulse to overcome static friction.**

This Arduino sketch is written as a test to make a simple manual throttle to drive a model train locomotive with an Arduino's PWM (pulse-width modulation) output.  The output's effective output voltage is controlled by varying the voltage on one of the Arduino's analog input pins, say with a potentiometer connected between the Arduino's 5-volt output pin and its ground, with the pot's wiper pin connected to the analog input.  The output PWM will need to be connected to some type of driver circuit to power the train motor's higher voltage and current needs.

The main feature of this sketch is the output of a "kickstart" pulse when trying to start the motor moving slowly from a full stop.  Motors and the moving parts connected to them need to overcome "stiction" (static friction) to get them moving.  Once the moving parts are moving, it takes less energy to keep them moving, even if slowly.  The source file includes "tuning" parameters near the top so the length (milliseconds) and output level (0 to full-scale 255) of the kickstart pulse can be adjusted during testing with an actual motor.  After the kickstart pulse finishes, the output PWM will track linearly with the voltage presented on the analog input pin.

The source code also includes a `#define LOGGING 1` line which enables output on the Arduino's Serial port, which can be monitored with the IDE's "Serial Monitor" or "Serial Plotter" tools during the tuning process.  Changing the LOGGING symbol from 1 to 0 will disable this output for actual use of the throttle.

### Dependencies

This sketch requires my *StateMachine* library, available from GitHub here.

- [github.com/twrackers/StateMachine-library](https://github.com/twrackers/StateMachine-library)

----------

*Author: Thomas W Rackers*

*Contact: twrackers@gmail.com*

*Written on: April 17, 2023*
