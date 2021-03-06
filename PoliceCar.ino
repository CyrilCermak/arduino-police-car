
#include "Config.h"
#include <Servo.h>
#include "Moves.h"


const long servoInterval = 40; // 40
const long moveInterval = 300;

unsigned long previousServoTime = 0;
unsigned long previousMoveTime = 0;
double maxMeasuredDistance = 0;
double minMeasuredDistance = 100000;
int desiredPosition = 0;

int lastMovePosition = -1;

int undesiredPosition = 0;

int servoDirection = MOVE_RIGHT;
double distances[3];

int backwardDirection = -1;
int forwardDirection = -1;

//Setting up servo
Servo servo;
int servoPosition = 0;
long duration = 0;
int distance = 0; 
//Setting up btn 
int buttonState = 0;
bool isVigilant = true;
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT); 
  pinMode(GREEN_LED, OUTPUT); 
  servo.attach(9); // Attaching servo to 9 pin
  pinMode(M_RIGHT_SPEED,OUTPUT);
  pinMode(M_RIGHT_DIR,OUTPUT);
  pinMode(M_LEFT_SPEED,OUTPUT);
  pinMode(M_LEFT_DIR,OUTPUT);
  pinMode(BTN_PIN, INPUT);
  Serial.begin(9600);
}

void loop() {
  setBtnState();
  if (isVigilant) {
    digitalWrite(GREEN_LED, HIGH);
    startVigilantMultitasking();
  } else {
    digitalWrite(GREEN_LED, LOW);
   startMultiTasking(); 
  }
}

void startVigilantMultitasking() {
  unsigned long currentTime = millis();
  if (currentTime - previousServoTime >= servoInterval) {
    previousServoTime = currentTime;
    scanSurroundings();
  }
  if (currentTime - previousMoveTime >= moveInterval) {
    previousMoveTime = currentTime;
    reactToWorld();
  }
}

void reactToWorld() {
  if (minMeasuredDistance < 20) {
      if (undesiredPosition > 40 && undesiredPosition < 110) {
        goBackwards(HIGH_SPEED);
      } else if (undesiredPosition <= 40) {
        turnRight(MOVE_BACK);
      } else if (undesiredPosition >= 100) {
       turnLeft(MOVE_BACK); 
       }
    } else {
    stop();
  }
  resetMeasurements();
}


void startMultiTasking() {
  unsigned long currentTime = millis();
//  Move with servo
  if (currentTime - previousServoTime >= servoInterval) {
    previousServoTime = currentTime;
    scanSurroundings();
  }
  if (currentTime - previousMoveTime >= moveInterval) {
    previousMoveTime = currentTime;
    moveWithCar();
  }
}

void scanSurroundings() {
  moveWithServo(); 
  distances[0] = measureDistance();
  distances[1] = measureDistance();
  distances[2] = measureDistance();
  sort(distances, 3);
  // Median
  distance = distances[1];
  if (distance > maxMeasuredDistance) {
    maxMeasuredDistance = distance;
    desiredPosition = servoPosition;
  }
  if (distance < minMeasuredDistance){
    minMeasuredDistance = distance;
    undesiredPosition = servoPosition;
  }
  signalizeDistance();
}

void setBtnState() {
  buttonState = digitalRead(BTN_PIN);
  if (buttonState == 1) {
    isVigilant = !isVigilant;
    stop();
    delay(1000);
    Serial.println(isVigilant);
  }
  
}

void moveWithCar() {
//  printStatus();
  if(minMeasuredDistance >= 30){
      goStraight(HIGH_SPEED);    
      backwardDirection = -1;
      forwardDirection = -1;
  } else if(minMeasuredDistance <= 15){
    if (undesiredPosition <= 90) {
      if ((backwardDirection == IS_MOVING_LEFT) || (backwardDirection == -1)) {
        turnLeft(MOVE_BACK); 
      }
      backwardDirection = IS_MOVING_LEFT;
    } 
    if (desiredPosition >= 91) {
      if ((backwardDirection == IS_MOVING_RIGHT) || (backwardDirection == -1 )) {
        turnRight(MOVE_BACK); 
      }
     backwardDirection == IS_MOVING_RIGHT; 
    }
  } else {
//    Serial.println(desiredPosition);
    if (desiredPosition >= 60  && desiredPosition < 100) {
      if (forwardDirection == IS_MOVING_LEFT) {
            turnLeft(MOVE_FRONT); 
//            Serial.println("Turn left");
      } else if ((forwardDirection == IS_MOVING_RIGHT) || (forwardDirection == -1 )) {
          turnRight(MOVE_FRONT);
//          Serial.println("Turn right");
       } else {
          goStraight(MID_SPEED);
       }
       goStraight(MID_SPEED);
    } else if (desiredPosition <= 59) {
        if ((forwardDirection == IS_MOVING_LEFT) || (forwardDirection == -1 )) {
              turnLeft(MOVE_FRONT); 
//              Serial.println("Turn left");
        }
        forwardDirection == IS_MOVING_LEFT;
        backwardDirection = -1;
      } else if (desiredPosition >= 101) {
          if ((forwardDirection == IS_MOVING_RIGHT) || (forwardDirection == -1 )) {
          turnRight(MOVE_FRONT);
//          Serial.println("Turn right");
         }
        forwardDirection == IS_MOVING_RIGHT;
        backwardDirection = -1;
      }
  }
  resetMeasurements();
}

void resetMeasurements() {
  minMeasuredDistance = 100000;
  maxMeasuredDistance = 0;
  maxMeasuredDistance = 0;
}

void moveWithServo() {
  servo.write(servoPosition); 
  if (servoPosition >= 140) {
    servoDirection = MOVE_LEFT;
  } 
  if (servoPosition <= 20) {
    servoDirection = MOVE_RIGHT;
  }
  if (servoDirection == MOVE_LEFT) {
    servoPosition -= 20;
  } else {
    servoPosition += 20;
  }
}

void signalizeDistance() {
     if (distance < 15) {  // This is where the BLUE_LED On/Off happens
      digitalWrite(BLUE_LED,HIGH); 
      digitalWrite(RED_LED,LOW);
    } else {
      digitalWrite(BLUE_LED,LOW);
      digitalWrite(RED_LED,HIGH);
    }
}

double measureDistance() {
    digitalWrite(TRIG_PIN, LOW); 
    delayMicroseconds(2); 
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH);
    return (duration/2) / 29.1; // returns distance
}

void sort(double a[], int size) {
    for(int i=0; i<(size-1); i++) {
        for(int o=0; o<(size-(i+1)); o++) {
                if(a[o] > a[o+1]) {
                    int t = a[o];
                    a[o] = a[o+1];
                    a[o+1] = t;
                }
        }
    }
}

// Debug

void printStatus() {
    Serial.println("---------------");
    Serial.print("Max measured distance: ");
    Serial.println(maxMeasuredDistance);
    Serial.print("Desired position: ");
    Serial.println(desiredPosition);
    Serial.print("Min measured distance: ");
    Serial.println(minMeasuredDistance);
    Serial.print("Undesired position: ");
    Serial.println(undesiredPosition);
    Serial.println("---------------");
}


