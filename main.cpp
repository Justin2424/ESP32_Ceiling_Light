



#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Led.h>
#include <Switch.h>
//#include <TempSensorDHT11.h>                                                 
//#include <LedStrip.h>   

// CHANGE: gpio pin, and "basement/hallway" portion of topic, if desired to suit your needs                            
// Format:(gpio pin, "mqqt publish tempature topic", "mqqt publish humidity topic", "mqqt publish heat_index topic" )
//TempSensorDHT11 tempSensorDHT11(9, (char*)"basement/hallway/tempature", (char*)"basement/hallway/humidity", (char*)"basement/hallway/heatindex");

/**********************************************************************************************************************************/
/******* USER CONFIGURED SECTION **************************************************************************************************/


static const char* ssid = "";                                     // change... use your wifi network name
static const char* password = "";                                 // change... use your wifi password
static const char* mqtt_server = "";                              // change... ip address  
static const int mqtt_port = 1883;                                // change... if required
static const char *mqtt_user = "";                                // change... if required
static const char *mqtt_pass = "";                                // change... if required
static const char *mqtt_client_name = "basement/hallway";         // change.. client names must be unique

// MQTT TOPICS  :  The portion of text "basement/hallway/row?" can be edited to your prefered naming needs
// these MQTT Topics we will use for individual led control: 
Led led0(26, 0, (char*)"basement/hallway/row0/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row0/on",           // publish "on" status - to update other devices
                (char*)"basement/hallway/row0/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row0/brightness");  // publish "brightness" status  

Led led1(27, 1, (char*)"basement/hallway/row1/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row1/on",           // publish "on" status
                (char*)"basement/hallway/row1/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row1/brightness");  // publish "brightness" status

Led led2(14, 2, (char*)"basement/hallway/row2/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row2/on",           // publish "on" status
                (char*)"basement/hallway/row2/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row2/brightness");  // publish "brightness" status

Led led3(13, 3, (char*)"basement/hallway/row3/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row3/on",           // publish "on" status
                (char*)"basement/hallway/row3/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row3/brightness");  // publish "brightness" status

Led led4(5, 4, (char*)"basement/hallway/row4/on",                   // subscribe to "on" topic
                (char*)"status/basement/hallway/row4/on",           // publish "on" status
                (char*)"basement/hallway/row4/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row4/brightness");  // publish "brightness" status

Led led5(17, 5, (char*)"basement/hallway/row5/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row5/on",           // publish "on" status
                (char*)"basement/hallway/row5/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row5/brightness");  // publish "brightness" status

Led led6(16, 6, (char*)"basement/hallway/row6/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row6/on",           // publish "on" status
                (char*)"basement/hallway/row6/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row6/brightness");  // publish "brightness" status

Led led7(4, 7, (char*)"basement/hallway/row7/on",                   // subscribe to "on" topic
                (char*)"status/basement/hallway/row7/on",           // publish "on" status
                (char*)"basement/hallway/row7/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row7/brightness");  // publish "brightness" status


/*
Led led8(15, 7, (char*)"basement/hallway/row8/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row8/on",           // publish "on" status
                (char*)"basement/hallway/row8/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row8/brightness");  // publish "brightness" status           
*/

// all leds:  MQTT topics.      // The portion of text "basement/hallway/all" can be edited to your prefered naming needs
// these MQTT Topics we will used to control all leds as a group: 
static const char ALL_ON[] =                         {"basement/hallway/all/on"};                 // subsribe to recieve
static const char STATUS_ALL_ON[] =                  {"status/basement/hallway/all/on"};          // publish to update other apps/clients

static const char ALL_BRIGHTNESS[] =                 {"basement/hallway/all/brightness"};         // subsribe 
static const char STATUS_ALL_BRIGHTNESS[] =          {"status/basement/hallway/all/brightness"};  // publish
// gBridge requires its own mqtt format if your using its cloud server.   
// CHANGE the #'s 1234567 (user_id and device id).. according to what is shown for your device in your gbridge account
static const char gBridge_ALL_ON[] =                 {"gBridge/u123/d4567/onoff"};                // subsribe 
static const char gBridge_STATUS_ALL_ON[] =          {"gBridge/u123/d4567/onoff/set"};            // publish 

static const char gBridge_ALL_BRIGHTNESS[] =         {"gBridge/u123/d4567/brightness"};           // subsribe
static const char gBridge_STATUS_ALL_BRIGHTNESS[] =  {"gBridge/u123/d4567/brightness/set"};       // publish

// topics for scenes or extra functionality
static const char NOTIFY[] =                         {"basement/hallway/notify"};                 // subsribe 
static const char STATUS_NOTIFY[] =                  {"status/basement/hallway/notify"};          // publish 

static const char ALARM[] =                         {"basement/hallway/alarm"};                   // subsribe 
static const char STATUS_ALARM[] =                  {"status/basement/hallway/alarm"};            // publish 

static const char SCENE[] =                         {"basement/hallway/scene"};                   // subsribe 
static const char STATUS_SCENE[] =                  {"status/basement/hallway/scene"};            // publish 

static const char TRACK[] =                         {"basement/hallway/track"};                   // subsribe 
static const char STATUS_TRACK[] =                  {"status/basement/hallway/track"};            // publish 

// MQTT CheckIn topic. - We'll publish to this topic regularly so other devices/we know this mcu is up and running
static const char CHECK_IN[] =                      {"checkin/basement/hallwaymcu"};              // publish  

// RGBW Topic: You can attach a dumb (non addressable) rgbw led strip to led channels led0-led3 (defined above) .. 
// and control it with this topic.. 
static const char RGBW[] =                          {"basement/hallway/rgbw"};                    // subsribe
static const char STATUS_RGBW[] =                   {"status/basement/hallway/rgbw"};             // publish 
// RGBW2 topic: mqtt topic to control dumb led strip if you connected one to led channels led4-led7 (defined above)  
static const char RGBW2[] =                         {"basement/hallway/rgbw2"};                   // subsribe 
static const char STATUS_RGBW2[] =                  {"status/basement/hallway/rgbw2"};            // publish 

// End of MQTT Config //

/**********************************************************************************************************************************/
/******* Declarations *************************************************************************************************************/

/* isBoolean:  The topics ".../all/on" or ".../row?/on" accept boolean messages (0,1, true,false, on,off) to turn the led(s) on or off.. 
if required: you can also send brightness values (0-100) on this topic .. If this is done and brightness values are seen on this topic.. 
"1" will then be used as a brightness level, not a boolean 'on' message... -reboot mcu to reset*/ 
bool isBoolean = true;                                            // keeps compatibily with robs ceiling light
bool built_in_led_state = LOW;                                    // built led shows to indicate wifi state
unsigned long waitCount = 0;                                      // Wifi is trying to connect counter
uint8_t conn_stat = 0;                                            // Connection status for WiFi and MQTT:
                                                                  // status |   WiFi   |    MQTT    |   Led (built in)
                                                                  // -------+----------+------------------------
                                                                  //      0 |   down   |    down    |   blink 
                                                                  //      1 | starting |    down    |   "
                                                                  //      2 |    up    |    down    |   fast blink
                                                                  //      3 |    up    |  starting  |   ""
                                                                  //      4 |    up    | finalising |   ""
                                                                  //      5 |    up    |     up     |   dim

unsigned long last_led_millis = millis();                         // last time the builtin led state was updated
unsigned long lastStatus = 0;                                     // time of last published checkin. code where conn_stat == 5.
unsigned long lastTask = 0;                                       // time of last loop.              

WiFiClient espClient;                                             // wifi library init
PubSubClient mqttClient(espClient);                               // mqtt library init

void action_A(bool switchOn);                                     // declare actions so we can assign them to switches below
void action_B(bool switchOn);
void action_C(bool switchOn);


/**********************************************************************************************************************************/
/******* Swicth Config ************************************************************************************************************/

// CHOOSE YOUR SWITCH ACTION (A,B,C)                              // configure switch(action_x, gpio_pin, pinMode)     
Switch switchOne(action_A, 25, INPUT);                            // change action if required.. pin 25 was used for this switch
Switch switchTwo(action_A, 32, INPUT);                            // change action if required.. pin 32 
Switch switchThree(action_B, 33, INPUT);                          // change action if required.. pin 33 

// ACTION_A : default action that executes when a switch is toggled
void action_A(bool switchOn){
  Serial.println(F("Switch toggled: doing action_A... toggle all Leds."));
  bool somethingIsOn = false;                                      // true if any one of the leds was toggled on
  for(size_t i = 0; i < Led::instanceCount; i++ ){                 // loop for led list
    bool newOnState = Led::instances[i]->toggle();                 // toggle led, and get the new onoff state that is returned
    if (newOnState) somethingIsOn = true;
  }
  if(conn_stat == 5){                                              // if we have full wifi and mqtt connection
    if(somethingIsOn) mqttClient.publish(STATUS_ALL_ON,"true");    // puiblish new state, so other devices/apps can update
    else mqttClient.publish(STATUS_ALL_ON,"false");                // puiblish new state, so other devices/apps can update
  }
  if(somethingIsOn) { Serial.print(STATUS_ALL_ON); Serial.println (" = on"); }  // print 
  else { Serial.print(STATUS_ALL_ON); Serial.println (" = off"); }              // print  
  
}

// make a new action if needed:
void action_B(bool switchOn){
  Serial.println(F("Switch toggled: doing action_B..."));
   // add your code here
   // action_C was provided for reference to help build from
  Serial.println(F(" Switch is using action_B which has not been implimented.."));
  Serial.println(F(" ..please customize action_B to your needs, or use action_A"));
}

/* reference */    // to help make a new action                                  
void action_C(bool switchOn){
  Serial.println(F("Switch toggled: doing action_C..."));
  if(switchOn) {                                                  // if the switch was turned on
    Serial.println(F("Switch is ON..."));
    for(size_t i = 0; i < Led::instanceCount; i++ )               // cycle through all leds, (i = 0 to 7)
      if(!Led::instances[i]->isOn())                              // if led [i] is off
        Led::instances[i]->on();                                  // then turn led on (otherwise it does nothing) 
  }
  else {                                                          // switch was turned off
    Serial.println(F("Switch is OFF..."));
    for(size_t i = 0; i < Led::instanceCount; i++ )               // cycle through all leds, (i = 0 to 7)
      if(Led::instances[i]->isOn())                               // if led [i] is on
        Led::instances[i]->off();                                 // then turn led off (otherwise it does nothing) 
  }
}

/**********************************************************************************************************************************/
/******* We got a MQTT message ****************************************************************************************************/

void callback(char *topic, byte *bMessage, unsigned int length){

  Serial.print("Message arrived [");
  String newTopic = topic;                                          // get the String
  Serial.print(topic);
  Serial.print("] ");
  bMessage[length] = '\0';                                          // add null character to the end (so we can make it a String object)
  
  // For convenience, convert message into different data types now
  const String sMessage = String((char*)bMessage);                  // to String    
  const int intMessage = sMessage.toInt();                          // to integer ... returns zero if it fails
  char cMessage[length+1];
  sMessage.toCharArray(cMessage, length+1);
  Serial.println(sMessage);  
  //Serial.print("length = ");Serial.println(length); 

  // 'All' Leds (group) topics :
  if (newTopic == ALL_ON || newTopic == gBridge_ALL_ON ){     // onoff topic
    // All Leds: on
    if(sMessage == "true" || sMessage == "on" || (sMessage == "1" && isBoolean)){
      for(int i=0; i < Led::instanceCount; i++) {
        Led::instances[i]->on(); 
      }
      mqttClient.publish(STATUS_ALL_ON, "true");                             // show other devices your new status
      mqttClient.publish(gBridge_STATUS_ALL_ON, "true");                     // show other devices your new status
      Serial.print(STATUS_ALL_ON); Serial.println (" = true");
    }
    // All Leds: off 
    else if(sMessage == "false" || sMessage == "0" || sMessage == "off"){  
      for(int i=0; i < Led::instanceCount; i++) {
        Led::instances[i]->off(); 
      }
      mqttClient.publish(STATUS_ALL_ON, "false");                            // show other devices your new status
      mqttClient.publish(gBridge_STATUS_ALL_ON, "false");                    // show other devices your new status
      Serial.print(STATUS_ALL_ON); Serial.println (" = false");
    }
    // All Leds:  toggle
    else if (sMessage == "toggle" ){                                         // nice for testing
      bool somethingIsOn = false;                                            // true if any one of the leds was toggled on
      for(size_t i = 0; i < Led::instanceCount; i++ ){                       // loop for led list
        bool newOnState = Led::instances[i]->toggle();                       // toggle led, and get the new onoff state that is returned
        if (newOnState) somethingIsOn = true;
      }
      if(somethingIsOn) {
        mqttClient.publish(STATUS_ALL_ON,"true");   
        mqttClient.publish(gBridge_STATUS_ALL_ON, "true");                   // puiblish new state, so other devices/apps can update
        Serial.print(STATUS_ALL_ON); Serial.println (" = true");
      }
      else {
        mqttClient.publish(STATUS_ALL_ON,"false"); 
        mqttClient.publish(gBridge_STATUS_ALL_ON, "false");                  // puiblish new state, so other devices/apps can update
        Serial.print(STATUS_ALL_ON); Serial.println (" = false");            // print   
      }
    }
    // All Leds:  brightness and turn on/off
    else if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){    // avoiding intMessage=0, which could be an unidentified message too 
      //// if we are using brightness values in this 'on' topic...
      if(intMessage > 1)  isBoolean = false;  // use '1' as a brightness... disable it's use as boolean 'on/true'
      // set all 'brightness'
      for(int i=0; i < Led::instanceCount; i++) {
          Led::instances[i]->setBrightness(intMessage);
      }
      mqttClient.publish(STATUS_ALL_BRIGHTNESS, cMessage);
      mqttClient.publish(gBridge_STATUS_ALL_BRIGHTNESS, cMessage);  
      Serial.print(STATUS_ALL_BRIGHTNESS); Serial.print (" = "); Serial.println (sMessage);
      //  set all 'on' if needed 
      for(int i=0; i < Led::instanceCount; i++) {                             // loop for every led
        if(!Led::instances[i]->isOn()){                                      
          Led::instances[i]->on();                                            // turn on
        }
      } 
      mqttClient.publish(STATUS_ALL_ON, "true");                            // show other devices your new status
      mqttClient.publish(gBridge_STATUS_ALL_ON, "true");                    // show other devices your new status
      Serial.print(STATUS_ALL_ON); Serial.println (" = true");              // print
      
     
    }
  }
  
  // All Leds: brightness
  else if (newTopic == ALL_BRIGHTNESS || newTopic == gBridge_ALL_BRIGHTNESS ){ // brightness topic
    if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){           // avoiding intMessage=0, which could be an unidentified message too 
        for(int i=0; i < Led::instanceCount; i++) {
          Led::instances[i]->setBrightness(intMessage);
        }
        mqttClient.publish(STATUS_ALL_BRIGHTNESS, cMessage);
        mqttClient.publish(gBridge_STATUS_ALL_BRIGHTNESS, cMessage);  
        Serial.print(STATUS_ALL_BRIGHTNESS); Serial.print (" = "); Serial.println (sMessage);
    }  
  }
  
  //This will recognize rgbw ("0,0,0,255") values for dumb led strips,  and apply them to:
  //RGBW channels = led0, led1, led2, led3, or RGBW2 channels = led4, led5, led6, led7 ... 
  else if (newTopic == RGBW || newTopic == RGBW2){  
    // parse rgbw values..
    byte ind1 = sMessage.indexOf(',');                                          // finds location of first ,
    String red_string = sMessage.substring(0, ind1);                            // captures first data String
    int r = round(red_string.toInt() / 2.55);                                   // convert to int (0-100)

    byte ind2 = sMessage.indexOf(',', ind1+1 );                                 // finds location of second ,
    String green_string = sMessage.substring(ind1+1, ind2+1);                   // captures second data String
    int g = round(green_string.toInt() / 2.55);                                 // convert to int (0-100)

    byte ind3 = sMessage.indexOf(',', ind2+1 );                                 // finds location of third ,
    String blue_string = sMessage.substring(ind2+1, ind3+1);                    // captures third data String
    int b = round(blue_string.toInt() / 2.55);                                  // convert to int (0-100)

    //byte ind4 = sMessage.indexOf(',', ind3+1 );
    String white_string = sMessage.substring(ind3+1);                           // captures fourth data String
    int w = round(white_string.toInt() / 2.55);                                 // convert to int (0-100)

    uint8_t offset = (newTopic == RGBW)  ? 0 : 4;                               // identify if rgbw or rgbw2 topic 
    // set red channel
    Led::instances[0+offset]->setBrightness(r);                                 // set brightness
    if(!Led::instances[0+offset]->isOn())  Led::instances[0+offset]->on();      // turn on if needed
    // set green channel
    Led::instances[1+offset]->setBrightness(g);                                 // set brightness
    if(!Led::instances[1+offset]->isOn())  Led::instances[1+offset]->on();      // turn on if needed
    // set blue channel
    Led::instances[2+offset]->setBrightness(b);                                 // set brightness
    if(!Led::instances[2+offset]->isOn())  Led::instances[2+offset]->on();      // turn on if needed
    // set white channel
    Led::instances[3+offset]->setBrightness(w);                                 // set brightness
    if(!Led::instances[3+offset]->isOn())  Led::instances[3+offset]->on();      // turn on if needed
    // we are using values 0-100 internally, we have to convert back
    // publish new status
    char msgbuffer[20];                          
    sprintf(msgbuffer,"%d,%d,%d,%d", (int)round(r*2.55), (int)round(g*2.55), (int)round(b*2.55), (int)round(w*2.55));  // using sprintf to convert to char[] 
    if(offset==0){                                                              // if rgbw topic
      mqttClient.publish(STATUS_RGBW, msgbuffer);
      Serial.print(STATUS_RGBW); Serial.print(" = ");Serial.println(msgbuffer);
    }
    else{                                                                       // rgbw2 topic
      mqttClient.publish(STATUS_RGBW2, msgbuffer);
      Serial.print(STATUS_RGBW2); Serial.print(" = "); Serial.println(msgbuffer);
    }
  }

  // scenes 
  // notify message
  else if(newTopic == NOTIFY) {
    Led::notify();
    mqttClient.publish(STATUS_NOTIFY, cMessage);
    Serial.print(STATUS_NOTIFY); Serial.print (" = "); Serial.println (sMessage);
  }
  // alarm Message
  else if (newTopic == ALARM) {
    Led::alarm();    
    mqttClient.publish(STATUS_ALARM, cMessage);
    Serial.print(STATUS_ALARM); Serial.print (" = "); Serial.println (sMessage);
  }                     
  // scene Message
  else if (newTopic == SCENE) {
    Led::scene();
    mqttClient.publish(STATUS_SCENE, cMessage);
    Serial.print(STATUS_SCENE); Serial.print (" = "); Serial.println (sMessage);
  }                          
  // tracking Message
  else if (newTopic == TRACK) {
    Led::tracking();    
    mqttClient.publish(STATUS_TRACK, cMessage);
    Serial.print(STATUS_TRACK); Serial.print (" = "); Serial.println (sMessage);
  }                     
  // restart Message            // reboot... not sure if this can ever be handy
  else if (newTopic == "basement/hallway/restart") {
    ESP.restart();  
  }
  //
  
  else{ // Individual Led Topics: 
    for(int i=0; i < Led::instanceCount; i++) {
      if (newTopic == Led::instances[i]->sub_On ){
        // Led: on
        if(sMessage == "true" || sMessage == "on" || (sMessage == "1" && isBoolean)){
          Led::instances[i]->on(); 
          mqttClient.publish(Led::instances[i]->pub_On, "true");                          // show other devices your new status
          Serial.print(Led::instances[i]->pub_On); Serial.println (" = true");
        }
        // Led: off 
        else if(sMessage == "false" || sMessage == "0" || sMessage == "off"){  
          Led::instances[i]->off(); 
          mqttClient.publish(Led::instances[i]->pub_On, "false");                         // show other devices your new status
          Serial.print(Led::instances[i]->pub_On); Serial.println (" = false");
        }
        else if (sMessage == "toggle" ){                                                  // nice for testing
          bool newOnState = Led::instances[i]->toggle();
          if(newOnState){
            mqttClient.publish(Led::instances[i]->pub_On,"true");
            Serial.print(Led::instances[i]->pub_On); Serial.println (" = true");
          } 
          else {
            mqttClient.publish(Led::instances[i]->pub_On,"false");
            Serial.print(Led::instances[i]->pub_On); Serial.println (" = flase");
          }
        }
         // Led: brightness and on
        else if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){  // avoiding intMessage=0, which could be an unidentified message too 
          //// if we are using brightness values in this 'on' topic...
          if(intMessage > 1)  isBoolean = false;  // use '1' as a brightness... disable it's use as a boolean 'on/true'
          // set brightness
          Led::instances[i]->setBrightness(intMessage);
            mqttClient.publish(Led::instances[i]->pub_Brightness, cMessage);
            Serial.print(Led::instances[i]->pub_Brightness); Serial.print (" = "); Serial.println(sMessage);
          // set 'on' if required
          if(!Led::instances[i]->isOn()){                                      // if not on
            Led::instances[i]->on();                                           // turn on
            mqttClient.publish(Led::instances[i]->pub_On, "true");             // show other devices your new status
            Serial.print(Led::instances[i]->pub_On); Serial.println (" = true");// print
          }
       
        }
      }
      // Led: brightness
      else if (newTopic == Led::instances[i]->sub_Brightness ){  
        if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){    // avoiding intMessage=0, which could be an unidentified message too 
            Led::instances[i]->setBrightness(intMessage);
            mqttClient.publish(Led::instances[i]->pub_Brightness, cMessage);
            Serial.print(Led::instances[i]->pub_Brightness); Serial.print (" = "); Serial.println(sMessage);
        }
      }       
    }// end of loop
   
  } // end of individual led section
  
} // end of callback

/********************************************************************************************************************************/
/********* Setup ****************************************************************************************************************/

void setup(void) {
  delay(500);                     // 
  Serial.begin(115200);           // activate the Serial connnection(9600);
   
  Led::setup();                   // sets up EEPROM ,and restores previous Led states before power was interupted

  // Init WiFi
  WiFi.mode(WIFI_STA);                                            //       activate WiFi client / station mode
  
  ArduinoOTA.setHostname(mqtt_client_name);                       //       initialize and start OTA
  //ArduinoOTA.setPassword("password");                           //       set OTA password
  ArduinoOTA.onError([](ota_error_t error) {ESP.restart();});     //       restart in case of an error during OTA
  ArduinoOTA.begin();                                             //       at this point OTA is set up
  delay(5);                                          
  Serial.print("OTA is configured... ");
  Serial.print("Host name = " ); Serial.println( mqtt_client_name);
  
  // Init status Led that is built in to esp32
  pinMode(BUILTIN_LED, OUTPUT);

  // Optional hardware support:
  // DHT11 temp and humidity sensor
  // tempSensorDHT11.setup();

  
}


/********************************************************************************************************************************/
/********* MAIN LOOP ************************************************************************************************************/

void loop() {                                                   
  // start of non-blocking connection setup section
  if ((WiFi.status() != WL_CONNECTED) && (conn_stat != 1)) { conn_stat = 0; }
  if ((WiFi.status() == WL_CONNECTED) && !mqttClient.connected() && (conn_stat != 3))  { conn_stat = 2; }
  if ((WiFi.status() == WL_CONNECTED) && mqttClient.connected() && (conn_stat != 5)) { conn_stat = 4;}
  switch (conn_stat) {
    case 0:                                                       // MQTT and WiFi down: start WiFi
      Serial.println("MQTT and WiFi down: start WiFi");  
      //WiFi.disconnect();  
      WiFi.begin(ssid, password);
      //delay(1);                                                  
      conn_stat = 1;
      break;
    case 1:                                                       // WiFi starting, 
      Serial.println("WiFi starting, wait : " + String(waitCount)); 
      waitCount++;

      if(waitCount < 500){ /*  do nothing  */ }                  // give wifi some time to try and connect..
      //////   Wifi recovery attempts  /////                                                                
      else if(waitCount == 500) conn_stat = 0;                    // after 0.5 seconds,reset and try again
      else if(waitCount == 5000) conn_stat = 0;                   // after 5 seconds,  reset and try again
      else if(waitCount == 120000) conn_stat = 0;                 // after 2 minutess, reset and try again
      else if(waitCount == 3600000) conn_stat = 0;                // after 1 hour,     reset and try again
      else if(waitCount == 18000000) conn_stat = 0;               // after 5 hours,    reset and try again
      else if(waitCount == 43200000) ESP.restart();               // after 12 hours,   hell mary 
      
      break;
    case 2:                                                       // WiFi up, MQTT down: start MQTT
      Serial.println("WiFi up, MQTT down: start MQTT");
      mqttClient.setServer(mqtt_server, mqtt_port);               // config MQTT Server
      mqttClient.setCallback(callback);                           // where to send incoming messages.... callback()
      mqttClient.connect(mqtt_client_name, mqtt_user, mqtt_pass); // connect
      conn_stat = 3;
      waitCount = 0;
      break;
    case 3:                                                       // WiFi up, MQTT starting, do nothing here
      Serial.println("WiFi up, MQTT starting, wait : "+ String(waitCount));
      waitCount++;
      break;
    case 4:                                                       // WiFi up, MQTT up: finish MQTT configuration
      Serial.println("WiFi up, MQTT up: finish MQTT configuration");
      //subscribe to topics
      for(int i=0; i < Led::instanceCount; i++)
      {
        mqttClient.subscribe(Led::instances[i]->sub_On);
        Serial.print("subscribed to: "); Serial.println(Led::instances[i]->sub_On);
        delay(20);
        mqttClient.subscribe(Led::instances[i]->sub_Brightness);
        Serial.print("subscribed to: "); Serial.println(Led::instances[i]->sub_Brightness);
        delay(20);
      }
      mqttClient.subscribe(ALL_ON);                                                   // subsribe // home bridge
      Serial.print("subscribed to: "); Serial.println(ALL_ON);                        // print to Serial
      delay(20);
      mqttClient.subscribe(ALL_BRIGHTNESS);
      Serial.print("subscribed to: "); Serial.println(ALL_BRIGHTNESS); 
      delay(20);
      mqttClient.subscribe(gBridge_ALL_ON);
      Serial.print("subscribed to: "); Serial.println(gBridge_ALL_ON);
      delay(20);
      mqttClient.subscribe(gBridge_ALL_BRIGHTNESS);
      Serial.print("subscribed to: "); Serial.println(gBridge_ALL_BRIGHTNESS);
      delay(20);
      mqttClient.subscribe(RGBW);
      Serial.print("subscribed to: "); Serial.println(RGBW);
      delay(20);
      mqttClient.subscribe(RGBW2);
      Serial.print("subscribed to: "); Serial.println(RGBW2);
      delay(20);
      mqttClient.subscribe(NOTIFY);
      Serial.print("subscribed to: "); Serial.println(NOTIFY);
      delay(20);
      mqttClient.subscribe(ALARM);
      Serial.print("subscribed to: "); Serial.println(ALARM);
      delay(20);
      mqttClient.subscribe(SCENE);
      Serial.print("subscribed to: "); Serial.println(SCENE);
      delay(20);
      mqttClient.subscribe(TRACK);
      Serial.print("subscribed to: "); Serial.println(TRACK);
      delay(20);

      conn_stat = 5;                    
      break;
      // finished MQTT configuration
  }
// end of non-blocking connection setup section



// start section for tasks which should run regardless of WiFi/MQTT
  // show connection status using builtin led on esp32
  if ( millis() - last_led_millis > 505 - (100*conn_stat)){
    last_led_millis = millis();
    built_in_led_state = !built_in_led_state;
    digitalWrite(LED_BUILTIN, built_in_led_state);
  }
  

  Switch::loop();
  Led::loop();
  // LedStrip::loop();
  // ..add any code you want in the loop() here.. (don't use delay)


  
// end of section for tasks which should run regardless of WiFi/MQTT

// start of section with tasks where WiFi/MQTT is required
  if (conn_stat == 5 && !Led::areAnyLedsFading()) {
    if (millis() - lastStatus > 3600000) {                          // Start send status every 1 hour 
      //Serial.println(Status);
      mqttClient.publish(CHECK_IN, "online");                       // send status to broker
      mqttClient.loop();                                            // give control to MQTT to send message to broker
      lastStatus = millis();                                        // remember time of last sent status message
    }
    ArduinoOTA.handle();                                            // internal household function for OTA
    
    //tempSensorDHT11.loop(mqttClient);                               // optional hardware support
    mqttClient.loop();                                              // internal household function for MQTT
    
    // ...add any code you want in the loop that requires wifi here..

  
  } 
// end of section for tasks where WiFi/MQTT are required
}



