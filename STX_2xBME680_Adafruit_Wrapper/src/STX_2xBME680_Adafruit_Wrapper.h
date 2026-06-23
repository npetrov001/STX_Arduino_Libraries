#pragma once

#include "STX_MinMaxAvgDiff_Simple.h"

#include "Adafruit_BME680_i2c/Adafruit_BME680.h"

/******************************************************************************
STX_2xBME680_Adafruit_Wrapper
Reads two BME680 sensors using the Adafruit BME680 library.
Supports two BME680 sensors on the same I2C bus using addresses 0x76 and 0x77.
Compatible with the SimpleTronix 2xBME680 board or any two BME680 modules.

Notes:
- Gas heater is enabled immediately after startup.
- Every Read() performs a full BME680 measurement including gas.
- Gas readings are ignored during warmup period because the gas sensor
  requires time to stabilize after power-up.
******************************************************************************/

class STX_2xBME680_Adafruit_Wrapper
{
public:

    // return TRUE if sensors started OK
    // require_both = TRUE  - return FALSE if both sensors don't start
    // require_both = FALSE - return TRUE if only one sensor starts
    bool Start(bool require_both = true);

    // set maximum tolerances for Diff between temperature, humidity, pressure & gas
    void SetMaxDiffTolerance(
        int Tdiff = 4,
        int Hdiff = 15,
        int Pdiff = 100,
        int Gdiff = 5000);

    // read values
    void Read();

    // return TRUE if last readings are within tolerance diffs
    bool DiffToleranceOK_Last();

    // return TRUE if all readings since restart are within tolerance diffs
    bool DiffToleranceOK_Total();

    // get average T / H / P / G, ignore bad sensor readings
    float T_Avg();
    float H_Avg();
    float P_Avg();
    float G_Avg();

    // gas readings become valid after warmup period
    bool GasValid();

protected:

    // return ID or 0 if sensor did not start
    uint32_t _startSensor(Adafruit_BME680& sensor, uint8_t addr);
    uint8_t  _readChipID(uint8_t addr);
    uint32_t _readBoschID(uint8_t addr);
    void     _readSensor(uint32_t id, Adafruit_BME680& sensor, float& T, float& H, float& P, float& G);
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
    
    float G1 = -404;
    float G2 = -404;

    // tolerances which we check
    int MaxTdiff = 4;
    int MaxHdiff = 15;
    int MaxPdiff = 100;
    
    // gas can differ a lot between sensors.  
    // 2xBME680 boards have sensors from the same batches
    // so the difference should be smaller.
    int MaxGdiff = 5000; 

    STX_MinMaxAvgDiff_Simple Tdiff;
    STX_MinMaxAvgDiff_Simple Hdiff;
    STX_MinMaxAvgDiff_Simple Pdiff;
    STX_MinMaxAvgDiff_Simple Gdiff;

    // gas sensor requires warmup after power-up
    uint32_t StartMs = 0;

    // default = 5 minutes
    uint32_t GasWarmupMs = 300000;

public:

    Adafruit_BME680 sensor1;
    Adafruit_BME680 sensor2;
};