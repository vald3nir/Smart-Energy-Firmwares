// =========================================================================================
// ESP32 Libraries
// =========================================================================================

#include <Arduino.h>
#include <ArduinoJson.h>
#include <driver/adc.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>

// =========================================================================================
// Constants and Variables
// =========================================================================================

#define NUMBER_OF_SAMPLES 5 * 64 // 5 periods with 64 samples each
#define I_VCC 3.3f
#define ADC_BITS 10
#define ADC_COUNTS (1 << ADC_BITS)
#define CURRENT_CONVERSION_CONSTANT (I_VCC / ADC_COUNTS) * 0.10f
#define INPUT_PIN_ANALOG 34

// =========================================================================================

double offsetI = 484.0; //Low-pass filter output [ADC_COUNTS >> 1]
double sumI = 0.0f;
double filteredI;
double power;
unsigned int n = 0;
int currentWaveForm[NUMBER_OF_SAMPLES];

// =========================================================================================

const char *ssid = "severino";
const char *password = "17210000";
const char *serverName = "http://192.168.0.40:1880/time_series";

#define SIZE_PAYLOAD 4098
char payload[SIZE_PAYLOAD];

// =========================================================================================

void setup()
{
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  analogReadResolution(10);

  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

// =========================================================================================

void process_wave_form()
{
  for (n = 0; n < NUMBER_OF_SAMPLES; n++)
  {
    currentWaveForm[n] = analogRead(INPUT_PIN_ANALOG);
    delayMicroseconds(148);
  }

  for (n = 0; n < NUMBER_OF_SAMPLES; n++) // Current RMS
  {
    offsetI = (offsetI + (currentWaveForm[n] - offsetI) / 1024);
    filteredI = currentWaveForm[n] - offsetI;
    sumI += (filteredI * filteredI);
  }

  power = 220 * (CURRENT_CONVERSION_CONSTANT * sqrt(sumI / NUMBER_OF_SAMPLES));
  sumI = 0;
}

// =========================================================================================

void loop()
{

  process_wave_form();
  // Serial.println("offsetI: " + String(offsetI, 6));
  Serial.println("power: " + String(power, 6));

  DynamicJsonDocument JSONencoder(SIZE_PAYLOAD);
  JSONencoder["power"] = power;
  JSONencoder["device_id"] = WiFi.macAddress();
  JsonArray wave_form = JSONencoder.createNestedArray("wave_form");
  for (n = 0; n < NUMBER_OF_SAMPLES; n++)
  {
    wave_form.add(currentWaveForm[n]);
  }

  serializeJson(JSONencoder, payload);
  // Serial.println(payload);

  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(payload);
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    http.end();
  }

  delay(1000);
}
