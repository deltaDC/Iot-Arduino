#include <WiFi.h>          // For WiFi connection
#include <PubSubClient.h>   // For MQTT
#include <DHT.h>

#define DHT22_PIN 23
#define LDR_PIN 32           // Keep using GPIO2 (D2)
#define LED_PIN 5
#define LED_PIN_2 18
#define LED_PIN_3 19

// DHT sensor
DHT dht22(DHT22_PIN, DHT22);

// WiFi and MQTT settings
const char* ssid = "Duc Chinh";        
const char* password = "ducchinh13122003"; 
const char* mqtt_server = "192.168.1.93"; 
const char* mqtt_username = "b21dccn181"; 
const char* mqtt_password = "b21dccn181";

WiFiClient espClient;
PubSubClient client(espClient);

bool blinkLED = false;        // Flag to track blinking state
unsigned long previousMillis = 0;  // Store the last time the LED was updated
const long interval = 500;    // Interval for blinking (in milliseconds)

void publishLedState(int ledPin, const char* ledName, bool state) {
  String payload = "{\"led\":\"" + String(ledName) + "\", \"state\":\"" + (state ? "ON" : "OFF") + "\"}";
  client.publish("led/status", payload.c_str());
}

void publishAlertState(String message) {
  client.publish("led/status", message.c_str());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Convert payload to a String
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  Serial.println(message);

  // Check the message and control LEDs accordingly
  if (message == "LED1 ON") {
    digitalWrite(LED_PIN, HIGH);  // Turn LED 1 on
    publishLedState(LED_PIN, "LED1", true);  // Publish status after toggling
  } else if (message == "LED1 OFF") {
    digitalWrite(LED_PIN, LOW);   // Turn LED 1 off
    publishLedState(LED_PIN, "LED1", false); // Publish status after toggling
  } else if (message == "LED2 ON") {
    digitalWrite(LED_PIN_2, HIGH);  // Turn LED 2 on
    publishLedState(LED_PIN_2, "LED2", true);  // Publish status after toggling
  } else if (message == "LED2 OFF") {
    digitalWrite(LED_PIN_2, LOW);   // Turn LED 2 off
    publishLedState(LED_PIN_2, "LED2", false);  // Publish status after toggling
  } else if (message == "LED3 ON") {
    digitalWrite(LED_PIN_3, HIGH);  // Turn LED 3 on
    publishLedState(LED_PIN_3, "LED3", true);  // Publish status after toggling
  } else if (message == "LED3 OFF") {
    digitalWrite(LED_PIN_3, LOW);   // Turn LED 3 off
    publishLedState(LED_PIN_3, "LED3", false);  // Publish status after toggling
  } else if (message == "ON ALL") {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_PIN_2, HIGH);  
    digitalWrite(LED_PIN_3, HIGH);  
  } else if (message == "OFF ALL") {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN_2, LOW);  
    digitalWrite(LED_PIN_3, LOW);
  } else if (message == "LED BLINK") {
    publishAlertState("LED BLINK");
    blinkLED = true;
  } else if (message == "LED NOT BLINK") {
    publishAlertState("LED NOT BLINK");
    blinkLED = false;  // Stop blinking
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_PIN_2, LOW);
    digitalWrite(LED_PIN_3, LOW);
  } else {
    Serial.println("Unknown command");
  }
}


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(2000);
    Serial.println("WiFi not connected");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Print the IP address
}

String generateRandomClientId() {
  String clientId = "ESP32Client-";
  clientId += String(random(10000)); // Append a random number
  return clientId;
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = generateRandomClientId();
    Serial.print("Attempting MQTT connection with ID: ");
    Serial.println(clientId);
    
    // Print WiFi status
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
    Serial.println(WiFi.localIP());

    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      client.subscribe("led/control");  // Subscribe to the control topic
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.print(" (");
      Serial.print(getMqttErrorMessage(client.state()));
      Serial.println(")");
      delay(5000);
    }
  }
}

String getMqttErrorMessage(int code) {
  switch (code) {
    case -1: return "Connect failed";
    case -2: return "Connect failed, no network";
    case -3: return "Connect failed, invalid protocol";
    case -4: return "Connect failed, invalid client ID";
    case -5: return "Connect failed, invalid credentials";
    case -6: return "Connect failed, invalid topic";
    case -7: return "Connect failed, invalid payload";
    case -8: return "Connect failed, invalid QoS";
    case -9: return "Connect failed, invalid retain flag";
    default: return "Unknown error";
  }
}

void setup() {
  Serial.begin(9600);
  dht22.begin(); // Initialize the DHT22 sensor
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);

  setup_wifi();            // Connect to WiFi
  client.setServer(mqtt_server, 1884);  // Set MQTT broker
  client.setCallback(callback);         // Set callback function
}

// Global variables for tracking timing
unsigned long previousDataMillis = 0; // Store the last time data was sent
const long dataInterval = 5000;       // Interval for sending data (in milliseconds)

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Get the current time
  unsigned long currentMillis = millis();

  // Handle LED blinking
  if (blinkLED) {
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;  // Save the current time
      int ledState = digitalRead(LED_PIN); 
      int ledState2 = digitalRead(LED_PIN_2);
      int ledState3 = digitalRead(LED_PIN_3);
      digitalWrite(LED_PIN, !ledState);  // Toggle the LED state
      digitalWrite(LED_PIN_2, !ledState2);
      digitalWrite(LED_PIN_3, !ledState3);
    }
  }

  // Handle data publishing every 5 seconds
  if (currentMillis - previousDataMillis >= dataInterval) {
    previousDataMillis = currentMillis; // Save the current time

    // Read humidity and temperature
    float humi = dht22.readHumidity();
    float tempC = dht22.readTemperature();

    // Check if the sensor readings are valid
    if (isnan(humi) || isnan(tempC)) {
      Serial.println("Failed to read from DHT22 sensor!");
      return; // Skip this loop iteration if readings are invalid
    }

    // Read light sensor value
    int lightValue = analogRead(LDR_PIN);

    // Verify the lightValue range
    if (lightValue < 0 || lightValue > 4095) {
      Serial.println("Invalid light value");
      lightValue = 0; // Reset to a default value if out of range
    }

    lightValue = 4095 - lightValue;

    // Calculate lux value
    float maxLux = 1000.0;
    float maxAdcValue = 4095.0;
    float luxValue = (maxLux / maxAdcValue) * lightValue;

    float someData = random(0, 101);

    // Create JSON payload
    String payload = "{\"name\":\"DHT22 and LDR\", \"data\": {"
                     "\"temperature\":{\"value\":" + String(tempC) + ",\"unit\":\"Celsius\"},"
                     "\"humidity\":{\"value\":" + String(humi) + ",\"unit\":\"Percentage\"},"
                     "\"brightness\":{\"value\":" + String(luxValue) + ",\"unit\":\"Lux\"},"
                     "\"someData\":{\"value\":" + String(someData) + ",\"unit\":\"Unit\"}}}";

    Serial.println("Publishing MQTT message: ");
    Serial.println(payload);

    // Publish to the MQTT topic
    client.publish("iot-data", payload.c_str());
  }
}


// void loop() {
//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();

//   // Add delay to avoid conflict with WiFi operations
//   delay(100); // Short delay before reading analog values

//   // Handle LED blinking
//   if (blinkLED) {
//     unsigned long currentMillis = millis();
    
//     // Check if it's time to toggle the LED state
//     if (currentMillis - previousMillis >= interval) {
//       previousMillis = currentMillis;  // Save the current time
//       int ledState = digitalRead(LED_PIN); 
//       digitalWrite(LED_PIN, !ledState);  // Toggle the LED state
//       digitalWrite(LED_PIN_2, !ledState);
//       digitalWrite(LED_PIN_3, !ledState);
//     }
//   }

//   // Read humidity and temperature
//   float humi = dht22.readHumidity();
//   float tempC = dht22.readTemperature();

//   // Check if the sensor readings are valid
//   if (isnan(humi) || isnan(tempC)) {
//     Serial.println("Failed to read from DHT22 sensor!");
//     return; // Skip this loop iteration if readings are invalid
//   }

//   // Read light sensor value
//   int lightValue = analogRead(LDR_PIN);

//   // Verify the lightValue range
//   if (lightValue < 0 || lightValue > 4095) {
//     Serial.println("Invalid light value");
//     lightValue = 0; // Reset to a default value if out of range
//   }

//   lightValue = 4095 - lightValue;

//   // Calculate lux value
//   float maxLux = 1000.0;
//   float maxAdcValue = 4095.0;
//   float luxValue = (maxLux / maxAdcValue) * lightValue;

//   float someData = random(0, 101);

//   // Create JSON payload
//   String payload = "{\"name\":\"DHT22 and LDR\", \"data\": {"
//                    "\"temperature\":{\"value\":" + String(tempC) + ",\"unit\":\"Celsius\"},"
//                    "\"humidity\":{\"value\":" + String(humi) + ",\"unit\":\"Percentage\"},"
//                    "\"brightness\":{\"value\":" + String(luxValue) + ",\"unit\":\"Lux\"},"
//                    "\"someData\":{\"value\":" + String(someData) + ",\"unit\":\"Unit\"}}}";


//   Serial.println("Publishing MQTT message: ");
//   Serial.println(payload);

//   // Publish to the MQTT topic
//   client.publish("iot-data", payload.c_str());

//   delay(5000); // Wait 5 seconds before sending next reading
// }
