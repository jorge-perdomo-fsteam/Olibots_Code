#include <Wire.h>

const int i2c_dir = 8;

void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
Wire.begin();

Serial.print("I2C started!");
}

void loop() {
  int numero = random(1, 10);
  Serial.println(numero);
  if(numero % 2 == 0){
    Serial.println("par!");
    Wire.beginTransmission(i2c_dir);
    Wire.write(1);
    Wire.endTransmission();
  }
  else if(numero % 2 != 0){
    Serial.println("impar!");
    Wire.beginTransmission(i2c_dir);
    Wire.write(0);
    Wire.endTransmission();
  } 
  delay(2000);
}
