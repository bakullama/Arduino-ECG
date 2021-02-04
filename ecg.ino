#include <SAMDUETimerInterrupt.h>
#include <SAMDUE_ISR_Timer.h>
#define HALF_MINUTE_MS 30000
#define FULL_MINUTE_MS 60000

int currentval = 0;
int thresholdcount = 0;

bool disp = true;

float beats = 0;
float minuteBeats = 0;
double bpm;
double minutesPassed;
bool halfMinuteTrigger = false;
bool minuteTrigger = false;

void handleHalfMinute(void) {
  halfMinuteTrigger = true;
}

void handleMinute(void) {
  minuteTrigger = true;
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
  attachDueInterrupt(FULL_MINUTE_MS * 1000, handleMinute);


}

void loop() {
  if (halfMinuteTrigger) {
    if (disp) {
      double multiplier = (millis() / 1000) / (double)60;
      bpm = minuteBeats * 2;
      Serial.print("Current HR: ");
      Serial.print(bpm);
      Serial.println(" bpm");
    }
    halfMinuteTrigger = false;
    disp = !disp;
    
  } if (minuteTrigger) {
    minutesPassed++;
    Serial.print("HR avg: ");
    Serial.print(beats/minutesPassed);
    Serial.println(" bpm");
    minuteBeats = 0;
    minuteTrigger = false;
    

    
  } else {
    currentval = analogRead(A0);
    if (currentval >= 1000) {
      thresholdcount++;
    } else if (thresholdcount > 0 && currentval < 1000){
      if (thresholdcount > 40) {
//        Serial.println("beat");
        beats+=1;    
        minuteBeats+=1;
      }
      thresholdcount = 0;
    }

  }
  //Wait for a bit to keep serial data from saturating
  delay(1);
}
