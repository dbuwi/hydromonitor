//##################################################################################################################
//##                                      ELET2415 DATA ACQUISITION SYSTEM CODE                                   ##
//##                                                                                                              ##
//##################################################################################################################

// LIBRARY IMPORTS
#include <rom/rtc.h> 
#include <math.h>  // https://www.tutorialspoint.com/c_standard_library/math_h.htm 
#include <ctype.h>

// ADD YOUR IMPORTS HERE
#include <FastLED.h>       // Library for controlling addressable LEDs
#include <ArduinoJson.h>   // Library for handling JSON data
#include <PubSubClient.h>  // Library for MQTT communication
#include <DHT.h>          // Library for DHT sensor

#include <WiFi.h>         // Library for WiFi connectivity
//#include <NTPClient.h>    // Library for Network Time Protocol (NTP)
#include <WiFiUdp.h>      // Required for NTPClient

#ifndef _WIFI_H 
#include <WiFi.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif 
 
#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

 

// DEFINE VARIABLES
#define ARDUINOJSON_USE_DOUBLE      1 
#define LED_PIN 4
#define NUM_LEDS 7 

// DEFINE THE CONTROL PINS FOR THE DHT22 
#define DHT_PIN 5       
#define DHT_TYPE DHT22  

// MQTT CLIENT CONFIG  
static const char* pubtopic      = "620162321";                    // Add your ID number here
static const char* subtopic[]    = {"620162321_sub","/elet2415"};  // Array of Topics(Strings) to subscribe to
static const char* mqtt_server   = "broker.emqx.io";         // Broker IP address or Domain name as a String 
static uint16_t mqtt_port        = 1883;

// WIFI CREDENTIALS
const char* ssid       = "MonaConnect";     // Add your Wi-Fi ssid
const char* password   = ""; // Add your Wi-Fi password 




// TASK HANDLES 
TaskHandle_t xMQTT_Connect          = NULL; 
TaskHandle_t xNTPHandle             = NULL;  
TaskHandle_t xLOOPHandle            = NULL;  
TaskHandle_t xUpdateHandle          = NULL;
TaskHandle_t xButtonCheckeHandle    = NULL;  

// FUNCTION DECLARATION   
void checkHEAP(const char* Name);   // RETURN REMAINING HEAP SIZE FOR A TASK
void initMQTT(void);                // CONFIG AND INITIALIZE MQTT PROTOCOL
unsigned long getTimeStamp(void);   // GET 10 DIGIT TIMESTAMP FOR CURRENT TIME
void callback(char* topic, byte* payload, unsigned int length);
void initialize(void);
bool publish(const char *topic, const char *payload); // PUBLISH MQTT MESSAGE(PAYLOAD) TO A TOPIC
void vButtonCheck( void * pvParameters );
void vUpdate( void * pvParameters );  
bool isNumber(double number);
 

/* Declare your functions below */ 
double convert_Celsius_to_fahrenheit(double c);
double convert_fahrenheit_to_Celsius(double f);
double calcHeatIndex(double Temp, double Humid);


/* Init class Instances for the DHT22 etcc */
DHT dht(DHT_PIN, DHT_TYPE); // Create a DHT22 sensor object





//############### IMPORT HEADER FILES ##################
#ifndef NTP_H
#include "NTP.h"
#endif

#ifndef MQTT_H
#include "mqtt.h"
#endif

// Temporary Variables 
CRGB leds[NUM_LEDS]; // Define LED array


void setup() {
  Serial.begin(115200);  // INIT SERIAL  

  // INITIALIZE ALL SENSORS AND DEVICES
  dht.begin(); // Initialize DHT22 sensor
  
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); // Initialize FastLED
  FastLED.clear();
  FastLED.show();
  /* Add all other necessary sensor Initializations and Configurations here */
 

  
  initialize();     // INIT WIFI, MQTT & NTP 
   //vButtonCheckFunction(); // UNCOMMENT IF USING BUTTONS INT THIS LAB, THEN ADD LOGIC FOR INTERFACING WITH BUTTONS IN THE vButtonCheck FUNCTION
 }
  
void loop() {
    // put your main code here, to run repeatedly:       
    // PRINT SENSOR VALUES TO SERIAL MONITOR
  float Temp = dht.readTemperature();
  float Humid = dht.readHumidity();
  float HeatIndex = dht.computeHeatIndex(Temp, Humid, false);
   
  // PRINT SENSOR VALUES TO SERIAL MONITOR
  Serial.print("Temperature: ");
  Serial.print(Temp);
  Serial.print(" °C, Humidity: ");
  Serial.print(Humid);
  Serial.println(" %");
  Serial.print(HeatIndex);
  Serial.println(" °C");


  delay(2000); 
    vTaskDelay(1000 / portTICK_PERIOD_MS);    
}




  
//####################################################################
//#                          UTIL FUNCTIONS                          #       
//####################################################################
void vButtonCheck( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );     
      
    for( ;; ) {
        // Add code here to check if a button(S) is pressed
        // then execute appropriate function if a button is pressed  

        vTaskDelay(200 / portTICK_PERIOD_MS);  
    }
}

void vUpdate( void * pvParameters )  {
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );    
           
    for( ;; ) {
          // #######################################################
          // ## This function must PUBLISH to topic every second. ##
          // #######################################################
   
          // 1. Read Humidity and save in variable below
          double h = dht.readHumidity();
           
          // 2. Read temperature as Celsius   and save in variable below
          double t = dht.readTemperature();    

          double heatIndex = calcHeatIndex(t,h); 

          if(isNumber(t) && isNumber(h) && isNumber(heatIndex)){
              // ##Publish update according to ‘{"id": "student_id", "timestamp": 1702212234, "temperature": 30, "humidity":90, "heatindex": 30}’

              // 1. Create JSon object
                  DynamicJsonDocument doc(256);  
                                      doc["id"] = "620162321";
                                      doc["timestamp"] = getTimeStamp();
                                      doc["temperature"] = t;
                                      doc["humidity"] = h;
                                      doc["heatindex"] = heatIndex;
              // 2. Create message buffer/array to store serialized JSON object
                     String jsonString;
                     
              // 3. Add key:value pairs to JSon object based on above schema
                    
              // 4. Seralize / Covert JSon object to JSon string and store in message array
                  serializeJson(doc, jsonString);
              // 5. Publish message to a topic sobscribed to by both backend and frontend                
                    if (publish(pubtopic, jsonString.c_str())) {  // Use jsonString.c_str()
                        Serial.println("Published sensor data:");
                        Serial.println(jsonString);
                    } else {
                         Serial.println("Failed to publish sensor data!");
                    }
          }

        vTaskDelay(1000 / portTICK_PERIOD_MS);  
    }
}

 

unsigned long getTimeStamp(void) {
          // RETURNS 10 DIGIT TIMESTAMP REPRESENTING CURRENT TIME
          time_t now;         
          time(&now); // Retrieve time[Timestamp] from system and save to &now variable
          return now;
}


void callback(char* topic, byte* payload, unsigned int length) {
  // ############## MQTT CALLBACK  ######################################
  // RUNS WHENEVER A MESSAGE IS RECEIVED ON A TOPIC SUBSCRIBED TO
  
  Serial.printf("\nMessage received : ( topic: %s ) \n",topic ); 
  char *received = new char[length + 1] {0}; 
  
  for (int i = 0; i < length; i++) { 
    received[i] = (char)payload[i];    
  }

  // PRINT RECEIVED MESSAGE
  Serial.printf("Payload : %s \n",received);

 
  // CONVERT MESSAGE TO JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, received);  

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }


  // PROCESS MESSAGE
  const char* type = doc["type"]; 
  
  if (strcmp(type, "controls") == 0){
    // 1. EXTRACT ALL PARAMETERS: NODES, RED,GREEN, BLUE, AND BRIGHTNESS FROM JSON OBJECT
  int brightness = doc["brightness"] | 0; // Default to 0 if not present
  int leds_to_turn_on = doc["leds"] | 0;      // Default to 0 if not present

  int r = 0;
  int g = 0;
  int b = 0;
  int a = 255; // Default alpha

  if (doc.containsKey("color") && doc["color"].is<JsonObject>()) {
    JsonObject color = doc["color"].as<JsonObject>();
    r = color["r"] | 0;
    g = color["g"] | 0;
    b = color["b"] | 0;
    a = color["a"] | 255;
  }

    // 2. ITERATIVELY, TURN ON LED(s) BASED ON THE VALUE OF NODES. Ex IF NODES = 2, TURN ON 2 LED(s)
          FastLED.setBrightness(brightness);

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < leds_to_turn_on) {
      leds[i] = CRGB(r, g, b); // Set color
    } 
    // 3. ITERATIVELY, TURN OFF ALL REMAINING LED(s).
            else {
              leds[i] = CRGB::Black; // Turn off
      }
    }
      FastLED.show();
    }
  }

bool publish(const char *topic, const char *payload){   
     bool res = false;
     try{
        res = mqtt.publish(topic,payload);
        // Serial.printf("\nres : %d\n",res);
        if(!res){
          res = false;
          throw false;
        }
     }
     catch(...){
      Serial.printf("\nError (%d) >> Unable to publish message\n", res);
     }
  return res;
}



//***** Complete the util functions below ******

double convert_Celsius_to_fahrenheit(double c){    
    // CONVERTS INPUT FROM °C TO °F. RETURN RESULTS     
       return (c * 9.0 / 5.0) + 32.0;  
}

double convert_fahrenheit_to_Celsius(double f){    
    // CONVERTS INPUT FROM °F TO °C. RETURN RESULT    
    return (f - 32.0) * 5.0 / 9.0;  
}

double calcHeatIndex(double Temp, double Humid){
    // CALCULATE AND RETURN HEAT INDEX USING EQUATION FOUND AT https://byjus.com/heat-index-formula/#:~:text=The%20heat%20index%20formula%20is,an%20implied%20humidity%20of%2020%25
   double T = convert_Celsius_to_fahrenheit(Temp);

    // Constants for Heat Index Formula
    double c1 = -42.379;
    double c2 = -2.04901523;
    double c3 = -10.14333127;
    double c4 = -0.22475541; 
    double c5 = -6.83783e-3; 
    double c6 = -5.481717e-2;
    double c7 = 1.22874e-3;
    double c8 = 8.5282e-4; 
    double c9 = -1.99e-6;

    double HI = c1 + (c2 * T) + (c3 * Humid) + (c4 * T * Humid) + 
                (c5 * T * T) + (c6 * Humid * Humid) + 
                (c7 * T * T * Humid) + (c8 * T * Humid * Humid) + 
                (c9 * T * T * Humid * Humid);

            return convert_fahrenheit_to_Celsius(HI);

}
 

bool isNumber(double number){       
        char item[20];
        snprintf(item, sizeof(item), "%f\n", number);
        if( isdigit(item[0]) )
          return true;
        return false; 
} 
