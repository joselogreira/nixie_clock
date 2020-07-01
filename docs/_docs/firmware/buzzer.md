---
title: Buzzer themes
permalink: /docs/buzzer/
---


Although the [piezoelectric buzzer](https://en.wikipedia.org/wiki/Buzzer) used is mechanically design for a resonant frequency of 4KHz, it can also produce tones at a wide range of frequencies. For the song themes implemented, more than 2 octaves were used to play all notes (from A6 to C9).

<iframe width="560" height="315" src="https://www.youtube.com/embed/HfNS7TEvCeo?start=131" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

The mechanism for playng song themes in a non-blocking fashion is described below.

## Timer 

The 16-bit __TIMER 4__ is used. It's initialized and configured in `timers.c` with `timer_buzzer_init()`. Its behavior is to produce a PWM signal with 50% duty cycle by toggling an output pin on every compare match. That way, by changing the compare match value, the PWM frequency that drives the buzzer changes. `timer_buzzer_set()` is responsible for changing the buzzer frequency.

Base timer frequency is __2MHz__. Values for the compare match register can be derived for any note. It was chosen to use the [equal-tempered scale](https://pages.mtu.edu/~suits/notefreqs.html). These values are defined as macros inside `buzzer.c`:

```c
// TIMER COUNTER 3 MACROS (notes)
// All notes are divided by 8, since the operation frequency was changed from
// 16MHz to 2MHz
#define N_A6        4545/8  // 1760Hz
#define N_B6        4050/8  // 1975.53Hz
#define N_C7        3822/8  // 2093.00Hz
#define N_Db7       3608/8  // 2217.46Hz
//
// more notes
//
#define N_A8        1136/8  // 7040.00Hz
#define N_B8        1012/8  // 7902.13Hz
#define N_C9        956/8   // 8372.02Hz
#define N_SIL       0xFFFF  // Silence
```

## Tempo

On the other hand, there must be a mechanism to compute the duration of a musical note. This is achieved by using the system-wide __1ms__ time base. Thus, the note duration can now be expressed as a counter value, which translates to a time multiple of __1ms__. 

But, when dealing with sheet music the notes' duration is determined by the song's [tempo](https://en.wikipedia.org/wiki/Tempo) in combination with the [music symbol](https://en.wikipedia.org/wiki/List_of_musical_symbols) (I hope not to say some blasphemy for musicians). That way, a base tempo of __100__ (beats per minute) is chosen to determine the duration of notes. Any song with different tempo must take 100 bpm as a reference to make an equivalence for its own tempo. Notes' duration with Tempo of 100 are:

```c
// Music notes duration (reference tempo: 100)
#define SIXTEENTH_NOTE  (QUARTER_NOTE / 4)
#define TWELVETH_NOTE   (QUARTER_NOTE / 3)
#define EIGHTH_NOTE     (QUARTER_NOTE / 2)
#define SIXTH_NOTE      (TWELVETH_NOTE * 2)
#define QUARTER_NOTE    (60000 / 100)
#define HALF_NOTE       (QUARTER_NOTE * 2)
#define WHOLE_NOTE      (QUARTER_NOTE * 4)
```

## Song themes array

Putting both previous concepts together, any song based on tones can be expressed as an array of note-duration pairs. The [Star Wars theme](https://www.youtube.com/watch?v=lK25-we91L8) can now be represented as shown below. It has a tempo of __108__, so an adjustment must be made to the notes' duration in order to properly play the song.

```c
/*
* Melodies are stored as an array of note-duration structures. "note" field 
* determines the musical note (the PWM frequency of the buzzer), and "duration"
* determines how long the note is to be played.
*/
typedef struct {
    uint16_t note;
    uint16_t duration;
} note_s;

static const note_s star_wars_theme[] PROGMEM = {
    {N_SIL, QUARTER_NOTE},
    {N_C7, TWELVETH_NOTE},
    {N_C7, TWELVETH_NOTE},
    {N_C7, TWELVETH_NOTE},
    {N_F7, HALF_NOTE},
    {N_C8, HALF_NOTE},
    {N_B7, TWELVETH_NOTE},
    {N_A7, TWELVETH_NOTE},
    {N_G7, TWELVETH_NOTE},
    {N_F8, HALF_NOTE},
    {N_C8, QUARTER_NOTE},
    {N_B7, TWELVETH_NOTE},
    {N_A7, TWELVETH_NOTE},
    {N_G7, TWELVETH_NOTE},
    {N_F8, HALF_NOTE},
    {N_C8, QUARTER_NOTE},
    {N_B7, TWELVETH_NOTE},
    {N_A7, TWELVETH_NOTE},
    {N_B7, TWELVETH_NOTE},
    {N_G7, HALF_NOTE},
};
```

## Playing the songs

Much like the [buttons debouncing routine](/nixie_clock/docs/debounce/), the `buzzer_music()` function works the same way. It relies on a __1ms__ loop period, so that the function may be called once per millisecond. Its internal variables need to be `static` to preserve the context between subsequent executions.

First time the function is called, an initialization step occurs, which initializes some variables and adjusts notes' duration, storing the new value in a dynamically created vector.

```c
duration_p = (uint16_t *)calloc(size, sizeof(uint16_t));

// calculate proper notes' duration and store the result in vector
for(uint8_t i = 0; i < size; i++)
    duration_p[i] = note_duration(pgm_read_word(&theme_p[i].duration), tempo);

//
// a few lines  below:
/*===========================================================================*/
/*
*   Base tempo is 100 (100 bits per minute). 
*/
static uint16_t note_duration(uint16_t counts, uint8_t tempo)
{
    float f_tempo = (float) tempo;
    float f_counts = (float) counts;
    float x = (100.0 / f_tempo) * f_counts;

    return (uint16_t) x;
}
```

After that, a couple of nested counters `count` and `n` iterate through the entire song theme array, setting the buzzer timer with the corresponding note frequency, and letting it play for the corresponding note duration.

```c
if(count >= duration_p[n]){
    n++;
    if(pgm_read_word(&theme_p[n].note) != N_SIL)
        timer_buzzer_set(ENABLE, pgm_read_word(&theme_p[n].note));
    else
        timer_buzzer_set(DISABLE, pgm_read_word(&theme_p[n].note));
    count = 0;
}
```

Once the cycle reaches the end of the array, it disables the buzzer and outputs a `TRUE` value to indicate the end of the song.

```c
if(n >= size){
    timer_buzzer_set(DISABLE, N_C8);
    free((void *)duration_p);
    theme_p = NULL;
    duration_p = NULL;
    intro = TRUE;
    out = TRUE;
}
```