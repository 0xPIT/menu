// -----------------------------------------------------------------------------
// MicroMenu
// 
// (c) 2011, 2012 karl@pitrich.com
// (c) 2007 dean_camera@fourwalledcubicle.com 
// -----------------------------------------------------------------------------

#ifndef __have_menu_h__
#define __have_menu_h__

#include "Arduino.h"

// ----------------------------------------------------------------------------- 

//#ifdef __cplusplus
//extern "C" {
//#endif
  
// ----------------------------------------------------------------------------- 

#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

// ----------------------------------------------------------------------------- 

#ifndef FALSE
# define FALSE 0
#endif

#ifndef TRUE
# define TRUE !FALSE
#endif

// ----------------------------------------------------------------------------- 

typedef enum menuAction_e {
  menuActionNone    = 0,
  menuActionLabel   = (1<<0),
  menuActionDisplay = (1<<1),
  menuActionTrigger = (1<<2),
  menuActionParent  = (1<<3),
  menuActionCustom  = (1<<7)
} menuAction_t;

// ----------------------------------------------------------------------------- 

/*!
  menu callback
  returning FALSE will stop navigating to parent node
  useful for save-before-exit
*/
typedef uint8_t (*MenuCallback_t)(menuAction_t);

// ----------------------------------------------------------------------------- 

typedef struct menuInfo_s {
  uint8_t siblings;
  uint8_t position;
} menuInfo_t;

// ----------------------------------------------------------------------------- 

typedef struct MenuItem_s {
  struct MenuItem_s const * Next;
  struct MenuItem_s const * Previous;
  struct MenuItem_s const * Parent;
  struct MenuItem_s const * Child;

  const MenuCallback_t Callback;
  const char * Label;  
} MenuItem_t; 

// ----------------------------------------------------------------------------- 

#define menuItem(Name, Label, Next, Previous, Parent, Child, Callback) \
  extern const MenuItem_t Next, Previous, Parent, Child; \
  const MenuItem_t __attribute__((__progmem__)) Name = { \
    &Next, &Previous, &Parent, &Child, \
    &Callback, \
    Label \
  }

#define menuLabel(item) ((const char *)pgm_read_word(&item->Label))
#define menuAccessor(item, type) ((const MenuItem_t *)pgm_read_word(&item->type))

#define menuPrev(item)    menuAccessor(item, Previous)
#define menuCurrentPrev   menuPrev(menuCurrentItem)

#define menuNext(item)    menuAccessor(item, Next)
#define menuCurrentNext   menuNext(menuCurrentItem)

#define menuParent(item)  menuAccessor(item, Parent)
#define menuCurrentParent menuParent(menuCurrentItem)

#define menuChild(item)   menuAccessor(item, Child)
#define menuCurrentChild  menuChild(menuCurrentItem)

// ----------------------------------------------------------------------------- 

extern const MenuItem_t menuNull;           // typesafe null menu item

extern const MenuItem_t * menuCurrentItem;  // ptr to currently selected menu item
extern const MenuItem_t * menuPreviousItem; // ptr to previously selected menu item

/*! 
 navigates to newmenuitem 
 and invokes registered callback with argument 'menuActionLabel'

 update menuCurrentItem and menuPreviousItem

 The argument is either a pointer to the menu item to navigate to,
 or one of the navigation macros: menuPrev, menuNext, MenuParent, menuChild
*/
extern void menuNavigate(const MenuItem_t * newmenuitem);

/*! 
  invokes currently selected menuitem and fires the registered callback 
  with correct menuAction_t argument, being one of

  menuActionDisplay   - menu item is displayed initially
  menuActionTrigger   - menu item opened, visible on screen and click occured
*/
extern void menuInvoke(void);

/*!
  exposed, so that actions can be invoked manually
*/
extern uint8_t menuExecuteCallbackAction(const menuAction_t action);

/*!
  reads total sibling count and position of menu item
*/
extern menuInfo_t menuInfo(const MenuItem_t * mi);

/*!
  renders whole menu using callback
    callback is passed the item to render and the prospect positon on screen
    (n-th item with regard to maxDisplayedMenuItems)

  render() shall calculate position of item to be rendered and
  decide using menuCurrentItem, menuLastItem and position passed to callback,
  weather rendering is neccessary
*/
typedef void (*MenuRenderCallback_t)(const MenuItem_t *, uint8_t);

extern void menuRender(const MenuRenderCallback_t render, uint8_t maxDisplayedMenuItems);


// ----------------------------------------------------------------------------- 

//#ifdef __cplusplus
//} // extern "C"
//#endif

// ----------------------------------------------------------------------------- 

#endif // __have_menu_h__

// ----------------------------------------------------------------------------- 

/*!
  template for callback handler

void menuActionCallback(menuAction_t action) {  
  if (action == menuActionDisplay) {
    // initialy entering this menu item
  }

  if (action == menuActionTrigger) {
    // click on already active item
  }

  if (action == menuActionLabel) {
    // show thy label but don't do anything yet
  }

  if (action == menuActionParent) { 
    // navigating to self->parent
  }
}
*/
