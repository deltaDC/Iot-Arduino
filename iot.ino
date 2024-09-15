#include <DHT.h>
#define DHT22_PIN 23
#define LDR_PIN 2
#define LED_PIN 4

DHT dht22(DHT22_PIN, DHT22);

void setup() {
  Serial.begin(9600);
  dht22.begin(); // initialize the DHT22 sensor
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Blink LED
  Serial.println("LED high voltage");
  digitalWrite(LED_PIN, HIGH);
  delay(1000); // LED on for 1 second

  // Read humidity
  float humi = dht22.readHumidity();
  // Read temperature in Celsius
  float tempC = dht22.readTemperature();
  // Read temperature in Fahrenheit
  float tempF = dht22.readTemperature(true);

  int lightValue = analogRead(LDR_PIN);
  lightValue = 4095 - lightValue;

  float maxLux = 1000.0; // Replace this with the maximum lux value you measured
  float maxAdcValue = 4095.0; // The max ADC value for ESP32's 12-bit ADC

  float luxValue = (maxLux / maxAdcValue) * lightValue - 500;

  // Check whether the reading is successful or not
  if (isnan(tempC) || isnan(tempF) || isnan(humi)) {
    Serial.println("Failed to read from DHT22 sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(tempC);
    Serial.print("°C  ~  ");
    Serial.print(tempF);
    Serial.println("°F");
  }
  Serial.println("-----------------------------");
  if (isnan(luxValue)) {
    Serial.println("Failed to read from LDR sensor!");
  } else {
    Serial.print("Lux Value is: ");
    Serial.print(luxValue);
    Serial.println("");
  }

  delay(5000); // Delay before next loop iteration

  Serial.println("LED low voltage");
  digitalWrite(LED_PIN, LOW);
  delay(1000); // LED off for 1 second
}
