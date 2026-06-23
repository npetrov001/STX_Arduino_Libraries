#pragma once

#include "STX_MinMaxAvgDiff_Simple.h"

#include "Adafruit_BME280_i2c/Adafruit_BME280.h"

/******************************************************************************
STX_2xBME280_Adafruit_Wrapper
Reads two BME280 sensors using the Adafruit BME280 library.
Supports two BME280 sensors on the same I2C bus using addresses 0x76 and 0x77. 
Compatible with the SimpleTronix 2xBME280 board or any two BME280 modules.
******************************************************************************/

class STX_2xBME280_Adafruit_Wrapper
{
public:
    // return TRUE if sensors started OK
    // require_both = TRUE  - return FALSE if both sensors don't start
    // require_both = FALSE - return TRUE if only one sensor starts
    bool Start(bool require_both = true);

    // set maximum tolerances for Diff between temperature, humidity & pressure
    void SetMaxDiffTolerance(int Tdiff = 4, int Hdiff = 15, int Pdiff = 100);

    // read values
    void Read();

    // return TRUE if last readings are within tolerance diffs
    bool DiffToleranceOK_Last();

    // return TRUE if all readings since restart are within tolerance diffs
    bool DiffToleranceOK_Total();

    // get average T / H / P, ignore bad sensor readings
    float T_Avg();
    float H_Avg();
    float P_Avg();

protected:
    // return ID or 0 if sensor did not start
    uint32_t _startSensor(Adafruit_BME280& sensor, uint8_t addr);
    uint32_t _readBoschID(uint8_t addr);
    void     _readSensor(uint32_t id, Adafruit_BME280& sensor, float& T, float& H, float& P);
    float    _avg(float v1, float v2);
    bool     _diff_ok(float v, float maxv);

public:
    bool require_both_sensors = true;

    // sensor IDs read from register 0x83
    uint32_t bosch_id1 = 0;
    uint32_t bosch_id2 = 0;

    // readings at < -400 are considered bad
    float T1 = -404;  // temperature (F)
    float H1 = -404;  // humidity (%)
    float P1 = -404;  // pressure (Pa)

    float T2 = -404;  // temperature (F)
    float H2 = -404;  // humidity (%)
    float P2 = -404;  // pressure (Pa)

    // tolerances which we check
    int MaxTdiff = 4;
    int MaxHdiff = 15;
    int MaxPdiff = 100;

    // diff checkers
    STX_MinMaxAvgDiff_Simple Tdiff;
    STX_MinMaxAvgDiff_Simple Hdiff;
    STX_MinMaxAvgDiff_Simple Pdiff;

public:
    // SparkFun BME280 sensor objects
    Adafruit_BME280 sensor1;
    Adafruit_BME280 sensor2;
};
