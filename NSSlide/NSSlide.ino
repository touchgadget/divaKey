/*
 * MIT License
 *
 * Copyright (c) 2021 touchgadgetdev@gmail.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//#define DEBUG_MPR121
#define DEBUG_ON  0
#if DEBUG_ON
#define DBG_begin(...)    Serial.begin(__VA_ARGS__)
#define DBG_print(...)    Serial.print(__VA_ARGS__)
#define DBG_println(...)  Serial.println(__VA_ARGS__)
#else
#define DBG_begin(...)
#define DBG_print(...)
#define DBG_println(...)
#endif

#define HAS_DOTSTAR_LED (defined(ADAFRUIT_TRINKET_M0) || defined(ADAFRUIT_ITSYBITSY_M0) || defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS))
#if HAS_DOTSTAR_LED
#include <Adafruit_DotStar.h>
#if defined(ADAFRUIT_ITSYBITSY_M4_EXPRESS)
#define DATAPIN    8
#define CLOCKPIN   6
#elif defined(ADAFRUIT_ITSYBITSY_M0)
#define DATAPIN    41
#define CLOCKPIN   40
#elif defined(ADAFRUIT_TRINKET_M0)
#define DATAPIN    7
#define CLOCKPIN   8
#endif
Adafruit_DotStar strip = Adafruit_DotStar(1, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
#endif

#include "NSGadget_tinyusb.h"
#include <Bounce2.h>
#include "Adafruit_MPR121.h"
#include <Adafruit_NeoPixel_ZeroDMA.h>

#define MIKU_TEAL   0x006080
#define PIN        12
#define NUM_PIXELS 40

Adafruit_NeoPixel_ZeroDMA slider_leds(NUM_PIXELS, PIN, NEO_GRB);

// BEWARE: Make sure all these pins are available on your board. Some pins
// may be connected to on board devices so cannot be used as inputs.
#define NUM_BUTTONS 5

typedef struct Button_t {
  uint8_t pin;
  uint8_t ns_button;
} Button_t;

const Button_t BUTTONS[NUM_BUTTONS] = {
  {A0, NSButton_A}, // Playstation Circle
  {A1, NSButton_B}, // Playstation Cross
  {A2, NSButton_Y}, // Playstation Square
  {A3, NSButton_X}, // Playstation Triangle
  {A4, NSButton_LeftTrigger}  // Playstation L1
};
Bounce * buttons = new Bounce[NUM_BUTTONS];

NSGamepad Gamepad;

const size_t NUM_MPR121 = 3;
Adafruit_MPR121 MPR121[NUM_MPR121];
const uint8_t MPR121_IRQ[NUM_MPR121] = {10, 9, 7};
bool Use_MPR121[NUM_MPR121];

void setup() {
#if HAS_DOTSTAR_LED
  // Turn off built-in Dotstar RGB LED
  strip.begin();
  strip.clear();
  strip.show();
#endif
  slider_leds.begin();
  slider_leds.show();

  // Sends a clean HID report to the host.
  Gamepad.begin();

  DBG_begin(115200);
  if (DEBUG_ON) {
    while (!Serial && millis() < 2000) delay(1);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Pin 13 has resistor and LED pulling the pin to ground so it does not work as an
  // input. Remove the resistor and/or LED to make pin 13/touch pad button work.
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTONS[i].pin, INPUT_PULLUP );
    buttons[i].interval(5);
  }

  for (uint8_t t = 0; t < NUM_MPR121; t++) {
    MPR121[t] = Adafruit_MPR121();
    DBG_print("MPR121 ");
    DBG_print(t);
    if (MPR121[t].begin(0x5A + t)) {
      DBG_println(" found");
      Use_MPR121[t] = true;
      MPR121[t].setThresholds(12, 6);
    }
    else {
      DBG_println(" not found");
      Use_MPR121[t] = false;
    }
  }
  Wire.setClock(400000);  // set i2c clock to 400,000 Hz
  // wait until device mounted
  while( !USBDevice.mounted() ) delay(1);
}

void slider_leds_update(uint32_t slider_bits)
{
  static uint32_t slider_old = 0;

  if (slider_old == slider_bits) return;
  slider_old = slider_bits;

  for (int i = 0; i < 32; i++) {
    int interp_pixel = ((i * NUM_PIXELS) + 16) >> 5;
    if (slider_bits & 1) {
      slider_leds.setPixelColor(interp_pixel, MIKU_TEAL);
    }
    else {
      slider_leds.setPixelColor(interp_pixel, 0x000000);
    }
    slider_bits >>= 1;
  }
  slider_leds.show();
}

void check_buttons(void)
{
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
    if ( buttons[i].fell() ) {
      Gamepad.press(BUTTONS[i].ns_button);
    }
    else if ( buttons[i].rose() ) {
      Gamepad.release(BUTTONS[i].ns_button);
    }
  }
}

void loop()
{
  static size_t mpr121_touch = 0;
  static size_t samd_touch = 0;
  static const int SHIFT_BITS[NUM_MPR121] = {20, 8, -4};
  static uint32_t slider = 0;
  digitalWrite(LED_BUILTIN, HIGH);

  if (Use_MPR121[mpr121_touch] && !digitalRead(MPR121_IRQ[mpr121_touch])) {
    uint16_t touchnow = MPR121[mpr121_touch].touched();
    static uint16_t touchold[NUM_MPR121];
    if (touchnow != touchold[mpr121_touch]) {
#ifdef DEBUG_MPR121
      DBG_print("touchnow=");
      DBG_println(touchnow, BIN);
      DBG_print("touchold=");
      DBG_println(touchold[mpr121_touch], BIN);
#endif
      int shift = SHIFT_BITS[mpr121_touch];
      if (shift > 0) {
        slider = slider & ~(0xFFF << shift);
        slider = slider | (touchnow << shift);
      }
      else if (shift < 0) {
        slider = slider & ~0xFF;
        slider = slider | (touchnow >> -shift);
        // Least significant 4 bits are the buttons XYBA
        static const uint8_t BUTTON_MAP[4] = {
          NSButton_A, NSButton_B, NSButton_Y, NSButton_X
        };
        for (int b = 0; b < 4; b++) {
          if (touchnow & (1 << b)) {
            Gamepad.press(BUTTON_MAP[b]);
          }
          else {
            Gamepad.release(BUTTON_MAP[b]);
          }
        }
      }
      Gamepad.allAxes(slider);
      slider_leds_update(slider);
      DBG_print("slider=");
      DBG_println(slider, BIN);
      touchold[mpr121_touch] = touchnow;
    }
  }
  mpr121_touch++;
  if (mpr121_touch >= NUM_MPR121) mpr121_touch = 0;

  check_buttons();
  if ( Gamepad.ready() ) Gamepad.loop();
  digitalWrite(LED_BUILTIN, LOW);
}
