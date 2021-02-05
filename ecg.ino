#include <SAMDUETimerInterrupt.h>
#include <SAMDUE_ISR_Timer.h>
#include <CircularBuffer.h>
#define FULL_MINUTE_MS 60000
#define SECOND_MS 1000
#define BUFFER_SIZE 60

int currentval = 0;
int thresholdcount = 0;

int beats = 0;
int minuteBeats = 0;
int minutesPassed = 0;
volatile bool minuteTrigger = false;
volatile bool secondTrigger = false;
int startTime = 0;

CircularBuffer<int,BUFFER_SIZE> secondBuffer;
int bufferSum = 0;
int secondBeats = 0;

// ISRs

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

  Serial.begin(115200);
  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -
  pinMode(13, OUTPUT); // indicator LED
  digitalWrite(13, HIGH);
  attachDueInterrupt(FULL_MINUTE_MS * 1000, handleMinute);
  Serial.println("Full minute interrupt setup\tOK");
  attachDueInterrupt(SECOND_MS * 1000, handleSecond);
  Serial.println("Second interrupt setup\tOK");
  
  delay(500);
  Serial.println("Starting, first reading will come after 30 seconds, and accurate readings from 60 seconds");
  digitalWrite(13, LOW);
}

void loop() {
  if (millis() - startTime >= 20) {
    digitalWrite(13, LOW);
  }
  if (minuteTrigger) {
    minutesPassed++;
    Serial.print("HR avg: ");
    Serial.print(beats/minutesPassed);
    Serial.println(" bpm");
    minuteBeats = 0;
    minuteTrigger = false;

  } 
  if (secondTrigger) {
    if (secondBuffer.isFull()){
    bufferSum = 0;
    secondBuffer.unshift(secondBeats);
    for (int i = 0; i < BUFFER_SIZE; ++i) {
      bufferSum += secondBuffer[i];
    }
    //Serial.println(secondBeats);
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
//    Serial.println(currentval);
    if (currentval >= 1000) {
      thresholdcount++;
    } else if (thresholdcount > 0 && currentval < 1000){
      if (thresholdcount > 25) {
        digitalWrite(13, HIGH);
        startTime = millis();
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
