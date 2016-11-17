#include <avr/io.h>

//header to enable data flow control over pins

#define F_CPU 1000000

//telling controller crystal frequency attached

#include <util/delay.h>

//header to enable delay function in program

#define    E   5

//giving name “enable”  to 5th pin of PORTD, since it Is connected to LCD enable pin

#define RS  6

//giving name “registerselection” to 6th pin of PORTD, since is connected to LCD RS pin

void send_a_command(unsigned char command);

void send_a_character(unsigned char character);

void send_a_string(char *string_of_characters);

ISR(TIMER1_COMPA_vect);

static volatile int SEC = 0; //allocating integer memory for storing seconds

static volatile int MIN = 0; // allocating integer memory for storing minutes

static volatile int HOU = 0; // allocating integer memory for storing hours

int main(void)

{

    DDRA = 0b11000000;//only pin7 and pin8 of port a as output

    DDRD = 0xFF;

    _delay_ms(50);//giving delay of 50ms

    DDRB = 00FF;//Taking portB as output.

    TCCR1B |= (1 << CS12) | (1 << CS10) | (1 << WGM12); // setting prescale and CTC mode

    OCR1A = 10800; //setting compare value equal to counter clock frequency to get an interrupt every second

    sei();// enabling global interrupts

    TIMSK |= (1 << OCIE1A); //compare match interrupt enable

    char SHOWSEC [2];//seconds displaying character on LCD

    char SHOWMIN [2];//minutes displaying character on LCD

    char SHOWHOU [2];// hours displaying character on LCD

    int ALSEC = 0;//alarm seconds storing memory

    int ALMIN = 0;//alarm minutes storing memory

    int ALHOU = 0;//alarm hours storing memory

    char SHOWALSEC [2];//alarm  seconds displaying character on LCD

    char SHOWALMIN [2];// alarm minutes displaying character on LCD

    char SHOWALHOU [2];//alarm hours displaying character on LCD

    send_a_command(0x01); //Clear Screen 0x01 = 00000001

    _delay_ms(50);

    send_a_command(0x38);//telling lcd we are using 8bit command /data mode

    _delay_ms(50);

    send_a_command(0b00001111);//LCD SCREEN ON and courser blinking

    while (1)

    {

        itoa(HOU / 10, SHOWHOU, 10); //command for putting variable number in LCD(variable number, in which character to replace, which base is variable(ten here as we are counting number in base10))

        send_a_string(SHOWHOU);// telling the display to show character(replaced by variable number) of first person after positioning the courser on LCD

// displaying tens place of hours above

        itoa(HOU % 10, SHOWHOU, 10);

        send_a_string(SHOWHOU);

// displaying ones place of hours above

        send_a_string (":");//displaying character

        send_a_command(0x80 + 3);// shifting cursor  to 4th shell

        itoa(MIN / 10, SHOWMIN, 10); ///as integer cannot store decimal values, when MIN=9, we have MIN/10 = 0.9(actual), = 0 for CPU(as integer cannot store decimal values)

        send_a_string(SHOWMIN);

// displaying tens place of minutes above

        itoa(MIN % 10, SHOWMIN, 10);

        send_a_string(SHOWMIN);

// displaying ones place of minutes above

        send_a_command(0x80 + 5);// shifting cursor  to 6th shell

        send_a_string (":");

        send_a_command(0x80 + 6);// shifting cursor  to 7th shell

        if (bit_is_set(PINA, 5)) //if alarm pin is high

        {

            send_a_string(" ALM:ON ");//show alarm is on

            if ((ALHOU == HOU) & (ALMIN == MIN) & (ALSEC == SEC)) //alarm minute=min //and alarm hours= time hours and alarm seconds= time seconds

            {

                PORTA |= (1 << PINB7); //buzzer on

            }

        }

        if (bit_is_clear(PINA, 5)) //if alarm pin is low

        {

            send_a_string(" ALM:OFF");//show alarm is off

            PORTA &= ~(1 << PINB7); //buzzer off

        }

        send_a_command(0x80 + 0x40 + 0);// move courser to second line zero position

        send_a_string ("ALARM:");//show string of characters

        send_a_command(0x80 + 0x40 + 7);//move to eight position on second line

        itoa(ALHOU / 10, SHOWALHOU, 10);

        send_a_string(SHOWALHOU);

        itoa(ALHOU % 10, SHOWALHOU, 10);

        send_a_string(SHOWALHOU);

        send_a_command(0x80 + 0x40 + 9);

        send_a_string (":");

        send_a_command(0x80 + 0x40 + 10);

// Showing alarm hours above

        itoa(ALMIN / 10, SHOWALMIN, 10);

        send_a_string(SHOWALMIN);

        itoa(ALMIN % 10, SHOWALMIN, 10);

        send_a_string(SHOWALMIN);

        send_a_command(0x80 + 0x40 + 12);

        send_a_string (":");

        send_a_command(0x80 + 0x40 + 13);

// Showing alarm minutes above

        itoa(ALSEC / 10, SHOWALSEC, 10);

        send_a_string(SHOWALSEC);

        itoa(ALSEC % 10, SHOWALSEC, 10);

        send_a_string(SHOWALSEC);

        send_a_command(0x80 + 0);

// Showing alarm seconds above

        send_a_command(0x80 + 0);// shifting cursor  to 0th position

        if (bit_is_set(PINA, 4)) // if switch is set to adjust TIME

        {

            if (bit_is_clear(PINA, 0)) //button 1 is pressed

            {

                if (MIN < 60)

                {

                    MIN++;//if minutes of TIME are less than 60 increment it by one

                    _delay_ms(220);

                }

                if (MIN == 60)

                {

                    if (HOU < 24)

                    {

                        HOU++;//if minutes of TIME =60 when button is pressed //and hours of TIME are less than 24, increment hour by one.

                    }

                    MIN = 0; //if minute of TIME=60,reset it to zero

                    _delay_ms(220);

                }

            }

            if (bit_is_clear(PINA, 1))

            {

                if (MIN > 0)

                {

                    MIN--; //if second button is pressed and minute of TIME are //greater than zero, decrease minutes by one

                    _delay_ms(220);

                }

            }

            if (bit_is_clear(PINA, 2))

            {

                if (HOU < 24)

                {

                    HOU++; //if third button is pressed and hours of TIME are less //than 24, increment the hour by one

                }

                _delay_ms(220);

                if (HOU == 24)

                {

                    HOU = 0; //if hour of TIME equal to 24, reset hour of TIME

                }

            }

            if (bit_is_clear(PINA, 3))

            {

                if (HOU > 0)

                {

                    HOU--;//if fourth button is pressed and hours of TIME are //greater than ZERO, decrement the hour by one

                    _delay_ms(220);

                }

            }

        }

        if (bit_is_clear(PINA, 4)) //if alarm adjust is set

        {

            if (bit_is_clear(PINA, 0))

            {

                if (ALMIN < 60)

                {

                    ALMIN++;

                    _delay_ms(220);

                }

                if (ALMIN == 60)

                {

                    if (ALHOU < 24)

                    {

                        ALHOU++;

                    }

                    ALMIN = 0;

                    _delay_ms(220);

                }

            }

            if (bit_is_clear(PINA, 1))

            {

                if (ALMIN > 0)

                {

                    ALMIN--;

                    _delay_ms(220);

                }

            }

            if (bit_is_clear(PINA, 2))

            {

                if (ALHOU < 24)

                {

                    ALHOU++;

                }

                _delay_ms(220);

                if (ALHOU == 24)

                {

                    ALHOU = 0;

                }

            }

            if (bit_is_clear(PINA, 3))

            {

                if (ALHOU > 0)

                {

                    ALHOU--;

                    _delay_ms(220);

                }

            }

        }

    }

}

// Everything follows the same as described above for TIME

ISR(TIMER1_COMPA_vect) //loop to be executed on counter compare match

{

    if (SEC < 60)

    {

        SEC++;

    }

    if (SEC == 60)

    {

        if (MIN < 60)

        {

            MIN++;

        }

        SEC = 0;

    }

    if (MIN == 60)

    {

        if (HOU < 24)

        {

            HOU++;

        }

        MIN = 0;

    }

    if (HOU == 24)

    {

        HOU = 0;

    }

}

void send_a_command(unsigned char command)

{

    PORTA = command;

    PORTD &= ~ (1 << RS); //putting 0 in RS to tell lcd we are sending command

    PORTD |= 1 << E; //telling lcd to receive command /data at the port

    _delay_ms(50);

    PORTD &= ~1 << E; //telling lcd we completed sending data

    PORTA = 0;

}

void send_a_character(unsigned char character)

{

    PORTA = character;

    PORTD |= 1 << RS; //telling LCD we are sending data not commands

    PORTD |= 1 << E; //telling LCD to start receiving command/data

    _delay_ms(50);

    PORTD &= ~1 << E; //telling lcd we completed sending data/command

    PORTA = 0;

}

}

void send_a_string(char *string_of_characters)

{

    while (*string_of_characters > 0)

    {

        send_a_character(*string_of_characters++);

    }

}
Code:
/* ---- Code for Digital Clock with Alarm using AVR Microcontroller ------ */
#include <avr/io.h>
#define F_CPU 11059200
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#define enable            5
#define registerselection 6
void send_a_command(unsigned char command);
void send_a_character(unsigned char character);
void send_a_string(char *string_of_characters);
ISR(TIMER1_COMPA_vect);
static volatile int SEC = 0;
static volatile int MIN = 0;
static volatile int HOU = 0;
int main(void)
{
    DDRA = 0b11000000;
    DDRB = 0xFF;
    DDRD = 0xFF;

    TCCR1B |= (1 << CS12) | (1 << CS10) | (1 << WGM12);
    OCR1A = 10800;
    sei();
    TIMSK |= (1 << OCIE1A);

    char SHOWSEC [2];
    char SHOWMIN [2];
    char SHOWHOU [2];

    int ALSEC = 0;
    int ALMIN = 0;
    int ALHOU = 0;
    char SHOWALSEC [2];
    char SHOWALMIN [2];
    char SHOWALHOU [2];
    send_a_command(0x01); //Clear Screen 0x01 = 00000001
    _delay_ms(50);
    send_a_command(0x38);
    _delay_ms(50);
    send_a_command(0b00001111);
    _delay_ms(50);


    while (1)
    {

        itoa(HOU / 10, SHOWHOU, 10);
        send_a_string(SHOWHOU);
        itoa(HOU % 10, SHOWHOU, 10);
        send_a_string(SHOWHOU);
        send_a_string (":");
        send_a_command(0x80 + 3);
        itoa(MIN / 10, SHOWMIN, 10);
        send_a_string(SHOWMIN);
        itoa(MIN % 10, SHOWMIN, 10);
        send_a_string(SHOWMIN);
        send_a_command(0x80 + 5);
        send_a_string (":");
        send_a_command(0x80 + 6);

        itoa(SEC / 10, SHOWSEC, 10);
        send_a_string(SHOWSEC);
        itoa(SEC % 10, SHOWSEC, 10);
        send_a_string(SHOWSEC);

        if (bit_is_set(PINA, 5))
        {
            send_a_string(" ALM:ON ");
            if ((ALHOU == HOU) & (ALMIN == MIN) & (ALSEC == SEC))
            {
                PORTA |= (1 << PINB7);
            }
        }
        if (bit_is_clear(PINA, 5))
        {
            send_a_string(" ALM:OFF");
            PORTA &= ~(1 << PINB7);
        }
        send_a_command(0x80 + 0x40 + 0);

        send_a_string ("ALARM:");
        send_a_command(0x80 + 0x40 + 7);

        itoa(ALHOU / 10, SHOWALHOU, 10);
        send_a_string(SHOWALHOU);
        itoa(ALHOU % 10, SHOWALHOU, 10);
        send_a_string(SHOWALHOU);
        send_a_command(0x80 + 0x40 + 9);
        send_a_string (":");
        send_a_command(0x80 + 0x40 + 10);
        itoa(ALMIN / 10, SHOWALMIN, 10);
        send_a_string(SHOWALMIN);
        itoa(ALMIN % 10, SHOWALMIN, 10);
        send_a_string(SHOWALMIN);
        send_a_command(0x80 + 0x40 + 12);
        send_a_string (":");
        send_a_command(0x80 + 0x40 + 13);

        itoa(ALSEC / 10, SHOWALSEC, 10);
        send_a_string(SHOWALSEC);
        itoa(ALSEC % 10, SHOWALSEC, 10);
        send_a_string(SHOWALSEC);
        send_a_command(0x80 + 0);
        if (bit_is_set(PINA, 4))
        {
            if (bit_is_clear(PINA, 0))
            {
                if (MIN < 60)
                {
                    MIN++;
                    _delay_ms(220);
                }
                if (MIN == 60)
                {
                    if (HOU < 24)
                    {
                        HOU++;
                    }
                    MIN = 0;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 1))
            {
                if (MIN > 0)
                {
                    MIN--;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 2))
            {
                if (HOU < 24)
                {
                    HOU++;
                }
                _delay_ms(220);
                if (HOU == 24)
                {
                    HOU = 0;
                }
            }
            if (bit_is_clear(PINA, 3))
            {
                if (HOU > 0)
                {
                    HOU--;
                    _delay_ms(220);
                }
            }
        }


        if (bit_is_clear(PINA, 4))
        {
            if (bit_is_clear(PINA, 0))
            {
                if (ALMIN < 60)
                {
                    ALMIN++;
                    _delay_ms(220);
                }
                if (ALMIN == 60)
                {
                    if (ALHOU < 24)
                    {
                        ALHOU++;
                    }
                    ALMIN = 0;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 1))
            {
                if (ALMIN > 0)
                {
                    ALMIN--;
                    _delay_ms(220);
                }
            }
            if (bit_is_clear(PINA, 2))
            {
                if (ALHOU < 24)
                {
                    ALHOU++;
                }
                _delay_ms(220);
                if (ALHOU == 24)
                {
                    ALHOU = 0;
                }
            }
            if (bit_is_clear(PINA, 3))
            {
                if (ALHOU > 0)
                {
                    ALHOU--;
                    _delay_ms(220);
                }
            }
        }
    }
}
ISR(TIMER1_COMPA_vect)
{
    if (SEC < 60)
    {
        SEC++;
    }
    if (SEC == 60)
    {
        if (MIN < 60)
        {
            MIN++;
        }
        SEC = 0;
    }
    if (MIN == 60)
    {
        if (HOU < 24)
        {
            HOU++;
        }
        MIN = 0;
    }
    if (HOU == 24)
    {
        HOU = 0;
    }
}
void send_a_command(unsigned char command)
{
    PORTB = command;
    PORTD &= ~ (1 << registerselection);
    PORTD |= 1 << enable;
    _delay_ms(3);
    PORTD &= ~1 << enable;
    PORTB = 0xFF;
}
void send_a_character(unsigned char character)
{
    PORTB = character;
    PORTD |= 1 << registerselection;
    PORTD |= 1 << enable;
    _delay_ms(3);
    PORTD &= ~1 << enable;
    PORTB = 0xFF;
}
void send_a_string(char *string_of_characters)
{
    while (*string_of_characters > 0)
    {
        send_a_character(*string_of_characters++);
    }
}