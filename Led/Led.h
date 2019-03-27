/*
  Led.h - Library for (non-blocking) pwm led control on esp32.
  Created by Justin Mallais, 2019.
  Released into the public domain.
*/

#include "Arduino.h"

#ifndef Led_h
#define Led_h

constexpr uint8_t MAX_LED_COUNT = 16;     // used to declare an array of Led //
constexpr uint8_t _UPDATE_SPEED = 3 ;     // ms per loop.. lower will speed loop up
constexpr uint16_t FREQ = 8000;           // PWM config: approx 400-15000
constexpr uint16_t RESOLUTION = 8;        // 8 bit resolution, is 0-255 values
constexpr uint16_t MAX_BRIGHTNESS = 100;  // pow(RESOLUTION,2)-1;  
constexpr uint16_t MAX_STEPS = 300;  	    // The max # of steps to fade 

                                                       

class Led{
  public:

    Led(uint8_t pin, uint8_t channel, char *sub_on, char *pub_on, char *sub_brightness, char *pub_brightness);
	  
    const char *sub_On;         
    const char *pub_On;
    const char *sub_Brightness;
    const char *pub_Brightness;
    void on(void);
    void off(void);
    bool toggle(void);
    bool isOn();
    bool isFading(void);
    static bool areAnyLedsFading();
    void setBrightness(int brightness);
    //int getBrightness(void);
    
    static void alarm(void);
    static void notify(void);
    static void scene(void);
    static void tracking(void);

    static void loop(void);			
    static void setup(void);				
    static Led* instances[MAX_LED_COUNT];
    static size_t instanceCount;
    
  private:

    uint8_t _pin;                       // gpio
    uint8_t _channel;                   // pwn channel 0-7
    uint8_t _memoryAddress;             // eeprom memory address location 0-512

    int _brightness;                    // the set brightness level 0-100
    bool _onState;                      // is on ?   (true,false)
    
    //void _setFadeCurve(int startValue, int endValue);   // calculate values that ramp up and an slow down to settle on final brightness
   	//uint16_t _fadeCurve[MAX_STEPS];                     // array to store _setFadeCurve values

    int _getNextFadeValue();  
    void _restoreLedState(int value);
    uint16_t _totalFadeSteps;
    uint32_t _fadeCounter;                                // stepCounter
    int _startFadeValue;
    int _endFadeValue;
	  static uint32_t _alarmCounter;
	  static uint32_t _notifyCounter;
	  static uint32_t _sceneCounter;
	  static uint32_t _trackingCounter;

    static uint32_t _last_update_time;  

};


#endif
