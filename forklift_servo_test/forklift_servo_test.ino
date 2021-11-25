#import <Servo.h>

Servo test;

uint16_t mic = 1400;

void setup() {
  // put your setup code here, to run once:
  test.attach(11);
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:

  test.writeMicroseconds(mic);

  Serial.println(mic);

  mic += 1;

  if (mic >= 1600) {
    mic = 1600;
  }

  delay(50);

}
