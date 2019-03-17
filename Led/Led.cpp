/*
 * Led.cpp
 *
 *  Created on: Dec 27, 2018
 *      Author: Justin-HTPC
 */


#include "Arduino.h"
#include "Led.h"
#include <EEPROM.h>

#define EEPROM_SIZE 20	
#define START 0		 																							
#define DISABLE 99999999

Led* Led::instances[MAX_LED_COUNT] ={};													// keep a list of the leds
size_t Led::instanceCount = 0;																	// # of leds in list
uint32_t Led::_last_update_time = 0;														// update only every 5ms or so

uint32_t Led::_alarmCounter = DISABLE;													// reset Counter to zero to activate this mode
uint32_t Led::_notifyCounter = DISABLE;													// reset Counter to zero to activate this mode
uint32_t Led::_sceneCounter = DISABLE;													// reset Counter to zero to activate this mode
uint32_t Led::_trackingCounter = DISABLE;												// reset Counter to zero to activate this mode

/* given a value 0-100, returns a value 0-255 */ 
static int scale255(double value){
	// expodential scaling to preserve low value resolution.. and looks more linear when fading (with my leds atleast)
  return pow(value, 1.203271);  
}


void Led::setup(void){
	EEPROM.begin(EEPROM_SIZE);
	delay(100);
	for(int i=0; i < instanceCount; i++){
		delay(5);
		//Serial.print("b4 read b "); Serial.print(instances[i]->_brightness); Serial.print(" b4 read s "); Serial.println(instances[i]->_onState);
		instances[i]->_brightness = EEPROM.read(i);
		delay(5);
    instances[i]->_onState = EEPROM.read(10+i);
		//Serial.print(" aftr read b "); Serial.print(instances[i]->_brightness); Serial.print("  after read s "); Serial.println(instances[i]->_onState);
		if(instances[i]->_onState) ledcWrite(instances[i]->_channel, scale255(instances[i]->_brightness));
		
	}
}

Led::Led(uint8_t pin, uint8_t channel, char* sub_on, char* pub_on, char* sub_brightness, char* pub_brightness){ // constructor 	// uint8_t is short for unsigned byte (0-255)
   
    _pin = pin;												 									// gpio pin
    sub_On = sub_on;																		// mqtt topic to subscribe and listen to, for 'on' messages
		pub_On = pub_on;																		// mqtt topic to publish and update other devices and apps, of our new 'on' state
		sub_Brightness = sub_brightness;										// mqtt topic to subscribe and listen to, for 'brightness' messages
		pub_Brightness = pub_brightness;										// mqtt topic to publish and update other devices and apps, of our new 'brightness' state
    _channel = channel;																	// Esp32 has 8 pwm channels, that can be routed to any of 16 outputs
    _brightness = 100;																	// 0-100 are valid values
    _onState = false;																		// led on / led off
		_fadeCounter = 9999999;															// fade progress / trigger..// reset Counter to zero to activate this mode
		_totalFadeSteps=1;																	// the total fadeCounter ticks required to complete the fade
		_memoryAddress = instanceCount;											// eeprom address for saving ledState and brightness
		ledcSetup(channel, FREQ, RESOLUTION);								// pwmsetup
  	ledcAttachPin(pin, channel);												// pwm setup
		instances[instanceCount++] = this; 									// add this instance to the arraylist
		
}

void Led::on(void){	
	if(!_onState){
		_startFadeValue = 0;																// brightness level to start the fade
		_endFadeValue = _brightness;												// brightness level the fade will stop
		_onState = true;																		// set the Led on
		EEPROM.write(10+_memoryAddress, _onState);  				// save new onState
    EEPROM.commit();
		_fadeCounter = START;																// activate fade
		_alarmCounter = DISABLE;														// make sure all other modes are canceled
		_notifyCounter = DISABLE;
		_sceneCounter = DISABLE;
		_trackingCounter = DISABLE; 
	}
}

void Led::off(void){	
	if(_onState){
		_startFadeValue = _brightness;											// brightness level to start the fade
		_endFadeValue = 0;																	// brightness level the fade will stop
		_onState = false;																		// set the led state to off
		EEPROM.write(10+_memoryAddress, _onState);  				// save new onState
    EEPROM.commit();
	 	_fadeCounter = START;																// activate fade
		_alarmCounter = DISABLE;														// make sure other modes are canceled
		_notifyCounter = DISABLE;
		_sceneCounter = DISABLE;
		_trackingCounter = DISABLE; 
		
	}
}


void Led::setBrightness(int newBrightness){					
	if(newBrightness != _brightness){																	// don't allow zero brightness (or possible negitive values)
		if (_onState){																				// set fade to newbrightness 
			_startFadeValue = _brightness;											// brightness level to start the fade
			_endFadeValue = newBrightness;											// brightness level the fade will stop
			_fadeCounter = START;																// activate fade
		}
		_brightness = newBrightness;													// set new brightness
		EEPROM.write(_memoryAddress, newBrightness);  				// save new brightness
		EEPROM.commit();
	}	
	_alarmCounter = DISABLE;																// cancel all other modes
	_notifyCounter = DISABLE;
	_sceneCounter = DISABLE;
	_trackingCounter = DISABLE; 		
}

bool Led::toggle(void){																		// toggle (off / last used brightness level)
	if(this->_onState)off();
	else on();
	return this->_onState;
}

bool Led::isOn(){
	return _onState;																				
}

void Led::alarm(void){
	_alarmCounter = START;																	//  activate
}
void Led::notify(void){
	_notifyCounter = START;																	//  activate
	
}
void Led::scene(void){
	_sceneCounter = START;																	//  activate
}
void Led::tracking(void){
	_trackingCounter = START;																//  activate
}


/*
 *	populates the partial COSIGNE_WAVE array with values (s-curve)
 */
/*
void Led::_setFadeCurve(int startValue, int endValue){ 		// Lets pre calculate a s curve to smoothen the fade

	int delta = abs(startValue - endValue);
	_steps = pow(100 * delta, 0.5);													// the # steps to transition (more is slower)
  double slice = (PI / _steps) ;													// radians to increment cosine per step
	
  double cosVal = 0;
	double value = 0;
	
  for (int i=0; i< _steps; i++){
		cosVal =  (cos(i*slice)) * -0.5 + 0.5;      						// a value between 1 and -1, then mapped 0-1
		value = (cosVal * delta + 0.5); 												// multiplied by delta and added 0.5 for roundind to int
		if (value!=0)																						// skip ahead to the first change
			_fadeCurve[i] = (startValue < endValue ) ? (startValue+value) : (startValue-value);			// set fadeCurve up, or down
		if (value==delta)	break;																// skip repeated end values
		
  }
	
}
*/


/* Given counter: returns a value between 0-1. The returned value ramps up to 1, then ramps down to 0, and continues			
 * to osillate every multiple of 'toggleDirectionEvery'...                                         			
 */
static float oscillate(long counter, float toggleDirectionEvery=100){
	return cos(counter/toggleDirectionEvery * PI) * -0.5 + 0.5;     // the y value of a cosine wave remapped (from 1 to -1) to 0-1
}

/* This calculates the next brightness level in the fade.. This is activated in the loop when fadeCounter is reset to zero,
 *	and continues to run until fadeCounter ==_totalFadeSteps.
 */
int Led::_getNextFadeValue(){ 		
	int delta = _endFadeValue - _startFadeValue;
		// we could just increment brightness till we reach the new target, but lets use cosine to remap those values
		// to more natural transition (s-curve vs line)
		_totalFadeSteps = 100;//pow(100 * abs(delta), 0.5);									// first, lets also remap the # steps used to transition, instead of using delta
		// get our y position on the s-curve (0-1) 
		if(delta!=0){
			float progress = oscillate(_fadeCounter,_totalFadeSteps) ;		// fade counter will be stopped at _totalFadeSteps, so we do dont oscillate 
			return (int)_startFadeValue + round(progress  * delta);				// return brightness level  
		}	
		else return _brightness;
}

/* The Notify, Alarm, Track and Scene modes (when triggered) may temporily manipulate the leds brightness 
 * and state without overwriting the settings... this function can then be used to return the led to its 
 * settings.. Given the leds current actual brightness.
 */
void Led::_restoreLedState(int fromBrightness){
	if(fromBrightness > 0){     						// the led is on
	  int temp = _brightness;
		_brightness = fromBrightness;
		if(_onState){ 							// resync: fade from value to _brightness
		  setBrightness(temp);
		}
		else{												// resync: fade from value to off
				_onState = true;
				off();
		} 	
	}
	else  {												// the led is off
		if(_onState){ 							// the led should be on...resync: fade from off to brightness
			_onState = false;
			on(); 
		}
		else{												// the led is off, and should be off
				
		} 
	}	
}





/*
 *  put this in the main loop
*/
void Led::loop(void){

	if (millis() - _last_update_time < _UPDATE_SPEED) return;						// only updated every 5ms 
	_last_update_time = millis();

	////////////////  FADE_MODE  ///////////////		 
	for(int i=0; i < instanceCount; i++){
 		if(instances[i]->_fadeCounter <= instances[i]->_totalFadeSteps){	
			ledcWrite(instances[i]->_channel, scale255(instances[i]->_getNextFadeValue()));
			instances[i]->_fadeCounter++;
		}
	}

	
	///////// ALARM_MODE ////////
	if(_alarmCounter <= 2000){ 																				  	// length of effect
		for (int i=0; i< instanceCount; i++){          											// loop for each led
		  	int rollOverCounter = _alarmCounter % 100;   										// is a rolling over value between 0-100
		  	float sinVal =  oscillate(rollOverCounter + (12.5*i), 50); 			// a value between 0 and 1 .. (12.5*i) is an offset for each led
		  	ledcWrite(instances[i]->_channel, sinVal * scale255(100));			// turn led on to brightness value

			  if (_alarmCounter == 2000) instances[i]->_restoreLedState( sinVal*100 );//finished, set all leds to fade to orginal brightness state

		}
		_alarmCounter++;
	}	
	

	/////////  NOTIFY_MODE  ///////// 
	else if(_notifyCounter <= 300){ 																// length of effect
		float effectBrightness = oscillate(_notifyCounter,150)*100;		// fade effect in, fade effect out... (0-1-0) *100 // (0-1 the first 150 ticks, 1-0 the next 150 ticks)
		float osc = oscillate(_notifyCounter,100)*100;								// 
		for (int i=0; i < instanceCount; i++){                        // for each led
			float ledPosition = oscillate(osc + (14.2857*i),100);    		// ((100/7*i) is a different offset for each pin/led
			int value = ledPosition * effectBrightness; 								// turn led to value
			//int value = abs(instances[i]->_brightness - int(ledPosition*effectBrightness) ); // not sure why this doesn't work as intended
			ledcWrite(instances[i]->_channel, scale255(value));
			
			if (_notifyCounter == 300) instances[i]->_restoreLedState(value); // finished, set all leds to fade back to their settings

		}
		_notifyCounter++;
	}
	

	//////// SCENE_MODE ////////
	else if(_sceneCounter == 0){
		for (int i=0; i < instanceCount; i++){                        	// loop each led
			float b = i * 14.2857;    																		// a different brightness 0-100 for each led   //  b=i*100/7 
			ledcWrite(instances[i]->_channel, scale255(b));					
		}
		_sceneCounter = DISABLE;
	}
	

	///////  TRACKING_MODE  ///////
	/*
	else if(mode == TRACKING_MODE){
		if (xPos > UNSET && yPos > UNSET){                                  // insure we have valid data
		  int brightness;
		  for (int i=0; i< instanceCount; i++){                                 // for each pin
			brightness =  255 * 300  / distanceSq(X_LIGHT_POS[i], Y_LIGHT_POS[i], xPos, yPos);  // full brighnesss at 0.5m and 0 at 5m
			instances[i]->fade( constrain(brightness, 0, 255) );
		  }
		}
		_updateCounter++;
		if (_updateCounter >= 240000)instances[i]->_restoreLedState(value);//finished, set all leds to fade to orginal brightness state

	}*/

}
