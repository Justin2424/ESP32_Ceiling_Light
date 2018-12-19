/*
  Led.h - Library for (non-blocking) pwm led control on esp32.
  Created by Justin Mallais, 2019.
  Released into the public domain.
*/

#include "Arduino.h"

#ifndef Led_h
#define Led_h




class Led 
{
  public:
  
    Led(int pin, char* pin_Name, int channel);
	char* pinName;
    //void on();
    void on(int brightness);
    void off();
    void fade(int brightness);
    void addFade(int brightness);
    void toggle();
    bool isOn();
    bool isFading();
    void update();						// called by a timer to process fades (approx 5ms recommended)

  private:
    static const int FREQ = 5000;       // PWM config: approx 400-15000   
    static const int RESOLUTION = 8;													
    int _pin = 13;
    int _channel;
    int _brightness = 0;
    int _targetBrightness = 0;
    int _previousBrightness = 0;
	  void _setSCurve(int height);
	  int _updateCounter = 0;
    static const int _STEPS = 300;  	//The # of steps to fade to _targetBrightness (small fades will be slow and large fades are quicker)
    double _sCurve[_STEPS] = {0};				// precalculated values to ramp up and an slow down to the _targetBrightness level
    
    
     

};

#endif