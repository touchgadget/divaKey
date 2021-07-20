/************************************************************************
MIT License

Copyright (c) 2021 touchgadgetdev@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*************************************************************************/
#pragma once
#include "Arduino.h"
#include "Adafruit_TinyUSB.h"

// Dpad directions
typedef uint8_t NSDirection_t;
#define NSGAMEPAD_DPAD_UP  0
#define NSGAMEPAD_DPAD_UP_RIGHT 1
#define NSGAMEPAD_DPAD_RIGHT 2
#define NSGAMEPAD_DPAD_DOWN_RIGHT 3
#define NSGAMEPAD_DPAD_DOWN 4
#define NSGAMEPAD_DPAD_DOWN_LEFT 5
#define NSGAMEPAD_DPAD_LEFT 6
#define NSGAMEPAD_DPAD_UP_LEFT 7
#define NSGAMEPAD_DPAD_CENTERED 0xF

enum NSButtons {
  NSButton_Y = 0,
  NSButton_B,
  NSButton_A,
  NSButton_X,
  NSButton_LeftTrigger,
  NSButton_RightTrigger,
  NSButton_LeftThrottle,
  NSButton_RightThrottle,
  NSButton_Minus,
  NSButton_Plus,
  NSButton_LeftStick,
  NSButton_RightStick,
  NSButton_Home,
  NSButton_Capture,
  NSButton_Reserved1,
  NSButton_Reserved2
};

#define ATTRIBUTE_PACKED  __attribute__((packed, aligned(1)))

// 14 Buttons, 4 Axes, 1 D-Pad
typedef struct ATTRIBUTE_PACKED {
    uint16_t buttons;

    uint8_t	dPad;

    uint8_t	leftXAxis;
    uint8_t	leftYAxis;

    uint8_t	rightXAxis;
    uint8_t	rightYAxis;
    uint8_t filler;
} HID_NSGamepadReport_Data_t;

// HID report descriptor using TinyUSB's template
// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] =
{
  // Gamepad for Nintendo Switch Hori Horipad
  // 14 buttons, 1 8-way dpad, 2 analog sticks (4 axes)
  0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
  0x09, 0x05,        // Usage (Game Pad)
  0xA1, 0x01,        // Collection (Application)
  0x15, 0x00,        //   Logical Minimum (0)
  0x25, 0x01,        //   Logical Maximum (1)
  0x35, 0x00,        //   Physical Minimum (0)
  0x45, 0x01,        //   Physical Maximum (1)
  0x75, 0x01,        //   Report Size (1)
  0x95, 0x0E,        //   Report Count (14)
  0x05, 0x09,        //   Usage Page (Button)
  0x19, 0x01,        //   Usage Minimum (0x01)
  0x29, 0x0E,        //   Usage Maximum (0x0E)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x95, 0x02,        //   Report Count (2)
  0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
  0x25, 0x07,        //   Logical Maximum (7)
  0x46, 0x3B, 0x01,  //   Physical Maximum (315)
  0x75, 0x04,        //   Report Size (4)
  0x95, 0x01,        //   Report Count (1)
  0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
  0x09, 0x39,        //   Usage (Hat switch)
  0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
  0x65, 0x00,        //   Unit (None)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x26, 0xFF, 0x00,  //   Logical Maximum (255)
  0x46, 0xFF, 0x00,  //   Physical Maximum (255)
  0x09, 0x30,        //   Usage (X)
  0x09, 0x31,        //   Usage (Y)
  0x09, 0x32,        //   Usage (Z)
  0x09, 0x35,        //   Usage (Rz)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x04,        //   Report Count (4)
  0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0x75, 0x08,        //   Report Size (8)
  0x95, 0x01,        //   Report Count (1)
  0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  0xC0,              // End Collection
};

class NSGamepad {
  public:
    inline NSGamepad(void);

    inline void begin(void);
    inline void end(void);
    inline void loop(void);
    inline void write(void);
    inline void write(void *report);
    inline void press(uint8_t b);
    inline void release(uint8_t b);
    inline void releaseAll(void);

    inline void buttons(uint16_t b);
    inline void leftXAxis(uint8_t a);
    inline void leftYAxis(uint8_t a);
    inline void rightXAxis(uint8_t a);
    inline void rightYAxis(uint8_t a);
    inline void allAxes(uint32_t RYRXLYLX);
    inline void allAxes(uint8_t RY, uint8_t RX, uint8_t LY, uint8_t LX);
    inline void dPad(NSDirection_t d);
    inline void dPad(bool up, bool down, bool left, bool right);
    inline bool ready(void);

    // Sending is public for advanced users.
    inline bool SendReport(void* data, size_t length);

  protected:
    HID_NSGamepadReport_Data_t _report;
    uint32_t startMillis;
    Adafruit_USBD_HID usb_hid;
};

NSGamepad::NSGamepad(void)
{
  // setStringDescriptor is undefined on nRF52840
  // this->usb_hid.setStringDescriptor("TinyUSB Gamepad");
}

void NSGamepad::begin(void)
{
  this->usb_hid.setPollInterval(1);
  this->usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  USBDevice.setID(0x0f0d, 0x00c1);
  this->usb_hid.begin();

  // release all buttons, center all sticks, etc.
  end();
  startMillis = millis();
}

void NSGamepad::loop(void)
{
  if (startMillis != millis()) {
    write();
    startMillis = millis();
  }
}

void NSGamepad::end(void)
{
  memset(&_report, 0x00, sizeof(_report));
  _report.leftXAxis = _report.leftYAxis = 0x80;
  _report.rightXAxis = _report.rightYAxis = 0x80;
  _report.dPad = NSGAMEPAD_DPAD_CENTERED;
  SendReport(&_report, sizeof(_report));
}

void NSGamepad::write(void)
{
  SendReport(&_report, sizeof(_report));
}

void NSGamepad::write(void *report)
{
  memcpy(&_report, report, sizeof(_report));
  SendReport(&_report, sizeof(_report));
}

void NSGamepad::press(uint8_t b)
{
  b &= 0xF; // Limit value between 0..15
  _report.buttons |= (uint16_t)1 << b;
}


void NSGamepad::release(uint8_t b)
{
  b &= 0xF; // Limit value between 0..15
  _report.buttons &= ~((uint16_t)1 << b);
}


void NSGamepad::releaseAll(void)
{
  _report.buttons = 0;
}

void NSGamepad::buttons(uint16_t b)
{
  _report.buttons = b;
}


void NSGamepad::leftXAxis(uint8_t a)
{
  _report.leftXAxis = a;
}


void NSGamepad::leftYAxis(uint8_t a)
{
  _report.leftYAxis = a;
}


void NSGamepad::rightXAxis(uint8_t a)
{
  _report.rightXAxis = a;
}


void NSGamepad::rightYAxis(uint8_t a)
{
  _report.rightYAxis = a;
}

void NSGamepad::allAxes(uint32_t RYRXLYLX)
{
  _report.rightYAxis = ((RYRXLYLX >> 24) & 0xFF) ^ 0x80;
  _report.rightXAxis = ((RYRXLYLX >> 16) & 0xFF) ^ 0x80;
  _report.leftYAxis  = ((RYRXLYLX >>  8) & 0xFF) ^ 0x80;
  _report.leftXAxis  = ((RYRXLYLX      ) & 0xFF) ^ 0x80;
}

void NSGamepad::allAxes(uint8_t RY, uint8_t RX, uint8_t LY, uint8_t LX)
{
  _report.rightYAxis = RY ^ 0x80;
  _report.rightXAxis = RX ^ 0x80;
  _report.leftYAxis  = LY ^ 0x80;
  _report.leftXAxis  = LX ^ 0x80;
}

void NSGamepad::dPad(NSDirection_t d)
{
  _report.dPad = d;
}

// The direction pad is limited to 8 directions plus centered. This means
// some combinations of 4 dpad buttons are not valid and cannot be sent.
// Button down = true(1)
// Valid: Any 1 button down, any 2 adjacent buttons down, no buttons down
// Invalid: all other combinations
void NSGamepad::dPad(bool up, bool down, bool left, bool right)
{
    static const NSDirection_t BITS2DIR [16] = {
        NSGAMEPAD_DPAD_CENTERED,    // 0000
        NSGAMEPAD_DPAD_RIGHT,       // 0001
        NSGAMEPAD_DPAD_LEFT,        // 0010
        NSGAMEPAD_DPAD_CENTERED,    // 0011
        NSGAMEPAD_DPAD_DOWN,        // 0100
        NSGAMEPAD_DPAD_DOWN_RIGHT,  // 0101
        NSGAMEPAD_DPAD_DOWN_LEFT,   // 0110
        NSGAMEPAD_DPAD_CENTERED,    // 0111
        NSGAMEPAD_DPAD_UP,          // 1000
        NSGAMEPAD_DPAD_UP_RIGHT,    // 1001
        NSGAMEPAD_DPAD_UP_LEFT,     // 1010
        NSGAMEPAD_DPAD_CENTERED,    // 1011
        NSGAMEPAD_DPAD_CENTERED,    // 1100
        NSGAMEPAD_DPAD_CENTERED,    // 1101
        NSGAMEPAD_DPAD_CENTERED,    // 1110
        NSGAMEPAD_DPAD_CENTERED     // 1111
    };
    uint8_t dpad_bits = (up << 3) | (down << 2) | (left << 1) | (right << 0);
    _report.dPad = BITS2DIR[dpad_bits];
}

bool NSGamepad::ready(void)
{
  return this->usb_hid.ready();
};

bool NSGamepad::SendReport(void* data, size_t length)
{
    return this->usb_hid.sendReport(0, data, (uint8_t)length);
};
