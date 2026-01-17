//music based on HitBit <https://www.hibit.dev/posts/31/play-christmas-melody-with-arduino-and-a-buzzer>
// all my LEDS

int ledPins[] = {5, 6};  // NEUE Pins - kein Konflikt mit Servo!
const int ledCount = sizeof(ledPins) / sizeof(ledPins[0]);

int brightness[10];     
int stepSize[10];       
int direction[10];     
unsigned long nextUpdate[10]; 
int updateDelay[10];

//for the Christmas music
#include "pitches.h"

#define BUZZER_PIN 8

int melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, 
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_D5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, 
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_G5, NOTE_F5, NOTE_E5, NOTE_D5,
  NOTE_C5,
};

int durations [] = {
  8, 8, 4,     
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 4,     
  8, 8, 4,
  8, 8, 8, 8,
  4, 4,
  8, 8, 4,     
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 4,     
  8, 8, 4,
  8, 8, 8, 8,
  2,
};

//music variables for non blocking
int currentNote = 0;
int melodySize = sizeof(durations) / sizeof(int);
unsigned long noteEndTime = 0;
bool waitingForNextNote = false;

//Servo time
#include <Servo.h>
Servo myservo;

// Servo variables for slow non-blocking movement
unsigned long servoNextMove = 0;
int servoPosition = 0;        // Current actual position (0-180)
int servoTargetPosition = 0;  // Where we want to go (0 or 180)
bool servoMoving = false;     // Are we currently moving?
int servoDelay = 10;          // Delay between each 1-degree step (in ms)

void setup() {
  //lights setup
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);

    brightness[i] = random(0, 255);
    stepSize[i] = random(5, 20);
    direction[i] = random(0, 2) ? 1 : -1;
    updateDelay[i] = random(20, 40);
    nextUpdate[i] = millis() + updateDelay[i];
  }
  
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Servo Setup
  myservo.attach(11);
  myservo.write(0);           // Start at position 0
  servoPosition = 0;
  servoTargetPosition = 150;
  servoNextMove = millis() + 2000;  // Wait 2 seconds before first move
}

void loop() {
  unsigned long currentTime = millis();

  // ===== SERVO LOOP (slow incremental movement) =====
  if (servoMoving) {
    // We're currently moving - check if it's time for the next step
    if (currentTime >= servoNextMove) {
      if (servoPosition < servoTargetPosition) {
        servoPosition++;
        myservo.write(servoPosition);
        servoNextMove = currentTime + servoDelay;
      } else if (servoPosition > servoTargetPosition) {
        servoPosition--;
        myservo.write(servoPosition);
        servoNextMove = currentTime + servoDelay;
      } else {
        // We've reached the target
        servoMoving = false;
        servoNextMove = currentTime + 2000;  // Wait 2 seconds before next movement
      }
    }
  } else {
    // We're not moving - check if it's time to start a new movement
    if (currentTime >= servoNextMove) {
      // Switch target between 0 and 180
      if (servoTargetPosition == 20) {
        servoTargetPosition = 150;
      } else {
        servoTargetPosition = 20;
      }
      servoMoving = true;
      servoNextMove = currentTime + servoDelay;  // Start moving immediately
    }
  }

  // ===== LIGHTS LOOP =====
  for (int i = 0; i < ledCount; i++) {
    if (currentTime >= nextUpdate[i]) {
      brightness[i] += stepSize[i] * direction[i];

      if (brightness[i] >= 255) {
        brightness[i] = 255;
        direction[i] = -1;
      } else if (brightness[i] <= 0) {
        brightness[i] = 0;
        direction[i] = 1;
      }

      analogWrite(ledPins[i], brightness[i]);

      updateDelay[i] = random(20, 40);
      nextUpdate[i] = currentTime + updateDelay[i];
    }
  }

  // ===== MUSIC LOOP but without delay =====
  if (!waitingForNextNote) {
    if (currentNote < melodySize) {
      int duration = 1000 / durations[currentNote];
      tone(BUZZER_PIN, melody[currentNote], duration);
      
      int pauseBetweenNotes = duration * 1.30;
      noteEndTime = currentTime + pauseBetweenNotes;
      waitingForNextNote = true;
    } else {
      currentNote = 0;
    }
  } else {
    if (currentTime >= noteEndTime) {
      noTone(BUZZER_PIN);
      currentNote++;
      waitingForNextNote = false;
    }
  }
}