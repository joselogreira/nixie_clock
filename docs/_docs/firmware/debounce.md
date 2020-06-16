---
title: Debounce routine
permalink: /docs/debounce/
---


Debouncing has the objective of filtering the buttons' input signal to detect the relevant button actions and discard all unwanted bouncing to prevent undesired behavior.

Bouncing button signals can vary from a couple milliseconds (a good quality button) to a couple tens of milliseconds and beyond (bad quality buttons). Bouncing time is also affected by the pressure exerted on the button and button aging. Thus, a method based on polling button signals for fixed amounts of time is not reliable enough.

Hardware design techniques can be implemented to alleviate this problem such as the use of [RC filters][rc] or even the addition of [Schmitt triggers][st]. None of these techniques have been used since the algorithm implemented has proven to be reliable enough, without significant impact on code execution time.

[rc]: https://electronics.stackexchange.com/questions/332074/schmitt-trigger-in-button-debouncer
[st]: http://hades.mech.northwestern.edu/index.php/Switch_Debouncing

### Debounce routine is NON-blocking

The `buttons_check()` debounce routine is defined inside [`util.c`](https://github.com/joselogreira/nixie_clock/blob/master/src/util.c).

It relies on the fact that the loop period is __1ms__. Once the `btnXYZ.query` flag is set by the ISR, this routine is executed once every millisecond as part of the loop tasks. It does poll the button state (by executing `(btn->check)()`) but never stops to wait for a certain time to ellapse. The buttons data structure is shown below (defined in `external_interrupt.h`).

```c
typedef struct {
    volatile uint8_t query; // flag; query button state
    uint8_t action;         // flag; button action activated
    uint8_t lock;           // flag; button locked
    uint8_t state;          // button state: IDLE, PUSHED, RELEASED
    uint16_t count;         // time counter
    uint8_t delay1;         // flag; delay 1 elapsed
    uint8_t delay2;         // flag; delay 2 elapsed
    uint8_t delay3;         // flag; delay 3 elapsed
    uint8_t (*check)();     // function pointer to a state check handler
} btn_s;
```

There're 3 button structures: one for every button. Since they are global, they preserve the current button state at each `buttons_check()` execution. 

There're three button states described as follows:

### `BTN_IDLE`:

This is the initial state. Once the button is pressed, the way to confirm that is trully pressed is by polling the pin state once every `buttons_check()` execution. If the button is effectively pressed, increase a counter. If not, decrease it.

```c
if(!(btn->check)()) btn->count++;
else if(btn->count > 0) btn->count--;
```

If the counter reaches the amount determined by the `BTN_DTCT_TIME` macro (7 times is OK), the button is assumed to be effectively pressed, the buzzer is enabled and it advances to the next state. If not, then it is kept in idle state.

```c
if(btn->count == BTN_DTCT_TIME){
    btn->action = TRUE;
    btn->lock = TRUE;
    btn->state = BTN_PUSHED;
    btn->count = 0;
    buzzer_set(ENABLE);
} else if(btn->count == 0){
    btn->action = FALSE;
    btn->lock = FALSE;
    btn->query = FALSE;
}
```

### `BTN_PUSHED`:

The routine keeps polling the button state and increasing a counter. Depending on the value of the counter, some flags are set. If the button is detected to be released, the state immediately chages to `BTN_RELEASED`. 

```c
if(!(btn->check)()){
    btn->count++;   
} else {
    btn->count = 0;
    btn->state = BTN_RELEASED;
}
if(btn->count == BTN_BEEP_TIME) buzzer_set(DISABLE);
if(btn->count == BTN_DLY1_TIME) btn->delay1 = TRUE;
if((btn->delay1) && (!(btn->count % BTN_DLY2_TIME))) btn->delay2 = TRUE;
if(btn->count >= BTN_DLY3_TIME) btn->delay3 = TRUE;
```

The buzzer beep was enabled when the button was detected to be pressed, and is disabled once `BTN_BEEP_TIME` milliseconds have elapsed. This accounts for a nice short _beep_ sound when any button is pressed.

There're 3 different delay time macros used for different purposes:

```c
#define BTN_DLY1_TIME   300     // time for delay 1
#define BTN_DLY2_TIME   65      // time for delay 2
#define BTN_DLY3_TIME   2000    // time for delay 3
```

* ___Delay 1___ and ___Delay 2___ resemble the times implemented in any key of a keyboard: once a key is pressed, the character is printed on the screen immediately. If the key remains pressed, nothing happens for about 500ms after which the key gets repeatedly printed on the screen, not with a period of 500ms but with a period much shorter (around 50ms). This is used on the clock for fast increment of digits (hours or minutes) when it's being configured.
* ___Delay 3___ is a _"long press"_ flag, which is set when the button press lasts for about 2000ms. Used in the clock to represent a ___"return"___ action when navigating through the clock menus.

### `BTN_RELEASED`:

The routine keeps polling the button state, this time looking for the button to be released (input HIGH) and increases a counter. The button must remain in active HIGH state (not pressed) for a time determined by the `BTN_LOCK_TIME` macro. Any further button detection or action is blocked unless the button remains released for that period of time, after which the button goes back to idle state.

```c
if((btn->check)())
    btn->count++;
if(btn->count == BTN_LOCK_TIME){
    btn->query = FALSE;
    btn->action = FALSE;
    btn->lock = FALSE;
    btn->state = BTN_IDLE;
    btn->count = 0;
    btn->delay1 = FALSE;
    btn->delay2 = FALSE;
    btn->delay3 = FALSE;
    buzzer_set(DISABLE);
}
```