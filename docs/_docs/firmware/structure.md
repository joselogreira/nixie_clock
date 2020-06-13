---
title: Overall Structure
permalink: /docs/structure/
---

## Overall Structure

I'll start describing the core functionality and then describing some of the details. Let's see the `main()` function.

### Main loop

It's best to first describe the system from an abstract perspective, instead of start describing the codebase line by line.

< Video featuring functionality >

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

The `main()` function includes an infinite loop in the form of a `switch() case:` structure. Each display state lies within one of the `case` statements in the form of a function call. Now, here comes a tricky part, and one of the most distinctive aspects of the firmware developed: typically you would expect that when the _Display time_ (just to mention one example) state is to be displayed (or active), the program execution jumps to the _Display time_ function, and remains inside the function for as long as the clock displays the time, and once the user switches o some other state, the function call returns and changes to the appropiate new function call. This is shown in the image below.

< img. Typical program flow >

Well, that's not the case here. This kind of application 