// Robotics with the BOE Shield - MovementsWithSimpleFunctions
// Move forward, left, right, then backward for testing and tuning.
#include <Arduino.h>
#include <Servo.h> // Include servo library
//#include <SoftwareSerial.h>
 
Servo servoLeft; // Declare left and right servos
Servo servoRight;
Servo servoForklift;

bool a = false;
bool b = false;
bool c = false;
bool d = false;
bool e = false;

uint8_t x0 = 0;
uint8_t x1 = 0;

uint8_t y0 = 0;
uint8_t y1 = 0;

uint8_t btn0 = 0;

int16_t x = 512;
int16_t y = 512;

int16_t finalX = 512;
int16_t finalY = 512;

int16_t offsetX = 0;
int16_t offsetY = 0;

uint8_t finalBtnVal = 0;
bool buttonPressed = false;

bool forkliftDir = false;
unsigned long stopForkliftAt = 0;

//SoftwareSerial btSerial = SoftwareSerial(2, 3);
 
void setup() // Built-in initialization block
{ 
  servoLeft.attach(13); // Attach left signal to pin 13
  servoRight.attach(12); // Attach right signal to pin 12
  servoForklift.attach(11); // Is forklift on pin 11? Probably? okee
  servoForklift.writeMicroseconds(1495);
  Serial.begin(115200);
//  btSerial.begin(115200);
  
} 
 
void loop() // Main loop auto-repeats
{
  
//  while (Serial.available()) {
//    delay(1);
//    btSerial.write(Serial.read());
//  }
//  delay(500);
//  while (btSerial.available()) {
//    Serial.write(btSerial.read());
//  }
  if (Serial.available()) {
    while (Serial.available()) {
      if (a) {
        x0 = Serial.read();
        x = x0 | (x1 << 8);
        a = false;
      }
      if (b) {
        x1 = Serial.read();
        x = x0 | (x1 << 8);
        b = false;
      }
      if (c) {
        y0 = Serial.read();
        y = y0 | (y1 << 8);
        c = false;
      }
      if (d) {
        y1 = Serial.read();
        y = y0 | (y1 << 8);
        d = false;
      }
      if (e) {
        btn0 = Serial.read();
        e = false;
      }
      if (Serial.peek() == 'a') {
        Serial.read();
        a = true;
        b = false;
        c = false;
        d = false;
        e = false;
      }
      else if (Serial.peek() == 'b') {
        Serial.read();
        b = true;
        a = false;
        c = false;
        d = false;
        e = false;
      }
      else if (Serial.peek() == 'c') {
        Serial.read();
        c = true;
        b = false;
        a = false;
        d = false;
        e = false;
      }
      else if (Serial.peek() == 'd') {
        Serial.read();
        d = true;
        b = false;
        c = false;
        a = false;
        e = false;
      }
      else if (Serial.peek() == 'e') {
        Serial.read();
        d = false;
        b = false;
        c = false;
        a = false;
        e = true;
      }
      else {
        Serial.read();
      }
    }

    // Button:

    finalBtnVal = (uint8_t)((float)(finalBtnVal) * 0.9f + (float)btn0 * 0.1f);
    buttonPressed = finalBtnVal < 128;

    // CAUTION: Stationary point for the forklift servo is 1495!

    // Replace this with actual logic
    if (buttonPressed && stopForkliftAt < millis()) {
      stopForkliftAt = millis() + 2000;
      forkliftDir = !forkliftDir;
    }
    if (stopForkliftAt > millis()) {
      if (forkliftDir) {
        servoForklift.writeMicroseconds(1600); 
      }
      else {
        servoForklift.writeMicroseconds(1400); 
      }
    }
    else {
      servoForklift.writeMicroseconds(1495);
    }
    

    // Need to map it so that 1000 is 1600 and 0 is 1400 for Y
    // 1000 x means one is 1600, one is 1400

    if (x < 0 || x > 1024 || y < 0 || y > 1024) {
      return;
    }
    
    finalX = (int16_t)((float)finalX * 0.5f + (float)x * 0.5f);
    finalY = (int16_t)((float)finalY * 0.5f + (float)y * 0.5f);
    
    if (finalX >= 496 && finalX <= 525) {
      finalX = 512;
    }
    if (finalY >= 496 && finalY <= 525) {
      finalY = 512;
    }

    float xF = (float)finalX;
    float yF = (float)finalY;

    float throttle = (float)(yF) / 1024.0f * 200.0f - 100.0f;
    float steering = (float)(xF) / 1024.0f * 200.0f - 100.0f;

    servoLeft.writeMicroseconds(max(1400, min(1600, 1500 + throttle + steering)));
    servoRight.writeMicroseconds(max(1400, min(1600, 1500 + throttle - steering)));
    
  }
  
}

// Original moving functions
 
void forward(int time) // Forward function
{ 
 servoLeft.writeMicroseconds(1550); // Left wheel counterclockwise
 servoRight.writeMicroseconds(1450); // Right wheel clockwise
 delay(time); // Maneuver for time ms
 servoLeft.writeMicroseconds(2000); // Left wheel counterclockwise
 servoRight.writeMicroseconds(1000); // Right wheel clockwise
 delay(time); // Maneuver for time ms
} 
void turnLeft(int time) // Left turn function
{ 
 servoLeft.writeMicroseconds(1000); // Left wheel clockwise
 servoRight.writeMicroseconds(1000); // Right wheel clockwise
 delay(time); // Maneuver for time ms
} 
void turnRight(int time) // Right turn function
{ 
 servoLeft.writeMicroseconds(2000); // Left wheel counterclockwise
 servoRight.writeMicroseconds(2000); // Right wheel counterclockwise
 delay(time); // Maneuver for time ms
} 
void backward(int time) // Backward function
{ 
 servoLeft.writeMicroseconds(1450); // Left wheel clockwise
 servoRight.writeMicroseconds(1550); // Right wheel counterclockwise
 delay(time); // Maneuver for time ms
 servoLeft.writeMicroseconds(1000); // Left wheel clockwise
 servoRight.writeMicroseconds(2000); // Right wheel counterclockwise
 delay(time); // Maneuver for time ms
} 
void disableServos() // Halt servo signals
{ 
 servoLeft.detach(); // Stop sending servo signals
 servoRight.detach();
} 
