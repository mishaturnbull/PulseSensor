/*
 * Header file for associated pulsesensor-bpm.c.
 * Much of this is copied from PulseSensor_timer.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>
#include <wiringPi.h>
#include <mcp3004.h>

#define OPT_R 10        // min uS allowed lag btw alarm and callback
#define OPT_U 2000      // sample time uS between alarms
#define OPT_O_ELAPSED 0 // output option uS elapsed time between alarms
#define OPT_O_JITTER 1  // output option uS jitter (elapsed time - sample time)
#define OPT_O 1         // defaoult output option
#define OPT_C 10000     // number of samples to run (testing)
#define OPT_N 1         // number of Pulse Sensors (only 1 supported)

#define TIME_OUT 30000000    // uS time allowed without callback response
// PULSE SENSOR LEDS
#define BLINK_LED 0
// MCP3004/8 SETTINGS
#define BASE 100
#define SPI_CHAN 0

// FIFO STUFF
#define PULSE_EXIT 0    // CLEAN UP AND SHUT DOWN
#define PULSE_IDLE 1    // STOP SAMPLING, STAND BY
#define PULSE_ON 2      // START SAMPLING, WRITE DATA TO FILE
#define PULSE_DATA 3    // SEND DATA PACKET TO FIFO
#define PULSE_CONNECT 9 // CONNECT TO OTHER END OF PIPE

#define LOG_FILE "/var/ssipc/pilot/heartDataLog.txt"
#define SSIPC_BPM_FILE "/var/ssipc/pilot/heartBPM.txt"

void getPulse(int sig_num);
void startTimer(int r, unsigned int u);
void stopTimer(void);
void initPulseSensorVariables(void);
void initJitterVariables(void);
