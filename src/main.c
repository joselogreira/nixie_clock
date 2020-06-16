/**
 * @file main.c
 * @brief MAIN program resides here
 *
 * Main program structure:
 * - Basically, there's one infinite loop: the main application loop
 * - Usr/operator decides which one to enter at boot time, either using 
 *   buttons or by other mechanism
 * - Application loop: implemented as a big switch statement, similar to a
 *   Finite State Machine. States are linked to the information displayed and 
 *   clock actions
 * - Interrupts are only allowed in one case:
 *      - Wihin each system state function, at the end of every loop execution.
 *
 * @author Jose Logreira
 * @date 24.04.2018
 */

/******************************************************************************
******************* I N C L U D E   D E P E N D E N C I E S *******************
******************************************************************************/

#include "adc.h"
#include "config.h"
#include "debug.h"
#include "init.h"
#include "menu_alarm.h"
#include "menu_time.h"
#include "menu_user.h"
#include "sleep.h"
#include "timers.h"
#include "uart.h"
#include "util.h"

#include <stdint.h>         /* Standard variable types */
#include <avr/io.h>         /* Device specific ports/peripherals */ 
#include <avr/interrupt.h>  /* Global interrupts */
#include <util/delay.h>     /* Delay utility */
#include <avr/pgmspace.h>   /* Program Memory reading */

/******************************************************************************
********************* F U S E S   D E F I N I T I O N S ***********************
******************************************************************************/
/*
* These fuse settings will be placed in a special section in the ELF output 
* file, after linking. Programming tools can take advantage of the fuse info
* embedded in the ELF file, by extracting this information and determining if 
* the fuses need to be programmed before programming the Flash and EEPROM 
* memories. This also allows a single ELF file to contain all the information 
* needed to program an AVR.
*/

#define FUSE_BITS_LOW       (FUSE_SUT_CKSEL0 & FUSE_CKDIV8)                             // 0x7E
#define FUSE_BITS_HIGH      (FUSE_SPIEN & FUSE_EESAVE & FUSE_BOOTSZ1 & FUSE_BOOTSZ0)    // 0xD1
#define FUSE_BITS_EXTENDED  (FUSE_CFD & FUSE_BODLEVEL0)                                 // 0xF6
#define LOCK_BITS           0xFF                                                        // No Locks

// Place fuses in a special section (.fuse) in the .ELF output file
FUSES = {
    FUSE_BITS_LOW,              // .low
    FUSE_BITS_HIGH,             // .high
    FUSE_BITS_EXTENDED,         // .extended
};
//Place lockbits in a special section (.lock) in the .ELF output file
LOCKBITS = LOCK_BITS;


/******************************************************************************
*************** G L O B A L   V A R S   D E F I N I T I O N S *****************
******************************************************************************/

// Stores the current SYSTEM STATE. It's passed by reference to functions.
// Modified also in ISR.
volatile state_t system_state = SYSTEM_INTRO;

// Main loop execute flag: set every 1ms by an ISR. Used to synchronize the
// system states' loop execution every 1ms.
volatile uint8_t loop = FALSE;

// sleep mode (RTC state when in sleep modes)
// - RTC disabled: means oscillator stopped (POWER DOWN mode): This is only 
//      used when battery is connected first: RTC is not running
// - RTC enabled: means oscillator running, time is counting and ISRs are 
//      generated when in POWER SAVE mode.
volatile uint8_t sleep_mode = RTC_DISABLE;

// System reset:
uint8_t system_reset = FALSE;

/******************************************************************************
*************************** M A I N   P R O G R A M ***************************
******************************************************************************/

int main(void)
{
    /*
    * During normal execution, inside DISPLAY_TIME, if all three buttons are
    * pushed for DELAY3 milliseconds, execution jumps to RESET label. The
    * purpose is to restore the system to an initial default state, even
    * when the coin cell battery has been inserted. It forces the system to
    * go to sleep in PWR_DOWN mode, so that no RTC is enabled, thus keeping
    * the system halted until the next reconnection of the power adapter
    */
RESET:
    if(system_reset){
        sleep_mode = RTC_DISABLE;
        peripherals_disable(sleep_mode);
        BOOST_SET(DISABLE);
        while(EXT_PWR){
            // Toggle fast the onboard LED to indicate a reset condition
            led_blink(1,30);
        }
        system_reset = FALSE;
    }

    /* 
    * INITIALIZATION ----------------------------------------------------------
    * Power sources:
    *   - coin cell battery
    *   - 12V adapter
    * Once power is applied, system boots with default values. At this point, 
    * peripherals are initialized but not enabled (no clock applied to them)
    */
    boot();

    /*
    * Power source check:
    * Look for EXT_PWR pin. If there's an external power adapter connected,
    * Initialize RTC, enable Boost and all other peripherals. Run system.
    * If no voltage in EXT_PWR, it means that battery has been connected first.
    * In that case, do nothing and go to sleep.
    */
    if(EXT_PWR) {
        sleep_mode = RTC_ENABLE;
        system_state = SYSTEM_INTRO;
        peripherals_enable();
        led_blink(3, 40);
        uart_send_string_p(PSTR("\n\rFirmware Version: "));
        uart_send_string_p(PSTR(FIRMWARE_DATE));
        uart_send_string_p(PSTR("\n\r"));
    } else {
        sleep_mode = RTC_DISABLE;
        system_state = SYSTEM_SLEEP;
    }    

    /*
    * Production Test Enabled?
    * Use the ADC to sense the V_TST pin. If it is short-circuited, it means
    * that the production test must be launched, and a serial communication
    * link will be estabished with a PC using the UART.
    */
    if(system_state != SYSTEM_SLEEP){
        if(adc_factory_test_check())
            system_state = PRODUCTION_TEST;
    }

    /*
    * Voltages check:
    * The following voltages are checked to see wether the voltage ranges are 
    * properly met: V_HV, V_CTL_REG, V_IN
    * If any of these is out of range, inform the situation and jump to RESET.
    * Delay is necessary mainly to wait for boost output stabilization
    */
    if((system_state != SYSTEM_SLEEP) && (system_state != PRODUCTION_TEST)){
        //_delay_ms(1500);
        _delay_ms(2000);
        if(!adc_voltages_test()){
            uart_send_string_p(PSTR("\n\r*** System going down. Please disconnect ***"));  
            system_reset = TRUE;
            goto RESET;
        }
    }

    /*-------------------------------------------------------------------------
    ---------------- M A I N   I N F I N I T E   L O O P ----------------------
    -------------------------------------------------------------------------*/

    /*
    * Almost every case within the switch block includes an infinite loop
    * inside, which means that they handle all the system events inside of
    * every function. Whenever there's an event that requires a change in 
    * system state, the internal loop breaks and program execution returns to
    * this main loop, to then jump into the appropiate function.
    *
    * The two exceptions to this rule are
    * - SYSTEM_SLEEP: if the execution enters SYSTEM_SLEEP, the MCU will go
    * to sleep and execution will be stopped within that funciton
    * - SYSTEM_RESET: execution jumps to the beginning of main() to reset
    * all system, and goes to sleep later
    *
    * ISRs are serviced inside every system state function, at the end of the
    * loop execution, where no additional tasks are being executed. That way,
    * ISR execution won't overlap nor cause data corruption with other tasks.
    */
    while (TRUE)
    {
     	switch(system_state){

            // System Goes to Sleep and CPU is halted, except for the 
            // asynchronous timer running the RTC
            case SYSTEM_SLEEP:
                go_to_sleep(&system_state, sleep_mode); break;
            
            // First introductory animation
            case SYSTEM_INTRO:
                intro(&system_state); break;
            // Normal operation: tells the time, including some animations
            case DISPLAY_TIME:
                display_time(&system_state); break;
            // Configuration menu: navigates through all menu options
            case DISPLAY_MENU:
                display_menu(&system_state); break;
            // Config Option 1:
            case SET_TIME:
                set_time(&system_state); break;
            // Config Option 2:
            case SET_ALARM:
                set_alarm(&system_state); break;
            // Config Option 3:
            case SET_ALARM_ACTIVE:
                set_alarm_active(&system_state); break;
            // Config Option 4:
            case SET_HOUR_MODE:
                set_hour_mode(&system_state); break;
            // Config Option 5:
            case SET_TRANSITIONS:
                set_transitions(&system_state); break;
            // Config Option 6:
            case SET_ALARM_THEME:
                set_alarm_theme(&system_state); break;
            
            // If alarm is triggered: 
            case ALARM_TRIGGERED:
                alarm_triggered(&system_state); break;
            
            // User-accessible test sequence: It tests all the tubes' digits,
            // the RTC 1Hz sync signal, the individual RGB LEDs and the buzzer.
            case USR_TEST:
                usr_test(&system_state); break;
            case PRODUCTION_TEST:
                production_test(&system_state); break;

            // Jump to a reset state
            case SYSTEM_RESET:
                goto RESET; break;
            default:
                system_state = DISPLAY_TIME; break;
        }
    } /* While Loop */

    return 0;
} /* Main Function */

/******************************************************************************
*******************************************************************************

                    I N T E R R U P T   H A N D L E R S

*******************************************************************************
******************************************************************************/

// ASCII table for numbers
static const char bcd_to_ascii[] PROGMEM = {'0','1','2','3','4','5','6','7','8','9'};

/*-----------------------------------------------------------------------------
                      R E A L   T I M E   C O U N T E R
-----------------------------------------------------------------------------*/
/*
* TIMER 2 interrupts are Asynchronous!, meaning that the peripheral uses the 
* external 32.768KHz watch crystal as clock source. Interrupts are generated 
* once every second. 
* - time structure is updated in every execution
* - alarm is checked to see if it matches
* - hour is reported via uart every second (provided that power adapter is 
*   plugged in)
*/
ISR(TIMER2_OVF_vect){

    time.update = TRUE;

    if(system_state != PRODUCTION_TEST){

        // update time variables
        time.sec++;
        if(time.sec == 60){
            time.sec = 0;
            time.min++;
            if(time.min == 60){
                time.min = 0;
                time.hour++;
                // different increments for 12/24h modes
                if(time.hour_mode == MODE_12H){
                    if(time.hour == 12){
                        // toggle AM/PM
                        if(time.day_period == PERIOD_AM) time.day_period = PERIOD_PM;
                        else if(time.day_period == PERIOD_PM) time.day_period = PERIOD_AM;
                    } else if(time.hour == 13){
                        // reset hours
                        time.hour = 1;
                    }
                } else if(time.hour_mode == MODE_24H){
                    if(time.hour == 12){
                        time.day_period = PERIOD_PM;
                    } else if(time.hour == 24){
                        time.day_period = PERIOD_AM;
                        time.hour = 0;
                    }
                }
            }
        }
        
        // update time BCD variables
        time.s_tens = time.sec / 10;
        time.s_units = time.sec % 10;
        time.m_tens = time.min / 10;
        time.m_units = time.min % 10;
        time.h_tens = time.hour / 10;
        time.h_units = time.hour % 10;

        // Check alarm match. If true, jump directly to the ALARM_TRIGGERED state,
        // no matter what the clock is doing
        if(check_alarm()) system_state = ALARM_TRIGGERED;

        // if power adapter is connected (if not, the MCU is powered be running 
        // with the coin cell battery):
        // - toggle LED
        // - send time with UART
        // if not connected, do not report time nor toggle led.
        if(EXT_PWR) {
            RTC_SIGNAL_TOGGLE();
            char string[9];
            string[0] = (char)pgm_read_byte(&bcd_to_ascii[time.h_tens]);
            string[1] = (char)pgm_read_byte(&bcd_to_ascii[time.h_units]);
            string[2] = ':';
            string[3] = (char)pgm_read_byte(&bcd_to_ascii[time.m_tens]);
            string[4] = (char)pgm_read_byte(&bcd_to_ascii[time.m_units]);
            string[5] = ':';
            string[6] = (char)pgm_read_byte(&bcd_to_ascii[time.s_tens]);
            string[7] = (char)pgm_read_byte(&bcd_to_ascii[time.s_units]);
            string[8] = '\0';
            uart_send_string(string);
            uart_send_string("\n\r");
        } else {
            RTC_SIGNAL_SET(LOW);
        }   
    }
}

/*-----------------------------------------------------------------------------
                    G E N E R A L   T I M E R   C O U N T E R
-----------------------------------------------------------------------------*/
/*
* TIMER 3 is used as a general purpose counter. Interrupts are generated every
* 1ms and this time base is used for multiple purposes:
* - loop flag is set in every execution
* - Nixie tubes multiplexing routine is handled based on an internal counter
* - Nixie tubes fading routine is handled based on an internal counter
*/
ISR(TIMER3_COMPA_vect){

    static uint8_t n_tube = 1;   // determines which tube to light up (1, 2, 3 or 4)
    static uint8_t n_fade = 5;      // 4 Fading levels (1 to 4)
                                    // 0: fully off; 5: fully on
    static uint16_t cnt = 0;        // general purpose counter

    // execute main loop every 1ms.
    loop = TRUE;

    if(system_state != PRODUCTION_TEST){
        // multiplex tubes' anode every 5ms. Independent of fading level
        if(!(cnt % 5)){
            // change tube selection
            n_tube++;
            if(n_tube >= 4) n_tube = 0;
            // enable tube anode
            set_tube(n_tube);
        }
        
        // to better perform digits' fading, the digits (cathodes) are updated 
        // every 1ms:
        if(display.set){
            if(n_fade <= display.fade_level[n_tube]){
                if(n_tube == TUBE_A) set_digit(display.d1);
                else if(n_tube == TUBE_B) set_digit(display.d2);
                else if(n_tube == TUBE_C) set_digit(display.d3);
                else if(n_tube == TUBE_D) set_digit(display.d4);
            } else {
                set_digit(BLANK);
            }
        } else {
            set_digit(BLANK);
        }

        // general counter reset
        cnt++;
        if(cnt == 1000) cnt = 0;
        // fade level counter
        n_fade++;
        if(n_fade > 5) n_fade = 1;
    }
}

/*-----------------------------------------------------------------------------
                    E X T E R N A L   I N T E R R U P T S
-----------------------------------------------------------------------------*/
/*
* EXTERNAL INTERRUPTS are active for two sources: buttons and external power
* adapter connection.
* - When buttons are pressed, btnXYZ.query flag is set and the buttons are
*   polled to determine its state.
* - When EXT_PWR pin triggers the ISR, the power adapter was either removed
*   or plugged. Thus, check EXT_PWR level and decide whether going to sleep
*   or keep normal execution.
*/

/*===========================================================================*/
/* 
* buttons     
* BTN_X       - PB5 - PCINT13 | -> PCI1
* BTN_Y       - PB6 - PCINT14 | -> PCI1
* BTN_Z       - PB7 - PCINT15 | -> PCI1 
*/
ISR(PCINT1_vect)
{
    if((!(btnX.check)()) && (!(btnX.lock))){
        btnX.query = TRUE;
    } else if((!(btnY.check)()) && (!(btnY.lock))){
        btnY.query = TRUE;
    } else if((!(btnZ.check)()) && (!(btnZ.lock))){
        btnZ.query = TRUE;
    }
}

/*===========================================================================*/
/* 
* External power
* if external power is removed, enter in power save mode 
* EXT_PWR     - PC2 - PCINT18  | -> PCI2
*/
ISR(PCINT2_vect)
{
    if(!EXT_PWR){
        // Boost is automatically powered off (12V removed)
        RTC_SIGNAL_SET(LOW);          // Dont use RTC LED
        system_state = SYSTEM_SLEEP;  // GO TO SLEEP !!!
    }
}