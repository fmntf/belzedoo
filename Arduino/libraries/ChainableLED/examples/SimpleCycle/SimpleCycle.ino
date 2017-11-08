
/* 
 * Example of using the ChainableRGB library for controlling a Grove RGB.
 * This code puts a red dot (led) moving along a strip of blue dots.
 */

#include <ChainableLED.h>

#define cose 8
#define NUM_LEDS  1

ChainableLED leds(cose, cose+1, NUM_LEDS);

void setup()
{
  leds.init();
}

byte pos = 0;

void loop()
{
  leds.setColorRGB(0, 255, 0, 0);
    delay(250);
  leds.setColorRGB(0, 0, 255, 0);
    delay(250);
  leds.setColorRGB(0, 0, 0, 255);
    delay(250);
  leds.setColorRGB(0, 255, 255, 0);
    delay(250);
  leds.setColorRGB(0, 0, 255, 255);
    delay(250);
  leds.setColorRGB(0, 255, 0, 255);
    delay(250);
  leds.setColorRGB(0, 255, 255, 255);
    delay(250);
     leds.setColorRGB(0, 0, 0, 0);
    delay(250);
}

