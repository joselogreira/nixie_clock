---
title: Overall Structure
permalink: /docs/structure/
---


I'll start describing the core functionality and then describing some of the details. Let's see the `main()` function.

### Main loop

It's best to first describe the system from an abstract perspective, instead of start describing the codebase line by line.

<iframe width="560" height="315" src="https://www.youtube.com/embed/HfNS7TEvCeo" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

The system functions are based on the display state, that is, on what's actually being displayed. Of course, each tube has limited amount of characters (namely, 0 to 9), so in order to display different information, the RGB LEDs come in handy to help understan what's actually being displayed. According to the state, the buttons have different behavior. These system states are:

* _Display time:_ Shows the actual time of the day. This is the default system state
* _Display menu:_ Shows the configuration menu for all options of the clock
* _Set time:_ Changes the actual minutes and hours
* _Set hour mode:_ Toggles between 12h and 24h
* _Set alarm:_ Changes the alarm time
* _Set alarm active:_ Enables/Disables the alarm
* _Set alarm theme:_ Changes the alarm song theme
* _Alarm triggered:_ The alarm event is triggered. Alarm song is playing.
* _Set transitions:_ Clock includes animations. These are configures in this state.

Some of these states include sub-states to implement some specific behavior or animation, or even to perform a visual test on all the tubes. They will be covered in their respective section.

The `main()` function includes an infinite loop in the form of a `switch() case:` structure. Each display state lies within one of the `case` statements in the form of a function call. When the _Display time_ state (just as example) is to be displayed (that is, active), the program execution jumps to the _Display time_ function, and remains inside the function for as long as the clock displays the time. Once the user switches to some other state, the function call returns and changes to the appropiate new function call. This is shown in the image below.

![Program Flow](/nixie_clock/assets/img/program_flow.png "Typical program flow")

This systems is event-based, meaning that program execution is determined by one of two things:
* _User input_ in the form of buttons pushed.
* _Software events_ in the form of timers or counters that determine a certain action or behavior.
This model implies a few important things:
* Whatever the current system state is, the MCU must constantly check if events have happened, and be flexible enough to break the current program flow and _"switch context"_ to a different system state. __NOTE:__ This situation demands some code repetition between all system states to handle events. Although it is possible to avoid code repetition, the resulting code is harder to understand and maintain. Further details will be given below.

* Many of the events require a time base, thus, a __1ms__ period timebase is chosen, and configured in one of the ISRs.

### System states' timed loop

Inside of every system state function, there's an infinite loop inside of which the actions to be performed are synchronized with timers, external events or internal counters. Almost all system states have the same structure, with the exception being `SYSTEM_SLEEP`, `SYSTEM_RESET` and `PRODUCTION_TEST`.

The general structure follows the pattern below:

```c
void system_state_handler(state_t *state)
{
	// initialize variables

	/* INFINITE LOOP */
	while(TRUE){

		/*
		*	DISPLAY TRANSITIOS
		*	Animations and transitions depend on the systeme state
		*/
		
		/* 
		* BUTTONS check: Buttons are detected using an ISR which sets btnXYZ
	    * structure flags. Once set, the rest of the detection and debounce routine is
	    * handled within buttons_check(), based on the 1ms execution period of
	    * the main infinite loop
		*/
		if(btnX.query) buttons_check(&btnX);
	    if(btnY.query) buttons_check(&btnY);
	    if(btnZ.query) buttons_check(&btnZ);

	    /*
	    * BUTTONS actions:
		* - executed according to the buttons state flags
		*
		* If X pressed, then ...
		* If Y pressed, then ...
		* If Z pressed, then ...
		*/

		/*
		* 	GENERAL FUNCTION COUNTER
		*/
		count++;
		// some code to handle counter

		/* 
		* LOOP DELAY AND INTERRUPT ENABLE TIME --------------------------------
		* All interrupts are served within the sei()-cli() block. This is to 
		* avoid the extra care required for arbitrarily triggered ISRs and the 
		* use of atomic operations. "loop" flag is set every 1ms by a timer
		* whose ISR is enabled to produce interrupts every 1ms
		*/
		sei();
		// Wait for the next ms.
		while(!loop);
		loop = FALSE;
		cli();
		// If system state changed, exit fuction.
		if(*state != SET_ALARM_ACTIVE)
			break;

	}	/* END INFINITE LOOP */
}

```

Key points:

* __ISRs are NOT enabled all the time__. They're only enabled once the whole loop tasks get executed. Loop period is 1ms, and all the loop tasks are executed in much less than 1ms. Thus, ISRs are enabled for all the remaining time.
* The External Interrupt for the buttons sets the `btnXYZ.query` flag. Once set, it means that the `buttons_check()` routine gets executed. But this is NOT a blocking routine, instead, buttons' states are evaluated after multiple runs of the same routine over the same button. More explanations [here](/nixie_clock/docs/debounce/index.html).
* Most system states require some sort of internal counter to synchronize actions. Limit counter values are handled individually in every system state.

### Global structures

To better organize data across all modules, a few global data structures are created related to: time data, alarm data, display data and buttons data. Data structures are first prototyped using `typedef` and exposed globally using `extern` in the respective `.h` header file, and defined in the respective `.c` file.

> __NOTE:__ no bit fields are used.

* ___Time data___: Includes data related to current time, time format, and an important flag that signals whether current time has been updated or not (updates happen every second). Files involved: `menu_time.c` and `menu_time.h`:

```c
/* menu_time.h */
typedef volatile struct {
	uint8_t	sec;			// seconds
	uint8_t min;			// minutes
	uint8_t hour;			// hours
	uint8_t s_units;		// BCD seconds' units
	uint8_t s_tens;			// BCD seconds' tens
	uint8_t m_units;		// BCD minutes' units
	uint8_t m_tens;			// BCD minutes' tens
	uint8_t h_units;		// BCD hours' units
	uint8_t h_tens;			// BCD hours' tens
	uint8_t update;			// flag. 1Hz update?
	uint8_t hour_mode;		// 12/24h 
	uint8_t day_period;		// AM/PM
} time_s;

extern time_s time;

/* menu_time.c */
time_s time;
```

* ___Alarm data___: Includes data related to alarm configuration, and an important flag that signals whether alarm is enabled or not. Files involved: `menu_alarm.c` and `menu_alarm.h`:

```c
/* menu_alarm.h */
typedef struct {
	uint8_t	sec;			// seconds
	uint8_t min;			// minutes
	uint8_t hour;			// hours
	uint8_t s_units;		// BCD seconds' units
	uint8_t s_tens;			// BCD seconds' tens
	uint8_t m_units;		// BCD minutes' units
	uint8_t m_tens;			// BCD minutes' tens
	uint8_t h_units;		// BCD hours' units
	uint8_t h_tens;			// BCD hours' tens
	uint8_t hour_mode;		// 12/24h 
	uint8_t day_period;		// AM/PM
	uint8_t active;			// flag. Alarm ON?
	volatile uint8_t triggered;	// flag. Alarm MATCH?
	uint8_t theme;			// Alarm melody
} alarm_s;

extern alarm_s alarm;

/* menu_alarm.c */
alarm_s alarm;
```

* ___Display data___: Includes data related to current display contents, fading level and display mode (used to differentiate transitions or animations). Files involved: `timers.c` and `timers.h`:

```c
/* timers.h */
typedef volatile struct {
    uint8_t mode;
    uint8_t d1;
    uint8_t d2;
    uint8_t d3;
    uint8_t d4;
    uint8_t set;
    uint8_t fade_level[4];
} display_s;

extern display_s display;

/* timers.c */
display_s display;
```

* ___Buttons data___: Includes data related to current buttons state. Operations upon this structure occur only during the buttons debounce routine, and its state gets polled to check for buttons activity. Further explanation on debounce routine can be found [here](/nixie_clock/docs/debounce/index.html). Files involved: `external_interrupt.c` and `external_interrupt.h`:

```c
/* external_interrupt.h */
typedef struct {
	volatile uint8_t query;	// flag; query button state
	uint8_t action;			// flag; button action activated
	uint8_t lock;			// flag; button locked
	uint8_t state;			// button state: IDLE, PUSHED, RELEASED
	uint16_t count;			// time counter
	uint8_t delay1;			// flag; delay 1 elapsed
	uint8_t delay2;			// flag; delay 2 elapsed
	uint8_t delay3;			// flag; delay 3 elapsed
	uint8_t (*check)();		// function pointer to a state check handler
} btn_s;

extern btn_s btnX, btnY, btnZ;

/* external_interrupt.c */
btn_s btnX, btnY, btnZ;;
```

### Macros and constants

Many modules require their own set of private macros (either for configuration or for constants definition). Some others require to expose their macros to other modules. The private macros are defined in the respective modules' `.c` file, and those shared with other modues are declared in the `.h` file. Appart from those, there're some "generic" macros that are declared in the `config.h` file, which is included in most of the modules. 

This file also includes a couple of `extern` variables declared in `main.c` required to be accessible by other modules. 

### Interrupt Service Routines

All the ISR handler functions are defined at the end of `main.c`. These are:

* `ISR(TIMER2_OVF_vect){}`: Used as Real Time Counter. It's an asynchronous peripheral that has its own clock source from the 32,768KHz external crystal.
* `ISR(TIMER3_COMPA_vect){}`: Used as general purpose timer. It has a 1ms period and is responsible for:
	* Nixie tubes [multiplexing](/nixie_clock/docs/multiplexing/index.html)
	* System states' loop timed execution.
* `ISR(PCINT1_vect){}`: Pin change interrupt. Used to detect the buttons press.
* `ISR(PCINT2_vect){}`: Pin change interrupt. Used to detect the external power removal or connection.

Further information can be found [here](/nixie_clock/docs/isr/index.html).