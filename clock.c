#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/LCDBlocking.h"
#include "Include/TCPIP_Stack/Delay.h"

#define DIVISOR 25000
#define HIGH ((0xFFFF - DIVISOR) & 0xFF00) >> 8
#define LOW (0xFFFF - DIVISOR) & 0xFF

const char *state2str[] = 
{
  "STARTUP",
  "WAIT_FOR_RELEASE",
  "SET_TIME",
  "INC_TIME",
  "CHOICE",
};

typedef enum
{
    STARTUP,
    WAIT_FOR_RELEASE,
    SET_TIME,
    INC_TIME,
    CHOICE,
} FSM_STATE;

enum Mode
{
  CURRENT,
  ALARM,
  SET,
} ;

struct Time
{
  unsigned char hours;
  unsigned char minutes;
  unsigned char seconds;
};

struct Time current_time, alarm_time;
unsigned char alarm_triggered = 0;

void DisplayString(BYTE pos, char* text);
void DisplayTop(char* text);
char* current_time_string(enum Mode mode);
void display_state(FSM_STATE state);
void change_time(unsigned char hms, enum Mode mode);
void display_time(enum Mode mode);
void setup(void);
void init_Time(struct Time* time, unsigned char hours, unsigned char minutes, unsigned char seconds);

#if defined(__SDCC__)
/*************************************************
 Function DisplayString:
 Writes the first characters of the string in the remaining
 space of the 32 positions LCD, starting at pos
 (does not use strlcopy, so can use up to the 32th place)
*************************************************/
void DisplayString(BYTE pos, char* text)
{
  BYTE        l = strlen(text);/*number of actual chars in the string*/
  BYTE      max = 32 - pos;  /*available space on the lcd*/
  char       *d = (char*)&LCDText[pos];
  const char *s = text;
  size_t      n = (l < max) ? l : max;
  /* Copy as many bytes as will fit */
  if (n != 0)
    while (n-- != 0)*d++ = *s++;
  LCDUpdate();

}

/* Same as DisplayString, but only displays on top row of LCD */
void DisplayTop(char* text)
{
  BYTE        l = strlen(text);/*number of actual chars in the string*/
  char       *d = (char*)&LCDText[0];
  const char *s = text;
  size_t      n = (l < 16) ? l : 16;
  /* Copy as many bytes as will fit */
  unsigned char i = 16-n;
  if (n != 0)
  {
    while (n-- != 0)*d++ = *s++;
    while (i-- != 0)*d++ = ' ';
  }
  LCDUpdate();
}

#endif


void high_isr (void) interrupt 1
{
  static unsigned char ticks = 0;
  static unsigned char led_data = 0;
  static unsigned char led_on_time = 0;

  if(INTCONbits.TMR0IF)  //If TMR0 flag is set
  {
    INTCONbits.TMR0IE = 0;  // Disable TMR0 interrupts

    if ((++ticks % 125 == 0) && alarm_triggered)
    {
      if(led_on_time++ < 60)
        led_data ^= 2;
      else
      {
        led_data = 0;
        led_on_time = 0;
        alarm_triggered = 0;
      }
      LED_PUT(led_data);
    }
    

    if (ticks == 250)
    {
      current_time.seconds++;
      ticks = 0;
      if (!alarm_triggered)
      {
        if((current_time.hours == alarm_time.hours) && (current_time.minutes == alarm_time.minutes) && (current_time.seconds == alarm_time.seconds))
          alarm_triggered = 1;
      }
    }
    
    INTCONbits.TMR0IF = 0;  // Reset TMR0 flag
    INTCONbits.TMR0IE = 1;  // Re-enable TMR0 interrupts
    TMR0H=HIGH;             // Set TMR0 values
    TMR0L=LOW; 
  }
}


char* current_time_string(enum Mode mode)
{
  char string[12];
  unsigned char i = 8;
  struct Time* time = (mode == ALARM? &alarm_time : &current_time);

  string[0] = (time->hours/10) + '0';
  string[1] = (time->hours % 10) + '0';
  string[2] = ':';
  string[3] = (time->minutes/10) + '0';
  string[4] = (time->minutes % 10) + '0';
  string[5] = '.';
  string[6] = (time->seconds/10) + '0';
  string[7] = (time->seconds % 10) + '0';
  for(;i<12;i++) string[i] = ' ';

  return string;
}

/* Displays current time on lower row of LCD display */
void display_time(enum Mode mode)
{
  DisplayString(20, current_time_string(mode));
}

/* Displays the current FSM state in the upper row of LCD display. Used in debug. */
void display_state(FSM_STATE state)
{
  DisplayTop(state2str[state]);
}

/* Changes the current or alarm time, depending on the mode. */
void change_time(unsigned char hms, enum Mode mode)
{
  struct Time *time = (mode == ALARM? &alarm_time: &current_time);

  DelayMs(20);  //arbitrary delay
  if(hms==1)
  {
    if(mode != ALARM) DisplayTop("Set current hrs");
    else  DisplayTop("Set alarm hours");

    if(BUTTON0_IO == 0u && BUTTON1_IO == 1u)
      time->hours = (time->hours > 0? time->hours-1:23);
    else if(BUTTON0_IO == 1u && BUTTON1_IO == 0u)
      time->hours = (time->hours < 23? time->hours+1:0);
  }
  else if(hms==2)
  {
    if(mode != ALARM) DisplayTop("Set current mins");
    else  DisplayTop("Set alarm mins");

    if(BUTTON0_IO == 0u && BUTTON1_IO == 1u)
      time->minutes = (time->minutes > 0? time->minutes-1:59);
    else if(BUTTON0_IO == 1u && BUTTON1_IO == 0u)
      time->minutes = (time->minutes < 59? time->minutes+1:0);
  }
  else if(hms==3)
  {
    if(mode != ALARM) DisplayTop("Set current secs");
    else  DisplayTop("Set alarm secs");

    if(BUTTON0_IO == 0u && BUTTON1_IO == 1u)
      time->seconds = (time->seconds > 0? time->seconds-1:59);
    else if(BUTTON0_IO == 1u && BUTTON1_IO == 0u)
      time->seconds = (time->seconds < 59? time->seconds+1:0);
  }
}

/* Setup initialization. Run at startup . */
void setup(void)
{
  LED0_TRIS = 0; //configure 1st led pin as output (yellow)
  LED1_TRIS = 0; //configure 2nd led pin as output (red)
  LED2_TRIS = 0; //configure 3rd led pin as output (red)

  BUTTON0_TRIS = 1; //configure button0 as input
  BUTTON1_TRIS = 1; //configure button1 as input

  // TMR0 SETUP
  TMR0H=HIGH;
  TMR0L=LOW;
  T0CONbits.TMR0ON = 0; //stop timer
  T0CONbits.T08BIT = 0;  //16bit
  T0CONbits.T0CS = 0;   //Clock source = instruction cycle CLK
  T0CONbits.T0SE = 0;   //Rising edge
  T0CONbits.PSA = 1;    //No prescaler

  //  INTERRUPT CONFIG
  INTCONbits.GIE = 1;   //enable global interrupts
  INTCONbits.TMR0IE=0;  //enable timer0 interrupts
  INTCON2bits.TMR0IP=1; //TMR0 has high prio

  LCDInit();
  DelayMs(10);
  LED_PUT(0x00);

  T0CONbits.TMR0ON = 1;  //Enable TMR0

}

/* Initializes instance of struct Time. */
void init_Time(struct Time* time, unsigned char hours, unsigned char minutes, unsigned char seconds)
{
  time->hours = hours;
  time->minutes = minutes;
  time->seconds = seconds;
}

void main(void)
{
  unsigned char hms = 0;
  FSM_STATE state = STARTUP, previous_state = STARTUP;
  enum Mode mode = SET;
  setup();
  
  while(1) // STATE MACHINE
  {
    //display_state(state); //Display current FSM state (debug)
    display_time(mode);
    if(alarm_triggered)
    {
      DisplayTop("WAKE UP!");
      state = INC_TIME;
    }

    switch(state)
    {
      case(STARTUP):
        init_Time(&current_time,0,0,0);
        init_Time(&alarm_time,0,0,0);

        hms = 1;
        previous_state = STARTUP;
        state = SET_TIME;
        break;

      case(WAIT_FOR_RELEASE):
        if (BUTTON0_IO == 1u && BUTTON1_IO == 1u)
        {
          hms++;
          state = SET_TIME;
        }
        if(hms > 3)
        {
          hms = 0;
          if(previous_state == STARTUP)
          {
            mode = ALARM;
            hms++;
          }
          else
          {
            state = INC_TIME;
            mode = CURRENT;
            INTCONbits.TMR0IE=1;  //enable timer0 interrupts
          }
          previous_state = WAIT_FOR_RELEASE;    
        }
        break;

    case(SET_TIME):
      INTCONbits.TMR0IE = (mode == SET? 0: 1);  //disable timer0 interrupts when setting time

      change_time(hms,mode);

      if (BUTTON0_IO == 0u && BUTTON1_IO == 0u)
        state = WAIT_FOR_RELEASE;

      break;

    case(INC_TIME):

      if(current_time.seconds >= 60)
      {
        current_time.seconds=0;
        if(++current_time.minutes >= 60)
        {
          current_time.minutes=0;
          if(++current_time.hours >= 24)
            init_Time(&current_time,0,0,0);
        }
      }

      if(!alarm_triggered)
      {
        if (previous_state == CHOICE || (BUTTON0_IO == 0u && BUTTON1_IO == 0u))
        {
          previous_state = state;
          state = CHOICE;
        }
        else
          DisplayTop("Have a nice day!");
      }

      break;

    case(CHOICE):
      previous_state = CHOICE;
      DisplayTop("^Alarm  vCurrent");

      if (BUTTON0_IO == 0u && BUTTON1_IO == 1u)
        mode = SET;
      else if(BUTTON0_IO == 1u && BUTTON1_IO == 0u)
        mode = ALARM;

      while ((BUTTON0_IO == 0u || BUTTON1_IO == 0u) && current_time.seconds != 60); //wait for release

      if(current_time.seconds == 60)
        state = INC_TIME;
      else if(mode == SET || mode == ALARM)
        state = WAIT_FOR_RELEASE;

      break;

    default:
      state = STARTUP;

    } //end switch
  }   //end while
}