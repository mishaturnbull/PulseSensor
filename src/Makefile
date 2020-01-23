# Let's see if I remember how to make Makefiles!

CC = gcc
FLAGS = -Wall -Wextra -lwiringPi -g
DEMO_TARGETNAME = pulseTimer
BPM_TARGETNAME = bpmMonitor

main: clean pulsesensor_demo

clean:
	-rm $(DEMO_TARGETNAME)
	-rm $(BPM_TARGETNAME)

pulsesensor_demo:
	$(CC) $(CFLAGS) PulseSensor_Timer.c $(DEMO_TARGETNAME)
