// Mum Alert
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid[] = {"ssid1","ssid2"};  // your network SSID (name)
const char* pass[] = {"pass1","pass2"};                 
int network=1;	// select 0=SSID1 

char MQTThost[] ="mqtt.xyz.com";
char MQTTpass[] ="mqttpass";
char MQTTuser[] ="mqttuser";
char MQTTtopic[]="mqtttopic";

char Alert[]="Mother pressed the button";

int status = WL_IDLE_STATUS;        // the WiFi radio's status
int led_state=HIGH; 

#define LINK_OUT D6   // used to hold power on till finished


void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

WiFiClient espClient;
PubSubClient mqttClient(MQTThost,1883,callback,espClient);

void toggleLED()
{
  digitalWrite(LED_BUILTIN,led_state);
  led_state=!led_state;
}

void MQTTreconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Create a random client ID
    String mqttClientId = "ESP8266Client-";
    mqttClientId += String(random(0xffff), HEX);

    Serial.print("Attempting MQTT connection... ClientId ");
    Serial.println(mqttClientId);

    // Attempt to connect
    if (mqttClient.connect(mqttClientId.c_str(),MQTTuser,MQTTpass)) {
      Serial.println("MQTT connected sending alert.");
      
      // Once connected, publish an alert message...
      mqttClient.publish(MQTTtopic, Alert);
     
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");

      // Wait 5 seconds before retrying
      toggleLED();
      delay(5000);
    }
  }
}

boolean tryToConnectToWifi(uint8_t network)
{
  while ( status != WL_CONNECTED) {
    Serial.printf("Trying to connect to WPA SSID: %s\n",ssid[network]);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid[network], pass[network]);

    for (uint d=0;d<10;d++){
      if  ( status == WL_CONNECTED) { 
        Serial.printf("Wifi connected to SSID %s\n",ssid[network]);
        return true;
      }
      toggleLED();
      delay(1000);
    }

  }
  Serial.printf("Wifi connection to SSID %s failed after 10 retries\n",ssid[network]);
  return false;
}


void setup() {

  // is a link is fitted between D6-D7
  pinMode(LINK_OUT,OUTPUT);
  digitalWrite(LINK_OUT,HIGH);

  // WiFi.begin() may crash
  WiFi.persistent(false);

  // show power is on by turning on the builtin led

  pinMode(LED_BUILTIN, OUTPUT);   
  toggleLED();

  //Initialize serial and wait for port to open:
  // mostly there is no delay
  Serial.begin(115200);
  while (!Serial) {
    yield(); // prevent watchdog timeouts
  }
  
  // disconnect if previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

 
  // attempt to connect to WiFi network:
  // this will block till a connection is made to one of the networks
  boolean res=false;

  while (res==false)
  {
    res=tryToConnectToWifi(network);
    toggleLED();
  }

  // now connect to MQTT server and publish an alert message
  MQTTreconnect(); 

  // turn off the system. 
  // there will be a 10s delay due to the R-C values connected
  // to the MOSFET switch gate
  
  digitalWrite(LINK_OUT,LOW);
}


void loop() {
  toggleLED();  //blink the WeMOS led to show we are alive
  delay(1000);
}