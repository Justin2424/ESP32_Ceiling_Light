# ESP32_Led_Control_Board

An ESP32 based Led control board, for 8 individually dimmable led channels (6-24v).. This project is compatible and extends the functionality of the ESP32_Ceiling_light project from the hookup. 

https://www.youtube.com/watch?v=PVQhGzo-Dtg

This control board is intended to control the lighting of an entire room, using a powersupply and matching leds(6-24v), standard wall switches wired directly to the board (!NO HIGH VOLTAGE!), and supporting MQTT software (like Home Assistant, homebridge, gbridge ect) 
    
Features:
  - 8 dimmable led channels (6-24v), (plus 1 additional non-dimmable channel for relay)
  - ability to use led channels (0-3) or (4-7) for rgbw led strip control. (or just rgb)
  - smoothly Fades between on / off states and different brightness settings
  - Switches:  easily add and configure physical switches (board has 3 dedicated connections)
  - Non blocking wifi connection.. (the Switches will always work) and improved wifi connection recovery
  - Restores led state on reboot from power failure... without the need of wifi support
  - extended MQTT support: 
              - compatible with Ceiling_Light format (ON,OFF, int(0-100) commands in single topic)(note:currently 0-100, not 255)
              - added topic to control "All" lights simutaniously
              - added seperate brightness topic for compatiblity with other apps (eg homebridge)
                ( and allows changing brightness level when light is off)
              - added mqtt topics for gbridge cloud support (for google assistant support)
              -added rgbw and rgbw2 topic for non-addressable led strips
  - Supports alarm, notification, custom scene / animations
  - gerber files for the pcb design (optional)
  - pcb supports 3 switches, 8 + 1 led channels, power distrubution, headers for esp32 / easy access to all pins on esp32
 
  Note : I have not tested the power handling limit of the board / components.. do so at own risk. 
  
  please let me know if you find any bugs, have suggestions, and or questions. Contributions to the code are more than welcomed..
  I'll be adding sensors and other features to this board.
  
    - temp and humidity sensor support. 
    - Addressable led control via i2c
    - relay switching for wall plugs 
     
 eventually:
    - Motion detection/tracking (ir array with logic/analsys) and the associated animations/light tracking
 
    
  
  
  justinmallais@gmail.com
