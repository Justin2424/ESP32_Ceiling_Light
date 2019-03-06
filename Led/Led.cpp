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

Led* Led::instances[MAX_LED_COUNT] ={};													// keep a list of the leds
size_t Led::instanceCount = 0;																	// # of leds in list
uint32_t Led::_last_update_time = 0;														// update only every 5ms or so


uint32_t Led::_alarmCounter = 9999999;													// reset Counter to zero to activate this mode
uint32_t Led::_notifyCounter = 9999999;													// reset Counter to zero to activate this mode
uint32_t Led::_blinkCounter = 9999999;													// reset Counter to zero to activate this mode
uint32_t Led::_trackingCounter = 9999999;												// reset Counter to zero to activate this mode

/* given a value 0-100, returns a value 0-255 */ 
static int percentTo255(double value){
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
		if(instances[i]->_onState) ledcWrite(instances[i]->_channel, percentTo255(instances[i]->_brightness));
		
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
		_fadeCounter = 0;																		// activate fade
		/*
		_alarmCounter = 9999999;														// make sure all other modes are canceled
		_notifyCounter = 9999999;
		_blinkCounter = 9999999;
		_trackingCounter = 9999999; 
		*/
	}
}

void Led::off(void){	
	if(_onState){
		_startFadeValue = _brightness;											// brightness level to start the fade
		_endFadeValue = 0;																	// brightness level the fade will stop
		_onState = false;																		// set the led state to off
		EEPROM.write(10+_memoryAddress, _onState);  				// save new onState
    EEPROM.commit();
	 	_fadeCounter = 0;																		// activate fade
		 /*
		_alarmCounter = 9999999;														// make sure other modes are canceled
		_notifyCounter = 9999999;
		_blinkCounter = 9999999;
		_trackingCounter = 9999999; 
		*/
	}
}


void Led::setBrightness(int newBrightness){					
	if(newBrightness > 0){																	// don't allow zero brightness (or possible negitive values)
		if (_onState){																				// set fade to newbrightness 
			_startFadeValue = _brightness;											// brightness level to start the fade
			_endFadeValue = newBrightness;											// brightness level the fade will stop
				_fadeCounter = 0;																	// activate fade
			/*_alarmCounter = 9999999;													// cancel all other modes
			_notifyCounter = 9999999;
			_blinkCounter = 9999999;
			_trackingCounter = 9999999; 
			*/
		}
		_brightness = newBrightness;													// set new brightness
		EEPROM.write(_memoryAddress, newBrightness);  				// save new brightness
		EEPROM.commit();
	}
	else off();						// if zero brightness is requested, keep the current brightness setting and fade to off	state		
}

bool Led::toggle(void){																	// toggle (off / last used brightness level)
	if(_onState)off();
	else on();
	return _onState;
}


bool Led::isOn(){
	return _onState;																			// return if led is on
}

void Led::alarm(void){
	_alarmCounter = 0;																		//  activate
}
void Led::notify(void){
	_notifyCounter = 0;																		//  activate
	
}
void Led::blink(void){
	_blinkCounter = 0;																		//  activate
}
void Led::tracking(void){
	_trackingCounter = 0;																	//  activate
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

/*
 *	
 */
int Led::_getNextFadeValue(){ 		
	// (could remove declarations from the loop for speed)
	int delta = _endFadeValue - _startFadeValue;
	_totalFadeSteps = pow(100 * abs(delta), 0.5);															// the # steps used to transition 
  double position = cos(PI * _fadeCounter / _totalFadeSteps) ;					    // a position on the half cosine value, somewhere between 1 and -1 
	int value = round((position  * -0.5 + 0.5) * delta);      								//  mapped to 0-1, then multiplied by delta and rounded
	
	return (int)_startFadeValue + value;
		
}

void Led::_restoreLedState(int value){
	Serial.println(" Test 1");
	if(value > 0){     						// the led is on
	Serial.println(" Test 2");
		int temp = _brightness;
		_brightness = value;
		if(_onState){ 							// resync: fade from value to _brightness
		Serial.println(" Test 3");
			setBrightness(temp);
		}
		else{												// resync: fade from value to off
				Serial.println(" Test 4");
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

	if (millis() - _last_update_time < _UPDATE_SPEED) return;						// only updated every 5ms or so
	_last_update_time = millis();

	////////////////  FADE_MODE  ///////////////		 
	for(int i=0; i < instanceCount; i++){

		if(instances[i]->_fadeCounter <= instances[i]->_totalFadeSteps){	
			ledcWrite(instances[i]->_channel, percentTo255(instances[i]->_getNextFadeValue()));
			instances[i]->_fadeCounter++;
		}
	}

	
	///////// ALARM_MODE ////////
	if(_alarmCounter <= 20000){ 			
		for (int i=0; i< instanceCount; i++){          											// for each pin
		  	int rollOverCounter = int(_alarmCounter + (12.5*i)) % 100;      // is a rolling over value between 0-100 // (12.5*i) is an offset for each pin/led
		  	float sinVal =  (sin(rollOverCounter * 0.062831));  //(2PI/100) // a value between -1 and 1 // as _alarmCounter increments: SinVal starts at 0, ramps up to 1, and falls back to zero.. (for the first pin)// half sinewave, with no negitive values.
		  	int value = (sinVal * 0.5 + 0.5 ) * 255;												// mapped to 0-1, then multiplied
				ledcWrite(instances[i]->_channel, value);											  // turn led to value

			  if (_alarmCounter == 20000) instances[i]->_restoreLedState(value);//finished, set all leds to fade to orginal brightness state

		}
		_alarmCounter++;
	}	
	

	/////////  NOTIFY_MODE  ///////// 
	else if(_notifyCounter <= 200){ 		//    a subtle variation of Alarm Mode
		
		for (int i=0; i < instanceCount; i++){                        	// for each led
			int rollOverCounter = int(_notifyCounter + (12.5*i)) % 100;   // is a rolling over value between 0-100 // (12.5*i) is a different offset for each pin/led
			float sinVal =  (sin(rollOverCounter*0.062831));        			//(2PI/100)    // a value between -1 and 1
			int value = (sinVal * 0.5 + 0.5 ) * 120;											// mapped to 0-1, then multiplied
			ledcWrite(instances[i]->_channel, value);											// turn led to value
			
			if (_notifyCounter == 200) instances[i]->_restoreLedState(value);//finished, set all leds to fade to orginal brightness state

		}
		_notifyCounter++;
	}
	

	//////// BLINK_MODE ////////
	else if(_blinkCounter <= 400){
		int value = 255;
		for (int i=0; i< instanceCount; i++){          			// for each pin
			if (_blinkCounter < 100) value = 255;
			else if (_blinkCounter < 200) value = 0;
			else if (_blinkCounter < 300) value = 255;
			else value = 0;
			ledcWrite(instances[i]->_channel, value);
			
			if (_blinkCounter == 400) instances[i]->_restoreLedState(value);//finished, set all leds to fade to orginal brightness state

		}
		_blinkCounter++;
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











