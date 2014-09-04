// ----------------------------------------------------------------------------
// MicroMenu
// Copyright (c) 2014 karl@pitrich.com
// All rights reserved.
// License: MIT
// ----------------------------------------------------------------------------

#include <Menu.h>

// ----------------------------------------------------------------------------

namespace Menu {

// ----------------------------------------------------------------------------

const Item_t NullItem PROGMEM = { (const Menu::Item_s *)NULL, (const Menu::Item_s *)NULL, (const Menu::Item_s *)NULL, (const Menu::Item_s *)NULL, (const Menu::Callback_t)NULL, (const char *)NULL };

// ----------------------------------------------------------------------------

Engine::Engine() 
  : currentItem(&Menu::NullItem), previousItem(&Menu::NullItem), lastInvokedItem(&Menu::NullItem)
{
}

Engine::Engine(const Item_t *initialItem) 
  : currentItem(initialItem), previousItem(&Menu::NullItem), lastInvokedItem(&Menu::NullItem)
{
}

// ----------------------------------------------------------------------------

const char * Engine::getLabel(const Item_t * item) const {
  return (const char *)pgm_read_word((item == NULL) ? &currentItem->Label : &item->Label);
}

const Item_t * Engine::getPrev(const Item_t * item) const {
  return (const Item_t *)pgm_read_word((item == NULL) ? &currentItem->Previous : &item->Previous);
}

const Item_t * Engine::getNext(const Item_t * item) const {
  return (const Item_t *)pgm_read_word((item == NULL) ? &currentItem->Next : &item->Next);
}

const Item_t * Engine::getParent(const Item_t * item) const {
  return (const Item_t *)pgm_read_word((item == NULL) ? &currentItem->Parent : &item->Parent);
}

const Item_t * Engine::getChild(const Item_t * item) const {
  return (const Item_t *)pgm_read_word((item == NULL) ? &currentItem->Child : &item->Child);
}

// ----------------------------------------------------------------------------

void Engine::navigate(const Item_t * targetItem) {
  uint8_t commit = true;
  if (targetItem && targetItem != &Menu::NullItem) {
    if (targetItem == getParent(currentItem)) { // navigating back to parent
      commit = executeCallbackAction(actionParent); // exit/save callback
      lastInvokedItem = &Menu::NullItem;
    }
    if (commit) {
      previousItem = currentItem;
      currentItem = targetItem;
      executeCallbackAction(actionLabel);
    }
  }
}

// ----------------------------------------------------------------------------

void Engine::invoke(void) {
  bool preventTrigger = false;

  if (lastInvokedItem != currentItem) { // prevent 'invoke' twice in a row
    lastInvokedItem = currentItem;
    preventTrigger = true; // don't invoke 'trigger' at first Display event
    executeCallbackAction(actionDisplay);
  }

  const Item_t *child = getChild();
  if (child && child != &Menu::NullItem) { // navigate to registered submenuitem
    navigate(child);
  }
  else { // call trigger in already selected item that has no child
    if (!preventTrigger) {
      executeCallbackAction(actionTrigger);
    }
  }
}

// ----------------------------------------------------------------------------

bool Engine::executeCallbackAction(const Action_t action) const {
  if (currentItem && currentItem != NULL) {
    Callback_t callback = (Callback_t)pgm_read_word(&currentItem->Callback);

    if (callback != NULL) {
      return (*callback)(action);
    }
  }
  return true;
}

// ----------------------------------------------------------------------------

Info_t Engine::getItemInfo(const Item_t * item) const {
  Info_t result = { 0, 0 };
  const Item_t * i = getChild(getParent());
  for (; i && i != &Menu::NullItem && &i->Next && i->Next != &Menu::NullItem; i = getNext(i)) {
    result.siblings++;
    if (i == item) {
      result.position = result.siblings;
    }
  }

  return result;
}

// ----------------------------------------------------------------------------

void Engine::render(const RenderCallback_t render, uint8_t maxDisplayedMenuItems) const {    
  if (!currentItem || currentItem == &Menu::NullItem) {
    return;
  }

  uint8_t start = 0;
  uint8_t itemCount = 0;
  const uint8_t center = maxDisplayedMenuItems >> 1;
  Info_t mi = getItemInfo(currentItem);
  
  if (mi.position >= (mi.siblings - center)) { // at end
    start = mi.siblings - maxDisplayedMenuItems;
  } 
  else {
    start = mi.position - center;
    if (maxDisplayedMenuItems & 0x01) start--; // center if odd
  }

  if (start & 0x80) start = 0; // prevent overflow

  // first item in current menu level
  const Item_t * i = getChild(getParent());  
  for (; i && i != &Menu::NullItem && &i->Next && i->Next != &Menu::NullItem; i = getNext(i)) {
    if (itemCount - start >= maxDisplayedMenuItems) break;
    if (itemCount >= start) render(i, itemCount - start);
    itemCount++;
  }
}

// ----------------------------------------------------------------------------

}; // end namespace

// ----------------------------------------------------------------------------
