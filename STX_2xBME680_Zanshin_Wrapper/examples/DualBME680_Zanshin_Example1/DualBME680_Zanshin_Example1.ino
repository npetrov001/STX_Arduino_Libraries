/* 2xBME680 example using Zanshin Wrapper */

#include <Wire.h>

#include <STX_2xBME680_Zanshin_Wrapper.h>

#define PIN_SCL            D1     // D1 & D2 work as default I2C
#define PIN_SDA            D2

STX_2xBME680_Zanshin_Wrapper DualBME680;

bool DualBME680_Started = false;

void setup()
{
    Serial.begin(115200);
    Serial.printf("\r\n\r\n");

    Serial.printf("Starting I2C... ms=%ld\r\n", millis());
    Wire.begin(PIN_SDA, PIN_SCL);
    delay(20);

    Serial.printf("Starting 2xBME680 via Zanshin Wrapper... ms=%ld\r\n", millis());
    DualBME680_Started = DualBME680.Start(false);

    if (DualBME680_Started)
    {
        Serial.printf(
            "BME680 ID1=%08lX ID2=%08lX ms=%ld\r\n",
            DualBME680.bosch_id1,
            DualBME680.bosch_id2,
            millis());
    }
}

long read_seq = 0;

void loop()
{
    if (DualBME680_Started)
    {
        read_seq++;

        // read both sensors
        DualBME680.Read();

        char sT1[20];
        char sH1[20];
        char sT2[20];
        char sH2[20];

        // Temperature & Humidity - width=4, precision=2
        dtostrf(DualBME680.T1, 4, 2, sT1);
        dtostrf(DualBME680.H1, 4, 2, sH1);
        dtostrf(DualBME680.T2, 4, 2, sT2);
        dtostrf(DualBME680.H2, 4, 2, sH2);

        int p1 = (int)DualBME680.P1;
        int p2 = (int)DualBME680.P2;

        int g1 = (int)DualBME680.G1;
        int g2 = (int)DualBME680.G2;

        // print message
        char sMsg[200];
        sprintf(
            sMsg,
            "SEQ=%ld T1=%s H1=%s T2=%s H2=%s P1=%d P2=%d - "
            "GasValid=%d G1=%d G2=%d - "
            "sec=%ld",
            read_seq, sT1, sH1, sT2, sH2, p1, p2,
            DualBME680.GasValid(), g1, g2,
            millis() / 1000);

        Serial.printf("%s\r\n", sMsg);
    }

    // read every ~30 sec
    delay(30 * 1000);
}
