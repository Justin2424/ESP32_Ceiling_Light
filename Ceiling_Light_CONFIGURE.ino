

/******************  LIBRARY SECTION *************************************/
#include <SimpleTimer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


//USER CONFIGURED SECTION START//
static const char* ssid = "";
static const char* password = "";
static const char* mqtt_server = "";
static const int mqtt_port = 1883;
static const char *mqtt_user = "";
static const char *mqtt_pass = "";
static const char *mqtt_client_name = "Basement/HallwayMCU"; // Client connections can't have the same connection name
//USER CONFIGURED SECTION END//

/*****************  DECLARATIONS  ****************************************/

WiFiClient espClient;
PubSubClient client(espClient);
SimpleTimer timer;
SimpleTimer adjustDimmerTimer;

/*****************  GLOBAL VARIABLES  ************************************/
static const int PINS_COUNT = 9;                                             // the number of pwm pins we'll use. The following arrays need to be this length. 
static const int PIN_NUMBERS[] = {4, 16, 17, 13, 5, 14, 27, 26, 15};         // the pin numbers we'll use
static const char PIN_NAMES[PINS_COUNT][40] = {                              //  These PIN_NAMES are the topics that this MQTT client will subscibe to
                                                                             //  The number of PIN_NAMES has to equal pinCount. and [40] is max character length
                                      "Basement/Hallway/row0",                               
                                      "Basement/Hallway/row1",            
                                      "Basement/Hallway/row2",      //        
                                      "Basement/Hallway/row3",            
                                      "Basement/Hallway/row4",      //        
                                      "Basement/Hallway/row5",           
                                      "Basement/Hallway/row6",           
                                      "Basement/Hallway/row7", 
                                      "Basement/Hallway/row8"
                                     };         
static const char ALL_PINS_NAME[40] = {"Basement/Hallway/All"};                       // Choose a topic to address all pins 

static int pinsBrightness[] = {100,100,100,100,100,100,100,100,100};                // The brightness each pin is currently set to 
static int pinsTargetBrightness[] = {100,100,100,100,100,100,100,100,100};          // The target brightness each pin will be 

static const int TRANSITION_SPEED = 5;                                               // (1-10) default is 5, larger is slower 
static const String DIMMER_MODE = "DIMMER_MODE";                                // This mode is used to soft tranistion brightness levels 
static const String NOTIFY_MODE = "NOTIFY_MODE";                                // These modes are a work in progress
static const String ALARM_MODE = "ALARM_MODE";
static String triggerMode = "DIMMER_MODE";                                      // A flag that contains 1 of the 3 strings above    
static int modeCounter = 0;                                                     

static const int FREQ = 5000;                                                   // PWM freguency    
static const int RESOLUTION = 8;                                                // PWM RESOLUTION bits

static bool boot = true;


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
       // char checkInTopic[40];                                              //eg: "checkIn/Livingroom/ceilingLightMCU"
       // printf(checkInTopic,"checkIn %s",String(mqtt_client_name));                 // using printf to join strings into the char[] 
       // Serial.print("--checkInTopic = ");
        //Serial.println(checkInTopic);
        if(boot == true)
        {
          client.publish("checkIn/Basement/HallwayMCU","Rebooted");
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
        
        for(int i=0; i < PINS_COUNT; i++)
        {
          client.subscribe(PIN_NAMES[i]);
          delay(20);
          //Serial.print("subscribed to: ");    
          //Serial.println(PIN_NAMES[i]);
        }
        client.subscribe(ALL_PINS_NAME);
                 
        //Serial.print("subscribed to: ");
        //Serial.println(ALL_PINS_NAME);
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
  
  for(int i=0; i < PINS_COUNT; i++)                                        // loop through our PIN_NAMES....(Topics)
  {
    
    sprintf(publishTopicStatus,"%s%s",PIN_NAMES[i],"/Status/");            // using sprintf to join the two strings (and assign)
    //Serial.print("create publishTopicStatus: ");
    //Serial.println(publishTopicStatus);
                                      
    
    if (newTopic == PIN_NAMES[i] || newTopic == ALL_PINS_NAME)               // found a topic we are subscribe to
    {
      Serial.print("Found Topic: ");
      Serial.println(PIN_NAMES[i]);
      
      
      //ON Message
      if(newPayload == "ON")     // immediate full brightness.. alternatively use int 0-255 to soft transition to any brightness
      {
        ledcWrite(i, 255);                          //Turn light on
        pinsBrightness[i] = 255;                    //set brightness
        pinsTargetBrightness[i] = 255;              // and targetBrightness to match
        client.publish( publishTopicStatus,"ON");   //report back
        triggerMode = DIMMER_MODE ;
        
      }
      //OFF Message
      else if (newPayload == "OFF" )   //immediate full off.. alternatively use int 0-255 to soft transition to any brightness
      {
        ledcWrite(i, 0);
        pinsBrightness[i] = 0;         
        pinsTargetBrightness[i] = 0;  
        client.publish(publishTopicStatus,"OFF");
        triggerMode= DIMMER_MODE ;
      }
      /* 
       * soft transition to a new brighness level using Int's 0-255
       * - We set a new pinTargetBrightness value, and a timer will call moverDimmer() 
       * to increamentally adjust brightness until target is reached
      */ 
      // DIMMER Message
      else if (newPayload == "0" || (intPayload >= 1 && intPayload < 256) )    // avoiding intPayload=0, which can be unidentified messages too 
      {
        triggerMode = DIMMER_MODE ;
        pinsTargetBrightness[i] = intPayload;                    // set new target brightness , moveDimmer() does the rest
        char pl[40];                                 
        sprintf(pl,"%d",newPayload);    // Using sprintf to convert to char[]
        client.publish(publishTopicStatus, pl);
      }
      // NOTIFY Message
      else if (newPayload == "NOTIFY")
      {
        triggerMode = NOTIFY_MODE;                     // just triggers the code in moveDimmer()
        
      }
      // ALARM Message                                 //expects "OFF" payload to turn it off, but 0-255 or "ON" works too
      else if (newPayload == "ALARM")
      {
        triggerMode = ALARM_MODE;                       // just triggers the code in moveDimmer()
        Serial.println(triggerMode);
      }
     
      // UNIDENTIFIED Message -  Print and publish to help catch any issues 
      else // if(intPayload ==0  )                              
      {   
        char publishTopicStatus[40];                                 // init the topic (char array) used to report back 
        sprintf(publishTopicStatus,"%s%s",PIN_NAMES[i],"/Status/Error");    // Using sprintf to join the two strings (and assign)
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


/*****************  GLOBAL LIGHT FUNCTIONS  *******************************/


/* 
 *  Increments pinBrightness towards TargetBrightness  
*/
void moveDimmer()
{ 
  ////////////////  DIMMER_MODE  ///////////////
  if(triggerMode == DIMMER_MODE)
  {
    for(int i=0; i < PINS_COUNT; i++)  
    {
      // BRIGHTNESS UP
      if(pinsBrightness[i] < pinsTargetBrightness[i])
      { 
        pinsBrightness[i]++;
        // go faster when the target Brighness is far away
        int distance = pinsTargetBrightness[i]-pinsBrightness[i];
        if (distance > 20) 
        { 
          pinsBrightness[i] += (int) distance*0.05;   // adds 1 to 12 
        }
        ledcWrite(i, pinsBrightness[i]); 
      } 

      // BRIGHTNESS DOWN
      else if(pinsBrightness[i] > pinsTargetBrightness[i])
      {
        pinsBrightness[i]--;
        // go faster when the target Brighness is far away
        int distance = pinsBrightness[i]-pinsTargetBrightness[i];
        if (distance > 20)
        {
          pinsBrightness[i] -= (int) distance*0.05;
        }
        ledcWrite(i, pinsBrightness[i]);  
      }
    }
  }

  /////////  NOTIFY_MODE   ////////// Short and subtle variation of AlarmMode
  if(triggerMode == NOTIFY_MODE){                        // this could be changed to just set the targetBrightnes and still run the dimmerMode to smoothen 
    // uses modeCounter to cycle through this section 200 times
    // assumes the pins are in sequence, and the ambience is last
    for (int i=0; i<PINS_COUNT-1; i++)                   // for each pin except last
    {             
      int rollOverCounter = int(modeCounter + (12.5*i)) % 100;    // is a rolling over value between 0-100 // (12.5*i) is an offset for each pin/led
  
      float sinVal =  (sin(rollOverCounter*0.031415));  //(3.1415/100)    // a value between 0 and 1// as NotifyCounter increments: SinVal starts at 0, ramps up to 1, and falls back to zero.. (for the first pin)// half sinewave, with no negitive values. 
      int offset = pinsTargetBrightness[i] *.5;
      
      pinsBrightness[i] = int((sinVal*127)+offset);                //set new brightness level
      
      ledcWrite(i, pinsBrightness[i]);
                          
    }
    modeCounter++;
    if (modeCounter >= 200)
    {
      triggerMode = DIMMER_MODE;                // finished.. // DimmerMode will return pinsbrightness back to their orginal value 
      modeCounter = 0;                //reset 
    } 
  }
  //ALARM_MODE
  else if(triggerMode == ALARM_MODE)
  {
    // uses modeCounter to cycle through this 
    // assumes the pins are in sequence, and the ambience is last
    for (int i=0; i< PINS_COUNT; i++)          // for each pin
    {             
      int rollOverCounter = int(modeCounter + (12.5*i)) % 100;     // is a rolling over value between 0-100 // (12.5*i) is an offset for each pin/led
  
      float sinVal = (sin(rollOverCounter*0.031415));  //(3.1415/100)     // a value between 0 and 1// as NotifyCounter increments: SinVal starts at 0, ramps up to 1, and falls back to zero.. (for the first pin)// half sinewave, with no negitive values. 
  
      pinsBrightness[i] = int((sinVal*255));       //writecLed
      
      ledcWrite(i, pinsBrightness[i]); 
      
    }
    modeCounter++;
    if (modeCounter >= 240000)            // self cancels after 20 minutes (assuming default transition speed of 5ms)
    {
      triggerMode = DIMMER_MODE;                // finished.. exit this mode // DimmerMode will return pinsrightness back to their orginal value 
      modeCounter = 0;                //reset 
    } 
  }
}


/*****************  SETUP FUNCTIONS  ****************************************/


void setup() 
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
    
  // Initialize channels for pwm
  // channels can be 0-15, RESOLUTION 1-16 bits, FREQ limit depend on RESOLUTION
  for(int i=0; i <  PINS_COUNT; i++){   
    ledcSetup(i, FREQ, RESOLUTION);
    ledcAttachPin(PIN_NUMBERS[i], i);
    Serial.print("ledcSetup ");
    Serial.println(i);
  }
   
  adjustDimmerTimer.setInterval(TRANSITION_SPEED, moveDimmer);   
     
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  timer.setInterval(600000, checkIn);        // 10 minutes
  
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
  timer.run();
  adjustDimmerTimer.run();
  


}
