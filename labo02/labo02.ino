#include "MeAuriga.h"

enum State {NORMAL, RALENTI, DANGER, RONDE};
State currentState = NORMAL;

enum DangerSubState {D_STOP, D_BACKWARD, D_TURN, D_DONE};
DangerSubState dangerState = D_STOP;
unsigned long dangerTimer = 0;

MeUltrasonicSensor ultraSensor(PORT_10);

#define LEDNUM  12
#define LEDPIN  44
MeRGBLed led(PORT0, LEDNUM);

const int m1_pwm = 11;
const int m1_in1 = 48;
const int m1_in2 = 49;

const int m2_pwm = 10;
const int m2_in1 = 47;
const int m2_in2 = 46;

const int maxPwm   = 255;
const int cruisePwm = 180;   
const int slowPwm   = 90;    
const int turnPwm   = 160;

unsigned long lastDebug = 0;

void setup() {
  Serial.begin(115200);

  pinMode(m1_pwm, OUTPUT);
  pinMode(m1_in1, OUTPUT);
  pinMode(m1_in2, OUTPUT);

  pinMode(m2_pwm, OUTPUT);
  pinMode(m2_in1, OUTPUT);
  pinMode(m2_in2, OUTPUT);

  led.setpin(LEDPIN);
  clearLeds();
}

void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastDebug >= 100) {
    lastDebug = currentTime;
    float d = ultraSensor.distanceCm();
    Serial.print("Distance = ");
    Serial.print(d);
    Serial.print(" cm | Etat = ");
    printState();
  }

  float distance = ultraSensor.distanceCm();

  if (currentState != DANGER) {
    if (distance >= 80) {
      currentState = NORMAL;
    } else if (distance >= 40) {
      currentState = RALENTI;
    } else {
      currentState = DANGER;
      dangerState = D_STOP;  
    }
  }

  stateManager();
}


void stateManager() {
  switch (currentState) {
    case NORMAL:
      driveForward(cruisePwm);
      ledsNormal();
      break;

    case RALENTI:
      driveForward(slowPwm);
      ledsRalenti();
      break;

    case DANGER:
      handleDanger();
      break;

    default:
      Stop();
      clearLeds();
      break;
  }
}

void handleDanger() {
  unsigned long now = millis();

  switch (dangerState) {
    case D_STOP:
      Stop();
      ledsDanger();
      dangerTimer = now;
      dangerState = D_BACKWARD;
      break;

    case D_BACKWARD:
      if (now - dangerTimer < 500) return; 
      driveBackward(slowPwm);
      dangerTimer = now;
      dangerState = D_TURN;
      break;

    case D_TURN:
      if (now - dangerTimer < 1000) return; 
      TurnLeft();                           
      dangerTimer = now;
      dangerState = D_DONE;
      break;

    case D_DONE:
      if (now - dangerTimer < 1000) return;  
      Stop();
      currentState = NORMAL;   
      dangerState = D_STOP;  
      break;
  }
}

void clearLeds() {
  for (int i = 0; i < LEDNUM; i++) led.setColor(i, 0, 0, 0);
  led.show();
}

void ledsNormal() {
  clearLeds();
  for (int i = 6; i < 12; i++) led.setColor(i, 0, 255, 0);
  led.show();
}

void ledsRalenti() {
  clearLeds();
  for (int i = 6; i < 12; i++) led.setColor(i, 0, 0, 255);
  led.show();
}

void ledsDanger() {
  for (int i = 0; i < LEDNUM; i++) led.setColor(i, 255, 0, 0);
  led.show();
}

// --- Fonctions moteurs ---
void driveForward(int pwm) {
  digitalWrite(m1_in2, LOW);
  digitalWrite(m1_in1, HIGH);
  analogWrite(m1_pwm, pwm);

  digitalWrite(m2_in2, LOW);
  digitalWrite(m2_in1, HIGH);
  analogWrite(m2_pwm, pwm);
}

void driveBackward(int pwm) {
  digitalWrite(m1_in2, HIGH);
  digitalWrite(m1_in1, LOW);
  analogWrite(m1_pwm, pwm);

  digitalWrite(m2_in2, HIGH);
  digitalWrite(m2_in1, LOW);
  analogWrite(m2_pwm, pwm);
}

void Stop() {
  analogWrite(m1_pwm, 0);
  analogWrite(m2_pwm, 0);
}

void TurnLeft() {
  
  digitalWrite(m1_in2, LOW);
  digitalWrite(m1_in1, HIGH);
  analogWrite(m1_pwm, turnPwm);

  digitalWrite(m2_in2, HIGH);
  digitalWrite(m2_in1, LOW);
  analogWrite(m2_pwm, turnPwm);
}


void printState() {
  switch (currentState) {
    case NORMAL:  Serial.println("NORMAL"); break;
    case RALENTI: Serial.println("RALENTI"); break;
    case DANGER:  Serial.println("DANGER"); break;
    default:      Serial.println("INCONNU"); break;
  }
}
