#include <Arduino.h>

// =========================================================================================
// Definitions
// =========================================================================================

#define NUMBER_OF_SAMPLES 5 * 64 // 5 periods with 64 samples each

#define ADC_BITS 10
#define ADC_COUNTS (1 << ADC_BITS)
#define VCC 3.3f
#define CURRENT_CONVERSION_CONSTANT (VCC / ADC_COUNTS) * 0.15f

#define INPUT_PIN A0

#define ACTIVATION_CHARACTER 116 //'t'

// =========================================================================================

double offsetI = 340.0f; //Low-pass filter output [ADC_COUNTS >> 1]

double sumI = 0.0f;
double filteredI;
double power;
unsigned int n = 0;
int currentWaveForm[NUMBER_OF_SAMPLES];

// =========================================================================================

void process_wave_form()
{
  for (n = 0; n < NUMBER_OF_SAMPLES; n++)
  {
    currentWaveForm[n] = analogRead(INPUT_PIN);
    delayMicroseconds(148);
  }

  for (n = 0; n < NUMBER_OF_SAMPLES; n++) // Current RMS and Voltage RMS
  {
    offsetI = (offsetI + (currentWaveForm[n] - offsetI) / 1024);
    filteredI = currentWaveForm[n] - offsetI;
    sumI += (filteredI * filteredI);
  }

  power = 220 * (CURRENT_CONVERSION_CONSTANT * sqrt(sumI / NUMBER_OF_SAMPLES));
  sumI = 0;
}

void setup()
{
  Serial.begin(115200);
}

void loop()
{

  process_wave_form();
  // Serial.println("offsetI: " + String(offsetI, 6));
  // Serial.println("power: " + String(power, 6));
  // Serial.println("\n");
  // delay(250);

  if (Serial.available() > 0 && Serial.read() == ACTIVATION_CHARACTER)
  {

    process_wave_form();

    for (n = 0; n < NUMBER_OF_SAMPLES; n++)
    {
      Serial.println(currentWaveForm[n]);
    }
  }
}
