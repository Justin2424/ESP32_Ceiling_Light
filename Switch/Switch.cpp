/*
 * Switch.cpp
 *
 *  Created on: Dec 27, 2018
 *      Author: Justin-HTPC
 */
#include "Arduino.h"
#include "Switch.h"
#include <EEPROM.h>

#define EEPROM_SIZE 5

////////////////////	SWITCH  CLASS   /////////////////////

size_t Switch::instanceCount = 0;
Switch* Switch::instances[MAX_SWITCH_COUNT] ={}; 

Switch::Switch(functionPtr action, uint8_t switchPin, int mode){
  
  _pin = switchPin;                                     // gpio pin 
  pinMode(_pin, mode);                                  // assign gpio pin #, and set pin as an input (INPUT, INPUT_PULLDOWN, INPUT_PULLUP)
  _callback = action;                                   // action to perform when switch is toggled
  _lastState = digitalRead(_pin) == 0 ? false : true;   // set current state on reboot (does not save state to detect changes on reboot)
  _lastPressMillis = millis();                          // time to debounce 
  instances[instanceCount++] = this;                    // add switch to the list
}

// void Switch::setup(void){   // TODO: if recalling saved state is desired
  
// 	//EEPROM.begin(EEPROM_SIZE);
// 	//delay(100);
// 	for(int i=0; i < instanceCount; i++){
//     /*
// 		delay(5);
// 		Serial.print("b4 read b "); Serial.print(instances[i]->_brightness); Serial.print(" b4 read s "); Serial.println(instances[i]->_onState);
// 		instances[i]->_onSt = EEPROM.read(i);
// 		delay(5);
//     instances[i]->_onState = EEPROM.read(10+i);
// 		Serial.print("read b "); Serial.print(instances[i]->_brightness); Serial.print("  read s "); Serial.println(instances[i]->_onState);
// 		ledcWrite(instances[i]->_channel, percentTo255(instances[i]->_brightness));
// 		*/

//      instances[i]->_lastState = digitalRead(instances[i]->_pin) == 0 ? false : true;
//      instances[i]->_lastPressMillis = millis();
// 	}
// }


void Switch::loop(void){
  for (size_t i = 0; i < instanceCount; i++){

    bool currentState = digitalRead(instances[i]->_pin) == 0 ? false : true;
    if (instances[i]->_lastState != currentState and millis() - instances[i]->_lastPressMillis > DEBOUNCE_TIME){
      //if(currentState == true){   uncomment for push button instead of switch functionality
        if (instances[i]->_callback){
          instances[i]->_callback(currentState);
        }
      //}
      instances[i]->_lastState = currentState;
      instances[i]->_lastPressMillis = millis();
    }
  }
}

void Switch::turnOn(void){
	_lastState = true;
}

void Switch::turnOff(void){
	_lastState = false;
}

void Switch::toggle(void){
	_lastState = !_lastState;
}

bool Switch::isOn(void){
	return _lastState;
}

///////////////////////////////
