#define __18F97J60
#define __SDCC__
#define THIS_INCLUDES_THE_MAIN_FUNCTION
#include "Include/HardwareProfile.h"

#include <string.h>
#include <stdlib.h>

#include "Include/LCDBlocking.h"
#include "Include/TCPIP_Stack/Delay.h"

#define LOW(a)     (a & 0xFF)
#define HIGH(a)    ((a>>8) & 0xFF)

#define DIVISOR 5*25*25*16
#define HIGH ((65535 - DIVISOR) & 0xFF00) >> 8
#define LOW (65535 - DIVISOR) & 0xFF

const char *state2str[] = 
{
  "STARTUP",
  "WAIT_FOR_RELEASE",
  "WAIT_HOURS",
  "WAIT_MINS",
  "WAIT_SECS",
  "SET_TIME",
  "INC_HOURS",
  "INC_MINS",
  "INC_SECS",
  "INC_SECS_2",
  "INC_MINS_2",
  "INC_HOURS_2",
  "RESET",
  "INC_HOURS_WAIT",
  "INC_MINS_WAIT",
  "INC_SECS_WAIT",
  "DEBOUNCE",
  "CHOICE"
};

typedef enum
{
    STARTUP,
    WAIT_FOR_RELEASE,
    WAIT_HOURS,
    WAIT_MINS,
    WAIT_SECS,
    SET_TIME,
    INC_HOURS,
    INC_MINS,
    INC_SECS,
    INC_SECS_2,
    INC_MINS_2,
    INC_HOURS_2,
    RESET,
    INC_HOURS_WAIT,
    INC_MINS_WAIT,
    INC_SECS_WAIT,
    DEBOUNCE,
    CHOICE
} FSM_STATE;

enum Mode
{
  CURRENT,
  ALARM,
  SET
} ;

struct Time
{
  unsigned char hours;
  unsigned char minutes;
  unsigned char seconds;
};

struct Time current_time, alarm_time;
/*
current_time.hours = 0;
current_time.minutes = 0;
current_time.seconds = 0;
alarm_time.hours = 0;
alarm_time.minutes = 0;
alarm_time.seconds = 0;
*/

void DisplayString(BYTE pos, char* text);
void DisplayWORD(BYTE pos, WORD w);
void DisplayIPValue(DWORD IPdw);
size_t strlcpy(char *dst, const char *src, size_t siz);
char* current_time_string(enum Mode mode);
//void blink_time(void);
//void debug_display_time(void);
void display_state(FSM_STATE state);
//void flush_state(void);
void change_time(unsigned char hms, struct Time* time);
void display_time(enum Mode mode);


/*************************************************
 Function DisplayWORD:
 writes a WORD in hexa on the position indicated by
 pos.
 - pos=0 -> 1st line of the LCD
 - pos=16 -> 2nd line of the LCD

 __SDCC__ only: for debugging
*************************************************/
#if defined(__SDCC__)
void DisplayWORD(BYTE pos, WORD w) //WORD is a 16 bits unsigned
{
  BYTE WDigit[6]; //enough for a  number < 65636: 5 digits + \0
  BYTE j;
  BYTE LCDPos = 0; //write on first line of LCD
  unsigned radix = 10; //type expected by sdcc's ultoa()

  LCDPos = pos;
  ultoa(w, WDigit, radix);
  for (j = 0; j < strlen((char*)WDigit); j++)
  {
    LCDText[LCDPos++] = WDigit[j];
  }
  if (LCDPos < 32u)
    LCDText[LCDPos] = 0;
  LCDUpdate();
}

// /*************************************************
//  Function DisplayString:
//  Writes string to the LCD display starting at pos
//  since strlcopy writes the final \0, only 31 characters
//  are really usable on the LCD
//  *************************************************/
// void DisplayString(BYTE pos, char* text)
// {
//   BYTE l= strlen(text)+1;/* l must include the finam \0, so, it is strlen+1*/
//    BYTE max= 32-pos;
//    strlcpy((char*)&LCDText[pos], text,(l<max)?l:max );
//    LCDUpdate();
//
// }

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

#endif



/*-------------------------------------------------------------------------
 *
 * strlcpy.c
 *    strncpy done right
 *
 * This file was taken from OpenBSD and is used on platforms that don't
 * provide strlcpy().  The OpenBSD copyright terms follow.
 *-------------------------------------------------------------------------
 */

/*  $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $    */

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 * Function creation history:  http://www.gratisoft.us/todd/papers/strlcpy.html
 */
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
  char       *d = dst;
  const char *s = src;
  size_t      n = siz;

  /* Copy as many bytes as will fit */
  if (n != 0)
  {
    while (--n != 0)
    {
      if ((*d++ = *s++) == '\0')
        break;
    }
  }

  /* Not enough room in dst, add NUL and traverse rest of src */
  if (n == 0)
  {
    if (siz != 0)
      *d = '\0';          /* NUL-terminate dst */
    while (*s++)
      ;
  }

  return (s - src - 1);       /* count does not include NUL */
}

/*
unsigned char current_hours = 0;
unsigned char current_minutes = 0;
unsigned char current_seconds = 0;
*/

unsigned char ticks = 0;


void high_isr (void) interrupt 1
{
  if(INTCONbits.TMR0IF)  //If TMR0 interrupt
  {
    INTCONbits.TMR0IE = 0;
    if (++ticks == 125)
    {
      current_time.seconds++;
      ticks = 0;
    }
    
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1; 
    TMR0H=HIGH;
    TMR0L=LOW; 
  }

  /*
  if(PIR1bits.TMR1IF == 1) //If TMR1 interrupt
  {
    current_seconds++;
    display_time();
    PIR1bits.TMR1IF = 0; //Clear flag
  }
  */
}


char* current_time_string(enum Mode mode)
{
  char string[16];
  unsigned char i = 0;

  struct Time* time;
  time = &alarm_time;
  if(mode == SET || mode == CURRENT)
    time = &current_time;
    

  for (;i<8;i++)
      string[i] = '0';


  if (time->hours > 9)
  {
      string[1] += time->hours % 10;
      string[0] += time->hours/10;
  }
  else
    string[1] += time->hours;

  string[2] = ':';

  if (time->minutes > 9)
  {
      string[4] += time->minutes % 10;
      string[3] += time->minutes/10;
  }
  else
    string[4] += time->minutes;

  string[5] = '.';

  if (time->seconds > 9)
  {
      string[7] += time->seconds % 10;
      string[6] += time->seconds/10;
  }
  else
    string[7] += time->seconds;

  return string;
}

/*
void debug_display_time(void)
{
  LED_PUT(0x00);
  //DelayMs(40);
  display_time(set_time);
  LED_PUT(0x01);
}
*/

/* Displays current time on LCD display */
void display_time(enum Mode mode)
{
  DisplayString(16, current_time_string(mode));
}

void display_state(FSM_STATE state)
{
  DisplayString(0,"                ");
  DisplayString(0,state2str[state]);
}

/*
void flush_state(void)
{
  DisplayString(0,"                ");
}
*/

/* Blinks current time on display */
/*
void blink_time(void)
{
  DisplayString(16,"");
  DelayMs(100);
  display_time(set_time);
}
*/

void change_time(unsigned char hms, struct Time* time)
{
  if(hms==1)
  {
    if(BUTTON0_IO == 0u)
    {
      time->hours = (time->hours > 0? time->hours-1:23);
    }
    else if(BUTTON1_IO == 0u)
      time->hours = (time->hours < 23? time->hours+1:0);
  }
  else if(hms==2)
  {
    if(BUTTON0_IO == 0u)
      time->minutes = (time->minutes > 0? time->minutes-1:59);
    else if(BUTTON1_IO == 0u)
      time->minutes = (time->minutes < 59? time->minutes+1:0);
  }
  else if(hms==3)
  {
    if(BUTTON0_IO == 0u)
      time->seconds = (time->seconds > 0? time->seconds-1:59);
    else if(BUTTON1_IO == 0u)
      time->seconds = (time->seconds < 59? time->seconds+1:0);
  }
}


void main(void)
{
  unsigned char hms = 0;
  FSM_STATE state = STARTUP;
  //char set_time = 1;
  struct Time *ptr;
  enum Mode mode = SET;
  

  LED0_TRIS = 0; //configure 1st led pin as output (yellow)
  LED1_TRIS = 0; //configure 2nd led pin as output (red)
  LED2_TRIS = 0; //configure 3rd led pin as output (red)

  BUTTON0_TRIS = 1; //configure button0 as input
  BUTTON1_TRIS = 1; //configure button1 as input

  /*
  // INTERRUPT SETUP
  INTCONbits.GIE = 1;  //enable global interrupts
  INTCONbits.PEIE = 1; //enable peripheral interrupts (such as TMR1)
  //PIE1bits.TMR1IE = 1; //enable TMR1 interrupts
  PIR1bits.TMR1IF = 0; //reset TMR1 flag
  IPR1bits.TMR1IP = 1; //TMR1 interrupts have high prio
  RCONbits.IPEN = 1;   //Enable priority levels

  //  TIMER1 SETUP

  TMR1H = 0x80;
  TMR1L = 0x0;
  //TMR1H = (0xFF00 & ticks_per_sec) >> 8;
  //TMR1L = 0x00FF & ticks_per_sec;

  T1CONbits.RD16 = 0; //2x8bit operation
  T1CONbits.T1RUN = 0; //???
  //T1CONbits.TCKPS = 0b11; //Prescale 1:8
  //T1CON |= 0x30;    //Prescale 1:8
  T1CONbits.T1OSCEN = 1; //Enable clock
  T1CONbits.T1SYNC = 1;  //??? dont care
  T1CONbits.TMR1CS = 1; //Use external clock sourse
  T1CONbits.TMR1ON = 1;   //Enable TMR1

  PIE1bits.TMR1IE = 1; //enable TMR1 interrupts
  */

  TMR0H=HIGH;
  TMR0L=LOW;
  //TMR0H = 0x0;
  //TMR0L = 0x0;

  // TMR0 SETUP
  T0CONbits.TMR0ON = 0; //stop timer
  T0CONbits.T08BIT = 0;  //16bit
  T0CONbits.T0CS = 0;   //Clock source = instruction cycle CLK
  T0CONbits.T0SE = 0;   //Rising edge
  T0CONbits.PSA = 1;    //No prescaler
  

  //  INTERRUPT CONFIG
  INTCONbits.GIE = 1;   //enable global interrupts
  INTCONbits.TMR0IE=0;  //enable timer0 interrupts

  INTCON2bits.TMR0IP=1; //TMR0 has high prio
  T0CONbits.TMR0ON = 1;  //Enable TMR0

  /*INTCON=0xA0;    
  INTCON2=0x4;
  */

  LCDInit();
  DelayMs(10);
  LED_PUT(0x00);


  //display_time();
  /*
  while(1)
  {
    
    LED_PUT(0x02);
    DelayMs(100);
    LED_PUT(0x0);
    DelayMs(100);
    
    display_time();
  }
  */
  
  // STATE MACHINE
  
  while(1)
  {
    display_state(state);
    //display_time(set_time);
    display_time(mode);

    switch(state)
    {
      case(STARTUP):
        current_time.hours = 0;
        current_time.minutes = 0;
        current_time.seconds = 0;

        alarm_time.hours = 0;
        alarm_time.minutes = 0;
        alarm_time.seconds = 0;

        hms = 0;
        if (BUTTON0_IO == 0u && BUTTON1_IO == 0u) state = WAIT_FOR_RELEASE;
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
          state = INC_SECS_2;
          mode = CURRENT;
          INTCONbits.TMR0IE=1;  //enable timer0 interrupts
        }
        break;

    case(SET_TIME):
      if (mode == SET) INTCONbits.TMR0IE=0;  //disable timer0 interrupts
      else INTCONbits.TMR0IE=1;

      if (BUTTON0_IO == 0u && BUTTON1_IO == 0u)
      {
        state = WAIT_FOR_RELEASE;
      }

      if(mode == SET) ptr = &current_time;
      else if(mode == ALARM) ptr = &alarm_time;
      change_time(hms, ptr);

      while(BUTTON0_IO == 0u || BUTTON1_IO == 0u); //wait for release
      break;

    case(INC_SECS_2):
      if(current_time.seconds == 60)
        state = INC_MINS_2;
      else if (BUTTON0_IO == 0u && BUTTON1_IO == 0u)
        state = CHOICE;
      break;

    case(CHOICE):
      //while(BUTTON0_IO == 0u || BUTTON1_IO == 0u); //wait for release

      if (BUTTON0_IO == 0u && BUTTON1_IO == 1u)
      {
        while (BUTTON0_IO == 0u); //wait for release
        mode = SET;
        state = SET_TIME;
      }
      else if(BUTTON1_IO == 0u && BUTTON0_IO == 1u)
      {
        while (BUTTON1_IO == 0u); //wait for release
        mode = ALARM;
        state = SET_TIME;
      }
      break;

    case(INC_MINS_2):
      current_time.minutes++;
      current_time.seconds=0;
      state = (current_time.minutes == 60? INC_HOURS_2:INC_SECS_2);
      break;

    case(INC_HOURS_2):
      current_time.hours++;
      current_time.minutes=0;
      state = (current_time.hours == 24? RESET:INC_SECS_2);
      break;

    case(RESET):
      current_time.hours = 0;
      current_time.minutes = 0;
      current_time.seconds = 0;
      state = INC_SECS_2;
      break;

    default:
      state = STARTUP;
    } //end switch
  }   //end while
}