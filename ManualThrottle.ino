#include <StateMachine.h>

// Set to 0 to disable serial output to IDE's Serial Monitor or Serial Plotter tools.
#define LOGGING 1

// Begin TUNING PARAMETERS

// Analog input returns value between 0 and 1023.
// When in eStopped state, any input above START_THRESHOLD will change
// state to eStarting.  
// When in eRunning state, any input below STOP_THRESHOLD will change
// state to eStopped.
// Having two thresholds creates some hysteresis in the triggering logic
// to avoid thrashing back and forth between eStopped and eRunning states.
// When running, output will be scaled linearly from input.
#define START_THRESHOLD 12
#define STOP_THRESHOLD 8

// Analog input is read every this many milliseconds.
#define UPDATE_PERIOD 10

// When starting, spike output to this value (up to 255).
#define STARTING_OUTPUT 127

// When starting, spike output for this many milliseconds.
// Actual time will round up to a multiple of UPDATE_PERIOD,
// so if UPDATE_PERIOD is 10, a STARTING_TIME of 15 will be
// treated as 20 milliseconds.
#define STARTING_TIME 30

// End TUNING PARAMETERS

// Arduino-specific values for Arduino UNO and most Arduino-compatible boards
const byte PWM_MAX = 255;           // maximum value allowed by analogWrite()
const unsigned int ADC_MAX = 1023;  // maximum value returned by analogRead()

// Finite state machine (FSM) to handle state transitions based on analog input
// and Arduino's millisecond clock.  FSM can only be in one of three states:
// - sStopped
// - eStarting
// - eRunning
// A Throttle object IS A StateMachine object, so it has all the data
// members and methods of a StateMachine.  Here we add more data members
// to the class, and more code to the class's update() method.
class Throttle : public StateMachine {

  private:
  
    // Data members, intialized by Constructor method
    const int m_analogIn;             // analog input (ADC) pin
    const int m_analogOut;            // analog output (PWM) pin
    unsigned long m_nextTime;         // used in timing eStarting state
  
    // Declare the FSM states and define the variable for the
    // current state at any time.
    enum E_STATE {
      eStopped, eStarting, eRunning
    } m_state;                        // current state

    // Calculate output value for PWM pin from ADC input.
    // Input value less than STOP_THRESHOLD will return zero.
    // This method is private to Throttle class, cannot be called
    // from outside this class.
    byte calc_output(const unsigned int inval) {
      byte output;
      // 'input' must be in range 0 to 1023.
      const unsigned int input = constrain(inval, 0, ADC_MAX);
      if (input < STOP_THRESHOLD) {
        output = (byte) 0;
      } else {
        // 'x' will be in range 0.0 to 1.0.
        float x = (float) (input - STOP_THRESHOLD)
                / (float) (ADC_MAX - STOP_THRESHOLD);
        // 'output' will be in range 1 to 255.
        output = (byte) max(1.0, ((float) PWM_MAX * x));
      }
      return output;
    }

  public:
  
    // Constructor, requires input and output pin numbers.
    // This is where a Throttle object is initialized when
    // it is created.
    Throttle(const int anIn, const int anOut) :
    StateMachine(UPDATE_PERIOD, true),      // set timing period, hard real-time mode
    m_analogIn(anIn), m_analogOut(anOut),   // save input and output pin numbers
    m_nextTime(0L),                         // used when in Starting state
    m_state(eStopped)                       // initially in Stopped state
    {
      // Set input and output pins' modes.
      pinMode(m_analogIn, INPUT);
      pinMode(m_analogOut, OUTPUT);
      analogWrite(m_analogOut, 0);
    }

    // Update the finite state machine, called upon every pass
    // through loop() function.
    // Returns true if it was time to update the FSM state,
    // false otherwise.
    virtual bool update() {
      
      // Base class object checks if it's time to update this object,
      // happens once every UPDATE_PERIOD milliseconds.
      if (StateMachine::update()) {
        
        // Read the analog input pin, return value between 0 and 1023.
        const unsigned int analogIn = analogRead(m_analogIn);
        // Define variable for computed analog output value (0 to 255).
        byte analogOut = 0;
        
        // Check conditions, based upon current FSM state.
        if (m_state == eStopped) {
          
          // If analog input just exceeded start threshold, set state to
          // Starting, set output value to STARTING_OUTPUT, and mark time
          // when Starting state should end.
          if (analogIn > START_THRESHOLD) {
            analogOut = (byte) STARTING_OUTPUT;
            m_nextTime = millis() + STARTING_TIME;
            m_state = eStarting;
          }
          
        } else if (m_state == eStarting) {
          
          // Input below lower threshold?
          if (analogIn < STOP_THRESHOLD) {
            // If so, go to Stopped state.
            analogOut = (byte) 0;
            m_state = eStopped;
          } else {
            // Otherwise, is it time to go to Running state?
            if (millis() >= m_nextTime) {
              // Yes, do so and set output based on input.
              m_state = eRunning;
              analogOut = calc_output(analogIn);
            } else {
              // No, hold output spike.
              analogOut = (byte) STARTING_OUTPUT;
            }
          }
          
        } else if (m_state == eRunning) {
          
          // Input below lower threshold?
          if (analogIn < STOP_THRESHOLD) {
            // If so, go to Stopped state.
            analogOut = (byte) 0;
            m_state = eStopped;
          } else {
            // Otherwise, continue setting output based on input.
            analogOut = calc_output(analogIn);
          }
          
        }
        // Now write the output to the PWM pin.
        analogWrite(m_analogOut, analogOut);
        
#if LOGGING
        // Input is 10-bit value and output is 8-bit value,
        // so scale both to between 0.0 and 1.0.
        // First value switches between 0.4, 0.5, 0.6 based on current state.
        // Second and third values keep Serial Plotter from rescaling.
        Serial.print((int) m_state * 0.1 + 0.4);
        Serial.print(' ');
        Serial.print(0.0);
        Serial.print(' ');
        Serial.print(1.0);
        Serial.print(' ');
        Serial.print((float) analogIn / (float) ADC_MAX, 4);
        Serial.print(' ');
        Serial.println((float) analogOut / (float) PWM_MAX, 4);
#endif
        
        // Has updated.
        return true;
        
      }
      
      // Not time to update this time.
      return false;
      
    }
  
};

// Define actual instance of Throttle class, specifying which analog input
// (ADC) pin and analog output (PWM) pin will be used.
// For Arduino UNO:
// - analog IN is any ADC input, one of { A0, A1, A2, A3, A4, A5 }.
// - analog OUT is any PWM output, one of { 3, 5, 6, 9, 10, 11 }.
Throttle thr(A0, 3);

void setup() {
  
#if LOGGING
  Serial.begin(115200);
#endif
  
}

void loop() {
  
  // Repeatedly try to update the Throttle object.
  // It will take action and return true at a fixed rate determined
  // by UPDATE_PERIOD.  Returns false every other time without taking
  // any action.
  thr.update();
  
}
