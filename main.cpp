



#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Led.h>
#include <Switch.h>


/**********************************************************************************************************************************/
/******* USER CONFIGURED SECTION **************************************************************************************************/

static const char* ssid = "aWiFiNetwork";                         // change... use your wifi network name
static const char* password = "classified";                       // change... use your wifi password
static const char* mqtt_server = "192.168.0.25";                  // change... ip address  
static const int mqtt_port = 1883;                                // change... if required
static const char *mqtt_user = "";                                // change... if required
static const char *mqtt_pass = "";                                // change... if required
static const char *mqtt_client_name = "basement/hallway";      // change.. client names must be unique

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
Led led8(15, 0, (char*)"basement/hallway/row8/on",                  // subscribe to "on" topic
                (char*)"status/basement/hallway/row8/on",           // publish "on" status
                (char*)"basement/hallway/row8/brightness",          // subsribe to "brightness" topic
                (char*)"status/basement/hallway/row8/brightness");  // publish "brightness" status           
*/
// all leds group topics.      // The portion of text "basement/hallway/all" can be edited to your prefered naming needs
static const char ALL_ON[] =                         {"basement/hallway/all/on"};                 // subsribe // home bridge
static const char STATUS_ALL_ON[] =                  {"status/basement/hallway/all/on"};          // publish // homebridge

static const char ALL_BRIGHTNESS[] =                 {"basement/hallway/all/brightness"};         // subsribe // new
static const char STATUS_ALL_BRIGHTNESS[] =          {"status/basement/hallway/all/brightness"};  // publish // new
// gBridge requires its own mqtt format if your using its cloud server.   
// CHANGE the #'s 1234567 (user_id and device id).. according to what is shown for your device in your gbridge account
static const char gBridge_ALL_ON[] =                 {"gBridge/u814/d2241/onoff"};                // subsribe 
static const char gBridge_STATUS_ALL_ON[] =          {"gBridge/u814/d2241/onoff/set"};            // publish 

static const char gBridge_ALL_BRIGHTNESS[] =         {"gBridge/u814/d2241/brightness"};           // subsribe
static const char gBridge_STATUS_ALL_BRIGHTNESS[] =  {"gBridge/u814/d2241/brightness/set"};       // publish
// RGBW Added for reference: currently uses "w" value as brightness value.. and ignores rgb 
static const char ALL_RGBW[] =                       {"basement/hallway/all/rgbw"};               // subsribe // home bridge
static const char STATUS_ALL_RGBW[] =                {"status/basement/hallway/all/rgbw"};        // publish // homebridge

// topics used by scenes / extra functionality
static const char NOTIFY[] =                         {"basement/hallway/notify"};                 // subsribe 
static const char STATUS_NOTIFY[] =                  {"status/basement/hallway/notify"};          // publish 

static const char ALARM[] =                         {"basement/hallway/alarm"};                   // subsribe 
static const char STATUS_ALARM[] =                  {"status/basement/hallway/alarm"};            // publish 

static const char BLINK[] =                         {"basement/hallway/blink"};                   // subsribe 
static const char STATUS_BLINK[] =                  {"status/basement/hallway/blink"};            // publish 

static const char TRACK[] =                         {"basement/hallway/track"};                   // subsribe 
static const char STATUS_TRACK[] =                  {"status/basement/hallway/track"};            // publish 

// MQTT CheckIn topic. - We'll publish to this topic regularly so other devices/we know this mcu is up and running
static const char CHECK_IN[] =                      {"checkin/basement/hallwaymcu"};              // publish  
// End of MQTT Config //

/**********************************************************************************************************************************/
/******* Declarations *************************************************************************************************************/

/* isBoolean: By default the topics ".../all/on" or ".../row?/on" accept boolean messages (0,1, true,false, on,off)  to turn the led(s) on or off.. 
if required: you can send brightness values (0-100) on this topic also.. If this is done and brightness values are seen on this topic.. 
"1" will then be used as a brightness level, not a boolean 'on' message... -untill mcu is reboot*/ 
bool isBoolean = true;                                            // keps compatibily with robs ceiling light

unsigned long waitCount = 0;                                      // Wifi is trying to connect counter
uint8_t conn_stat = 0;                                            // Connection status for WiFi and MQTT:
                                                                  // status |   WiFi   |    MQTT
                                                                  // -------+----------+------------
                                                                  //      0 |   down   |    down
                                                                  //      1 | starting |    down
                                                                  //      2 |    up    |    down
                                                                  //      3 |    up    |  starting
                                                                  //      4 |    up    | finalising
                                                                  //      5 |    up    |     up

unsigned long lastStatus = 0;                                     // time of last published checkin. code where conn_stat == 5.
unsigned long lastTask = 0;                                       // time of last loop.              code where conn_stat <> 5

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void action_A(bool switchOn);                                     // declare "actions" so we can assign them to switches below
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
  Serial.println(F("Switch toggled: doing action_A... Toggle all Leds."));
  bool somethingIsOn = false;                                            // true if any one of the leds was toggled on
  for(size_t i = 0; i < Led::instanceCount; i++ ){                       // loop for led list
    bool newOnState = Led::instances[i]->toggle();                       // toggle led, and get the new onoff state that is returned
    if (newOnState) somethingIsOn = true;
  }
  if(conn_stat == 5){                                                    // if we have full wifi and mqtt connection
    if(somethingIsOn) mqttClient.publish(STATUS_ALL_ON,"true");          // puiblish new state, so other devices/apps can update
    else mqttClient.publish(STATUS_ALL_ON,"false");                      // puiblish new state, so other devices/apps can update
  }
  if(somethingIsOn) { Serial.print(STATUS_ALL_ON); Serial.println (" = toggled = on"); } // print 
  else { Serial.print(STATUS_ALL_ON); Serial.println (" = toggled = off"); }          // print  
  
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
  if(switchOn) {                                    // if the switch was turned on
    Serial.println(F("Switch is ON..."));
    for(size_t i = 0; i < Led::instanceCount; i++ ) // cycle through all leds, (i = 0 to 7)
      if(!Led::instances[i]->isOn())                // if led [i] is off
        Led::instances[i]->on();                    // then turn led on (otherwise it does nothing) 
  }
  else {                                            // switch was turned off
    Serial.println(F("Switch is OFF..."));
    for(size_t i = 0; i < Led::instanceCount; i++ ) // cycle through all leds, (i = 0 to 7)
      if(Led::instances[i]->isOn())                 // if led [i] is on
        Led::instances[i]->off();                   // then turn led off (otherwise it does nothing) 
  }
}


/**********************************************************************************************************************************/
/******* We got a MQTT message ****************************************************************************************************/

void callback(char *topic, byte *bMessage, unsigned int length){

  Serial.print("Message arrived [");
  String newTopic = topic;                            // get the String
  Serial.print(topic);
  Serial.print("] ");
  bMessage[length] = '\0';                            // add null character to the end (so we can make it a String object)
  // For convenience, convert message into different data types now
  const String sMessage = String((char*)bMessage);    // to String    
  const int intMessage = sMessage.toInt();            // to integer ... returns zero if it fails
  char cMessage[length+1];
  sMessage.toCharArray(cMessage, length+1);
  Serial.println(sMessage);  
  
  // for 'All' Leds in a group topics :
  // onoff topic
  if (newTopic == ALL_ON || newTopic == gBridge_ALL_ON ){
    // on
    if(sMessage == "true" || sMessage == "on" || (sMessage == "1" && isBoolean)){
      for(int i=0; i < Led::instanceCount; i++) {
        Led::instances[i]->on(); 
      }
      mqttClient.publish(STATUS_ALL_ON, "true");                                         // show other devices your new status
      mqttClient.publish(gBridge_STATUS_ALL_ON, "true");                              // show other devices your new status
      Serial.print(STATUS_ALL_ON); Serial.println (" = true");
    }
    // off 
    else if(sMessage == "false" || sMessage == "0" || sMessage == "off"){  
      for(int i=0; i < Led::instanceCount; i++) {
        Led::instances[i]->off(); 
      }
      mqttClient.publish(STATUS_ALL_ON, "false");                                     // show other devices your new status
      mqttClient.publish(gBridge_STATUS_ALL_ON, "false");                             // show other devices your new status
      Serial.print(STATUS_ALL_ON); Serial.println (" = false");
    }
    else if (sMessage == "toggle" ){                                                  // nice for testing
      bool somethingIsOn = false;                                            // true if any one of the leds was toggled on
      for(size_t i = 0; i < Led::instanceCount; i++ ){                       // loop for led list
        bool newOnState = Led::instances[i]->toggle();                       // toggle led, and get the new onoff state that is returned
        if (newOnState) somethingIsOn = true;
      }
      if(somethingIsOn) {
        mqttClient.publish(STATUS_ALL_ON,"true");   
        mqttClient.publish(gBridge_STATUS_ALL_ON, "true");                             // puiblish new state, so other devices/apps can update
        Serial.print(STATUS_ALL_ON); Serial.println (" = true");
      }
      else {
        mqttClient.publish(STATUS_ALL_ON,"false"); 
        mqttClient.publish(gBridge_STATUS_ALL_ON, "false");                                     // puiblish new state, so other devices/apps can update
        Serial.print(STATUS_ALL_ON); Serial.println (" = false");           // print   
      }
    }
    else if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){    // avoiding intMessage=0, which could be an unidentified message too 
      for(int i=0; i < Led::instanceCount; i++) {   
            Led::instances[i]->setBrightness(intMessage);
            mqttClient.publish(Led::instances[i]->pub_Brightness, cMessage);
            Serial.print(Led::instances[i]->pub_Brightness); Serial.print (" = "); Serial.println(sMessage);
      }
    }
  }
  // All Leds:
  // brightness
  else if (newTopic == ALL_BRIGHTNESS || newTopic == gBridge_ALL_BRIGHTNESS ){  
    if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){    // avoiding intMessage=0, which could be an unidentified message too 
        for(int i=0; i < Led::instanceCount; i++) {
          Led::instances[i]->setBrightness(intMessage);
        }
        mqttClient.publish(STATUS_ALL_BRIGHTNESS, cMessage);
        mqttClient.publish(gBridge_STATUS_ALL_BRIGHTNESS, cMessage);  
        Serial.print(STATUS_ALL_BRIGHTNESS); Serial.print (" = "); Serial.println (sMessage);
    }
    
    
  }

  //This will recognise rgbw brightness values like this: "0,0,0,255"
  //RGB is not supported in hardware.
  else if (newTopic == ALL_RGBW){                             
    //Serial.println();
    //Serial.print("captured String is : "); 
    //Serial.println(sMessage);                                   // prints string to serial port out... 
    
    int ind1 = sMessage.indexOf(',');                           // finds location of first ,
    int r = round(sMessage.substring(0, ind1).toInt());                // captures first data String
    int ind2 = sMessage.indexOf(',', ind1+1 );                  // finds location of second ,
    int g = round(sMessage.substring(ind1+1, ind2+1).toInt());         // captures second data String
    int ind3 = sMessage.indexOf(',', ind2+1 );
    int b = round(sMessage.substring(ind2+1, ind3+1).toInt());
    //int ind4 = sMessage.indexOf(',', ind3+1 );
    int brightness = round(sMessage.substring(ind3+1).toInt() / 2.55); // captures remain part of data after last ,
    /* // print r g b brightness values
    Serial.print("r = "); Serial.println(r); 
    Serial.print("g = "); Serial.println(g);
    Serial.print("b = "); Serial.println(b);
    Serial.print("brightness = "); Serial.println(brightness);
    Serial.println(); Serial.println();
    */
    for(int i=0; i < Led::instanceCount; i++) 
        Led::instances[i]->setBrightness(brightness);  // We can use the w as brightness for now

    char str[40];  
    int w =   round(brightness*2.55);                            
    sprintf(str,"%d,%d,%d,%d",r,g,b,w);                        // using sprintf to convert to char[]
    
    mqttClient.publish(STATUS_ALL_RGBW, str);
   // mqttClient.publish(publishStatusTopicBrightness, (char*) brightness);
    Serial.print(STATUS_ALL_RGBW); Serial.println(str);
    
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
  // blink Message
  else if (newTopic == BLINK) {
    Led::blink();
    mqttClient.publish(STATUS_BLINK, cMessage);
    Serial.print(STATUS_BLINK); Serial.print (" = "); Serial.println (sMessage);
  }                          
  // tracking Message
  else if (newTopic == TRACK) {
    Led::tracking();    
    mqttClient.publish(STATUS_TRACK, cMessage);
    Serial.print(STATUS_TRACK); Serial.print (" = "); Serial.println (sMessage);
  }                     
  // restart Message
  else if (newTopic == "basement/hallway/restart") {
    ESP.restart();  // reboot... not sure if this can ever be handy
  }
  //
  
  else{ // Individual Leds: 
    for(int i=0; i < Led::instanceCount; i++) {
      if (newTopic == Led::instances[i]->sub_On ){
        // on
        if(sMessage == "true" || sMessage == "on" || (sMessage == "1" && isBoolean)){
          Led::instances[i]->on(); 
          mqttClient.publish(Led::instances[i]->pub_On, "true");                                         // show other devices your new status
          Serial.print(Led::instances[i]->pub_On); Serial.println (" = true");
        }
        // off 
        else if(sMessage == "false" || sMessage == "0" || sMessage == "off"){  
          Led::instances[i]->off(); 
          mqttClient.publish(Led::instances[i]->pub_On, "false");                                     // show other devices your new status
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
         // brightness 
        else if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){    // avoiding intMessage=0, which could be an unidentified message too 
            Led::instances[i]->setBrightness(intMessage);
            mqttClient.publish(Led::instances[i]->pub_Brightness, cMessage);
            Serial.print(Led::instances[i]->pub_Brightness); Serial.print (" = "); Serial.println(sMessage);
        }


      }
      // brightness
      else if (newTopic == Led::instances[i]->sub_Brightness ){  
        if (sMessage == "0" || (intMessage >= 1 && intMessage <= 100) ){    // avoiding intMessage=0, which could be an unidentified message too 
            Led::instances[i]->setBrightness(intMessage);
            mqttClient.publish(Led::instances[i]->pub_Brightness, cMessage);
            Serial.print(Led::instances[i]->pub_Brightness); Serial.print (" = "); Serial.println(sMessage);
        }
      }       
    }           // end of loop
   
  }          // end of individual led section
}          // end of callback

/********************************************************************************************************************************/
/********* Setup ****************************************************************************************************************/

void setup(void) {
  
  Serial.begin(115200);           // (9600);

  //Init Leds   
  Led::setup();                   // Led class is setting up EEPROM so it can save the state incase of power failure

  // Init WiFi
  WiFi.mode(WIFI_STA);                                            //       WiFi client / station mode
  
  ArduinoOTA.setHostname(mqtt_client_name);                       //       initialize and start OTA
  //ArduinoOTA.setPassword("password");                           //       set OTA password
  ArduinoOTA.onError([](ota_error_t error) {ESP.restart();});     //       restart in case of an error during OTA
  ArduinoOTA.begin();                                             //       at this point OTA is set up
  Serial.print("OTA is configured... ");
  Serial.print("Host name = " ); Serial.println( mqtt_client_name);
  delay(100);
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
      WiFi.begin(ssid, password);
      //delay(100);                                                  
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
      mqttClient.subscribe(ALL_RGBW);
      Serial.print("subscribed to: "); Serial.println(ALL_RGBW);
      delay(20);
      mqttClient.subscribe(NOTIFY);
      Serial.print("subscribed to: "); Serial.println(NOTIFY);
      delay(20);
      mqttClient.subscribe(ALARM);
      Serial.print("subscribed to: "); Serial.println(ALARM);
      delay(20);
      mqttClient.subscribe(BLINK);
      Serial.print("subscribed to: "); Serial.println(BLINK);
      delay(20);
      mqttClient.subscribe(TRACK);
      Serial.print("subscribed to: "); Serial.println(TRACK);
      delay(20);

      conn_stat = 5;                    
      break;
      // finished MQTT configuration
  }
// end of non-blocking connection setup section

// start of section with tasks where WiFi/MQTT is required
  if (conn_stat == 5) {
    if (millis() - lastStatus > 3600000) {                          // Start send status every 1 hour 
      //Serial.println(Status);
      mqttClient.publish(CHECK_IN, "online");                       // send status to broker
      mqttClient.loop();                                            // give control to MQTT to send message to broker
      lastStatus = millis();                                        // remember time of last sent status message
    }
    ArduinoOTA.handle();                                            // internal household function for OTA
    mqttClient.loop();                                              // internal household function for MQTT
  } 
// end of section for tasks where WiFi/MQTT are required

// start section for tasks which should run regardless of WiFi/MQTT

  Switch::loop();
  Led::loop();
  // ..add any code you want in the loop() here.. (don't use delay)

  
  
// end of section for tasks which should run regardless of WiFi/MQTT
}



