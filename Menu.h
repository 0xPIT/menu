// ----------------------------------------------------------------------------
// MicroMenu
// Copyright (c) 2014 karl@pitrich.com
// All rights reserved.
// License: MIT
// ----------------------------------------------------------------------------

#ifndef __have_menu_h__
#define __have_menu_h__

#ifdef ARDUINO
# include <Arduino.h>
#else
# include <avr/pgmspace.h>
# include <inttypes.h>
# include <stdlib.h>
#endif

namespace Menu
{
  typedef enum Action_s {
    actionNone    = 0,
    actionLabel   = (1<<0), // render label when user scrolls through menu items
    actionDisplay = (1<<1), // display menu, after user selected a menu item
    actionTrigger = (1<<2), // trigger was pressed while menue was already active
    actionParent  = (1<<3), // before moving to parent, useful for e.g. "save y/n?" or autosave
    actionCustom  = (1<<7)  
  } Action_t;

  typedef bool (*Callback_t)(Action_t);

  typedef struct Info_s {
    uint8_t siblings;
    uint8_t position;
  } Info_t;

  typedef struct Item_s {
    struct Item_s const * Next;
    struct Item_s const * Previous;
    struct Item_s const * Parent;
    struct Item_s const * Child;
    const Callback_t Callback;
    const char * Label;  
  } Item_t;

  typedef void (*RenderCallback_t)(const Item_t *, uint8_t);

  // a typesafe null item
  extern const Item_t NullItem;

  class Engine {
  public:
    const Item_t * currentItem;
    const Item_t * previousItem;
    const Item_t * lastInvokedItem; 

  public:
    Engine();
    Engine(const Item_t * initialItem);

  public:
    void navigate(const Item_t * targetItem);
    void invoke(void);
    bool executeCallbackAction(const Action_t action) const;
    void render(const RenderCallback_t render, uint8_t maxDisplayedMenuItems) const;

  public:
    Info_t getItemInfo(const Item_t * item) const;
    const   char * getLabel (const Item_t * item = NULL) const;
    const Item_t * getPrev  (const Item_t * item = NULL) const;
    const Item_t * getNext  (const Item_t * item = NULL) const;
    const Item_t * getParent(const Item_t * item = NULL) const;
    const Item_t * getChild (const Item_t * item = NULL) const;
  };
}; // end namespace Menu

// ----------------------------------------------------------------------------

#define MenuItem(Name, Label, Next, Previous, Parent, Child, Callback) \
  extern const Menu::Item_t Next, Previous, Parent, Child; \
  const Menu::Item_t PROGMEM Name = { \
    &Next, &Previous, &Parent, &Child, \
    &Callback, \
    Label \
  }

// ----------------------------------------------------------------------------

#endif // __have_menu_h__

// ----------------------------------------------------------------------------

/*!
  template for callback handler

void menuCallback(menuAction_t action) {  
  if (action == Menu::actionDisplay) {
    // initialy entering this menu item
  }

  if (action == Menu::actionTrigger) {
    // click on already active item
  }

  if (action == Menu::actionLabel) {
    // show thy label but don't do anything yet
  }

  if (action == Menu::actionParent) { 
    // navigating to self->parent
  }
}
*/
