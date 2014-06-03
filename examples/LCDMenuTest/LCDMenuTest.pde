#include <Arduino.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Menu.h>

// ----------------------------------------------------------------------------

#define LCD_RS     8
#define LCD_RW     9
#define LCD_EN    10
#define LCD_D4     4
#define LCD_D5     5
#define LCD_D6     6
#define LCD_D7     7
#define LCD_CHARS 20
#define LCD_LINES  4

enum Icons {
  IconLeft = 0,
  IconRight = 1,
  IconBack = 2,
  IconBlock = 3
};

// ----------------------------------------------------------------------------
// helpers

#define clampValue(val, lo, hi) if (val > hi) val = hi; if (val < lo) val = lo;
#define maxValue(a, b) ((a > b) ? a : b)
#define minValue(a, b) ((a < b) ? a : b)

// ----------------------------------------------------------------------------
// display
LiquidCrystal lcd(LCD_RS, 
#if LCD_RW >= 0 // RW is not necessary if lcd is on dedicated pins
  LCD_RW,
#endif
  LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7
);

// ----------------------------------------------------------------------------
// encoder

ClickEncoder Encoder(A0, A1, A2, 2);

void timerIsr(void) {
  Encoder.service();
}

// ----------------------------------------------------------------------------

Menu::Engine *engine;

// ----------------------------------------------------------------------------

namespace State {  
  typedef enum SystemMode_e {
    None      = 0,
    Default   = (1<<0),
    Settings  = (1<<1),
    Edit      = (1<<2)
  } SystemMode;
};

uint8_t systemState = State::Default;
bool lastEncoderAccelerationState = true;
uint8_t previousSystemState = State::None;

// ----------------------------------------------------------------------------

bool menuExit(const Menu::Action_t a) {
  Encoder.setAccelerationEnabled(lastEncoderAccelerationState);  
  systemState = State::Default;
  return true;
}

bool menuDummy(const Menu::Action_t a) {
  return true;
}

bool menuBack(const Menu::Action_t a) {
  if (a == Menu::actionDisplay) {
    engine->navigate(engine->getParent(engine->getParent()));
  }
  return true;
}

// ----------------------------------------------------------------------------

uint8_t menuItemsVisible = LCD_LINES;

void renderMenuItem(const Menu::Item_t *mi, uint8_t pos) {
  lcd.setCursor(0, pos);

  // cursor
  if (engine->currentItem == mi) {
    lcd.write((uint8_t)IconBlock);
  }
  else {
    lcd.write(20); // space
  }

  lcd.print(engine->getLabel(mi));

  // mark items that have children
  if (engine->getChild(mi) != &Menu::NullItem) {
    lcd.write(20);
    lcd.write((uint8_t)IconRight);
    lcd.print("       ");
  }
}

// ----------------------------------------------------------------------------
// Name, Label, Next, Previous, Parent, Child, Callback

MenuItem(miExit, "", Menu::NullItem, Menu::NullItem, Menu::NullItem, miSettings, menuExit);

MenuItem(miSettings, "Settings", miTest1, Menu::NullItem, miExit, miCalibrateLo, menuDummy);

  MenuItem(miCalibrateLo,  "Calibrate Lo", miCalibrateHi,  Menu::NullItem,       miSettings, Menu::NullItem, menuDummy);
  MenuItem(miCalibrateHi,  "Calibrate Hi", miChannel0, miCalibrateLo,  miSettings, Menu::NullItem, menuDummy);

  MenuItem(miChannel0, "Channel 0", miChannel1, miCalibrateHi, miSettings, miChView0, menuDummy);
    MenuItem(miChView0,  "Ch0:View",  miChScale0,     Menu::NullItem, miChannel0, Menu::NullItem, menuDummy);    
    MenuItem(miChScale0, "Ch0:Scale", Menu::NullItem, miChView0,      miChannel0, Menu::NullItem, menuDummy);    

  MenuItem(miChannel1, "Channel 1", Menu::NullItem, miChannel0, miSettings, miChView1, menuDummy);
    MenuItem(miChView1,  "Ch1:View",  miChScale1,     Menu::NullItem, miChannel1, Menu::NullItem, menuDummy);    
    MenuItem(miChScale1, "Ch1:Scale", miChBack1,      miChView1,      miChannel1, Menu::NullItem, menuDummy); 
    MenuItem(miChBack1,  "Back \02",      Menu::NullItem, miChScale1,     miChannel1, Menu::NullItem, menuBack);

MenuItem(miTest1, "Test 1 Menu", miTest2,        miSettings, miExit, Menu::NullItem, menuDummy);
MenuItem(miTest2, "Test 2 Menu", miTest3,        miTest1,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest3, "Test 3 Menu", miTest4,        miTest2,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest4, "Test 4 Menu", miTest5,        miTest3,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest5, "Test 5 Menu", miTest6,        miTest4,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest6, "Test 6 Menu", miTest7,        miTest5,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest7, "Test 7 Menu", miTest8,        miTest6,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest8, "Test 8 Menu", Menu::NullItem, miTest7,    miExit, Menu::NullItem, menuDummy);

// ----------------------------------------------------------------------------

byte left[8] = {
  0b00010,
  0b00110,
  0b01110,
  0b11110,
  0b01110,
  0b00110,
  0b00010,
  0b00000
};

byte right[8] = {
  0b01000,
  0b01100,
  0b01110,
  0b01111,
  0b01110,
  0b01100,
  0b01000,
  0b00000
};

byte back[8] = {
  0b00000,
  0b00100,
  0b01100,
  0b11111,
  0b01101,
  0b00101,
  0b00001,
  0b00000
};

byte block[8] = {
  0b00000,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b00000
};

// ----------------------------------------------------------------------------

void setup() {
  Serial.begin(57600);
  Serial.print("Starting...");

  lcd.begin(LCD_CHARS, LCD_LINES);
  lcd.clear();
  lcd.setCursor(0, 0);

  lcd.createChar(IconLeft, left);
  lcd.createChar(IconRight, right);
  lcd.createChar(IconBack, back);
  lcd.createChar(IconBlock, block);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 

  engine = new Menu::Engine(&Menu::NullItem);
  menuExit(Menu::actionNone); // reset to initial state
}

// ----------------------------------------------------------------------------

int16_t encMovement;
int16_t encAbsolute;
int16_t encLastAbsolute = -1;
bool updateMenu = false;

// ----------------------------------------------------------------------------

void loop() {
  // handle encoder
  encMovement = Encoder.getValue();
  if (encMovement) {
    encAbsolute += encMovement;

    if (systemState == State::Settings) {
      engine->navigate((encMovement > 0) ? engine->getNext() : engine->getPrev());
      updateMenu = true;
    }
  }

  // handle button
  switch (Encoder.getButton()) {

    case ClickEncoder::Clicked:
      if (systemState == State::Settings) {
        engine->invoke();
        updateMenu = true;
      }
      break;

    case ClickEncoder::DoubleClicked:
      if (systemState == State::Settings) {
        engine->navigate(engine->getParent());
        updateMenu = true;
      }

      if (systemState == State::Default) {
          Encoder.setAccelerationEnabled(!Encoder.getAccelerationEnabled());

          if (LCD_LINES > 2) {
            lcd.setCursor(0, 2);
            lcd.print("Acceleration: ");
            lcd.print((Encoder.getAccelerationEnabled()) ? "on " : "off");
          }          
      }
      break;

    case ClickEncoder::Held:
      if (systemState != State::Settings) { // enter settings menu

        // disable acceleration, reset in menuExit()
        lastEncoderAccelerationState = Encoder.getAccelerationEnabled();
        Encoder.setAccelerationEnabled(false);

        engine->navigate(&miSettings);

        systemState = State::Settings;
        previousSystemState = systemState;
        updateMenu = true;
      }
      break;
  }

  if (updateMenu) {
    updateMenu = false;
    
    if (!encMovement) { // clear menu on child/parent navigation
      lcd.clear();
    }
  
    // render the menu
    engine->render(renderMenuItem, menuItemsVisible);

    /*
    if (LCD_LINES > 2) {
      lcd.setCursor(0, LCD_LINES - 1);
      lcd.print("Doubleclick to ");
      if (engine->getParent() == &miExit) {
        lcd.print("exit. ");
      }
      else {
        lcd.print("go up.");
      }
    }
    */
  }

  // dummy "application"
  if (systemState == State::Default) {
    if (systemState != previousSystemState) {
      previousSystemState = systemState;
      encLastAbsolute = -999;

      lcd.setCursor(0, 0);
      lcd.print("Main Screen");

      if (LCD_LINES > 2) {
        lcd.setCursor(0, LCD_LINES - 1);
        lcd.print("Hold for setup");
      }
    }

    if (encAbsolute != encLastAbsolute) {
      encLastAbsolute = encAbsolute; 
      char tmp[10];
      sprintf(tmp, "%4d", encAbsolute);
      lcd.setCursor(0, 1);
      lcd.print("Position: ");
      lcd.print(tmp);
    }
  }
}
