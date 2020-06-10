/**
 * @file debug.c
 * @brief Test functions
 *
 * Functions that test all system functions
 *
 * @author Jose Logreira
 * @date 08.10.2018
 *
 */

/******************************************************************************
*******************	I N C L U D E   D E P E N D E N C I E S	*******************
******************************************************************************/

#include "debug.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>

/******************************************************************************
******************* F U N C T I O N   D E F I N I T I O N S *******************
******************************************************************************/

static void print_rom_report(void);

/*===========================================================================*/
/*
* This test sequence is User-accessible, meaning that it can be easily accessed
* by holding the X button pressed at the end of the "Intro" animation. Here,
* the user can inspect:
* - Whether all tubes' digits light up
* - the asyunchronous timer peripheral is working (otherwise, the LEDs sync
* signal would not work, and the LEDs breathing effect would remain static)
* - All RGB LED colors are working
* - The buzzer sounds properly
*/
state_t usr_test(state_t state)
{
	static uint8_t intro = TRUE;
	static uint16_t count = 0;
	static uint8_t n = 0;
	// leds related quantities
	static uint8_t leds_cnt_up = TRUE;
	static uint16_t leds_count = 0;
	static uint8_t leds_color = 0;

	if(intro){
		intro = FALSE;
		count = 0;
		n = 0;
		leds_cnt_up = TRUE;
		leds_count = 0;
		leds_color = 0;
	}

	/*
	* LEDs SEQUENCES
	* - breathing sequence: leds are synced to the RTC by means of the 
	*   time.update flag.
	*/
	if(leds_cnt_up){
		if(leds_count < 995) leds_count++;
		else leds_count = 995;
	} else {
		if(leds_count > 5) leds_count--;
		else leds_count = 5;
	}
	// sync leds_count with the general counter (T = 1ms)
	if(time.update){
		time.update = FALSE;
		if((leds_cnt_up) && (leds_count > 500)){
			leds_cnt_up = FALSE;
			leds_count = 995;
		} else if((!leds_cnt_up) && (leds_count < 500)){
			leds_cnt_up = TRUE;
			leds_count 
			= 5;
			leds_color++;
			if(leds_color >= 3) leds_color = 0;
		}
	}
	// LEDs update value every 5ms, not every ms (LEDs value does not change every ms)
	if(!(leds_count % 5)){
		uint8_t led_r = 0, led_g = 0, led_b = 0;
		if(leds_color == 0){
			led_r = (uint8_t)(leds_count>>4);
			led_g = 0;
			led_b = 0;
		} else if(leds_color == 1){
			led_r = 0;
			led_g = (uint8_t)(leds_count>>4);
			led_b = 0;
		} else if(leds_color == 2){
			led_r = 0;
			led_g = 0;
			led_b = (uint8_t)(leds_count>>4);;
		}
		timer_leds_set(ENABLE, led_r, led_g, led_b);
	}

	/*
	* DISPLAY transition
	*/
	display.d1 = n;
	display.d2 = n;
	display.d3 = n;
	display.d4 = n;	

	/* 
	* BUTTONS actions
	* If any of the buttons is pressed, jump to SYSTEM_INTRO
	*/
	if((btnX.action) || (btnY.action) || (btnZ.action)){
		btnX.action = FALSE;
		btnY.action = FALSE;
		btnZ.action = FALSE;
		state = SYSTEM_INTRO;
		intro = TRUE;
	}

	/*
	*	GENERAL FUNCTION COUNTER and timeout
	*/
	count++;
	if(count >= 500){
		count = 0;
		n++;
		if(n >= 10){
			n = 0;
			buzzer_beep();
		}
	}

	return state;
}

/*===========================================================================*/
/*
* This is a SEQUENTIAL (or BLOCKING) function, meaning that it does not run 
* every 1ms (as the other functions in the main loop), but rather it executes
* some functions sequentially to test all possible testable things by the MCU.
* These are:
* - Voltages: 
* 	- Boost voltage: turns it on and off, and measures output
*   - Boost controller voltage regulator
*	- Input voltage
*   - Implicit test of MCU rail voltage (not 5V but a little lower)
* - RTC: is the asynchronous timer working properly?
* - Buzzer: make some sounds
* - LEDs: turns them on, one by one
*
* The MCU reports each step using the serial interface, and stores the results
* in ROM. This report can then be printed out later to further debug the system
*/
state_t production_test(state_t state)
{
 	uint16_t cnt = 0, x = 0;	// auxiliary counters
	uint8_t clock_ok = TRUE;	// flags to control the test execution flow
	uint8_t buzzer_ok = TRUE;	// flags to control the test execution flow
	char c;					// char received
	uint8_t test1[8];		// vectors to store results
	uint8_t test2[8];		// vectors to store results
	uint8_t test3[8];		// vectors to store results
	uint8_t *p[] = {test1, test2, test3};	// auxiliary pointer to vectors
	uint16_t times[3];		// vectors to store results
	uint8_t leds_ok[4];		// vectors to store results

	/* 
	* Fun fact: If the Serial-to-USB adapter is connected to the circuit first,
	* then the TX line (of the adapter), which is energized to approx 5V, 
	* backpowers the MCU, and the MCU presumably goes to sleep or halts forever
	* (go figure!). 
	* Thus, the correct order for energizing the circuit is as follows:
	* 1. Short circuit V_TST so that the MCU may enter the test mode
	* 2. Energize the circuit with the 12V adapter.
	* 3. Once in test mode, DISABLE the RX/TX pins and set RX as input. A
	*    pull-down resistor is required in RX.
	* 4. Poll the RX pin to look for a logic HIGH input. A logic HIGH means
	*    that the Serial-to-USB adapter has been connected. 
	* 5. Enable UART interface and launch the test
	*/
	uart_set(DISABLE);
	_delay_ms(1);
	while(!uart_check_rx()){
		led_blink(1, 20);
	}
	// wait for any voltage transient to stop
	uart_set(ENABLE);
	_delay_ms(500);
	

	// Initial presentation
	uart_send_string_p(PSTR("\r\n\r\n"));
	uart_send_string_p(PSTR("                                                    /\r\n"));
	uart_send_string_p(PSTR("                                                  //\r\n"));
	uart_send_string_p(PSTR("////////////////////////////////////            ///       ////////////////////////////////////\r\n"));
	uart_send_string_p(PSTR("                                              ////\r\n"));
	uart_send_string_p(PSTR("       //////////////////////////////////   ///////  //////////////////////////////////\r\n"));
	uart_send_string_p(PSTR("                                             ////\r\n"));
	uart_send_string_p(PSTR("               ///////////////////////////  ///   ///////////////////////////\r\n"));
	uart_send_string_p(PSTR("                                           //\r\n"));
	uart_send_string_p(PSTR("                                          /\r\n"));
	uart_send_string_p(PSTR("                               BILITRIUM TECHNOLOGIES S.A.S\r\n"));
	uart_send_string_p(PSTR("                                    BOGOTÁ - COLOMBIA\r\n"));
	uart_send_string_p(PSTR("\r\n"));
	uart_send_string_p(PSTR("                                      hEllO WoRld!!!\r\n"));
	uart_send_string_p(PSTR("\r\nTEST MODE ENTERED"));
	uart_send_string_p(PSTR("\n\rhit ENTER to continue...\n\r"));
	c = uart_read_char();

	// "hidden" commands:
	// if '.' is pressed three times, print the whole test report saved in ROM.
	// if 'q' is pressed, quit this test 
	if(c == '.'){
		c = uart_read_char();
		if(c == '.'){
			c = uart_read_char();
			if(c == '.'){
				// PRINT REPORT OF NO OF TESTS PERFORMED, AND RESULTS OF LAST TEST
				print_rom_report();
				uart_send_string_p(PSTR("\n\r\n\r < REPORT END >"));
				uart_send_string_p(PSTR("\n\rPress any key to exit\n\r"));
				uart_read_char();
				state = SYSTEM_INTRO;
				return state;
			}
		}
	} else if(c == 'q'){
		// quit factory test
		state = SYSTEM_INTRO;
		return state;
	}

	/*************************************************************************/
	// First step: Voltages check, switching the boost controller ON and OFF
	// to verify proper turn-on and off times and steady-state value
	uart_send_string_p(PSTR(" ( 1 ) SYSTEM VOLTAGES\n\r"));

	// Perform the voltages test 3 times.
	for(uint8_t i = 0; i < 3; i++){
		uart_send_string_p(PSTR("\n\rcheck #"));
		char str[3];
		itoa(i+1, str, 10);
		uart_send_string(str);
		BOOST_SET(DISABLE);
		uart_send_string_p(PSTR("\n\rboost converter DISABLED"));
		// Wait for the output cap to discharge. It takes 4 seconds to discharge
		_delay_ms(4000);
		// perform the 4 voltages' tests and store all 4 results in the first
		// 4 positions of the given vector
		adc_factory_voltages_test(BOOST_OFF, p[i]);
		BOOST_SET(ENABLE);
		uart_send_string_p(PSTR("\n\rboost converter ENABLED"));
		// wait for the output cap to charge back up. Takes only less than 200ms
		_delay_ms(2000);
		// perform the 4 voltages' tests and store all 4 results in the second
		// 4 positions of the given vector
		adc_factory_voltages_test(BOOST_ON, p[i] + 4);
 	}
 	// store all obtained results in ROM
 	rom_store_voltages_results(test1, test2, test3);

 	/*************************************************************************/
 	// Second step: Crystals and Timers check. Count how many milliseconds 
 	// elapse (counted with the 16MHz crystal) during 1 second (counted with
 	// the 32.765KHz quartz crystal). Should match pretty closely.
 	uart_send_string_p(PSTR("\n\r\n\r ( 2 ) TIMING & CRYSTALS\n\r"));
 	
 	// init vector that will store the timing results
 	times[0] = 0;
 	times[1] = 0;
 	times[2] = 0;
 	main_loop_execute = FALSE;
 	time.update = FALSE;
 	
 	// Enable ISRs for this step, since it requires the general timer to
 	// trigger the ISR
 	sei();

 	// First off, detect whether the quartz crystal is working, by waiting for
 	// the next second ISR to occur. If it does not trigger, then the crystal
 	// may be halted.
 	while(!time.update){
 		_delay_us(100);
 		x++;
 		if(x > 30000){ 		// 3 seconds elapsed
 			clock_ok = FALSE;
 			uart_send_string_p(PSTR("\n\r ===> FAIL! Clock signal halted"));
 			break;
 		}
 	}
 	
 	// If the asynchronous timer (run by the quartz crystal) is working, test
 	// its approximate accuracy by counting how many ms are there in a full
 	// second counted by the timer.
	if(clock_ok){
	 	x = 0;
 		while((x < 3) && (clock_ok)){
		 	time.update = FALSE;
		 	while(!time.update);
		 	time.update = FALSE;
		 	main_loop_execute = FALSE;
		 	while(!time.update){
		 		while(!main_loop_execute);
		 		main_loop_execute = FALSE;
		 		cnt++;
		 	}
		 	times[x] = cnt;
		 	x++;
		 	uart_send_string_p(PSTR("\n\r ms per RTC second: "));
		 	char str[5];
		 	itoa(cnt, str, 10);
		 	uart_send_string(str);
		 	// Accepted range: [990ms, 1010ms]
		 	if((cnt >= 990) && (cnt <= 1010)){
		 		uart_send_string_p(PSTR(" - PASS"));
		 	} else {
		 		uart_send_string_p(PSTR(" - FAIL!"));
		 		clock_ok = FALSE;
		 	}
		 	cnt = 0;
	 	}
 	}
 	cli();
 	// store all obtained results in ROM
 	rom_store_timing_results(clock_ok, times);

	/*************************************************************************/
 	// Third step: Buzzer sounds. Play some sounds on the buzzer and ask the
 	// operator whether the buzzer sounds OK
	uart_send_string_p(PSTR("\n\r\n\r ( 3 ) BUZZER & MUSIC"));

	// Enable ISRs for this step, since it requires the general timer to
	// trigger the ISR
	sei();
	
	uart_send_string_p(PSTR("\n\r > Does it sound good? y/n\n\r"));
	
	x = 0;
	while((x < 3) && (buzzer_ok)){
		while(!main_loop_execute);
		main_loop_execute = FALSE;
		// play three different themes, one after the other.
		if(x == 0) buzzer_music(MAJOR_SCALE, ENABLE);
		else if(x == 1) buzzer_music(SIMPLE_ALARM, ENABLE);
		else if(x == 2) buzzer_music(SUPER_MARIO, ENABLE);
		// asynchronously polling the UART receiver every 1ms, to avoid 
		// enabling the Rx interrupt.
		if(UCSR2A & (1<<RXC)){
			c = UDR2;
			uart_send_char(c);		// echo
			// when the operator inputs some char, stop current theme
			buzzer_music(MAJOR_SCALE, DISABLE);
			if((c == 'y') || (c == 'Y')){
				// If operator responds YES, switch to the next theme or go to
				// the next step if it was the last theme.
				x++;
				if(x < 3) uart_send_string_p(PSTR("\n\r > Does it sound good? y/n\n\r"));
			} else if((c == 'n') || (c == 'N')){
				// If operator responds NO, flag the buzzer as FAIL, and go
				// to the next step
				uart_send_string_p(PSTR("\n\r - Buzzer FAIL!. Please verify soldering and replace if necessary"));
				buzzer_ok = FALSE;
			} else  {
				uart_send_string_p(PSTR("\n\r ¿? Command not recognized. Please try again\n\r"));
				x = 0;
			}
		}
	}
	cli();
	// store obtained result in ROM
 	rom_store_buzzer_results(buzzer_ok);

	/*************************************************************************/
 	// Fourth step: LEDs brightnes and colour. Light up all colours and ask
 	// operator whether they look good.
	uart_send_string_p(PSTR("\n\r\n\r ( 4 ) LEDs COLORS & BRIGHTNESS"));
	
	// init vector that will store the results
	leds_ok[0] = FALSE;
	leds_ok[1] = FALSE;
	leds_ok[2] = FALSE;
	leds_ok[3] = FALSE;

	x = 0;
	while(x < 4){
		switch(x){
			case 0:
				timer_leds_set(ENABLE, 255, 0, 0);
				uart_send_string_p(PSTR("\n\r > RED."));
				break;
			case 1:
				timer_leds_set(ENABLE, 0, 255, 0);
				uart_send_string_p(PSTR("\n\r > GREEN."));
				break;
			case 2:
				timer_leds_set(ENABLE, 0, 0, 255);
				uart_send_string_p(PSTR("\n\r > BLUE."));
				break;
			case 3: 
				timer_leds_set(ENABLE, 255, 255, 255);
				uart_send_string_p(PSTR("\n\r > WHITE."));
				break;
		}
		// ask the operator whether colors look OK
		uart_send_string_p(PSTR(" Looks good? y/n\n\r"));
		c = uart_read_char();
		uart_send_char(c);		// echo
		timer_leds_set(DISABLE, 0, 0, 0);
		// If operator responds YES, switch to the next color or go to the next
		// step if it was the last color.
		if((c == 'y') || (c == 'Y')){
			leds_ok[x] = TRUE;
			x++;
			if(x == 4){
				uart_send_string_p(PSTR("\n\r - LEDs PASS.\n\r"));
			}			
		} else if((c == 'n') || (c == 'N')){
			uart_send_string_p(PSTR("\n\r - LEDs FAIL! Check solder joints or replace LEDs\n\r"));
			x = 4;
		} else {
			uart_send_string_p(PSTR("\n\r ¿? Command not recognized. Please try again\n\r"));
			x = 0;
			leds_ok[0] = FALSE;
			leds_ok[1] = FALSE;
			leds_ok[2] = FALSE;
			leds_ok[3] = FALSE;
		}
	}
	// store obtained results in ROM
	rom_store_leds_results(leds_ok);

	/*************************************************************************/
	// Increase the counter in ROM that contains the No of tests performed
	x = (uint16_t)rom_increase_test_cnt();
	char str[3];
	itoa(x, str, 10);
	uart_send_string_p(PSTR("\n\r ### Tests Count: "));
	uart_send_string(str);
	uart_send_string_p(PSTR("\n\r << TEST FINISHED >>\n\r"));
	
	uart_send_string_p(PSTR("                     __gggrgM**M#mggg__\r\n"));
	uart_send_string_p(PSTR("                __wgNN@*B*P**mp**@d#*@N#Nw__\r\n"));
	uart_send_string_p(PSTR("              _g#@0F_a*F#  _*F9m_ ,F9*__9NG#g_\r\n"));
	uart_send_string_p(PSTR("           _mN#F  aM*    #p*    !q@    9NL *9#Qu_\r\n"));
	uart_send_string_p(PSTR("          g#MF _pP*L  _g@*9L_  _g**#__  g*9w_ 0N#p\r\n"));
	uart_send_string_p(PSTR("        _0F jL**   7_wF     #_gF     9gjF   *bJ  9h_\r\n"));
	uart_send_string_p(PSTR("       j#  gAF    _@NL     _g@#_      J@u_    2#_  #_\r\n"));
	uart_send_string_p(PSTR("      ,FF_#* 9_ _#*  *b_  g@   *hg  _#*  !q_ jF **_09_\r\n"));
	uart_send_string_p(PSTR("      F N*    #p*      Ng@       `#g*      *w@    *# t\r\n"));
	uart_send_string_p(PSTR("     j p#    g*9_     g@*9_      gP*#_     gF*q    Pb L\r\n"));
	uart_send_string_p(PSTR("     0J  k _@   9g_ j#*   *b_  j#*   *b_ _d*   q_ g  ##\r\n"));
	uart_send_string_p(PSTR("     #F  `NF     *#g*       *Md*       5N#      9W*  j#\r\n"));
	uart_send_string_p(PSTR("     #k  jFb_    g@*q_     _**9m_     _**R_    _#Np  J#\r\n"));
	uart_send_string_p(PSTR("     tApjF  9g  J*   9M_ _m*    9/_ _**   *#  gF  9_jNF\r\n"));
	uart_send_string_p(PSTR("      k`N    *q#       9g@        #gF       ##*    #*j\r\n"));
	uart_send_string_p(PSTR("      `_0q_   #*q_    _&*9p_    _g*`L_    _**#   jAF,.\r\n"));
	uart_send_string_p(PSTR("       9# *b_j   *b_ g*    *g _gF    9_ g#*  *L_**qNF\r\n"));
	uart_send_string_p(PSTR("        *b_ *#_    *NL      _B#      _I@     j#* _#*\r\n"));
	uart_send_string_p(PSTR("          NM_0**g_ j**9u_  gP  q_  _w@ ]_ _g**F_g@\r\n"));
	uart_send_string_p(PSTR("           *NNh_ !w#_   9#g*    *m**   _#** _dN@*\r\n"));
	uart_send_string_p(PSTR("              9##g_0@q__ #*4_  j**k __*NF_g#@P*\r\n"));
	uart_send_string_p(PSTR("                *9NN#gIPNL_ *b@* _2M*Lg#N@F*\r\n"));
	uart_send_string_p(PSTR("                    **P@*NN#gEZgNN@#@P**\r\n"));

	uart_send_string_p(PSTR(" -----------> Press any key to continue\r\n\r\n"));
	uart_read_char();

 	state = SYSTEM_INTRO;
	return state;
}

/*===========================================================================*/
static void print_rom_report(void)
{
	uint8_t x;

	// Print No of tests performed 'til now
	x = (uint16_t)rom_query_test_cnt();
	uart_send_string_p(PSTR("No of Tests Performed: "));
	char str[6];
	itoa(x, str, 10);
	uart_send_string(str);

	// Print an array of all (24) voltage tests performed
	uint8_t v[24];
	rom_query_voltages_test(v);
	uart_send_string_p(PSTR("\n\r [ 1 ] SYSTEM VOLTAGES"));
	uart_send_string_p(PSTR("\n\r test No 1"));
	uart_send_string_p(PSTR("\n\r > Boost DISABLED:"));
	uart_send_string_p(PSTR("\n\r - V_hv      | V_ctl_reg | V_in      | V_dd\n\r   "));
	for(uint8_t i = 0; i < 4; i++){
		if(v[i] == PASS) uart_send_string_p(PSTR("PASS        "));
		else uart_send_string_p(PSTR("FAIL        "));
	}
	uart_send_string_p(PSTR("\n\r > Boost DISABLED:"));
	uart_send_string_p(PSTR("\n\r - V_hv      | V_ctl_reg | V_in      | V_dd\n\r   "));
	for(uint8_t i = 4; i < 8; i++){
		if(v[i] == PASS) uart_send_string_p(PSTR("PASS        "));
		else uart_send_string_p(PSTR("FAIL        "));
	}

	uart_send_string_p(PSTR("\n\r test No 2"));
	uart_send_string_p(PSTR("\n\r > Boost DISABLED:"));
	uart_send_string_p(PSTR("\n\r - V_hv      | V_ctl_reg | V_in      | V_dd\n\r   "));
	for(uint8_t i = 8; i < 12; i++){
		if(v[i] == PASS) uart_send_string_p(PSTR("PASS        "));
		else uart_send_string_p(PSTR("FAIL        "));
	}
	uart_send_string_p(PSTR("\n\r > Boost DISABLED:"));
	uart_send_string_p(PSTR("\n\r - V_hv      | V_ctl_reg | V_in      | V_dd\n\r   "));
	for(uint8_t i = 12; i < 16; i++){
		if(v[i] == PASS) uart_send_string_p(PSTR("PASS        "));
		else uart_send_string_p(PSTR("FAIL        "));
	}

	uart_send_string_p(PSTR("\n\r test No 3"));
	uart_send_string_p(PSTR("\n\r > Boost DISABLED:"));
	uart_send_string_p(PSTR("\n\r - V_hv      | V_ctl_reg | V_in      | V_dd\n\r   "));
	for(uint8_t i = 16; i < 20; i++){
		if(v[i] == PASS) uart_send_string_p(PSTR("PASS        "));
		else uart_send_string_p(PSTR("FAIL        "));
	}
	uart_send_string_p(PSTR("\n\r > Boost DISABLED:"));
	uart_send_string_p(PSTR("\n\r - V_hv      | V_ctl_reg | V_in      | V_dd\n\r   "));
	for(uint8_t i = 20; i < 24; i++){
		if(v[i] == PASS) uart_send_string_p(PSTR("PASS        "));
		else uart_send_string_p(PSTR("FAIL        "));
	}

	// Print List of Times registered during the RTC test
	uint16_t t[3];
	x = rom_query_rtc_ok_test();
	rom_query_rtc_time_test(t);
	uart_send_string_p(PSTR("\n\r\n\r [ 2 ] TIMING & CRYSTALS"));
	uart_send_string_p(PSTR("\n\rClock Timer: "));
	if(x == PASS) uart_send_string_p(PSTR("PASS"));
	else uart_send_string_p(PSTR("FAIL"));
	uart_send_string_p(PSTR("\n\r1 Hz times:"));
	for(uint8_t i = 0; i < 3; i++){
		itoa(t[i], str, 10);
		uart_send_string_p(PSTR("\n\r > "));
		uart_send_string(str);
	}

	// Print Buzer result during test
	x = rom_query_buzzer_ok_test();
	uart_send_string_p(PSTR("\n\r\n\r [ 3 ] BUZZER & MUSIC"));
	uart_send_string_p(PSTR("\n\rBuzzer test: "));
	if(x == PASS) uart_send_string_p(PSTR("PASS"));
	else uart_send_string_p(PSTR("FAIL"));

	// Print LEDs result during test
	uint8_t l[4];
	rom_query_leds_test(l);
	uart_send_string_p(PSTR("\n\r\n\r [ 4 ] LEDs COLORS & BRIGHTNESS"));
	uart_send_string_p(PSTR("\n\rRED: "));
	if(l[0] == PASS) uart_send_string_p(PSTR("PASS"));
	else uart_send_string_p(PSTR("FAIL"));
	uart_send_string_p(PSTR("\n\rGREEN: "));
	if(l[1] == PASS) uart_send_string_p(PSTR("PASS"));
	else uart_send_string_p(PSTR("FAIL"));
	uart_send_string_p(PSTR("\n\rBLUE: "));
	if(l[2] == PASS) uart_send_string_p(PSTR("PASS"));
	else uart_send_string_p(PSTR("FAIL"));
	uart_send_string_p(PSTR("\n\rWHITE: "));
	if(l[3] == PASS) uart_send_string_p(PSTR("PASS"));
	else uart_send_string_p(PSTR("FAIL"));

	uart_send_string_p(PSTR("\n\r\n\r < REPORT END >\n\r"));
}


