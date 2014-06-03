#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include <TimerOne.h>
#include <ClickEncoder.h>
#include <TimerOne.h>
#include <Menu.h>

// ----------------------------------------------------------------------------
// helpers

class ScopedTimer {
public:
  ScopedTimer(const char * Label)
    : label(Label), ts(millis())
  {
  }
  ~ScopedTimer() {
    Serial.print(label); Serial.print(": ");
    Serial.println(millis() - ts);
  }
private:
  const char *label;
  const unsigned long ts;
};

#define clampValue(val, lo, hi) if (val > hi) val = hi; if (val < lo) val = lo;
#define maxValue(a, b) ((a > b) ? a : b)
#define minValue(a, b) ((a < b) ? a : b)

// ----------------------------------------------------------------------------
// display

#define LCD_CS   9
#define LCD_DC   7
#define LCD_RST  8
Adafruit_ST7735 tft = Adafruit_ST7735(LCD_CS, LCD_DC, LCD_RST);

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

uint8_t menuItemsVisible = 5;
uint8_t menuItemHeight = 12;

void renderMenuItem(const Menu::Item_t *mi, uint8_t pos) {
  //ScopedTimer tm("  render menuitem");

  uint8_t y = pos * menuItemHeight + 2;

  tft.setCursor(10, y);

  // a cursor
  tft.drawRect(8, y - 2, 90, menuItemHeight, (engine->currentItem == mi) ? ST7735_RED : ST7735_BLACK);
  tft.print(engine->getLabel(mi));

  // mark items that have children
  if (engine->getChild(mi) != &Menu::NullItem) {
    tft.print(" >   ");
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
    MenuItem(miChBack1,  "Back",      Menu::NullItem, miChScale1,     miChannel1, Menu::NullItem, menuBack);

MenuItem(miTest1, "Test 1 Menu", miTest2,        miSettings, miExit, Menu::NullItem, menuDummy);
MenuItem(miTest2, "Test 2 Menu", miTest3,        miTest1,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest3, "Test 3 Menu", miTest4,        miTest2,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest4, "Test 4 Menu", miTest5,        miTest3,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest5, "Test 5 Menu", miTest6,        miTest4,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest6, "Test 6 Menu", miTest7,        miTest5,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest7, "Test 7 Menu", miTest8,        miTest6,    miExit, Menu::NullItem, menuDummy);
MenuItem(miTest8, "Test 8 Menu", Menu::NullItem, miTest7,    miExit, Menu::NullItem, menuDummy);

// ----------------------------------------------------------------------------

void setup() {
  Serial.begin(57600);
  Serial.print("Starting...");

  tft.initR(INITR_REDTAB);
  tft.setTextWrap(false);
  tft.setTextSize(1);
  tft.setRotation(3);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr); 

  pinMode(MISO, OUTPUT);
  pinMode(MOSI, OUTPUT);
  // enable pull up, otherwise display flickers
  PORTB |= (1 << MOSI) | (1 << MISO); 

  engine = new Menu::Engine(&Menu::NullItem);
  menuExit(Menu::actionDisplay); // reset to initial state
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
          tft.setTextSize(1);
          tft.setCursor(10, 42);
          tft.print("Acceleration: ");
          tft.print((Encoder.getAccelerationEnabled()) ? "on " : "off");
      }
      break;

    case ClickEncoder::Held:
      if (systemState != State::Settings) { // enter settings menu

        // disable acceleration, reset in menuExit()
        lastEncoderAccelerationState = Encoder.getAccelerationEnabled();
        Encoder.setAccelerationEnabled(false);

        tft.fillScreen(ST7735_BLACK);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);

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
      tft.fillRect(8, 1, 120, 100, ST7735_BLACK);
    }

    // simple scrollbar
    Menu::Info_t mi = engine->getItemInfo(engine->currentItem);
    uint8_t sbTop = 0, sbWidth = 4, sbLeft = 100;
    uint8_t sbItems = minValue(menuItemsVisible, mi.siblings);
    uint8_t sbHeight = sbItems * menuItemHeight;
    uint8_t sbMarkHeight = sbHeight * sbItems / mi.siblings;
    uint8_t sbMarkTop = ((sbHeight - sbMarkHeight) / mi.siblings) * (mi.position -1);
    tft.fillRect(sbLeft, sbTop,     sbWidth, sbHeight,     ST7735_WHITE);
    tft.fillRect(sbLeft, sbMarkTop, sbWidth, sbMarkHeight, ST7735_RED);

    // debug scrollbar values
#if 0
    char buf[30];
    sprintf(buf, "itms: %d, h: %d, mh: %d, mt: %d", sbItems, sbHeight, sbMarkHeight, sbMarkTop);
    Serial.println(buf);
#endif

    // render the menu
    {
      //ScopedTimer tm("render menu");
      engine->render(renderMenuItem, menuItemsVisible);
    }

    {
      ScopedTimer tm("helptext");
      tft.setTextSize(1);
      tft.setCursor(10, 110);
      tft.print("Doubleclick to ");
      if (engine->getParent() == &miExit) {
        tft.print("exit. ");
      }
      else {
        tft.print("go up.");
      }
    }
  }

  // dummy "application"
  if (systemState == State::Default) {
    if (systemState != previousSystemState) {
      previousSystemState = systemState;
      encLastAbsolute = -999; // force updateMenu
      tft.fillScreen(ST7735_WHITE);
      tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
      tft.setCursor(10, 10);
      tft.setTextSize(2);
      tft.print("Main Screen");

      tft.setTextSize(1);
      tft.setCursor(10, 110);
      tft.print("Hold button for setup");
    }

    if (encAbsolute != encLastAbsolute) {
      encLastAbsolute = encAbsolute; 
      tft.setCursor(10, 30);
      tft.setTextSize(1);
      tft.print("Position:");
      tft.setCursor(70, 30);
      char tmp[10];
      sprintf(tmp, "%4d", encAbsolute);
      tft.print(tmp);
    }
  }
}
