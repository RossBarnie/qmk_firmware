#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "split_util.h"
#include "matrix.h"
#include "keyboard.h"
#include "wait.h"

#ifdef USE_MATRIX_I2C
#  include "i2c.h"
#else
#  include "split_scomm.h"
#endif

#ifdef EE_HANDS
#    include "eeconfig.h"
#endif

#ifndef SPLIT_USB_TIMEOUT
#    define SPLIT_USB_TIMEOUT 2000
#endif

#ifndef SPLIT_USB_TIMEOUT_POLL
#    define SPLIT_USB_TIMEOUT_POLL 10
#endif

volatile bool isLeftHand = true;

bool waitForUsb(void) {
    for (uint8_t i = 0; i < (SPLIT_USB_TIMEOUT / SPLIT_USB_TIMEOUT_POLL); i++) {
        // This will return true if a USB connection has been established
        if (UDADDR & _BV(ADDEN)) {
            return true;
        }
        wait_ms(SPLIT_USB_TIMEOUT_POLL);
    }

    // Avoid NO_USB_STARTUP_CHECK - Disable USB as the previous checks seem to enable it somehow
    (USBCON &= ~(_BV(USBE) | _BV(OTGPADE)));

    return false;
}


bool is_keyboard_left(void) {
#if defined(SPLIT_HAND_PIN)
    // Test pin SPLIT_HAND_PIN for High/Low, if low it's right hand
    setPinInput(SPLIT_HAND_PIN);
    return readPin(SPLIT_HAND_PIN);
#elif defined(EE_HANDS)
    return eeconfig_read_handedness();
#elif defined(MASTER_RIGHT)
    return !is_helix_master();
#endif

    return is_helix_master();
}

bool is_helix_master(void) {
    static enum { UNKNOWN, MASTER, follower } usbstate = UNKNOWN;

    // only check once, as this is called often
    if (usbstate == UNKNOWN) {
#if defined(SPLIT_USB_DETECT)
        usbstate = waitForUsb() ? MASTER : follower;
#elif defined(__AVR__)
        USBCON |= (1 << OTGPADE);  // enables VBUS pad
        wait_us(5);

        usbstate = (USBSTA & (1 << VBUS)) ? MASTER : follower;  // checks state of VBUS
#else
        usbstate = MASTER;
#endif
    }

    return (usbstate == MASTER);
}

static void keyboard_master_setup(void) {

#ifdef USE_MATRIX_I2C
    i2c_master_init();
#else
    serial_master_init();
#endif
}

static void keyboard_follower_setup(void) {

#ifdef USE_MATRIX_I2C
    i2c_follower_init(follower_I2C_ADDRESS);
#else
    serial_follower_init();
#endif
}

void split_keyboard_setup(void) {
   isLeftHand = is_keyboard_left();

   if (is_helix_master()) {
      keyboard_master_setup();
   } else {
      keyboard_follower_setup();
   }
   sei();
}
