/* Saitek X45 Throttle unit USB adapter for Teensy++ 2.0

  The Throttle unit has keyboard diode matrix for buttons, hats and mode switches.
  
  Connecting throttle unit's D15 pins to Teensy (Arduino):
    - button matrix rows 1-4 from D15 pins 7, 10, 14, 2 to Teensy D0-D3 (0-3)
    - button matrix columns 1-4 from D15 pins 12, 15, 1, 9 to Teensy C0-D3 (10-13)
    - Rotary 2 from D15 pin 3 to Teensy F0 (A0)
    - Rotary 1 from D15 pin 6 to Teensy F1 (A1)
    - Throttle from D15 pin 11 to Teensy F2 (A2)
    - Rudder from D15 pin 13 to Teensy F3 (A3)
    - D15 pin 8 to Teensy Vcc
    - D15 pins 4 and 5 to Teensy GND
    - D15 shield to Teensy's USB shield

  X45's tri-state Mode switch (M1-M2-M3) makes all buttons and hats to have different
  USB Joystick buttons in each mode. Except for the X45's tri-state Aux switch, which
  is always mapped to issue button 31 when low, button 32 when high and nothing when
  in the middle position.
*/

#include <Keypad.h>

// Joystick's diode matrix for hats, buttons and mode switches
const byte ROWS = 4;
const byte COLS = 4;
byte rawButtonIds[ROWS][COLS] = {
  {1, 2, 3, 4},
  {5, 6, 7, 8},
  {9, 10, 11, 12},
  {13, 14, 15, 16}
};
byte rowPins[ROWS] = {0, 1, 2, 3}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {10, 11, 12, 13}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(rawButtonIds), rowPins, colPins, ROWS, COLS );

byte rawButtons[ROWS*COLS+1]; // state of each raw button from the keyboard diode matrix

// Mapping raw button indexes to joystick functions
const byte x45M1 = 4;
const byte x45M3 = 16;
const byte x45Aux1 = 12;
const byte x45Aux3 = 8;
const byte x45Hat3Up = 6;
const byte x45Hat3Down = 14;
const byte x45Hat3Left = 2;
const byte x45Hat3Right = 10;
const byte x45MouseUp = 7;
const byte x45MouseDown = 15;
const byte x45MouseLeft = 3;
const byte x45MouseRight = 11;
const byte x45MouseButton = 1;
const byte x45ButtonD = 13;

// Mode switch values
byte x45CurrentMode = 0; // M1 = 1, M2 = 2, M3 = 3
byte x45CurrentAux = 0; // Aux1 Low = 1, Mid = 2, Top = 3

void setup()
{
  Serial.begin(9600);
  Joystick.useManualSend(true);
  keypad.setDebounceTime(1);
  Serial.println("Begin Complete Joystick Test");
}

void loop()
{
  // read 4 analog inputs and use them for the joystick axis
  Joystick.X(analogRead(0));
  Joystick.Y(analogRead(1));
  Joystick.Z(analogRead(2));
  Joystick.Zrotate(analogRead(3));

  if (keypad.getKeys())
  {
    for (int i=0; i<LIST_MAX; i++) // Scan the whole key list
    {
      if ( keypad.key[i].stateChanged ) // Only find keys that have changed state
      {
      const byte rawButtonIndex = keypad.key[i].kchar;
      switch (keypad.key[i].kstate)
        { // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
          case HOLD:
            rawButtons[rawButtonIndex] = 1;
            break;
          case RELEASED:
          case IDLE:
            rawButtons[rawButtonIndex] = 0;
            break;
        }
      }
    }
  }

  // Calculate X45 Mode and Aux switch positions
  if (rawButtons[x45M1])
    x45CurrentMode = 1;
  else if (rawButtons[x45M3])
    x45CurrentMode = 3;
  else
    x45CurrentMode = 2;

  if (rawButtons[x45Aux1])
    x45CurrentAux = 1;
  else if (rawButtons[x45Aux3])
    x45CurrentAux = 3;
  else
    x45CurrentAux = 2;

  // AUX as 1-0-2
  if (x45CurrentAux == 1)
    {
      Joystick.button(31, 1);
      Joystick.button(32, 0);
    }
    else if (x45CurrentAux == 3)
    {
      Joystick.button(31, 0);
      Joystick.button(32, 1);
    }
    else
    {
      Joystick.button(31, 0);
      Joystick.button(32, 0);
    }
  
  // Calculate the USB Joystick button states based on the raw buttons
  const byte base = (x45CurrentMode - 1) * 6;
  Joystick.button(base + 1, rawButtons[x45ButtonD]);
  Joystick.button(base + 2, rawButtons[x45MouseButton]);
  
  // X45 Hat3 switch as just buttons
  Joystick.button(base + 3, rawButtons[x45Hat3Up]);
  Joystick.button(base + 4, rawButtons[x45Hat3Right]);
  Joystick.button(base + 5, rawButtons[x45Hat3Down]);
  Joystick.button(base + 6, rawButtons[x45Hat3Left]);

  // Mouse Hat angle
  if (rawButtons[x45MouseUp])
  {
    if (rawButtons[x45MouseRight])
      Joystick.hat(45);
    else if (rawButtons[x45MouseLeft])
      Joystick.hat(315);
    else
      Joystick.hat(0);
  }
  else if (rawButtons[x45MouseDown])
  {
    if (rawButtons[x45MouseRight])
      Joystick.hat(135);
    else if (rawButtons[x45MouseLeft])
      Joystick.hat(225);
    else
      Joystick.hat(180);
  }
  else if (rawButtons[x45MouseLeft])
      Joystick.hat(270);
  else if (rawButtons[x45MouseRight])
      Joystick.hat(90);
  else
      Joystick.hat(-1);
  
  // Because setup configured the Joystick manual send,
  // the computer does not see any of the changes yet.
  // This send_now() transmits everything all at once.
  Joystick.send_now();
    
  // a brief delay, so this runs "only" 200 times per second
  delay(5);
}
