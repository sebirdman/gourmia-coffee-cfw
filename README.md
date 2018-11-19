# gourmia-coffee-cfw

# Pinout from ESP8266 to HT1621

ESP8266	HT1621
GPIO4	CS
GPIO13 	DATA
GPIO14	WR

# Pinout from ESP8266 to MCP23S17

ESP8266	MCP23S17
GPIO16	CS
GPIO14	SCK
GPIO13	SI
GPIO12	SO

# MCP23S17 to coffee hardware

GPA0	LED UP
GPA1	LED DOWN
GPA2	SCREEN LED
GPA3	???
GPA4	GRINDER (header ???)
GPA5	PUMP WATER (water is hot) (header pin5)
GPA6	WARMING PAD (header 1)
GPA7	???

GPB0	BUTTON 1
GPB1	BUTTON 2
GPB2	BUTTON 3
GPB3	???
GPB4	???
GPB5	???
GPB6	BUTTON DOWN
GPB7	BUTTON UP

# Header pin note note:

With the board facing away from you the header pins start at 1 from the left.

# Time

In order to brew coffee predicably, we'll need to know how the hardware timings work

1) From cold start to first drop: 25sec
2) From first drop to full pot: 6min 30sec

# refs

http://ww1.microchip.com/downloads/en/devicedoc/20001952c.pdf