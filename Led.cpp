/*
  Library for (non-blocking) pwm led control on esp32.
  Created by Justin Mallais, 2019.
  Released into the public domain.
*/

#include "Arduino.h"
#include "Led.h"

 



Led::Led(int pin, char* pin_Name, int channel){      //constructor 	// uint8_t is short for unsigned byte (0-255)
    _pin = pin;													//
    pinName = pin_Name;									// intended for mqtt topic names used to id the led.. eg: "basement/hallway/led1"
    _channel = channel;							//1-9			// Esp32 has 9 pwm channels 1-9 (make sure you +1 when constructing with an iteration loop)
    ledcSetup(channel, FREQ, RESOLUTION);
    ledcAttachPin(pin, channel);
	// construct the _Quarter_COSIGNE_WAVE, used to fade to _targetBrightness
	
}
/*
void Led::on(){
    _previousBrightness = _targetBrightness;	// save 
    _targetBrightness = 255;                	// set new target
	_brightness = 255;                      	// dont wait to transition, set the current brightness
	ledcWrite(_channel, 255);               	// Turn led on
}
*/
void Led::on(int brightness){					// set led to given brightness
    _previousBrightness = _targetBrightness;	// save
    _targetBrightness = brightness;				// set new target
    _brightness = brightness;
    ledcWrite(_channel, brightness);
}

void Led::off(){								
    _previousBrightness = _targetBrightness;	// save
    _targetBrightness = 0;                		// set new target
	_brightness = 0;                      		// dont wait to transition, set the current brightness
	ledcWrite(_channel, 0);               		// Turn led off
}

void Led::toggle(){								// toggle (off / last used brightness)
	if(_targetBrightness > 0){					// when led is ON
		_previousBrightness = _targetBrightness;// save
		_targetBrightness = 0;					// fade to off
	}
	else{										// led was off
		_targetBrightness = _previousBrightness;// fade to previous 
		_previousBrightness = 0;				// save previous
    }			
}

void Led::fade(int brightness){					//fade to the given brightness
   _previousBrightness = _targetBrightness;	    //save
   _targetBrightness = brightness;				//set new target
}

void Led::addFade(int brightness){				// adds a given brightness to the targetBrightness (use a negitive value to subtract)
	_previousBrightness = _targetBrightness;	//save
    _targetBrightness = constrain(brightness + _targetBrightness, 0, 255); // set and insure brightness is  0-255
}

bool Led::isOn(){
    if (_brightness > 0){return true;}
    return false;
}

bool Led::isFading(){
    if (_brightness != _targetBrightness){ return true;}
    return false;
}


/* 
 *   Called by a timer every 5ms (TRANSITION_SPEED)
*/
void Led::update(){
	// check progress
	if(_brightness ==_targetBrightness ){   //target reached
		if(_updateCounter !=0){
			Serial.println("Step_counter should be 300"+_updateCounter);
			_updateCounter = 0;					// reset
		}
		return;								// were done
	}
	else{									
		if(_updateCounter == 0 ){							// this is a new _targetBrightness
			_setSCurve(abs(_targetBrightness - _brightness));	// set a new SCurve
		}
	}
	// Turn _brightness up
	if(_targetBrightness - _brightness > 0){
		_brightness += _sCurve[_updateCounter];
	}
	// Turn _brightness down
	else{
		_brightness -= _sCurve[_updateCounter];
	}

    ledcWrite(_channel, _brightness);
    _updateCounter++;

}

/* 
 *	populates the _Quarter_COSIGNE_WAVE array with values (s-curve)
 */
void Led::_setSCurve(int height){ 	// Lets pre calculate a quarter cosign wave to smoothen out a linear fade even further
    double slice = (PI /2 / _STEPS) ;
    double cosVal = 0;
	int hieght = 0;
    for (int i=0; i< _STEPS; i++){             
		cosVal =  (cos(i*slice));      							// a value between 1 and -1 
        hieght = int(((cosVal*0.5 + 0.5 ) * hieght)+0.5);       //mapped 0-1 //then multiplied by height and rounded  
		_sCurve[i] = max(1, hieght); 							// min value of 1
	}
}








/*
void Led::update(){
      // BRIGHTNESS UP
	  int theSign = sign(pinsTargetBrightness[i] < pinsBrightness[i]);
	  int distance = (pinsTargetBrightness[i]-pinsBrightness[i])*sign;
      if (distance > 20){ 
          pinsBrightness[i] += (int) distance*0.05*theSign;   // adds 1 to 12 
       }
       ledcWrite(_channel, _pinsBrightness); 
	  
	  
      if(pinsBrightness[i] < pinsTargetBrightness[i])
      { 
        pinsBrightness[i]++;
        // go faster when the target Brighness is far away
        int distance = pinsTargetBrightness[i]-pinsBrightness[i];
        if (distance > 20) 
        { 
          pinsBrightness[i] += (int) distance*0.05;   // adds 1 to 12 
        }
        ledcWrite(i, pinsBrightness[i]); 
      } 

      // BRIGHTNESS DOWN
      else if(pinsBrightness[i] > pinsTargetBrightness[i])
      {
        pinsBrightness[i]--;
        // go faster when the target Brighness is far away
        int distance = pinsBrightness[i]-pinsTargetBrightness[i];
        if (distance > 20)
        {
          pinsBrightness[i] -= (int) distance*0.05;
        }
        ledcWrite(i, pinsBrightness[i]);  
      }
    
}
*/
