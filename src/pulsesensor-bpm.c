/*
 * This code is designed to read a  heartrate from a pulse sensor
 * attached to the RPi and dump it into /var/ssipc/pilot/heartBPM.txt
 * as often as possible.
 *
 * Much of this is copied/adapted from PulseSensor_timer.c.
 */

#include "pulsesensor-bpm.h"

// VARIABLES USED TO DETERMINE SAMPLE JITTER & TIME OUT
volatile unsigned int eventCounter, thisTime, lastTime, elapsedTime, jitter;
volatile int sampleFlag = 0;
volatile int sumJitter, firstTime, secondTime, duration;
unsigned int timeOutStart, dataRequestStart, m;
// VARIABLES USED TO DETERMINE BPM
volatile int Signal;
volatile unsigned int sampleCounter;
volatile int threshSetting,lastBeatTime,fadeLevel;
volatile int thresh = 550;
volatile int P = 512;                               // set P default
volatile int T = 512;                               // set T default
volatile int firstBeat = 1;                      // set these to avoid noise
volatile int secondBeat = 0;                    // when we get the heartbeat back
volatile int QS = 0;
volatile int rate[10];
volatile int BPM = 0;
volatile int IBI = 600;                  // 600ms per beat = 100 Beats Per Minute (BPM)
volatile int Pulse = 0;
volatile int amp = 100;                  // beat amplitude 1/10 of input range.

char filename [100];
struct tm *timenow;

FILE* data;
FILE* ssipcBPM;

void sigHandler(int sig_num) {
  printf("\nKilling pulse timer jv937q\n");
  startTimer(OPT_R, 0);
  exit(EXIT_SUCCESS);
}

void fatal(int show_usage, char *fmt, ...) {
  char buf[128];
  va_list ap;
  char kill[20];

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  fprintf(stderr, "%s\n", buf);

  if (show_usage) {
    usage();
  }

  fflush(stderr);
  pritnf("\nKilling timer us7cv1\n");
  startTimer(OPT_R, 0);
  fprintf(data, "#%s", fmt);
  fclose(data);

  exit(EXIT_FAILURE);
}

void initPulseSensorVariables(void) {
  for (int i = 0; i < 10; i++) {
    rate[i] = 0;
  }
  QS = 0;
  BPM = 0;
  IBI = 600;  // 600ms/beat = 100 beats/minute
  Pulse = 0;
  sampleCounter = 0;
  lastBeatTime = 0;
  P = 512;  // peak at 1/2 input range of 0..1023
  T = 512;  // trough at 1/2 input range
  threshSetting = 550;  // used to seed/reset thresh variable
  thresh = 550;  // slightly above trough
  amp = 100;  // beat amplitude 1/10 of input range
  firstBeat = 1;  // looking for first beat?
  secondbeat = 0;  // looking for second beat?
  lastTime = micros();
  timeOutStart = lastTime;
}

int main(int argc, char *argv[]) {
  signal(SIGINT, sigHandler);
  time_t now = time(NULL);
  timenow = gmtime(&now);
  strftime(filename, sizeof(filename), LOG_FILE, timenow);
  data = fopen(filename, "w+");
  fprintf(data,"#Running with %d latency at %duS sample rate\n",OPT_R,OPT_U);
  fprintf(data,"#sampleCount\tSignal\tBPM\tIBI\tjitter\n");

  printf("About to start heartrate monitoring!\n");

  wiringPiSetup();  // sets wiringPi pin numbers
  mcp3004Setup(BASE, SPI_CHAN);
  initPulseSensorVariables();
  startTimer(OPT_R, OPT_U);  // start sampling

  while (1) {
    if (sampleFlag) {
      sampleFlag = 0;
      timeOutStart = micros();
      // record data in file
      fprintf(data, "%d\t%d\t%d\t%d\t%d\t%d\n",
        sampleCounter, Signal, IBI, BPM, jitter, duration);

      ssipcBPM = fopen(SSIPC_BPM_FILE, "w");
      fprintf(ssipcBPM, "%d", BPM);
      fclose(ssipcBPM);

      // record BPM in ssipc
      fprintf()
    }

    if ((micros() - timeOutStart) > TIME_OUT) {
      fatal(0, "0-program timed out jsluej", 0);
    }
  }

  return 0;
}

void startTimer(int r, unsigned int u) {
  int latency = r;
  unsigned int micros = u;

  signal(SIGALARM, getPulse);
  int err = ualarm(latency, micros);
  if (err == 0) {
    if (micros > 0) {
      printf("ualarm ON\n");
    } else {
      printf("ualarm OFF\n");
    }
  }
}

void getPulse(int sig_num) {
  if(sig_num == SIGALRM) {
    thisTime = micros();
    Signal = analogRead(BASE);
    elapsedTime = thisTime - lastTime;
    lastTime = thisTime;
    jitter = elapsedTime - OPT_U;
    sumJitter += jitter;
    sampleFlag = 1;
    sampleCounter += 2;         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;      // monitor the time since the last beat to avoid noise

    //  find the peak and trough of the pulse wave
    if (Signal < thresh && N > (IBI / 5) * 3) { // avoid dichrotic noise by waiting 3/5 of last IBI
      if (Signal < T) {                        // T is the trough
        T = Signal;                            // keep track of lowest point in pulse wave
      }
    }

    if (Signal > thresh && Signal > P) {       // thresh condition helps avoid noise
      P = Signal;                              // P is the peak
    }                                          // keep track of highest point in pulse wave

    //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
    // signal surges up in value every time there is a pulse
    if (N > 250) {                             // avoid high frequency noise
      if ( (Signal > thresh) && (Pulse == 0) && (N > ((IBI / 5) * 3)) ) {
        Pulse = 1;                             // set the Pulse flag when we think there is a pulse
        IBI = sampleCounter - lastBeatTime;    // measure time between beats in mS
        lastBeatTime = sampleCounter;          // keep track of time for next pulse

        if (secondBeat) {                      // if this is the second beat, if secondBeat == TRUE
          secondBeat = 0;                      // clear secondBeat flag
          for (int i = 0; i <= 9; i++) {       // seed the running total to get a realisitic BPM at startup
            rate[i] = IBI;
          }
        }

        if (firstBeat) {                       // if it's the first time we found a beat, if firstBeat == TRUE
          firstBeat = 0;                       // clear firstBeat flag
          secondBeat = 1;                      // set the second beat flag
          // IBI value is unreliable so discard it
          return;
        }


        // keep a running total of the last 10 IBI values
        int runningTotal = 0;                  // clear the runningTotal variable

        for (int i = 0; i <= 8; i++) {          // shift data in the rate array
          rate[i] = rate[i + 1];                // and drop the oldest IBI value
          runningTotal += rate[i];              // add up the 9 oldest IBI values
        }

        rate[9] = IBI;                          // add the latest IBI to the rate array
        runningTotal += rate[9];                // add the latest IBI to runningTotal
        runningTotal /= 10;                     // average the last 10 IBI values
        BPM = 60000 / runningTotal;             // how many beats can fit into a minute? that's BPM!
        QS = 1;                              // set Quantified Self flag (we detected a beat)
        //fadeLevel = MAX_FADE_LEVEL;             // If we're fading, re-light that LED.
      }
    }

    if (Signal < thresh && Pulse == 1) {  // when the values are going down, the beat is over
      Pulse = 0;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp / 2 + T;                  // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
    } // end if

    if (N > 2500) {                          // if 2.5 seconds go by without a beat
      thresh = threshSetting;                // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date
      firstBeat = 1;                      // set these to avoid noise
      secondBeat = 0;                    // when we get the heartbeat back
      QS = 0;
      BPM = 0;
      IBI = 600;                  // 600ms per beat = 100 Beats Per Minute (BPM)
      Pulse = 0;
      amp = 100;                  // beat amplitude 1/10 of input range.
    }  // end if
    duration = micros()-thisTime;
  }  // end if
}  // end of getPulse
