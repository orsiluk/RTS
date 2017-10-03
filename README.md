# RTS

Project for Real Time and Embedded Systems

An alarm clock was implemented on a development board based on the PIC18F97J60 microprocessorby Microchip.  
The alarm clock had to have the following features:
  1.  The format of the time displayed had to be hh:mm:ss, which meant that the display had to beupdated once a second
  2.  Hours had to be counted from 0 to 23; so the display had to jump from 23:59.59 to 00:00.00
  3.  The clock time and alarm time had to be set when the board was powered up.  Ringing had tobe replaced by blinking an LED every second for 30 seconds.  To achieve this, a timer had to beconfigured on the PIC to generate interrupts, making it possible to turn on/off an LED twice asecond and update a seconds counter once every second
  4.  While the clock was running, it had to be possible to change the alarm time and the the currenttime without influencing the clock
