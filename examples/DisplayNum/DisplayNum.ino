/*
    TM1637.cpp
    A library for the 4 digit display

    Copyright (c) 2012 seeed technology inc.
    Website    : www.seeed.cc
    Author     : Frankie.Chu
    Create Time: 9 April,2012
    Change Log :

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

// Pins definitions for TM1637 and can be changed to other ports
const int CLK = 6;
const int DIO = 7;
TM1637 tm1637(CLK, DIO);

float number = -1234.5;
int mult = 1;

void setup() {
    tm1637.init();
    tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

    float num = -123.45;

    // Without specifying decimal pointt it displays int
    tm1637.displayNum(num);    // -123
    delay(2000);

    // When numbers doesn't fit biggest digits are truncated
    tm1637.displayNum(num, 1); // -234
    delay(2000);

    // Colon light up as decimal point
    tm1637.displayNum(num, 2);
    delay(2000);

    // Note that adding 3 digits (-123450) makes number out of int range
    tm1637.displayNum(num, 3);
    delay(2000);

    // Display negative numbers without sign
    tm1637.displayNum(num, 0, false);
    delay(2000);
}

void loop() {
    tm1637.displayNum(number);

    number += 17.42 * mult;
    if (abs(number) > 1500) {
        mult *= -1;
    }

    delay(50);
}
