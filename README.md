MicroMenu
=========
Copyright (c) 2014 karl@pitrich.com


Simple and concise Embedded Menu System for Arduino.

**Please check out the comprehensive example applications.**


*Note: This has been developed and tested with Arduino 1.5.x only.*

## Demo Videos

**TODO**

## Feature Set

#### Simple API
 - Only three core methods, `navigate()`, `invoke()` and `render()`,
 - plus simple **helpers** for navigation: `next()`, `prev()`, `parent()` and `child()`.

#### Any hardware
 - You simply implement a handler for your
   - Clickwheels, Encoders (see examples)
   - < > Buttons
 - that will call `navigate()` with any of the *helpers* as an argument, depending on direction intended (e.g. `navigate(child())` or `navigate(next())`).

#### Great display flexibility 
 - Usable with any display: Text LCD, Graphic LCD, Serial Port, ...
 - Any menu type: vertical or horizontal, circular, ...
   - you only need to implement one method capable of rendering a single menu item

#### Single callback per menu item, multiple actions
Handles all events, eg.
 - Show the menu item label
   - can also be used to display current sensor data instead of the static text label
 - Display the menu widget
   - this is where you'd implement your 'widget', the actual funcktionality associate with a menu item
 - Trigger at the menu item
   - invoked when the user selects the item again, if already active
   - useful for: 
     - save
     - accept calibration data when displaying live values
     - switching between two options or functions within a single menu item
       - e.g. to configure *precision* and *unit* for numeric values in a single menu (000.0 -> 00.00 -> 0.000 <trigger> mV -> V -> kV
         - so you see a live preview 00.00kW or 000.0mV
 - Before Move to Parent (or exit the menu)
   - for instance to call a save method when you exit
   - the return value determines if the user can move to the parent or not, useful to handle unsave data
- Stores menu data in Flash, saving RAM


## Installation

[Clone] this repository or [download] the current version as an archive, copy or link it to your Arduino libraries folder, e.g. `~/Documents/Arduino/libraries` on a Mac, then restart the Arduino IDE.

[download]:https://github.com/0xPIT/menu/archive/master.zip
[Clone]:git@github.com:0xPIT/menu.git


## Usage and Concept

The code base resides in it's own namespace `Menu`. So you need to either prefix all methods with `Menu::` or add `using namespace Menu;` somewhere in your code. I prefer to be explicit by opting for the prefix.

#### 1) Define menu an structure
Begin with a dummy `exit` menu item. With it, you can utilize all events and functions, e.g. the parent event will be invoked, allowing you to save all data when you leave the menu or even prevent to leave.
Also, you can simply clear the display when the user leaves the menu.
The arguments to the macro `MenuItem` are: Name, Label, Next, Previous, Parent, Child, Callback. If you have no child or sibling or parent, substitute with `Menu::NullItem`, this is a typesafe replacement for `NULL`

    MenuItem(miExit, "", Menu::NullItem, Menu::NullItem, Menu::NullItem, miSettings, menuExit);
        MenuItem(miSettings, "Settings", miTest1, Menu::NullItem, miExit, miCalibrateLo, menuDummy);
          MenuItem(miCalibrateLo,  "Calibrate Lo", miCalibrateHi,  Menu::NullItem,       miSettings, Menu::NullItem, menuDummy);
          MenuItem(miCalibrateHi,  "Calibrate Hi", miChannel0, miCalibrateLo,  miSettings, Menu::NullItem, menuDummy);


#### 2) Implement a callback to render a single menu item
This example is for a 4-line text LCD, usable with the LiquidCrystal library shipped with Arduino.

    void renderMenuItem(const Menu::Item_t *mi, uint8_t pos) {
        lcd.setCursor(0, pos);
        lcd.print(engine->label(mi));
    }

#### 3) Handle menu events

     bool myMenuItemCallback(Menu::Action_t act) {
      switch (act) {
        case actionParent: 
          saveAll();
          return true; // return false to prevent navigation to parent
          break;
        case actionTrigger:
          adcSaveCalibrationData();
          break;
      }
    }

## Example Hardware Configuration

TODO: Image LCD and TFT Boards

## Licensing

```
The MIT License (MIT)

Copyright (c) 2014 karl@pitrich.com
All rights reserved.

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
```
