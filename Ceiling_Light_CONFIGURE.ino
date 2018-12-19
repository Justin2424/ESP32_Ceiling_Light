
/******************  LIBRARY SECTION *************************************/

#include <SimpleTimer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Led.h>

//USER CONFIGURED SECTION START//
static const char* ssid = "";                                 // your wifi network name
static const char* password = "";                               // your wifi password
static const char* mqtt_server = "";
static const int mqtt_port = 1883;
static const char *mqtt_user = "";
static const char *mqtt_pass = "";
static const char *mqtt_client_name = "Basement/HallwayMCU";              // Client connections can't have the same connection name
//USER CONFIGURED SECTION END//

/*****************  DECLARATIONS  ****************************************/

WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer checkInTimer;                                                   // publishes a "I'm alive " message every 10 minutes

/*****************  GLOBAL VARIABLES  ************************************/

static bool boot = true;

// LED CONFIG
static const int LED_COUNT = 9;                   
static Led leds[LED_COUNT] ={ 
              //Led(pin, (char*)"mqqt topic", pwm_channel),
                Led(26, (char*)"Basement/Hallway/row0", 1),
                Led(27, (char*)"Basement/Hallway/row1", 2),
                Led(14, (char*)"Basement/Hallway/row2", 3),
                Led(13, (char*)"Basement/Hallway/row3", 4),
                Led(5,  (char*)"Basement/Hallway/row4", 5),
                Led(17, (char*)"Basement/Hallway/row5", 6),
                Led(16, (char*)"Basement/Hallway/row6", 7),
                Led(4,  (char*)"Basement/Hallway/row7", 8),
                Led(15, (char*) "Basement/Hallway/row8", 0)
};  
static const char ALL_LEDS_NAME[40] = {"Basement/Hallway/All"};              // MQTT topic used to control all pins 

// SWITCH CONFIG
static const int SWITCH_PINS_COUNT = 3;                   // length of SWITCH_PIN_NUMBERS[] below
static const int SWITCH_PIN_NUMBERS[] = {25,32,33};       // list of the pin numbers the switches can be connected 
static int switchStates[] = {LOW, LOW, LOW};
static const unsigned long DEBOUNCE_DELAY = 10;           // the debounce time; increase if the output flickers on toggle 
static unsigned long lastDebounceTimes[] = {0, 0, 0};     // the last time the switch pin was toggled
SimpleTimer pollingTimer;   
static const int POLLING_SPEED = 10;

// Timer 
SimpleTimer changeBrightnessTimer;
static const int CHANGE_BRIGHTNESS_SPEED = 5;                                   // (1-10) default is 5, larger is slower 
static String FADE_MODE = "FADE_MODE";                                          // This mode is used to fade between brightness levels 
static String  NOTIFY_MODE = "NOTIFY_MODE";                                     // This mode is used for doorbell or something similar (The Leafs scored! :)
static String  ALARM_MODE = "ALARM_MODE";                                       // smoke or secuity alarm mode
static String  BLINK_MODE = "BLINK_MODE";                                       // an alternate notifation
static String  TRACKING_MODE = "TRACKING_MODE";                                 // use camera for location tracking: light control based on the traffic

static String currentMode = FADE_MODE;                                           // Identifies which mode were using 
static int modeCounter = 0;  
                                                   
// CAMERA / TRACKING_MODE 
static const int X_LIGHT_POS[] = {50, 150, 370, 490, 610, 730, 730, 0, 0};  // these represent the physical  xlocations of the lights
static const int Y_LIGHT_POS[] = {50, 50, 50, 50, 50, 50, 200, 0, 0};
#define UNSET -1
static int xPos = UNSET;
static int yPos = UNSET;
static int size = UNSET;
static int speed = UNSET;                                                   // note: Average walking speed is 1.4 m per second... 





/*****************  SYSTEM FUNCTIONS  *************************************/

void setup_wifi() 
{
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void reconnect() 
{
  // Loop until we're reconnected
  int retries = 0;
  while (!client.connected()) {
    if(retries < 150)
    {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) 
      {
        Serial.println("connected");
        // Once connected, publish an announcement...
        char checkInTopic[40];                                                      //eg: "checkIn/Livingroom/ceilingLightMCU"
        printf(checkInTopic,"checkIn %s",String(mqtt_client_name));                 // using printf to join strings into the char[] 
        Serial.print("--checkInTopic = ");
        Serial.println(checkInTopic);
        if(boot == true)
        {
          client.publish("checkIn/Basement/HallwayMCU","Rebooted");                 // TODO: if down time less than ten minute, have previous values sent out
      
          boot = false;
          Serial.print("checkIn/Basement/HallwayMCU");
          Serial.println(": Rebooted");
          
        }
        if(boot == false)
        {
          client.publish("checkIn/Basement/HallwayMCU","Reconnected"); 
          Serial.print("checkIn/Basement/HallwayMCU");
          Serial.println(": Reconnected");
        }
        
        //subscribe to Topics
        for(int i=0; i < LED_COUNT; i++)
        {
          client.subscribe(leds[i].pinName);
          delay(20);
          Serial.print("subscribed to: ");    
          Serial.println(leds[i].pinName);
        }
        client.subscribe(ALL_LEDS_NAME);
        Serial.print("subscribed to: ");
        Serial.println(ALL_LEDS_NAME);
      } 
      else 
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        retries++;
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    if(retries > 1500)
    {
    }
  }
}

void checkIn()
{
  client.publish("checkIn/Basement/HallwayMCU","OK");
}



////////// We got a message ///////////
/*Get the topic and use the payload*/
void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  String newTopic = topic;                                  // get the String
  Serial.print(topic);
  Serial.print("] ");
  payload[length] = '\0';                                   // add null character to end
  String newPayload = String((char *)payload);              // cast to String     ( might be cleaner to stick with char arrays)
  int intPayload = newPayload.toInt();                      // if the newPayload was meant to be an Integer value.... get it..returns zero if it wasn't meant to be an Int
  Serial.println(newPayload);
  char publishTopicStatus[40];                              // init the topic used to report back 
  
  for(int i=0; i < LED_COUNT; i++)                                        // loop through our PIN_NAMES....(Topics)
  {
    
    sprintf(publishTopicStatus,"%s%s",leds[i].pinName,"/Status/");            // using sprintf to join the two strings (and assign)
    //Serial.print("create publishTopicStatus: ");
    //Serial.println(publishTopicStatus);
                                      
    if (newTopic == leds[i].pinName || newTopic == ALL_LEDS_NAME)               // found a topic we are subscribe to
    {
      Serial.print("Found Topic: ");
      Serial.println(leds[i].pinName);
      
      //ON Message
      if(newPayload == "ON")     // immediate full brightness.. 
      {
        leds[i].on(255); 
        client.publish( publishTopicStatus,"ON");   //report back
        currentMode = FADE_MODE ;
      }

      //OFF Message
      else if (newPayload == "OFF" )   //immediate full off..
      {
        leds[i].off();
        client.publish(publishTopicStatus,"OFF");
        currentMode= FADE_MODE ;
      }
     
      // fade Message
       /* 0-255  int's are used to transition to a brighness level.
        * We'll set a new pinTargetBrightness, and a timer will updateBrightness()   */ 
      else if (newPayload == "0" || (intPayload >= 1 && intPayload < 256) )    // avoiding intPayload=0, which could be an unidentified messages too 
      {
        leds[i].fade(intPayload);
        currentMode = FADE_MODE ;
        char pl[40];                                 
        sprintf(pl,"%d",newPayload);                        // trying to use sprintf to convert to char[]
        client.publish(publishTopicStatus, pl);
      }

      // NOTIFY Message
      else if (newPayload == "NOTIFY")
      {
        currentMode = NOTIFY_MODE;                         // Uses the animation code in updateBrightness() 
        Serial.println(currentMode);
      }

      // ALARM Message                                  // The alarm code will play until a new message is sent 
      else if (newPayload == "ALARM")
      {
        currentMode = ALARM_MODE;                       // updateBrightness() will play the alarm animation 
        Serial.println(currentMode);
      }

       else if (newPayload == "BLINK")
      {
        currentMode = BLINK_MODE;                       // triggers the code in updateBrightness()
        Serial.println(currentMode);
      }

       else if (newPayload == "TRACKING")
      {
        currentMode = TRACKING_MODE;                       // trigger
        Serial.println(currentMode);
      }

      // UNIDENTIFIED Messages -  Print and publish to help catch any issues 
      else // if(intPayload == 0  )                              
      {   
        char publishTopicStatus[40];                                 // init the topic (char array) used to report back 
        //sprintf(publishTopicStatus,"%s%s",leds.pinName[i],"/Status/Error");    // Using sprintf to join the two strings (and assign)
        char t[20];  
        sprintf(t,"%s",newPayload);                                 // Using sprintf to convert to char[]
        Serial.print(publishTopicStatus);
        Serial.print("Error - Unidentified message recieved: "); 
        Serial.println(t);
        client.publish(publishTopicStatus, t); 
      } 
      
    }
  }   
}     



void updateSwitchState()
{
  for(int i=0; i < SWITCH_PINS_COUNT; i++)  
  {
    int reading = digitalRead(SWITCH_PIN_NUMBERS[i]);
      
    if (reading != switchStates[i]) 
    {
      if ((millis() - lastDebounceTimes[i]) > DEBOUNCE_DELAY) 
      {
        lastDebounceTimes[i] = millis();                          //reset 
        switchStates[i] = reading;                                // save new state 
        // The switch has changed state... 
        // perform desired action:
        
        currentMode = FADE_MODE;
        for(int i=0; i < LED_COUNT; i++)  
        {
          leds[i].toggle();
        }

      } 
    }
  }

     


}  

/*****************  Update Brightness *******************************/


/* 
 *   Called by a timer every 5ms (TRANSITION_SPEED)
*/
void updateBrightness(){ 
  
    ////////////////  FADE_MODE  ///////////////
  if(currentMode == FADE_MODE){   // Increments pinBrightness towards TargetBrightness 
    for(int i=0; i < LED_COUNT; i++){
      leds[i].update();
    }
  }
    /////////  NOTIFY_MODE  /////////     a subtle variation of AlarmMode
  else if(currentMode == NOTIFY_MODE){                     
      // uses modeCounter to cycle through this section 200 times
      for (int i=0; i < LED_COUNT-1; i++){                        // for each pin except last
        int rollOverCounter = int(modeCounter + (12.5*i)) % 100;    // is a rolling over value between 0-100 // (12.5*i) is a different offset for each pin/led
        float sinVal =  (sin(rollOverCounter*0.062831));        //(2PI/100)    // a value between -1 and 1   
        leds[i].on(int((sinVal*0.5 + 0.5 ) * 155) );                //set new brightness level
      } 
      modeCounter++;
      if (modeCounter >= 200){
        currentMode = FADE_MODE;          // finished.. // FadeMode will return pinsbrightness back to their orginal value 
        modeCounter = 0;                    //reset 
      } 
  }
    ///////// ALARM_MODE ////////
  else if(currentMode == ALARM_MODE){
    // uses modeCounter to cycle through this 
    for (int i=0; i< LED_COUNT; i++){          // for each pin
      int rollOverCounter = int(modeCounter + (12.5*i)) % 100;      // is a rolling over value between 0-100 // (12.5*i) is an offset for each pin/led
      float sinVal =  (sin(rollOverCounter*0.062831));  //(2PI/100)   // a value between 0 and 1// as NotifyCounter increments: SinVal starts at 0, ramps up to 1, and falls back to zero.. (for the first pin)// half sinewave, with no negitive values. 
      leds[i].on( int((sinVal*0.5 + 0.5 ) * 255) );                 //set new brightness level
    }
    modeCounter++;
    if (modeCounter >= 240000){                     // self cancels after 20 minutes (assuming default TRANSITION_SPEED of 5ms)
      currentMode = FADE_MODE;                  // finished.. exit this mode // FadeMode will return pinsBrightness back to their orginal value 
      modeCounter = 0;                            //reset 
    } 
  }
    ///////  TRACKING_MODE  ///////
  else if(currentMode == TRACKING_MODE){
    if (xPos > UNSET && yPos > UNSET){                                  // insure we have valid data
      int brightness;
      for (int i=0; i< LED_COUNT; i++){                                 // for each pin 
        brightness =  255 * 300  / distanceSq(X_LIGHT_POS[i], Y_LIGHT_POS[i], xPos, yPos);  // full brighnesss at 0.5m and 0 at 5m
        leds[i].fade( constrain(brightness, 0, 255) );
      }
    }
    modeCounter++;
    if (modeCounter >= 240000){            // self cancels after 20 minutes (assuming default TRANSITION_SPEED of 5ms)
      currentMode = FADE_MODE;          // exit this mode // FadeMode will return pinsBrightness back to their orginal value 
      modeCounter = 0;                    
    } 
  }

}


/*****************  SETUP FUNCTIONS  ****************************************/


void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
   
  //Initialize the switch pins
  for(int i=0; i <  SWITCH_PINS_COUNT; i++){   
   pinMode(SWITCH_PIN_NUMBERS[i], INPUT);
    Serial.print("switchSetup ");
    Serial.println(i);
  }  
   
   
  changeBrightnessTimer.setInterval(CHANGE_BRIGHTNESS_SPEED, updateBrightness);   
  pollingTimer.setInterval(POLLING_SPEED, updateSwitchState);  
  checkInTimer.setInterval(600000, checkIn);        // 10 minutes

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  ArduinoOTA.setHostname("HallwayMCU");
  ArduinoOTA.begin();
   
  
}


/*****************  MAIN LOOP  ****************************************/


void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();
  ArduinoOTA.handle();
  
  checkInTimer.run();                  // calls checkin every hour or so
  changeBrightnessTimer.run();         // calls updateBrightness()
  pollingTimer.run();                  // calls updateSwitchs()
  


}



double distance (int x1, int y1, int x2, int y2)
{
  double d = sqrt(((x2-x1)*(x2-x1)) + ((y2-y1)*(y2-y1)));
}

double distanceSq(int x1, int y1, int x2, int y2)
{
  double d=distance(x1, y1, x2, y2);
  return d*d;
}




/*
class Button {
private:
 byte down_at;
 byte down;
 
public:
 Button();
 byte downFor();
 void resetDown();
 bool isDown();
 void turnOn();
 void turnOff();
};

Button::Button()
{
 this->down_at = 0;
 this->down = 0;
}

// Find the number of beats the button has been down for. Function can compute max. 8 sec down before rolling over.
byte Button::downFor()
{
 byte time;
 if (this->down_at > sc_beat)
 {
  time = (sc_beat + 256) - this->down_at;
 }
 else
 {
   time = sc_beat - this->down_at;
 }
 return time;
}

bool Button::isDown()
{
 return (bool)this->down;
}

void Button::resetDown()
{
 this->down_at = sc_beat;
 this->down = 1;
}
 
void Button::turnOn()
{
 if (!this->down)
 {
   this->down_at = sc_beat;
   this->down = 1;
 }
 return;
}

void Button::turnOff()
{
 this->down = 0;
 return;
}
*/


