#include <Servo.h>
#include <Wire.h>
const int delay_uni = 200;
//I2C:
const int i2c_dir = 8;
const int BUFFER_SIZE = 1; // Tamaño del buffer para almacenar los datos recibidos
int receivedData[BUFFER_SIZE];
bool go = false;

//Ultrasonic pins:
const int ultra_R_echo = 7;
const int ultra_R_trig = 8;

const int ultra_F_echo = 9;
const int ultra_F_trig = 10;

const int ultra_L_echo = 11;
const int ultra_L_trig = 12;

const int ultra_B_echo = 13;
const int ultra_B_trig = 14;

unsigned long time_a, time_b, time_c, time_d;

//Motor pins:
const int IN1 = 2;  //AIN1
const int ENA = 3;  // PWMA
const int IN2 = 4;  //AIN2

//Steering servo pin:
int max_R_angle = 130; //max giro R
int max_L_angle = 60; //max giro L
Servo myservo;
const int servo_pin = 6;


void setup() {
  //Comunication setup:
  Serial.begin(115200);
  Wire.begin(i2c_dir);
  Wire.onReceive(receiveEvent);
  pinMode(5, INPUT);

  //motor pin setup:
  for(int i = 2; i<5; i++){
    pinMode(i, OUTPUT);
  }

  //ultrasonic pins setup:
  for (int i = 7; i < 15; i++){
    if(i % 2 == 0) pinMode(i, OUTPUT);
    else pinMode(i, INPUT);
  }  

  //Steering servo setup:
  myservo.attach(servo_pin);
  myservo.write(80);
  delay(500);
}

void loop() {
  while(go == false){
    if(digitalRead(5)) go = true;
    delay(100);
  }

  float dis_F_inicial = medir_distancia('F');
  float dis_R_inicial = medir_distancia('R');
  float dis_L_inicial = medir_distancia('L');
  float dis_B_inicial = medir_distancia('B');

  for(int i=0; i<4; i++){
  acelerar_recto(80, 120);
  auto_giro(100);
  }
  estacionarse(dis_F_inicial, dis_B_inicial);
  delay(500);
  go = false;
}

//Funciones:
void acelerar_recto(float umbral, int v){
  float dis_F_actual = medir_distancia('F');
  float dis_R_actual = medir_distancia('R');
  float dis_L_actual = medir_distancia('L');

  float dis_R_tem, dis_L_tem;

  while(dis_F_actual > umbral){
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, v);
    delay(20);
    dis_R_tem = medir_distancia('R');
    dis_L_tem = medir_distancia('L');

    if(dis_R_tem > dis_R_actual){
      myservo.write(95); 
      delay(10);
    }
    else if(dis_R_tem < dis_R_actual){
      myservo.write(85); 
      delay(10);
    }

    if(dis_L_tem > dis_L_actual){
      myservo.write(85); 
      delay(10);
    }
    else if(dis_L_tem < dis_L_actual){
      myservo.write(95); 
      delay(10);
    }

    dis_F_actual = medir_distancia('F');

  } 
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(delay_uni);
}

void acelerar(int v){
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, v);
}

void auto_giro(int v){
  float time = 1000 * (v/100);
  float angle;

  float dis_R_actual = medir_distancia('R');
  delay(5);
  float dis_L_actual = medir_distancia('L');
  delay(5);
  float dis_F_actual = medir_distancia('F');
  delay(5);
  float dis_B_actual = medir_distancia('B');
  delay(5);

  if(dis_R_actual > dis_L_actual){
    float angle = max_R_angle - (max_R_angle * v/time);
    myservo.write(angle);
    delay(10);
  }
  else{
    float angle = max_L_angle - (max_L_angle * v/time);
    myservo.write(angle);
    delay(10);
  }
  acelerar(v);
  delay(time);
  myservo.write(90);
  delay(20);

  float dis_LAT_tem = medir_distancia('R') + medir_distancia('L');
  while(dis_LAT_tem < 120 and dis_LAT_tem > 80){
    myservo.write(90);
    delay(20);
    myservo.write(95);
    delay(20);
    dis_LAT_tem = medir_distancia('R') + medir_distancia('L');
  }

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  delay(delay_uni);
}

void estacionarse(float dis_F, float dis_B){
  float dis_f_tem = dis_F / medir_distancia('F') - 10;
  delay(5);
  float dis_b_tem = dis_B / medir_distancia('B') - 10;
  delay(5);

  while(dis_f_tem < 1.2 and dis_f_tem > 0.8 or dis_b_tem > 1.2 and dis_b_tem < 0.8){
    acelerar(90);
    myservo.write(95);
    delay(20);
    myservo.write(85);
    delay(20);
    dis_f_tem = dis_F / medir_distancia('F') - 10;
    dis_b_tem = dis_B / medir_distancia('B') - 10;
  }
  stop();
}

float medir_distancia(char dir){
  int trig = 0;
  int echo = 0;
  switch(dir){
    case 'F':
      trig = ultra_F_trig;
      echo = ultra_F_echo;
      break;
    case 'R':
      trig = ultra_R_trig;
      echo = ultra_R_echo;
      break;
    case 'L':
      trig = ultra_L_trig;
      echo = ultra_L_echo;
      break;
    case 'B':
      trig = ultra_B_trig;
      echo = ultra_B_echo;
      break;
    default:
      return 0.0;
  }
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  float distance_cm = duration * 0.034 / 2;
  return distance_cm * 0.95;
}

void receiveEvent(int numBytes) {
  Serial.print("numbytes = ");
  Serial.println(numBytes);
  if(numBytes <= BUFFER_SIZE) { // Asegurarse de que la cantidad de bytes recibidos no exceda el tamaño del buffer
    for(int i = 0; i < numBytes; i++) {
      receivedData[i] = Wire.read(); // Leer los datos recibidos y almacenarlos en el buffer
    }
  }
  else{
    // Si se reciben más datos de los que puede manejar el buffer, descartar los datos adicionales
    while (Wire.available()) {
      Wire.read();
    }
  }
  
  if(receivedData[0] == 1){
    myservo.write(130);
    }
  else if(receivedData[0] == 0){
    myservo.write(70);
    }
      
}

void break_car(){
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH );
      delay(700);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      delay(100);
}

void stop(){
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH );
      delay(500);
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      delay(100);
      exit(0);
}

long medir_tiempo(char dir){
  int trig = 0;
  int echo = 0;
  switch(dir){
    case 'F':
      trig = ultra_F_trig;
      echo = ultra_F_echo;
      break;
    case 'R':
      trig = ultra_R_trig;
      echo = ultra_R_echo;
      break;
    case 'L':
      trig = ultra_L_trig;
      echo = ultra_L_echo;
      break;
    case 'B':
      trig = ultra_B_trig;
      echo = ultra_B_echo;
      break;
    default:
      return 0.0;
  }
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);
  return duration/2;
}
