
// NOTE: THERE ARE THREE THINGS TO DO AND CHECK IN HERE!!

#include <Arduino.h>
#include <Servo.h> // Include servo library
// #include <SoftwareSerial.h> // For when software serial is used

// Number of unique data points transmitted from remote control
#define N_IDENTIFIERS 7

// Declare left, right and forklift servos
Servo servoLeft;
Servo servoRight;
Servo servoForklift;

// All arrays initialized to [.., 0, ..] because they are global

// availChecks are used to record which datapoint will come next
// so that we can loop around and check if the datapoint
// has already arrived. Without this, we would be reading
// things that haven't arrived yet.
bool availChecks[N_IDENTIFIERS];

// dataInit keeps track of which datapoints were already updated
// so that the robot doesn't do anything crazy when being turned on
// until useful data has come through from the remote
bool dataInit[N_IDENTIFIERS];

// Once everything is set, save it to speed up later checks
bool allSet;

// someAvailable just helps to not check for new identifiers when
// one is already available
bool someAvailable = false;

// identifiers contains incoming datapoint identifiers (a,b,c...)
char identifiers[N_IDENTIFIERS] = {'a', 'b', 'c', 'd', 'e', 'f', 'g'};

// vals saves the incoming values as they are read
// they are then redistributed into the right variables later
byte vals[N_IDENTIFIERS];

// 
uint8_t rawButton0 = 0;
uint8_t rawButton1 = 0;
uint8_t rawButton2 = 0;
int16_t rawX = 512;
int16_t rawY = 512;

int16_t x = 512;
int16_t y = 512;

uint8_t button0 = 0;
bool prevButton0Pressed = false;
bool button0Pressed = false;
uint8_t button1 = 0;
bool button1Pressed = false;
uint8_t button2 = 0;
bool button2Pressed = false;

uint8_t topStopSwitchPin = 2;
uint8_t bottomStopSwitchPin = 3;
uint8_t topStopSwitch = 0;
uint8_t bottomStopSwitch = 0;

bool precisionMode = false;

// SoftwareSerial for testing if hardware serial needs to be used for logging
// It's not fast enough for actual usage, also uses pins we need
// SoftwareSerial btSerial = SoftwareSerial(2, 3);

// Setup everything that needs to happen once
void setup()
{ 
  // Set the stop switch pins to input
  pinMode(topStopSwitchPin, INPUT);
  pinMode(bottomStopSwitchPin, INPUT);
  
  // Attach servos
  servoForklift.attach(11); // Attach forklift signal to pin 11
  servoRight.attach(12); // Right signal to pin 12
  servoLeft.attach(13); // Left signal - pin 13
  
  // Initialize forklift servo to 1495, which is the mid-value and makes it stop
  servoForklift.writeMicroseconds(1495);
  
  // Start the serial comms
  Serial.begin(115200);
  // btSerial.begin(115200);
  
} 

// This literally runs in circles as fast as it can
void loop()
{
  
  // Read the incoming Bluetooth data
  readBTData();

  // If not all data is initialized, return and wait for more data
  if (notAllDataIsInitialized()) return;

  // Save the raw data into specific global variables
  saveRawData();

  // ------------------------------------------------------------------
  // At this point, all data from the remote must be up-to-date
  // ------------------------------------------------------------------
  
  // Update all button values to see if they are pressed
  updateButtonValues();

  // Update the precision mode
  updatePrecisionMode();

  // Update the stop switch values 
  getStopSwitchValues();

  // Update the forklift servo based on button values
  controlForklift();

  // Update joystick values
  updateJoystickValues();

  // Update the motor servos based on joystick value
  controlMotors();
  
}

void readBTData() {

  // The loop goes while serial data is available
  // Each time, we first check if the last transmission
  // was some identifier and we should expect a value to come in
  
  // Otherwise, we check if the next thing coming in is an
  // identifier

  // The purpose of this is to always check if data is available
  // before attempting to read it

  while (Serial.available()) {
    if (someAvailable) {
      // Some data should have come, check what it is
      for (uint8_t i = 0; i < N_IDENTIFIERS; i++) {
        if (availChecks[i]) {
          // Save data, reset the checks and confirm data as initialized
          vals[i] = Serial.read();
          availChecks[i] = false;
          someAvailable = false;
          dataInit[i] = true;
        }
      } 
    }
    else {
      // No data expected, check for next identifier
      char n_id = Serial.read();
      for (uint8_t i = 0; i < N_IDENTIFIERS; i++) {
        if (n_id == identifiers[i]) {
          // Remember which data is about to come in
          availChecks[i] = true;
          someAvailable = true;
        }
      }
    }
  }
  
}

bool notAllDataIsInitialized() {
  
  // Check if all data has been updated at least once
  // If it hasn't, indicate this so that loop can go again
  if (!allSet) {
    bool breakOut = false;
    for (uint8_t i = 0; i < N_IDENTIFIERS; i++) {
      if (!dataInit[i]) {
        breakOut = true;
        break;
      }
    }
    // If everything is set, remember this, so that we
    // don't need to check again
    if (!breakOut) { allSet = true; }
    return breakOut;
  }
  return false;
  
}

void saveRawData() {
  
  // Set the specific variables from the vals array
  // What goes where is determined on the remote side
  rawX = vals[0] | (vals[1] << 8);
  rawY = vals[2] | (vals[3] << 8);
  rawButton0 = vals[4];
  rawButton1 = vals[5];
  rawButton2 = vals[6];
  
}

void updateButtonValues() {
  
  // Update button values using memory efficient low-pass filters, such
  // that noise does trigger them accidentally
  // except for button0, thats handeled on the remote
  button0 = rawButton0;
  button1 = (uint8_t)((float)(button1) * 0.9f + (float)rawButton1 * 0.1f);
  button2 = (uint8_t)((float)(button2) * 0.9f + (float)rawButton2 * 0.1f);
  
  // Save the booleans of whether each button was pressed and also if button0 was
  // previously pressed
  prevButton0Pressed = button0Pressed;
  button0Pressed = button0 >= 128;
  button1Pressed = button1 >= 128;
  button2Pressed = button2 >= 128;
  
}

void updatePrecisionMode() {

  // If button0 was previously not pressed, but now is, change precision mode
  precisionMode = (button0Pressed && !prevButton0Pressed) ? !precisionMode : precisionMode;
  
}

void getStopSwitchValues() {

  // Set the digital stop switch values
  topStopSwitch = digitalRead(topStopSwitchPin);
  bottomStopSwitch = digitalRead(bottomStopSwitchPin);
  
}

void controlForklift() {

  // Check which button is pressed and if stop switches are activated
  if (button2Pressed && !button1Pressed && !topStopSwitch) {
    servoForklift.writeMicroseconds(1550);
  }
  else if (button1Pressed && !button2Pressed && !bottomStopSwitch) {
    servoForklift.writeMicroseconds(1440);
  }
  else {
    // Stationary point for the forklift servo is 1495
    servoForklift.writeMicroseconds(1495);
  }
  
}

void updateJoystickValues() {

  // Ignore this dataset if any value if out of bounds
  if (rawX < 0 || rawX > 1024 || rawY < 0 || rawY > 1024) return;

  // Update reasonable joystick values using memory-efficient low-pass filters
  // This is less agressive because this data is already filtered
  // on the remote side
  x = (int16_t)((float)x * 0.5f + (float)rawX * 0.5f);
  y = (int16_t)((float)y * 0.5f + (float)rawY * 0.5f);

  // If values are close to mid-point, treat them as if they
  // were exactly in the middle. This creates a dead zone, but makes
  // sure that if the joystick is released, the robot stops
  if (x >= 490 && x <= 534) x = 512;
  if (y >= 490 && y <= 534) y = 512;
  
}

void controlMotors() {

  // Convert x and y joystick data into throttle and steering input
  float throttle = (float)(x) / 1024.0f * 200.0f - 100.0f;
  float steering = (float)(y) / 1024.0f * 200.0f - 100.0f;

  // Set the power level of the servo motors according to throttle and steering values
  servoLeft.writeMicroseconds(max(1400, min(1600, 1500 + ((throttle - steering) * (precisionMode ? 0.3f : 1.0f)))));
  servoRight.writeMicroseconds(max(1400, min(1600, 1500 + ((throttle + steering) * (precisionMode ? 0.3f : 1.0f)))));

}
