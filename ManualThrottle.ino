#include <StateMachine.h>

#define LOGGING 1

// Begin TUNING PARAMETERS

// Analog input returns value between 0 and 1023.
// When stopped, any input above START_THRESHOLD will change
// state to Starting.  Input below STOP_THRESHOLD will change
// state to Stopped.  Having two thresholds creates some
// hysteresis in the triggering logic.
// When running, output will be scaled linearly from input.
#define START_THRESHOLD 12
#define STOP_THRESHOLD 8

// Analog input is read every this many milliseconds.
#define UPDATE_PERIOD 10

// When starting, spike output to this value (up to 255).
#define STARTING_OUTPUT 127

// When starting, spike output for this many milliseconds.
// Actual time will round up to a multiple of UPDATE_RATE,
// so if UPDATE_RATE is 10, a STARTING_TIME of 15 will be
// rounded up to 20.
#define STARTING_TIME 30

// End TUNING PARAMETERS

class Throttle : public StateMachine {

  private:
    const int m_analogIn;
    const int m_analogOut;
    unsigned long m_nextTime;
    enum E_STATE {
      eStopped, eStarting, eRunning
    } m_state;

    byte calc_output(const unsigned int input) {
      byte output;
      // 'input' is in range 0 to 1023
      if (input < STOP_THRESHOLD) {
        output = (byte) 0;
      } else {
        // 'x' will be in range 0.0 to 1.0.
        float x = (float) (input - STOP_THRESHOLD)
                / (float) (1023 - STOP_THRESHOLD);
        // 'output' will be in range 1 to 255.
        output = (byte) max(1.0, (255.0 * x));
      }
      return output;
    }

  public:
    Throttle(const int anIn, const int anOut) :
    StateMachine(UPDATE_PERIOD, true),
    m_analogIn(anIn), m_analogOut(anOut),
    m_nextTime(0L),
    m_state(eStopped)
    {
      pinMode(m_analogIn, INPUT);
      pinMode(m_analogOut, OUTPUT);
    }

    virtual bool update() {
      if (StateMachine::update()) {
        const unsigned int analogIn = analogRead(m_analogIn);
        byte analogOut = 0;
        if (m_state == eStopped) {
          if (analogIn > START_THRESHOLD) {
            analogOut = (byte) STARTING_OUTPUT;
            m_nextTime = millis() + STARTING_TIME;
            m_state = eStarting;
          }
        } else if (m_state == eStarting) {
          if (analogIn < STOP_THRESHOLD) {
            analogOut = (byte) 0;
            m_state = eStopped;
          } else {
            if (millis() >= m_nextTime) {
              m_state = eRunning;
              analogOut = calc_output(analogIn);
            } else {
              analogOut = (byte) STARTING_OUTPUT;
            }
          }
        } else if (m_state == eRunning) {
          if (analogIn < STOP_THRESHOLD) {
            analogOut = (byte) 0;
            m_state = eStopped;
          } else {
            analogOut = calc_output(analogIn);
          }
        }
        analogWrite(m_analogOut, analogOut);
#if LOGGING        
        Serial.print((int) m_state * 0.1 + 0.4);
        Serial.print(' ');
        Serial.print(0.0);
        Serial.print(' ');
        Serial.print(1.0);
        Serial.print(' ');
        Serial.print((float) analogIn / 1023.0, 4);
        Serial.print(' ');
        Serial.println((float) analogOut / 255.0, 4);
#endif
        return true;
      }
      return false;
    }
  
};

// For Arduino UNO:
// - analog IN is A0 through A5.
// - analog OUT is any PWM output, such as 3, 5, 6, 9, 10, or 11.

Throttle thr(A0, 3);

void setup() {
#if LOGGING
  Serial.begin(115200);
#endif
}

void loop() {
  thr.update();
}
