#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "MQTT_Client.h"

// =========================================================================================
// MQTT Setup
// =========================================================================================

#define TOPIC_MQTT_SUBSCRIBE "/smart_energy/subscribe/client/a32ab0af-970c-11ec-9779-a463a116a9e2"
#define TOPIC_MQTT_PUBLISH "/smart_energy/publish/client/a32ab0af-970c-11ec-9779-a463a116a9e2"
#define ID_MQTT "a32ab0af-970c-11ec-9779-a463a116a9e2"

const char *SSID = "dev severino";
const char *PASSWORD = "17210000";

const char *BROKER_MQTT = "broker.hivemq.com";
int BROKER_PORT = 1883;

WiFiClient wifiClient;
PubSubClient MQTT(wifiClient);

#define SIZE_PAYLOAD 64
char payload[SIZE_PAYLOAD];

// =========================================================================================
// Energy consumption
// =========================================================================================

#define NUMBER_OF_SAMPLES 5 * 64 // 5 periods with 64 samples each
#define I_VCC 3.3f
#define ADC_BITS 10
#define ADC_COUNTS (1 << ADC_BITS)
#define CURRENT_CONVERSION_CONSTANT (I_VCC / ADC_COUNTS) * 0.10f
#define INPUT_PIN_ANALOG A0
#define VOLTAGE 220

double offsetI = 484.0; //Low-pass filter output [ADC_COUNTS >> 1]
double sumI = 0.0f;
double filteredI;
double power;
unsigned int n = 0;
int currentWaveForm[NUMBER_OF_SAMPLES] = {
    336, 336, 336, 335, 335, 335, 335, 335, 335, 335, 335, 334, 334, 335, 352, 369, 353, 347,
    339, 330, 330, 329, 330, 330, 329, 330, 330, 330, 330, 330, 330, 330, 330, 331, 331, 330,
    332, 296, 306, 315, 321, 333, 335, 336, 335, 335, 335, 335, 335, 335, 335, 335, 335, 335,
    335, 335, 334, 335, 335, 340, 369, 355, 349, 340, 331, 330, 330, 330, 330, 330, 330, 330,
    330, 330, 330, 330, 330, 330, 331, 331, 331, 331, 300, 304, 312, 320, 330, 335, 336, 335,
    335, 336, 335, 335, 335, 335, 335, 335, 335, 335, 335, 335, 335, 334, 334, 371, 357, 350,
    340, 331, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 331, 331, 331, 331,
    331, 305, 303, 311, 318, 329, 335, 335, 335, 335, 335, 336, 335, 335, 335, 335, 335, 335,
    335, 335, 334, 334, 334, 334, 371, 357, 350, 342, 330, 330, 330, 330, 330, 330, 330, 330,
    330, 330, 330, 330, 330, 330, 331, 331, 331, 332, 312, 301, 312, 317, 329, 335, 335, 335,
    335, 335, 335, 336, 335, 335, 335, 335, 335, 335, 335, 335, 334, 335, 334, 371, 359, 351,
    343, 331, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 331, 331, 331, 332,
    331, 322, 298, 310, 318, 327, 335, 335, 335, 335, 335, 336, 336, 335, 335, 335, 335, 335,
    335, 335, 334, 335, 334, 334, 371, 359, 351, 344, 332, 330, 330, 330, 330, 330, 330, 330,
    330, 330, 330, 330, 330, 331, 331, 331, 330, 330, 328, 297, 309, 315, 324, 335, 336, 336,
    335, 335, 335, 335, 335, 335, 335, 335, 335, 335, 335, 334, 334, 335, 333, 364, 360, 352,
    345, 333, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 330, 331, 331, 331, 331,
    331, 333, 295, 309, 314, 325, 335, 335, 336, 335, 335, 336, 335, 335};

// =========================================================================================
// Prototypes
// =========================================================================================

void initWiFi();

void initMQTT();

void reconnectWiFi();

void checkConnection(void);

void mqtt_callback(char *topic, byte *payload, unsigned int length);

void process_wave_form();

// =========================================================================================
// Arduino functions
// =========================================================================================

void setup()
{
    Serial.begin(115200);
    initWiFi();
    initMQTT();
}

void loop()
{
    checkConnection();

    process_wave_form();

    DynamicJsonDocument dataJson(SIZE_PAYLOAD);
    dataJson["power"] = power;
    dataJson["device_id"] = WiFi.macAddress();
    // JsonArray waveForm = dataJson.createNestedArray("waveForm");
    // for (n = 0; n < NUMBER_OF_SAMPLES; n++)
    // {
    //     waveForm.add(currentWaveForm[n]);
    // }
    serializeJson(dataJson, payload);

    Serial.println(payload);
    MQTT.publish(TOPIC_MQTT_PUBLISH, payload);

    delay(1000);
    MQTT.loop();
}

// =========================================================================================
// Implementations
// =========================================================================================

void initWiFi()
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconnectWiFi();
}

void initMQTT()
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    String msg;
    for (int i = 0; i < length; i++)
    {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.println(msg);
}

void reconnectWiFi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void reconnectMQTT()
{
    while (!MQTT.connected())
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT))
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPIC_MQTT_SUBSCRIBE);
        }
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}

void checkConnection(void)
{
    if (!MQTT.connected())
    {
        reconnectMQTT();
    }
    reconnectWiFi();
}

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

    power = VOLTAGE * (CURRENT_CONVERSION_CONSTANT * sqrt(sumI / NUMBER_OF_SAMPLES));
    sumI = 0;
}
