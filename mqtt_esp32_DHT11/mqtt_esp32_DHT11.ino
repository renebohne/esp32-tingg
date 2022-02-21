#include <WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>


#define LED_PIN   12

#define DHTPIN 32     // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT11     // DHT 11

const char* ssid = networkSSID;
const char* password = networkPASSWORD;
const char* mqttServer = mqttSERVER;
const char* mqttUsername = mqttUSERNAME;
const char* mqttPassword = thingKEY;
const char* mqttDeviceId = thingID;

char subTopic[] = "ledControl";     //payload[0] will control/set LED
char pubTopic[] = "ledState";       //payload[0] will have ledState value
char temperatureTopic[] = "temperature";
char humidityTopic[] = "humidity";

WiFiClient wifiClient;
PubSubClient client(wifiClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int ledState = 0;

DHT_Unified dht(DHTPIN, DHTTYPE);


void setup_wifi() 
{
  delay(10);
  
  Serial.println();
  Serial.print("Connecting to wifi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("topic: ");
  Serial.print(topic);
  Serial.print(" message: ");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if ((char)payload[0] == '1') 
  {
    digitalWrite(LED_PIN, HIGH);   
    ledState = 1;
    char payLoad[1];
    itoa(ledState, payLoad, 10);
    client.publish(pubTopic, payLoad);
  } 
  else 
  {
    digitalWrite(LED_PIN, LOW); 
    ledState = 0;
    char payLoad[1];
    itoa(ledState, payLoad, 10);
    client.publish(pubTopic, payLoad);
  }

}

void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("connecting to ");
    Serial.print(mqttServer);
    Serial.print("...");
 
    if (client.connect(mqttDeviceId, mqttUsername, mqttPassword)) 
    {
      Serial.println("connected.");
      client.subscribe(subTopic);
    } else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() 
{
  pinMode(LED_PIN, OUTPUT);     
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);

  dht.begin();
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
}

void loop() 
{
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) 
  {
    lastMsg = now;
    char payLoad[1];
    itoa(ledState, payLoad, 10);
    client.publish(pubTopic, payLoad);

    // Get temperature event and print its value.
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature)) {
      Serial.println(F("Error reading temperature!"));
    }
    else {
      Serial.print(F("Temperature: "));
      float celsius = event.temperature;
      Serial.print(celsius);
      Serial.println(F("°C"));
      client.publish(temperatureTopic, String(celsius).c_str());
      
    }
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity)) {
      Serial.println(F("Error reading humidity!"));
    }
    else {
      Serial.print(F("Humidity: "));
      float humidity = event.relative_humidity;
      Serial.print(humidity);
      Serial.println(F("%"));
      client.publish(humidityTopic, String(humidity).c_str());
    }


    
  }
}
