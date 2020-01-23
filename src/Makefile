# Let's see if I remember how to make Makefiles!

CC = gcc
FLAGS = -Wall -Wextra -lwiringPi -g
DEMO_TARGETNAME = pulseTimer.x
BPM_TARGETNAME = bpmMonitor.x

main: clean pulsesensor_demo

clean:
	-rm $(DEMO_TARGETNAME)
	-rm $(BPM_TARGETNAME)

pulsesensor_demo:
	$(CC) $(FLAGS) PulseSensor_timer.c $(DEMO_TARGETNAME)
