# ESP32_Ceiling_Light

ESP32 based dimmable ceiling light

https://www.youtube.com/watch?v=PVQhGzo-Dtg

I plan to take this a couple steps further..  This Fork is for a hallway light setup.. 9 rows of lights down a hallway..
After basic assembly and functionality is completed, I plan to integrate an OpenMV Cam M7 to track movement and control the lighting.  

  -Added soft trasition to brightness levels
  -Added Topic to address "All" lights, with single payload
  -Started a notifation animation, and alarm animation 
  -payload opions: ON,OFF, int(0-255), ALARM, NOTIFY
  -Alarm/Notify Animations depend on PIN_NUMBERS and associated PIN_NAMES to be in a clock or counter clockwise sequence.
  -Alarm Animation assumes ambience pin is last
  
-gerber files to order a pcb were added.
-I have recieved my pcb 2018-12-03, and will assemble for testing shortly 
-TODO reminder: add manual switch controls
