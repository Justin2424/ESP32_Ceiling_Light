# ESP32_Led_Control_Board

An ESP32 based Led control board, for 8 individually dimmable led channels (6-24v).. This project is compatible and extends the functionality of the ESP32_Ceiling_light project from the hookup. 

https://www.youtube.com/watch?v=PVQhGzo-Dtg

This control board is intended to control the lighting of an entire room, using a powersupply, matching leds(6-24v), standard wall switches wired directly to the board (!NO HIGH VOLTAGE!), and supporting MQTT software (like Home Assistant, homebridge, gbridge ect) 
    
Features:
  - 8 dimmable led channels (6-24v), plus 1 additional non-dimmable channel.
  - Fades between on / off states and different brightness settings
  - Switches:  easily add and configure physical switches for individual/group channel control 
  - Non blocking wifi connection.. (Switches fuctions without internet) and improved wifi connection recovery
  - Saves and restores Led state on reboot from power failure... without the need of wifi support
  - extended MQTT support: 
              - compatible with Ceiling_Light format (ON,OFF, int(0-100) commands in single topic)
              - added topic to control "All" lights simutaniously
              - added seperate brightness topic for compatiblity with other apps (eg homebridge)
                ( and allows changing brightness level when light is off)
              - added mqtt topics for gbridge cloud support (for google assistant support)
  - Supports alarms, notifications and scenes
  - gerber files for the pcb design (but not required)
  - pcb supports 3 switches, 8 + 1 led channels, power distrubution, headers for esp32 / easy access to all pins on esp32
 
  - Note : I have not tested the power handling limit of the board / components.. do so at own risk  
