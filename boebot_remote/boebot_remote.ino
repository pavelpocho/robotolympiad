#include <Arduino.h>
// #include <AltSoftSerial.h> // For when software serial is used

// Number of unique data points transmitted from remote control
#define N_IDENTIFIERS 7

// sendInterval is how often (in ms) Bluetooth data is sent
unsigned long sendInterval = 25;

// Keep track of the last time Bluetooth data was sent
unsigned long lastSend = 0;

// This number is decremented on each cycle and when it's 0,
// the system is assumed to have settled and the offset of the
// joystick read is saved (we assume the joystick is at middle position)
uint8_t cyclesUntilCalibration = 100;

// identifiers saves the ids that are sent before each
// data point
char identifiers[N_IDENTIFIERS] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

// Raw (no calibration applied) joystick values need to be stored
// due to design
int16_t rawJoystickX = 512;
int16_t rawJoystickY = 512;
int16_t button0analog = 512;

// Button values
byte button0 = 0;
byte button1 = 0;
byte button2 = 0;

// Joystick values after calibration offset applied
int16_t joystickX = 512;
int16_t joystickY = 512;

// Joystick calibration offsets
int16_t offsetX = 0;
int16_t offsetY = 0;

// SoftwareSerial for testing if hardware serial needs to be used for logging
// It's not fast enough for actual usage, also uses pins we need
// AltSoftSerial btSerial;
 
void setup()
{
  // Set pinmodes
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  
  // Start serial comms
  Serial.begin(115200);
  // btSerial.begin(115200);
  
} 


void loop()
{

  // Check if enough time has elapsed since the last
  // time data was sent
  if (lastSend + sendInterval >= millis()) return;
  lastSend = millis();

  // Wait for 100 loops until we record the joystick
  // reading offset, don't continue if not calibrated
  if (!doAndCheckIfCalibrated) return;

  // Retrieve the joystick values and clean them up
  getCalibratedJoystickValues();

  // Check if we should treat the joystick position as centered
  checkForJoystickDeadspace();

  // Get the button values as a digital signal
  getButtonValues();

  // Combine the data into an array and send
  combineAndSendData();
  
}

bool doAndCheckIfCalibrated() {

  // Check if we are still waiting for calibration
  // While we wait, we return false to indicate the
  // system isn't calibrated
  // Also calibrate the system if it's time
  
  if (cyclesUntilCalibration > 0) {
    cyclesUntilCalibration -= 1;
    return false;
  }
  else if (cyclesUntilCalibration == 0) {
    offsetX = rawJoystickX - 512;
    offsetY = rawJoystickY - 512;
  }
  return true;
  
}

void getCalibratedJoystickValues() {

  // Read the raw joystick values and apply a memory-efficient low-pass filter
  // Note: There is another low-pass filter on the robot side, which is why
  // this one is not very aggressive
  rawJoystickX = (int16_t)((float)rawJoystickX * 0.75f + (float)analogRead(A0) * 0.25f);
  rawJoystickY = (int16_t)((float)rawJoystickY * 0.75f + (float)analogRead(A1) * 0.25f);
  
  // Next, applied the offsets we got when calibrating the system, this helps
  // to make sure the robot stops when the joystick is released
  // This needs to be done separately so that the raw values can be used in the filter
  // on the next loop
  joystickX = rawJoystickX - offsetX;
  joystickY = rawJoystickY - offsetY;
  
}

void getButtonValues() {

  // This is not cleaned up, that is dealt with on the robot side, where we have
  // more processing power
  // TODO: Fix this mess. Setting a byte value to 255 makes it return some garbage
  // when read because a byte can't have a value of 255. This does not break anything
  // because it's read as a uint8_t on the other side, but still...

  // button0 is the joystick button, which outputs about 0.5V when pressed,
  // no actually it does whatever the hell it wants to. Its literally different
  // every time I log it to fix it because it always breaks.
  // This time around its just following the x axis of the joystick and oscillates
  // around it in some weird ways. Once I fix this and it inevitably breaks again,
  // that will be it, because it will just follow the damn joystick precisely.
  // "Quality second hand parts" my ass, this shit doesnt work in the slightest
  // so it's read as an analog signal and the threshold is set to 50 / 1024
  // yeah yeah threshold whatever. it has to have a low pass filter now, so
  // if somebody presses it very rapidly, it wont activate. nice.
  // it also has to be limited to when the joystick is centered, because it's getting
  // worse and worse and every time I fix it, it breaks the next day again.
  
  button0analog = (button0analog * 0.2 + analogRead(A2) * 0.8);
  button0 = (button0analog == 0 && joystickX == 512 && joystickY == 512) ? 255 : 0;
  // One of these other two is open when pressed, the other is closed when pressed
  button1 = digitalRead(2) ? 255 : 0;
  button2 = digitalRead(3) ? 255 : 0;
  
}

void checkForJoystickDeadspace() {
  
  // If values are close to mid-point, treat them as if they
  // were exactly in the middle. This creates a dead zone, but makes
  // sure that if the joystick is released, the robot stops
  // This code is repeated on the robot side to really make sure
  // the robot stops if we ask it to do that
  
  if (joystickX >= 496 && joystickX <= 525) joystickX = 512;
  if (joystickY >= 496 && joystickY <= 525) joystickY = 512;
  
}

void combineAndSendData() {

  // vals saves the outgoing values as they are created
  // they are then sent all at once

  // Save all data into an array
  byte vals[N_IDENTIFIERS] = { 
    (byte)(joystickX), (byte)(joystickX >> 8),
    (byte)(joystickY), (byte)(joystickY >> 8),
    button0, button1, button2
  };

  // Send each identifier (a,b,c,...) followed
  // by the value
  for (int i = 0; i < N_IDENTIFIERS; i++) {
    Serial.print(identifiers[i]);
    Serial.write(vals[i]);
  }
  
}
 
