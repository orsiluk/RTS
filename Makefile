AS = gpasm
CC = sdcc
CFLAGS= -c -mpic16 -p18f97j60  -o$@ 
LD = sdcc
LDFLAGS= -mpic16 -p18f97j60 -L/usr/local/share/sdcc/lib/pic16 \
         -L/usr/local/share/sdcc/lib/src/pic16/libc -llibio18f97j60.lib \
         -llibdev18f97j60.lib -llibc18f.a
AR = ar
RM = rm

OBJECTS= Objects/LCDBlocking.o

SDCC_HEADERS=/usr/local/share/sdcc/include/string.h \
   /usr/local/share/sdcc/include/stdlib.h \
   /usr/local/share/sdcc/include/stdio.h \
   /usr/local/share/sdcc/include/stddef.h \
   /usr/local/share/sdcc/include/stdarg.h 

SDCC_PIC16_HEADERS=/usr/local/share/sdcc/include/pic16/pic18f97j60.h

TCPIP_HEADERS=   Include/TCPIP_Stack/ETH97J60.h \
   Include/TCPIP_Stack/LCDBlocking.h 

APP_HEADERS=Include/GenericTypeDefs.h \
   Include/Compiler.h \
   Include/HardwareProfile.h 

clock : Objects/clock.o $(OBJECTS)
	$(LD) $(LDFLAGS) Objects/clock.o $(OBJECTS)

test : Objects/test.o $(OBJECTS)
	$(LD) $(LDFLAGS) Objects/test.o $(OBJECTS)

testint : Objects/testint.o $(OBJECTS)
	$(LD) $(LDFLAGS) Objects/testint.o $(OBJECTS)

led : Objects/led.o
	$(LD) $(LDFLAGS) Objects/led.o

Objects/clock.o : clock.c $(SDCC_HEADERS) $(SDCC_PIC16_HEADERS) \
   $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) $(CFLAGS) clock.c

Objects/testint.o : testint.c $(SDCC_HEADERS) $(SDCC_PIC16_HEADERS) \
   $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) $(CFLAGS) testint.c

Objects/test.o : test.c $(SDCC_HEADERS) $(SDCC_PIC16_HEADERS) \
   $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) $(CFLAGS) test.c

Objects/led.o : led.c $(SDCC_HEADERS) $(SDCC_PIC16_HEADERS) \
   $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) $(CFLAGS) led.c


Objects/LCDBlocking.o : TCPIP_Stack/LCDBlocking.c $(SDCC_HEADERS)  \
   $(SDCC_PIC16_HEADERS) $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) -c -mpic16 -p18f97j60  -o"Objects/LCDBlocking.o" \
              -L/usr/local/lib/pic16  TCPIP_Stack/LCDBlocking.c

Objects/Tick.o : TCPIP_Stack/Tick.c  $(SDCC_HEADERS)  \
   $(SDCC_PIC16_HEADERS) $(APP_HEADERS) $(TCPIP_HEADERS)
	$(CC) -c -mpic16 -p18f97j60  -o"Objects/Tick.o" \
              -L/usr/local/lib/pic16  TCPIP_Stack/Tick.c

clean : 
	$(RM) $(OBJECTS)

