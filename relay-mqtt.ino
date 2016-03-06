/*
 *  This code switches a relay connected to port 5 (GPIO5) on an ESP8266.
 *
 *  It will connect to MQTT and listens on topic demo/smart/socket/1/relay
 *  for 'on' and 'off' commands. Every 60 seconds, it will publishes te current
 */


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int RelayPin = 5;    // RELAY connected to digital pin 5

const char* ssid     = "YOUR_OWN_SSID";
const char* password = "YOUR_OWN_PASSWORD";

//change with your MQTT server IP address
const char* mqtt_server = "10.0.0.1";


WiFiClient espClient;
PubSubClient client(espClient);


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print(topic);
  Serial.print(" => ");

  char* payload_str;
  payload_str = (char*) malloc(length + 1);
  memcpy(payload_str, payload, length);
  payload_str[length] = '\0';
  Serial.println(String(payload_str));
  
  if ( String(topic) == "demo/smart/socket/1/relay" ) {
    if (String(payload_str) == "on" ) {
      digitalWrite(RelayPin, HIGH);   // turn the RELAY on
      client.publish("demo/smart/socket/1/relay_state","on");
    } else if ( String(payload_str) == "off" ) {
      digitalWrite(RelayPin, LOW);    // turn the RELAY off
      client.publish("demo/smart/socket/1/relay_state","off");
    } else {
      Serial.print("I do not know what to do with ");
      Serial.print(String(payload_str));
      Serial.print(" on topic ");
      Serial.println( String(topic));
    }
  }
}

void connect_to_MQTT() {
 client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  if (client.connect("thermostat_relay")) {
    Serial.println("(re)-connected to MQTT");
    client.subscribe("demo/smart/socket/1/relay");
  } else {
    Serial.println("Could not connect to MQTT");
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // Connecting to our WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  connect_to_MQTT();

  // initialize pin 5, where the relay is connected to.
  pinMode(RelayPin, OUTPUT);
}

int tellstate = 0;

void loop() {
  client.loop();

  if (! client.connected()) {
    Serial.println("Not connected to MQTT....");
    connect_to_MQTT();
    delay(5000);
  }

  // Tell the current state every 60 seconds
  if ( (millis() - tellstate) > 60000 ) {
    if ( digitalRead(RelayPin) ) {
       client.publish("demo/smart/socket/1/relay_state","on");
    } else {
      client.publish("demo/smart/socket/1/relay_state","off");
    }
    tellstate = millis();
  }
}
