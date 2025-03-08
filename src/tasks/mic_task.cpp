#include "mic_task.h"
#include <Arduino.h>


#define MIC_PIN 3  // GPIO pin connected to the microphone analog output


void mic_task(void *pvParam)
{
    analogReadResolution(12);  // Set ADC resolution (12-bit is typical)

    while (1)
    {
        int micValue = analogRead(MIC_PIN);  // Read the analog microphone signal

    Serial.println(micValue);  // Print the value to the Serial monitor

    delay(10);  // Small delay to reduce Serial output speed
    }
}