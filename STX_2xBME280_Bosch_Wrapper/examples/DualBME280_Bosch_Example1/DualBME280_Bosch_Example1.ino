/* 2xBME280 example using Bosch Wrapper */

#include <Wire.h>

#include <STX_2xBME280_Bosch_Wrapper.h>

#define PIN_SCL            D1     // D1 & D2 work as default I2C
#define PIN_SDA            D2

STX_2xBME280_Bosch_Wrapper DualBME280;
bool DualBME280_Started = false;

void setup()
{
    Serial.begin(115200);
    Serial.printf("\r\n\r\n");

    Serial.printf("Starting I2C... ms=%ld\r\n", millis());
    Wire.begin(PIN_SDA, PIN_SCL);
    delay(20); // 20ms delay

    Serial.printf("Starting 2xBME280 via Bosch Wrapper... ms=%ld\r\n", millis());
    DualBME280_Started = DualBME280.Start(false);
    
    if (DualBME280_Started)
    {
        Serial.printf(
            "BME280 ID1=%08lX ID2=%08lX ms=%ld\r\n",
            DualBME280.bosch_id1,
            DualBME280.bosch_id2,
            millis());
    }
    else
    {
        Serial.printf("ERROR: No BME280 sensors started.\r\n");
    }
}

long read_seq = 0;

void loop()
{
    if (DualBME280_Started)
    {
        read_seq++;

        // read BME280
        DualBME280.Read();

        char sT1[20];
        char sH1[20];
        char sT2[20];
        char sH2[20];

        // Temperature & Humidity - width=4, precision=2
        dtostrf(DualBME280.T1, 4, 2, sT1);
        dtostrf(DualBME280.H1, 4, 2, sH1);
        dtostrf(DualBME280.T2, 4, 2, sT2);
        dtostrf(DualBME280.H2, 4, 2, sH2);

        // pressure
        int p1 = (int)DualBME280.P1;
        int p2 = (int)DualBME280.P2;

        // print message
        char sMsg[200];
        sprintf(sMsg, "SEQ=%ld T1=%s H1=%s T2=%s H2=%s P1=%d P2=%d", read_seq, sT1, sH1, sT2, sH2, p1, p2);

        Serial.printf("%s\r\n", sMsg);
    }

    // read every 10 sec
    delay(10 * 1000);
}
