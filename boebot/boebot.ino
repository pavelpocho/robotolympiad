// Robotics with the BOE Shield - MovementsWithSimpleFunctions
// Move forward, left, right, then backward for testing and tuning.
#include <Servo.h> // Include servo library
//#include <SoftwareSerial.h>
 
Servo servoLeft; // Declare left and right servos
Servo servoRight;

bool a = false;
bool b = false;
bool c = false;
bool d = false;

byte x0 = 0;
byte x1 = 0;

byte y0 = 0;
byte y1 = 0;

int16_t x = 512;
int16_t y = 512;

int16_t finalX = 512;
int16_t finalY = 512;

int16_t offsetX = 0;
int16_t offsetY = 0;

//SoftwareSerial btSerial = SoftwareSerial(2, 3);
 
void setup() // Built-in initialization block
{ 
  servoLeft.attach(13); // Attach left signal to pin 13
  servoRight.attach(12); // Attach right signal to pin 12
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
      if (Serial.peek() == 'a') {
        Serial.read();
        a = true;
        b = false;
        c = false;
        d = false;
      }
      else if (Serial.peek() == 'b') {
        Serial.read();
        b = true;
        a = false;
        c = false;
        d = false;
      }
      else if (Serial.peek() == 'c') {
        Serial.read();
        c = true;
        b = false;
        a = false;
        d = false;
      }
      else if (Serial.peek() == 'd') {
        Serial.read();
        d = true;
        b = false;
        c = false;
        a = false;
      }
      else {
        Serial.read();
      }
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
    
//    Serial.print("Throttle:");
//    Serial.print(throttle);
//    Serial.print(",Steering:");
//    Serial.println(steering);

    servoLeft.writeMicroseconds(max(1400, min(1600, 1500 + throttle + steering)));
    servoRight.writeMicroseconds(max(1400, min(1600, 1500 + throttle - steering)));
    
  }
  
}
 
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