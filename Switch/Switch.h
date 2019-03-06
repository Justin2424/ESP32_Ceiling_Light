/*
 * Switch.h
 *
 *  Created on: Dec 27, 2018
 *      Author: Justin-HTPC
 */
#include <Arduino.h>

#ifndef Switch_h
#define Switch_h


using functionPtr = void(*)(bool isOn);					// used to pass in a callback function
constexpr uint32_t DEBOUNCE_TIME = 400;         // time to filter bounces
constexpr uint8_t MAX_SWITCH_COUNT = 5;         // allow this many switches in the list 

class Switch{
  public:
    // constructor : Switch(your callbackfuntion, gpio pin #, and the pin mode: INPUT or INPUT_PULLDOWN or INPUT_PULLUP)
    Switch(functionPtr action, uint8_t switchPin, int mode);    
     
    void turnOn(void);
    void turnOff(void);
    void toggle(void);
    bool isOn(void);
    
    //static void setup(void);                       
    static void loop(void);                        //  add to main loop.  It polls the binded pin for state change..
    static Switch* instances[MAX_SWITCH_COUNT];
    static size_t instanceCount;
  private:
    uint8_t _pin;
    uint32_t _lastPressMillis;
    bool _lastState;
    functionPtr _callback;
   
};
#endif 
