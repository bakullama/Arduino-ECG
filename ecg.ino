#include <SAMDUETimerInterrupt.h>
#include <SAMDUE_ISR_Timer.h>
#include <CircularBuffer.h>
#define HALF_MINUTE_MS 30000
#define FULL_MINUTE_MS 60000
#define SECOND_MS 1000
#define BUFFER_SIZE 60

int currentval = 0;
int thresholdcount = 0;

float beats = 0;
float minuteBeats = 0;
int waittime;
double bpm;
double minutesPassed;
bool halfMinuteTrigger = false;
bool minuteTrigger = false;
bool secondTrigger = false;

CircularBuffer<int,BUFFER_SIZE> secondBuffer;
int bufferSum = 0;
int secondBeats = 0;

void handleHalfMinute(void) {
  halfMinuteTrigger = true;
}

void handleMinute(void) {
  minuteTrigger = true;
}

void handleSecond(void) {
  secondTrigger = true;
}

uint16_t attachDueInterrupt(double microseconds, timerCallback callback) {
  DueTimerInterrupt dueTimerInterrupt = DueTimer.getAvailable();
  dueTimerInterrupt.attachInterruptInterval(microseconds, callback);
  uint16_t timerNumber = dueTimerInterrupt.getTimerNumber();
  return timerNumber;
}


void setup() {
  // initialize the serial communication:
  Serial.begin(115200);
  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -

  attachDueInterrupt(HALF_MINUTE_MS * 1000, handleHalfMinute);
  Serial.println("Half minute interrupt setup\tOK");
  attachDueInterrupt(FULL_MINUTE_MS * 1000, handleMinute);
  Serial.println("Full minute interrupt setup\tOK");
  attachDueInterrupt(SECOND_MS * 1000, handleSecond);
  Serial.println("Second interrupt setup\tOK"
  Serial.println("Starting, first reading will come after 30 seconds, and accurate readings from 60 seconds");

}

void loop() {
  if (minuteTrigger) {
    minutesPassed++;
    Serial.print("HR avg: ");
    Serial.print(beats/minutesPassed);
    Serial.println(" bpm");
    minuteBeats = 0;
    minuteTrigger = false;
  
  } if (halfMinuteTrigger) {
    double multiplier = (millis() / 1000) / (double)60;
    bpm = minuteBeats * 2;
    Serial.print("Current HR: ");
    Serial.print(bpm);
    Serial.println(" bpm");
    halfMinuteTrigger = false;
  } if (secondTrigger) {
    if (secondBuffer.isFull()){
    bufferSum = 0;
    secondBuffer.unshift(secondBeats);
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      bufferSum += secondBuffer[i];
    }
//    Serial.println(secondBeats);
    Serial.print("Heart Rate: ");
    Serial.print(bufferSum);
    Serial.println("bpm");
    } else {
      secondBuffer.unshift(secondBeats);
    }
    secondBeats = 0;
    secondTrigger = false;
  } else {
    currentval = analogRead(A0);
    if (currentval >= 1000) {
      thresholdcount++;
    } else if (thresholdcount > 0 && currentval < 1000){
      if (thresholdcount > 20) {
//        Serial.println("beat");
        secondBeats++;
        beats+=1;    
        minuteBeats+=1;
      }
      thresholdcount = 0;
    }
   }
//Wait for a bit to keep serial data from saturating
  delay(1);
}
