/*
    TM1637.cpp
    A library for the 4 digit display

    TM1637 chip datasheet:
    https://www.mcielectronics.cl/website_MCI/static/documents/Datasheet_TM1637.pdf

    Copyright (c) 2012 seeed technology inc.
    Website    : www.seeed.cc
    Author     : Frankie.Chu
    Create Time: 9 April,2012
    Change Log : 14 April,2020 - KORG style alphabets

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/

#include "TM1637.h"
#include <Arduino.h>

//  --0x01--
// |        |
//0x20     0x02
// |        |
//  --0x40- -
// |        |
//0x10     0x04
// |        |
//  --0x08--

namespace {
    uint8_t char2segments(char c) {
        switch (c) {
            case '_' : return 0x08;
            case '^' : return 0x01; // ¯
            case '-' : return 0x40;
            case '*' : return 0x63; // °
            case ' ' : return 0x00; // space
            case 'A' : return 0x77; // upper case A
            case 'a' : return 0x5f; // lower case a
            case 'B' :              // lower case b
            case 'b' : return 0x7c; // lower case b
            case 'C' : return 0x39; // upper case C
            case 'c' : return 0x58; // lower case c
            case 'D' :              // lower case d
            case 'd' : return 0x5e; // lower case d
            case 'E' :              // upper case E
            case 'e' : return 0x79; // upper case E
            case 'F' :              // upper case F
            case 'f' : return 0x71; // upper case F
            case 'G' :              // upper case G
            case 'g' : return 0x35; // upper case G
            case 'H' : return 0x76; // upper case H
            case 'h' : return 0x74; // lower case h
            case 'I' : return 0x06; // 1
            case 'i' : return 0x04; // lower case i
            case 'J' : return 0x1e; // upper case J
            case 'j' : return 0x16; // lower case j
            case 'K' :              // upper case K
            case 'k' : return 0x75; // upper case K
            case 'L' :              // upper case L
            case 'l' : return 0x38; // upper case L
            case 'M' :              // twice tall n
            case 'm' : return 0x37; // twice tall ∩
            case 'N' :              // lower case n
            case 'n' : return 0x54; // lower case n
            case 'O' :              // lower case o
            case 'o' : return 0x5c; // lower case o
            case 'P' :              // upper case P
            case 'p' : return 0x73; // upper case P
            case 'Q' : return 0x7b; // upper case Q
            case 'q' : return 0x67; // lower case q
            case 'R' :              // lower case r
            case 'r' : return 0x50; // lower case r
            case 'S' :              // 5
            case 's' : return 0x6d; // 5
            case 'T' :              // lower case t
            case 't' : return 0x78; // lower case t
            case 'U' :              // lower case u
            case 'u' : return 0x1c; // lower case u
            case 'V' :              // twice tall u
            case 'v' : return 0x3e; // twice tall u
            case 'W' : return 0x7e; // upside down A
            case 'w' : return 0x2a; // separated w
            case 'X' :              // upper case H
            case 'x' : return 0x76; // upper case H
            case 'Y' :              // lower case y
            case 'y' : return 0x6e; // lower case y
            case 'Z' :              // separated Z
            case 'z' : return 0x1b; // separated Z
        }
        return 0;
    }
}

static int8_t tube_tab[] = {0x3f, 0x06, 0x5b, 0x4f,
                            0x66, 0x6d, 0x7d, 0x07,
                            0x7f, 0x6f, 0x77, 0x7c,
                            0x39, 0x5e, 0x79, 0x71
                           }; //0~9,A,b,C,d,E,F

TM1637::TM1637(uint8_t clk, uint8_t data) {
    clkpin = clk;
    datapin = data;
    pinMode(clkpin, OUTPUT);
    pinMode(datapin, OUTPUT);
}

void TM1637::init(void) {
    clearDisplay();
}

int TM1637::writeByte(int8_t wr_data) {
    for (uint8_t i = 0; i < 8; i++) { // Sent 8bit data
        digitalWrite(clkpin, LOW);

        if (wr_data & 0x01) {
            digitalWrite(datapin, HIGH);    // LSB first
        } else {
            digitalWrite(datapin, LOW);
        }

        wr_data >>= 1;
        digitalWrite(clkpin, HIGH);
    }

    digitalWrite(clkpin, LOW); // Wait for the ACK
    digitalWrite(datapin, HIGH);
    digitalWrite(clkpin, HIGH);
    pinMode(datapin, INPUT);

    bitDelay();
    uint8_t ack = digitalRead(datapin);

    if (ack == 0) {
        pinMode(datapin, OUTPUT);
        digitalWrite(datapin, LOW);
    }

    bitDelay();
    pinMode(datapin, OUTPUT);
    bitDelay();

    return ack;
}

// Send start signal to TM1637 (start = when both pins goes low)
void TM1637::start(void) {
    digitalWrite(clkpin, HIGH);
    digitalWrite(datapin, HIGH);
    digitalWrite(datapin, LOW);
    digitalWrite(clkpin, LOW);
}

// End of transmission (stop = when both pins goes high)
void TM1637::stop(void) {
    digitalWrite(clkpin, LOW);
    digitalWrite(datapin, LOW);
    digitalWrite(clkpin, HIGH);
    digitalWrite(datapin, HIGH);
}

// Display function.Write to full-screen.
void TM1637::display(int8_t disp_data[]) {
    int8_t seg_data[DIGITS];
    uint8_t i;

    for (i = 0; i < DIGITS; i++) {
        seg_data[i] = disp_data[i];
    }

    coding(seg_data);
    start();              // Start signal sent to TM1637 from MCU
    writeByte(ADDR_AUTO); // Command1: Set data
    stop();
    start();
    writeByte(cmd_set_addr); // Command2: Set address (automatic address adding)

    for (i = 0; i < DIGITS; i++) {
        writeByte(seg_data[i]);    // Transfer display data (8 bits x num_of_digits)
    }

    stop();
    start();
    writeByte(cmd_disp_ctrl); // Control display
    stop();
}

//******************************************
void TM1637::display(uint8_t bit_addr, int8_t disp_data) {
    int8_t seg_data;

    seg_data = coding(disp_data);
    start();               // Start signal sent to TM1637 from MCU
    writeByte(ADDR_FIXED); // Command1: Set data
    stop();
    start();
    writeByte(bit_addr | 0xc0); // Command2: Set data (fixed address)
    writeByte(seg_data);        // Transfer display data 8 bits
    stop();
    start();
    writeByte(cmd_disp_ctrl); // Control display
    stop();
}

//--------------------------------------------------------

void TM1637::displayNum(float num, int decimal, bool show_minus) {
    // Displays number with decimal places (no decimal point implementation)
    // Colon is used instead of decimal point if decimal == 2
    // Be aware of int size limitations (up to +-2^15 = +-32767)

    int number = round(fabs(num) * pow(10, decimal));

    if (decimal == 2) {
        point(true);
    } else {
        point(false);
    }

    for (int i = 0; i < DIGITS - (show_minus && num < 0 ? 1 : 0); ++i) {
        int j = DIGITS - i - 1;

        if (number != 0) {
            display(j, number % 10);
        } else {
            display(j, 0x7f);    // display nothing
        }

        number /= 10;
    }

    if (show_minus && num < 0) {
        display(0, '-');    // Display '-'
    }
}

void TM1637::displayStr(char str[], uint16_t loop_delay) {
		int end = strlen(str);
		if(end <= DIGITS){
			for (int i = 0; i < DIGITS; i++) {
				if(i<0 || i>=end){ // display nothing on the remaining display
						display(i,0x7f);
				}
				else{
        	display(i, str[i]);
        }
    	}
		}
		else{
			int offset=-DIGITS;

			for (int i = 0; i <= end+DIGITS; i++) {
				for (int j = offset, k=0; j < DIGITS+offset; j++,k++) {
					if(j<0 || j>=end){
						display(k,0x7f);
					}
					else{
        		display(k, str[j]);
        	}
    		}
    		offset++;
    		delay(loop_delay); //loop delay
			}
		}
}

void TM1637::clearDisplay(void) {
    display(0x00, 0x7f);
    display(0x01, 0x7f);
    display(0x02, 0x7f);
    display(0x03, 0x7f);
}

// To take effect the next time it displays.
void TM1637::set(uint8_t brightness, uint8_t set_data, uint8_t set_addr) {
    cmd_set_data = set_data;
    cmd_set_addr = set_addr;
    //Set the brightness and it takes effect the next time it displays.
    cmd_disp_ctrl = 0x88 + brightness;
}

// Whether to light the clock point ":".
// To take effect the next time it displays.
void TM1637::point(boolean PointFlag) {
    _PointFlag = PointFlag;
}

void TM1637::coding(int8_t disp_data[]) {
    for (uint8_t i = 0; i < DIGITS; i++) {
        disp_data[i] = coding(disp_data[i]);
    }
}

int8_t TM1637::coding(int8_t disp_data) {
    if (disp_data == 0x7f) {
        disp_data = 0x00;    // Clear digit
    } else if (disp_data >= 0 && disp_data < int(sizeof(tube_tab) / sizeof(*tube_tab))) {
        disp_data = tube_tab[disp_data];
    } else if (disp_data >= '0' && disp_data <= '9') {
        disp_data = tube_tab[int(disp_data) - 48];    // char to int (char "0" = ASCII 48)
    } else {
        disp_data = char2segments(disp_data);
    }
    disp_data += _PointFlag == POINT_ON ? 0x80 : 0;

    return disp_data;
}

void TM1637::bitDelay(void) {
    delayMicroseconds(50);
}
