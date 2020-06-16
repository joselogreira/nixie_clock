---
title: ISR and multiplexing
permalink: /docs/isr/
---


There're 4 Interrupt Service Routines in the systems. 2 related to timers and 2 related to digital pins state change. Each of them is configured and enabled during their respective peripheral initialization routine.


## Real Time Clock

A distinctive feature of this project is the implementation of the RTC using one of the MCU timer peripherals (__TIMER 2__). This way, code complexity is reduced since no communication with an external RTC is required, and the ability to keep track of time when there's no external power is preserved, since the backup coin cell battery powers the MCU just to allow the internal RTC peripheral to work. Further explanation about it can be found [here](/nixie_clock/docs/sleep/).

Timer 2 works asynchronously: it has its own 32,768KHz low power watch crystal oscillator. Interrupts are generated once every second. 

The handler updates the `time` structure with new values, taking into account the period of day (AM/PM), and time format (12h/24h):

```c
time.sec++;
if(time.sec == 60){
    time.sec = 0;
    time.min++;
    if(time.min == 60){
        time.min = 0;
        time.hour++;

        // more code
    }
}
```

Also updates the BCD values of the digits to be displayed by the tubes:

```c
time.s_tens = time.sec / 10;
time.s_units = time.sec % 10;
time.m_tens = time.min / 10;
time.m_units = time.min % 10;
time.h_tens = time.hour / 10;
time.h_units = time.hour % 10;
```

If the alarm is `ENABLE`, it checks for a match:

```c
if(alarm.active){
    if(alarm.hour == time.hour){
        if(alarm.min == time.min){
            if(alarm.sec == time.sec){
                if(alarm.day_period == time.day_period){
                    match = TRUE;
                    alarm.triggered = TRUE;
                }
            }
        }
    }
}
```

And finally, if the clock is running with external power applied (12V adapter), it toggles an onboard LED and sends via the serial port (UART peripheral), a string with the current time in format `hh:mm:ss`:

```c
if(EXT_PWR) {
    RTC_SIGNAL_TOGGLE();
    char string[9];

    /* string construction */

    uart_send_string(string);
    uart_send_string("\n\r");
} else {
    RTC_SIGNAL_SET(LOW);
}   
```

When no external power is applied, the RTC is the only peripheral that remains awake keeping track of time. Every second the ISR awakens the MCU to process the time update and goes back to sleep again.

## General timer counter

Uses __TIMER 3__. This timer generates interrupts every __1ms__. It has two main functions:

* This is the one responsible for the system states' timed loop execution by setting the `loop = TRUE;` flag.

* It controls the Nixies' multiplexing and fading scheme. Further explanation found [here](/nixie_clock/docs/multiplexing/). Tubes' anodes are updated once every 5ms (thus, overall refresh rate is 50Hz for all four tubes), and tubes' cathodes are updated once every 1ms (same as the timer period), the latter being the one responsible for the fading effect used in some animations. 

Using internal counters, once 5ms have elapsed, switch to the next tube:

```c
// multiplex tubes' anode every 5ms. Independent of fading level
if(!(cnt % 5)){
    // change tube selection
    n_tube++;
    if(n_tube >= 4) n_tube = 0;
    // enable tube anode
    set_tube(n_tube);
}
```

`n_fade` is also a counter (from 1 to 5) synced with the tube that's currently being displayed. Since every tube's anode is active for a period of 5ms, fading is achieved by varying the cathode's active time: 0ms is fully OFF, 5ms if fully ON. Values in between give intermediate brightness levels. Since each tube may have different brightness (or fade) levels, individual values for each tube are stored within the `display.fade_level[]` structure member.

```c
if(n_fade <= display.fade_level[n_tube]){
    if(n_tube == TUBE_A) set_digit(display.d1);
    else if(n_tube == TUBE_B) set_digit(display.d2);
    else if(n_tube == TUBE_C) set_digit(display.d3);
    else if(n_tube == TUBE_D) set_digit(display.d4);
} else {
    set_digit(BLANK);
}
```

## Buttons interrupts

Buttons' inputs to the MCU are pulled HIGH with internal pull-up resistors. When pressed, the input is pulled low. The interrupt handler is triggered with any edge detected (rising or falling edge). Thus, the filter for _falling edge_ must be filtered inside the handler by checkig whether the actual input value is low. If so, then the `btnXYZ.query` flag gets set:

```c
if((!(btnX.check)()) && (!(btnX.lock))){
    btnX.query = TRUE;
} else if((!(btnY.check)()) && (!(btnY.lock))){
    btnY.query = TRUE;
} else if((!(btnZ.check)()) && (!(btnZ.lock))){
    btnZ.query = TRUE;
}
```

The `btnXYZ.lock` flag is used to implement a _"dead time"_ when the button is released, so that program execution is able to ignore button actions for a period of time. This also makes part of the [debounce](/nixie_clock/docs/debounce/) routine implemented.

## External power interrupt {#external}

There's a digital input pin in the MCU that senses the presence of external power (12V adapter). When the external power is removed, the system must immediately go to sleep while allowing the RTC to work. This ISR handles this case:

```c
if(!EXT_PWR){
    // Boost is automatically powered off (12V removed)
    RTC_SIGNAL_SET(LOW);          // Dont use RTC LED
    system_state = SYSTEM_SLEEP;  // GO TO SLEEP !!!
}
```

It disables the onboard debug LED and overrides the `system_state` variable to put it to sleep within the next millisecond.