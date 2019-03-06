# ESP32_Led_Control_Board

An ESP32 based Led control board, for 8 individually dimmable led channels (6-24v).. This project is compatible and extends the functionality of the ESP32_Ceiling_light project from the hookup. 

https://www.youtube.com/watch?v=PVQhGzo-Dtg

This control board is intended to control the lighting of an entire room, using a powersupply, matching leds(6-24v), standard wall switches wired directly to the board (!NO HIGH VOLTAGE!), and supporting MQTT software (like Home Assistant, homebridge, gbridge ect) 
    
Features:
  - 8 dimmable led channels (6-24v), plus 1 additional non-dimmable channel.
  - Fades between on / off states and different brightness settings
  - Switches:  easily add and configure physical switches for individual/group channel control 
  - Non blocking wifi connection.. (the Switches will always work) and improved wifi connection recovery
  - Saves and restores led state on reboot from power failure... without the need of wifi support
  - extended MQTT support: 
              - compatible with Ceiling_Light format (ON,OFF, int(0-100) commands in single topic)
              - added topic to control "All" lights simutaniously
              - added seperate brightness topic for compatiblity with other apps (eg homebridge)
                ( and allows changing brightness level when light is off)
              - added mqtt topics for gbridge cloud support (for google assistant support)
  - Supports alarm, notification, custom scene or animation
  - gerber files for the pcb design (optional)
  - pcb supports 3 switches, 8 + 1 led channels, power distrubution, headers for esp32 / easy access to all pins on esp32
 
  Note : I have not tested the power handling limit of the board / components.. do so at own risk.
  
  please let me know if you find any bugs, or have suggestions. Contributions to the code would be more than welcomed..
 Its a project in progress.. I'm interested in adding as many sensors and features as practical into this, or supporting boards.
  including:
    - Addressable led control via i2c
    - full connection manager (ap for phone browser config / connection).
    - Various sensors like temp and humidity sensors/support.
    - Presence detection (who's phone is online)
    and hopefully:
    - Motion detection/tracking (ir array with logic/analsys).
    and perhaps 
      -a simple mesh network/peer to peer/relay mqtt messages, for clients who are out of the wifi range/ disconnected from the broker 
     
  
  
  justinmallais@gmail.com
