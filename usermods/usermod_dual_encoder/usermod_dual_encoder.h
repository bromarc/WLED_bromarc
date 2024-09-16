//PRE-COMPILE: Jan 24, 2024 reformatting after copy into VS code was fucked with tabs. tried adding in the variables too. Holding here before copying into WIP doc.
//Compile worked.

#pragma once

#include "wled.h"
#include <stdbool.h>

//v2 usermod that allows to change brightness and color using a rotary encoder, 
//change between modes by pressing a button (many encoders have one included)
class DualEncoderMod : public Usermod
{
private:
	//Private class members. You can declare variables and functions only accessible to your usermod here
	unsigned long lastReleaseTime1 = 0;		//Unused in original...?
	unsigned long currentTime1;
	unsigned long loopTime1;
	unsigned long lastReleaseT1;

	unsigned long lastReleaseTime2 = 0;		//Unused in original...?
	unsigned long currentTime2;
	unsigned long loopTime2;
	unsigned long lastReleaseT2;

	unsigned char b1_state = HIGH;
	unsigned char prev_b1_state = HIGH;
	CRGB fastled_col;
	CHSV prim_hsv;
	int16_t new_val;

	unsigned char b2_state = HIGH;
	unsigned char prev_b2_state = HIGH;
	//CRGB fastled_col;
	//CHSV prim_hsv;
	//int16_t new_val;

	unsigned char Enc_1A;
	unsigned char Enc_1B;
	unsigned char Enc_1A_prev = 0;

	unsigned char Enc_2A;
	unsigned char Enc_2B;
	unsigned char Enc_2A_prev = 0;

	bool btn1_press = false;			//Is button1 being pressed
	bool btn1_hold = false;				//Is button1 being held
	bool btn1_turn = false;				//Has button been turned while pushed (reset after release
	bool short_clk1 = false;
	unsigned long btn1_time_p = 0;		//Button1 time press holder
	//unsigned char btn1_count = 0;		//Loop counter for button1 press hold, (if needed)

	bool btn2_press = false;			//Is button2 being pressed
	bool btn2_hold = false;				//Is button2 being held
	bool btn2_turn = false;				//Has button been turned while pushed (reset after release
	bool short_clk2 = false;
	unsigned long btn2_time_p = 0;		//Button2 time press holder
	//unsigned char btn2_count = 0;		//Loop counter for button2 press hold, (if needed)
	
	bool Tshiftevent_1 = false;
	bool Tshiftevent_2 = false;

	bool isSeg1On;
	bool isSeg2On;

	unsigned char bri1;
	unsigned char r1;
	unsigned char g1;
	unsigned char b1;

	unsigned char bri2;
	unsigned char r2;
	unsigned char g2;
	unsigned char b2;


	// private class memebers configurable by Usermod Settings (defaults set inside readFromConfig())
	int8_t pins[6]; // pins[0] = CLK from encoder1, pins[1] = DT from encoder1, pins[2] = SW (switch) from encoder1 (optional). 3-5 are same for encoder 2
	int fadeAmount; // how many points to fade the Neopixel with each step
	int shiftAmount=3; //amount to shift the color temp
	int click_wait=350;
	int click_counter1=0;	//count number of clicks to determine multi clicks
	int click_counter2=0;	//count number of clicks to determine multi clicks
	

public:
	//Functions called by WLED

	/*
	* setup() is called once at boot. WiFi is not yet connected at this point.
	* You can use it to initialize variables, sensors or similar.
	*/
	void setup()
	{
		//Serial.println("Hello from my usermod!");
		pinMode(pins[0], INPUT_PULLUP);
		pinMode(pins[1], INPUT_PULLUP);
		if(pins[2] >= 0) pinMode(pins[2], INPUT_PULLUP);
		currentTime1 = millis();
		loopTime1 = currentTime1;
		pinMode(pins[3], INPUT_PULLUP);
		pinMode(pins[4], INPUT_PULLUP);
		if(pins[5] >= 0) pinMode(pins[5], INPUT_PULLUP);
		//currentTime2 = millis();
		loopTime2 = currentTime1;
	}

  /*
   * loop() is called continuously. Here you can check for events, read sensors, etc.
   * 
   * Tips:
   * 1. You can use "if (WLED_CONNECTED)" to check for a successful network connection.
   *    Additionally, "if (WLED_MQTT_CONNECTED)" is available to check for a connection to an MQTT broker.
   * 
   * 2. Try to avoid using the delay() function. NEVER use delays longer than 10 milliseconds.
   *    Instead, use a timer check as shown here.
   */
	void loop()
	{
		currentTime1 = millis(); // get the current elapsed time

		if (currentTime1 >= (loopTime1 + 2)) // 2ms since last check of encoder = 500Hz
		{
			Segment& seg1 = strip.getSegment(0);
			Segment& seg2 = strip.getSegment(1);
			if(pins[2] >= 0) {		//Making button pins "optional" is probably unnecessary and a waste? Did not work when tried to remove... Could maybe try again?
				b1_state = digitalRead(pins[2]);	//Check button 1 state
				if (prev_b1_state != b1_state)		//Button1 changed
				{
					if (b1_state == LOW)				//Button1 was pressed
					{
						btn1_press=true;
						btn1_time_p=currentTime1;		//Save time of start of press
						if((btn1_time_p-lastReleaseTime1)>(click_wait+1)) {			//Click-counting previously expired				time between last release and current press is greater than cool down. Why did I add 1 here?
							click_counter1=0;
						}
						//Compare current time to _____ ???????? huh?
					}
					else		//Button1 was released
					{
						if((btn1_hold==false) && (btn1_turn==false)) {	//SHORT PRESS (Button is released after not being held)
							if(Tshiftevent_1==false) {
								click_counter1++;
								short_clk1=true;
							}
							else {
								Tshiftevent_1 = false;
							}
						}
						
						lastReleaseTime1=currentTime1;		/////****Should these all be moved outside of the IFs?
						btn1_press=false;
						//btn1_time_p=0;		//Reset press time? Or no?
						btn1_turn=false; 	//Reset
						btn1_hold=false;
					}
					prev_b1_state = b1_state;
				}
				else {	//Button1 has not changed
					if((btn1_press==false) && ((currentTime1 - lastReleaseTime1) > click_wait) && (click_counter1 > 0)) {		//after short click(s) and cool-down has elapsed
						switch(click_counter1) {		//Multi-click switch case
							//Make these selectable functions from the UI eventually...
							case 1:{		//single click
								//TOGGLE SEGMENT
								//Segment& seg1 = strip.getSegment(0);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
								//seg1.setOption(SEG_OPTION_SELECTED, 1);
								//seg2.setOption(SEG_OPTION_SELECTED, 0);
								isSeg1On = seg1.getOption(SEG_OPTION_ON);
								seg1.setOption(SEG_OPTION_ON, !isSeg1On);
								//Trigger Change
								strip.trigger();  // force strip refresh
								stateChanged = true;  // inform external devices/UI of change
								stateUpdated(CALL_MODE_DIRECT_CHANGE);
								
								//seg1.setOption(SEG_OPTION_SELECTED, 0);		//Set back to being selected (default state)
								break;
							}
							case 2:{		//double click
								//MAX BRIGHTNESS (if on)
								isSeg1On = seg1.getOption(SEG_OPTION_ON);
								if(isSeg1On) {
									bri1 = 255;
									//Segment& seg1 = strip.getSegment(0);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
									//seg1.setOption(SEG_OPTION_SELECTED, 1);
									//seg2.setOption(SEG_OPTION_SELECTED, 0);
									
									seg1.opacity = bri1;		//Set brightness of segment
									
									//Trigger Change
									strip.trigger();  // force strip refresh
									stateChanged = true;  // inform external devices/UI of change
									stateUpdated(CALL_MODE_DIRECT_CHANGE);
									
									//seg1.setOption(SEG_OPTION_SELECTED, 0);		//Set back to being selected (default state)
								}
								break;
							}
							case 3:{		//triple click
								//Turn off or toggle OTHER segment?
								//Segment& seg2 = strip.getSegment(1);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
								//seg2.setOption(SEG_OPTION_SELECTED, 1);
								//seg1.setOption(SEG_OPTION_SELECTED, 0);
								isSeg2On = seg2.getOption(SEG_OPTION_ON);
								seg2.setOption(SEG_OPTION_ON, !isSeg2On);
								//Trigger Change
								strip.trigger();  // force strip refresh
								stateChanged = true;  // inform external devices/UI of change
								stateUpdated(CALL_MODE_DIRECT_CHANGE);
								
								//seg2.setOption(SEG_OPTION_SELECTED, 0);		//Set back to being selected (default state)
								break;
							}
							case 4:{		//4 clicks
								//IDK
								break;
							}
							case 5:{		//5 clicks
								//IDK
								break;
							}
						}		//END OF MULTI-CLICK SWITCH
						
						//reset conditions
						click_counter1=0;
						short_clk1 = false;

					}
					if((btn1_press==true) && ((currentTime1 - btn1_time_p) > 1000) && (btn1_turn==false)) {	//Button is held for more than 'x' ms  ///LONG PRESS
						if(btn1_hold==false) {		//Initial Hold detection
							btn1_hold=true;
							//SET TO THE DESIRED COLOR TEMP (255, 191, 127)? half.
							//strip.setPixelColor(strip.getSegment(0).start, CRGB(255, 191, 127));
							
							//Segment& seg1 = strip.getSegment(0);										//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
							//seg1.setOption(SEG_OPTION_SELECTED, 0);
							//seg2.setOption(SEG_OPTION_SELECTED, 0);
							uint32_t neutcol1 = RGBW32(255, 191, 127, 0);
							seg1.setColor(0, neutcol1);
							
							strip.trigger();  // force strip refresh
							stateChanged = true;  // inform external devices/UI of change
							//colorUpdated(CALL_MODE_DIRECT_CHANGE);	//colorUpdated or stateUpdated? what's the difference?
							stateUpdated(CALL_MODE_DIRECT_CHANGE);
							
							//seg1.setOption(SEG_OPTION_SELECTED, 1); 								//Set back to being selected (default state)
						}
					}
				}
			}
			///////// Copy button handling for encoder 2
			if(pins[5] >= 0) {		//Making button pins "optional" is probably unnecessary and a waste? Did not work when tried to remove... Could maybe try again?
				b2_state = digitalRead(pins[5]);	//Check button 2 state
				if (prev_b2_state != b2_state)		//Button2 changed
				{
					if (b2_state == LOW)				//Button2 was pressed
					{
						btn2_press=true;
						btn2_time_p=currentTime1;		//Save time of start of press
						if((btn2_time_p-lastReleaseTime2)>(click_wait+1)) {			//Click-counting previously expired				time between last release and current press is greater than cool down. Why did I add 1 here?
							click_counter2=0;
						}
						//Compare current time to _____ ???????? huh?
					}
					else		//Button2 was released
					{
						if((btn2_hold==false) && (btn2_turn==false)) {	//SHORT PRESS (Button is released after not being held)
							if(Tshiftevent_2==false) {
								click_counter2++;
								short_clk2=true;
							}
							else {
								Tshiftevent_2 = false;
							}
						}
						
						lastReleaseTime2=currentTime1;		/////****Should these all be moved outside of the IFs?
						btn2_press=false;
						//btn2_time_p=0;		//Reset press time? Or no?
						btn2_turn=false; 	//Reset ...... might not need this here actually?
						btn2_hold=false;
					}
					prev_b2_state = b2_state;
					}
				else {	//Button2 has not changed
					if((btn2_press==false) && ((currentTime1 - lastReleaseTime2) > click_wait) && (click_counter2 > 0)) {		//after short click(s) and cool-down has elapsed
						switch(click_counter2) {		//Multi-click switch case
							//Make these selectable functions from the UI eventually...
							case 1:{		//single click
								//TOGGLE SEGMENT
								//Segment& seg1 = strip.getSegment(0);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
								//seg2.setOption(SEG_OPTION_SELECTED, 0);		//Unselect segment because it has to be..? While the other segment not being changed does have to be selected for some reason?
								isSeg2On = seg2.getOption(SEG_OPTION_ON);
								seg2.setOption(SEG_OPTION_ON, !isSeg2On);
								//Trigger Change
								strip.trigger();  // force strip refresh
								stateChanged = true;  // inform external devices/UI of change
								stateUpdated(CALL_MODE_DIRECT_CHANGE);
								
								//seg2.setOption(SEG_OPTION_SELECTED, 1);		//Set back to being selected (default state)
								break;
							}
							case 2:{		//double click
								//MAX BRIGHTNESS (if on)
								isSeg2On = seg2.getOption(SEG_OPTION_ON);
								if(isSeg2On) {
									bri2 = 255;
									//Segment& seg2 = strip.getSegment(1);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
									//seg2.setOption(SEG_OPTION_SELECTED, 0);		//Unselect segment because it has to be..? While the other segment not being changed does have to be selected for some reason?
									
									seg2.opacity = bri2;		//Set brightness of segment
									
									//Trigger Change
									strip.trigger();  // force strip refresh
									stateChanged = true;  // inform external devices/UI of change
									stateUpdated(CALL_MODE_DIRECT_CHANGE);
									
									//seg2.setOption(SEG_OPTION_SELECTED, 1);		//Set back to being selected (default state)
								}
								break;
							}
							case 3:{		//triple click
								//Turn off or toggle OTHER segment?
								//Segment& seg1 = strip.getSegment(0);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
								//seg1.setOption(SEG_OPTION_SELECTED, 0);		//Unselect segment because it has to be..? While the other segment not being changed does have to be selected for some reason?
								isSeg1On = seg1.getOption(SEG_OPTION_ON);
								seg1.setOption(SEG_OPTION_ON, !isSeg1On);
								//Trigger Change
								strip.trigger();  // force strip refresh
								stateChanged = true;  // inform external devices/UI of change
								stateUpdated(CALL_MODE_DIRECT_CHANGE);
								
								//seg1.setOption(SEG_OPTION_SELECTED, 1);		//Set back to being selected (default state)
								break;
							}
							case 4:{		//4 clicks
								//IDK
								break;
							}
							case 5:{		//5 clicks
								//IDK
								break;
							}
						}		//END OF MULTI-CLICK SWITCH
						
						//reset conditions
						click_counter2=0;
						short_clk2 = false;

					}
					if((btn2_press==true) && ((currentTime1 - btn2_time_p) > 1000) && (btn2_turn==false)) {	//Button is held for more than 'x' ms  ///LONG PRESS
						if(btn2_hold==false) {		//Initial Hold detection
							btn2_hold=true;
							//SET TO THE DESIRED COLOR TEMP (255, 191, 127)? half.
							//strip.setPixelColor(strip.getSegment(0).start, CRGB(255, 191, 127));
							
							//Segment& seg2 = strip.getSegment(1);										//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
							//seg2.setOption(SEG_OPTION_SELECTED, 0);									//Unselect segment because it has to be..? While the other segment not being changed does have to be selected for some reason?
							uint32_t neutcol2 = RGBW32(255, 191, 127, 0);
							seg2.setColor(0, neutcol2);
							
							strip.trigger();  // force strip refresh
							stateChanged = true;  // inform external devices/UI of change
							//colorUpdated(CALL_MODE_DIRECT_CHANGE);	//colorUpdated or stateUpdated? what's the difference?
							stateUpdated(CALL_MODE_DIRECT_CHANGE);
							
							//seg2.setOption(SEG_OPTION_SELECTED, 1); 								//Set back to being selected (default state)
						}
					}
				}
			}
				
				
				
			int Enc_1A = digitalRead(pins[0]); // Read encoder 1 pins
			int Enc_1B = digitalRead(pins[1]);
			
			int Enc_2A = digitalRead(pins[3]); // Read encoder 2 pins
			int Enc_2B = digitalRead(pins[4]);
			

			if ((!Enc_1A) && (Enc_1A_prev)) { // A has gone from high to low   ENCODER IS TURNED
				btn1_time_p=currentTime1;		//turning knob resets determination of held (IS THIS NECESSARY?)  THINK ABOUT THIS ONE MORE.... THERE IS A HANG UP FOR CLICK COUNTING AFTER KNOB TURNING
				btn1_turn=true;
				//GET SEGMENT DATA FOR KNOB TURNS?
				bri1 = seg1.opacity;
				//Segment& seg1 = strip.getSegment(0);
				r1 = R(seg1.colors[0]);
				g1 = G(seg1.colors[0]);
				b1 = B(seg1.colors[0]);
				
				if (!btn1_press)	//Button1 is currently not pressed, so modify brightness
				{
					if (Enc_1B == HIGH)	// B is high so clockwise -- INCREASE BRIGHTNESS
					{
						if (bri1 + fadeAmount <= 255) {
              				bri1 += fadeAmount; // increase the brightness, dont go over 255
						}
						else {
							bri1 = 255;
						}
					}
					else if (Enc_1B == LOW)	// counter-clockwise -- DECREASE BRIGHTNESS
					{
						if (bri1 - fadeAmount >= 15)
              				bri1 -= fadeAmount; // decrease the brightness, dont go below 0
						else
							bri1 = 15;
					}
					//Apply brightness change to segment 1:
					//Segment& seg1 = strip.getSegment(0);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
					//seg1.setOption(SEG_OPTION_SELECTED, 1);
					//seg2.setOption(SEG_OPTION_SELECTED, 0);
					
					seg1.opacity = bri1;		//Set brightness of segment
					
					//Trigger Change
					strip.trigger();  // force strip refresh
					stateChanged = true;  // inform external devices/UI of change
					stateUpdated(CALL_MODE_DIRECT_CHANGE);
					
					//seg1.setOption(SEG_OPTION_SELECTED, 0);		//Set back to being selected (default state)
				}
				else if (btn1_press)	//Button 1 is pressed, so modify TEMPERATURE
				{
					Tshiftevent_1 = true;
					if (Enc_1B == HIGH)	// B is high so clockwise
					{
						r1=255;
						if((g1+shiftAmount)<=255) {
							g1=g1+shiftAmount;
						}
						else
							g1=255;
						if((b1+shiftAmount+shiftAmount)<=255) {
							b1=b1+shiftAmount+shiftAmount;
						}
						else
							b1=255;
					}
					else if (Enc_1B == LOW)	// counter-clockwise
					{
						r1=255;
						if((g1-shiftAmount)>=127) {
							g1=g1-shiftAmount;
						}
						else
							g1=127;
						if((b1-shiftAmount-shiftAmount)>=0) {
							b1=b1-shiftAmount-shiftAmount;
						}
						else
							b1=0;
					}
					//Apply Color change to segment1:
					//Segment& seg1 = strip.getSegment(0);										//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
					//seg2.setOption(SEG_OPTION_SELECTED, 1);
					//seg1.setOption(SEG_OPTION_SELECTED, 0);
					
					uint32_t newcolr_1 = RGBW32(r1, g1, b1, 0);
					seg1.setColor(0, newcolr_1);
					
					strip.trigger();  // force strip refresh
					stateChanged = true;  // inform external devices/UI of change
					//colorUpdated(CALL_MODE_DIRECT_CHANGE);	//colorUpdated or stateUpdated? what's the difference? Pretty sure colorupdated sucks?
					stateUpdated(CALL_MODE_DIRECT_CHANGE);
					
					//seg1.setOption(SEG_OPTION_SELECTED, 0);
				}
			}
			else if((Enc_1A) && (Enc_1A_prev)) {		//encoder has stopped being turned
				//Need to add a timer/debounce, like 50ms somehow?
				//Trying without:
				btn1_turn=false;
			}
			Enc_1A_prev = Enc_1A;     // Store value of A for next time
			//loopTime1 = currentTime1; // Updates loopTime1
			
			/////////////////////COPY rotation handling for encoder 2
			if ((!Enc_2A) && (Enc_2A_prev)) { // A has gone from high to low   ENCODER IS TURNED
				btn2_time_p=currentTime1;		//turning knob resets determination of held (IS THIS NECESSARY?)  THINK ABOUT THIS ONE MORE.... THERE IS A HANG UP FOR CLICK COUNTING AFTER KNOB TURNING
				btn2_turn=true;
				//GET SEGMENT DATA FOR KNOB TURNS?
				bri2 = seg2.opacity;
				//Segment& seg1 = strip.getSegment(0);
				r2 = R(seg2.colors[0]);
				g2 = G(seg2.colors[0]);
				b2 = B(seg2.colors[0]);
				
				if (!btn2_press)	//Button2 is currently not pressed, so modify brightness
				{
					if (Enc_2B == HIGH)	// B is high so clockwise -- INCREASE BRIGHTNESS
					{
						if (bri2 + fadeAmount <= 255) {
              				bri2 += fadeAmount; // increase the brightness, dont go over 255
						}
						else {
							bri2 = 255;
						}
					}
					else if (Enc_2B == LOW)	// counter-clockwise -- DECREASE BRIGHTNESS
					{
						if (bri2 - fadeAmount >= 15)
              				bri2 -= fadeAmount; // decrease the brightness, dont go below 0
						else
							bri2 = 15;
					}
					//Apply brightness change to segment 1:
					//Segment& seg1 = strip.getSegment(0);		//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
					//seg2.setOption(SEG_OPTION_SELECTED, 0);		//Unselect segment because it has to be..? While the other segment not being changed does have to be selected for some reason?

					seg2.opacity = bri2;		//Set brightness of segment
					
					//Trigger Change
					strip.trigger();  // force strip refresh
					stateChanged = true;  // inform external devices/UI of change
					stateUpdated(CALL_MODE_DIRECT_CHANGE);
					
					//seg2.setOption(SEG_OPTION_SELECTED, 1);		//Set back to being selected (default state)
				}
				else if (btn2_press)	//Button 1 is pressed, so modify TEMPERATURE
				{
					Tshiftevent_2 = true;
					if (Enc_2B == HIGH)	// B is high so clockwise
					{
						r2=255;
						if((g2+shiftAmount)<=255) {
							g2=g2+shiftAmount;
						}
						else
							g2=255;
						if((b2+shiftAmount+shiftAmount)<=255) {
							b2=b2+shiftAmount+shiftAmount;
						}
						else
							b2=255;
					}
					else if (Enc_2B == LOW)	// counter-clockwise
					{
						r2=255;
						if((g2-shiftAmount)>=127) {
							g2=g2-shiftAmount;
						}
						else
							g2=127;
						if((b2-shiftAmount-shiftAmount)>=0) {
							b2=b2-shiftAmount-shiftAmount;
						}
						else
							b2=0;
					}
					//Apply Color change to segment1:
					//Segment& seg1 = strip.getSegment(0);										//Make seg1 designation. Should this just be done before the loop for both segments? Beginning of the loop, if before doesn't work?
					//seg2.setOption(SEG_OPTION_SELECTED, 0);									//Unselect segment because it has to be..? While the other segment not being changed does have to be selected for some reason?
					uint32_t newcolr_2 = RGBW32(r2, g2, b2, 0);
					seg2.setColor(0, newcolr_2);
					
					strip.trigger();  // force strip refresh
					stateChanged = true;  // inform external devices/UI of change
					//colorUpdated(CALL_MODE_DIRECT_CHANGE);	//colorUpdated or stateUpdated? what's the difference?
					stateUpdated(CALL_MODE_DIRECT_CHANGE);
					
					//seg2.setOption(SEG_OPTION_SELECTED, 1);
				}
			}
			else if((Enc_2A) && (Enc_2A_prev)) {		//encoder has stopped being turned
				//if(btn2_turn == true) {		//First detection of stopped turning
				//	click_counter2 = 0;
				//}
				//Need to add a timer/debounce, like 50ms somehow?
				//Trying without:
				btn2_turn=false;
			}
			Enc_2A_prev = Enc_2A;     // Store value of A for next time
		
			loopTime1 = currentTime1; // Updates loopTime (looptime and currenttime should not need to be numbered
		}
	} //END OF LOOP

  void addToConfig(JsonObject& root)
  {
    JsonObject top = root.createNestedObject("rotEncBrightness");
    top["fadeAmount"] = fadeAmount;
    //add shift amount here
    JsonArray pinArray = top.createNestedArray("pin");
    pinArray.add(pins[0]);
    pinArray.add(pins[1]); 
    pinArray.add(pins[2]);
    pinArray.add(pins[3]);
    pinArray.add(pins[4]); 
    pinArray.add(pins[5]); 
  }

  /* 
   * This example uses a more robust method of checking for missing values in the config, and setting back to defaults:
   * - The getJsonValue() function copies the value to the variable only if the key requested is present, returning false with no copy if the value isn't present
   * - configComplete is used to return false if any value is missing, not just if the main object is missing
   * - The defaults are loaded every time readFromConfig() is run, not just once after boot
   * 
   * This ensures that missing values are added to the config, with their default values, in the rare but plauible cases of:
   * - a single value being missing at boot, e.g. if the Usermod was upgraded and a new setting was added
   * - a single value being missing after boot (e.g. if the cfg.json was manually edited and a value was removed)
   * 
   * If configComplete is false, the default values are already set, and by returning false, WLED now knows it needs to save the defaults by calling addToConfig()
   */
  bool readFromConfig(JsonObject& root)
  {
    // set DEFAULTS here, they will be set before setup() is called, and if any values parsed from ArduinoJson below are missing, the default will be used instead
    fadeAmount = 8;
    //add shift amount default here
    pins[0] = 36;
    pins[1] = 2;
    pins[2] = 32;
    //ADD ENCODER 2
    pins[3] = 0;
    pins[4] = 15;
    pins[5] = 12;
		

    JsonObject top = root["rotEncBrightness"];

    bool configComplete = !top.isNull();
    configComplete &= getJsonValue(top["fadeAmount"], fadeAmount);
    //add shiftamount field here
    configComplete &= getJsonValue(top["CLK1"][0], pins[0]);
    configComplete &= getJsonValue(top["DT1"][1], pins[1]);
    configComplete &= getJsonValue(top["SW1"][2], pins[2]);
    configComplete &= getJsonValue(top["CLK2"][3], pins[3]);
    configComplete &= getJsonValue(top["DT2"][4], pins[4]);
    configComplete &= getJsonValue(top["SW2"][5], pins[5]);

    return configComplete;
  }
};
