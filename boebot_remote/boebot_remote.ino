// Robotics with the BOE Shield - MovementsWithSimpleFunctions
// Move forward, left, right, then backward for testing and tuning.
//#include <SoftwareSerial.h>
#include <Arduino.h>

unsigned long lastSend = 0;
unsigned long sendInterval = 25;

float calibrationDoneF = 0.0f;

int16_t x = 512;
int16_t y = 512;

int16_t finalX = 512;
int16_t finalY = 512;

int16_t offsetX = 0;
int16_t offsetY = 0;

//SoftwareSerial btSerial = SoftwareSerial(2, 3);
 
void setup()
{
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
  Serial.begin(115200);
//  btSerial.begin(115200);
  
} 
 
void loop() // Main loop auto-repeats
{

  if (lastSend + sendInterval < millis()) {
    lastSend = millis();

    if (calibrationDoneF < 1.0f) {
      offsetX = x - 512;
      offsetY = y - 512;
      calibrationDoneF += 0.01f;
    }
    
    x = (int16_t)((float)x * 0.75f + (float)analogRead(A0) * 0.25f);
    y = (int16_t)((float)y * 0.75f + (float)analogRead(A1) * 0.25f);

    finalX = x - offsetX;
    finalY = y - offsetY;

    if (finalX >= 496 && finalX <= 525) {
      finalX = 512;
    }
    if (finalY >= 496 && finalY <= 525) {
      finalY = 512;
    }
    
    byte x0 = (byte)(finalX);
    byte x1 = (byte)(finalX >> 8);
    
    byte y0 = (byte)(finalY);
    byte y1 = (byte)(finalY >> 8);

    Serial.print('a');
    Serial.write(x0);
    Serial.print('b');
    Serial.write(x1);
    Serial.print('c');
    Serial.write(y0);
    Serial.print('d');
    Serial.write(y1);
    Serial.println("");    
  }
  
//  while (Serial.available()) {
//    delay(1);
//    btSerial.write(Serial.read());
//  }
//  delay(500);
//  while (btSerial.available()) {
//    Serial.write(btSerial.read());
//  }
}
 
