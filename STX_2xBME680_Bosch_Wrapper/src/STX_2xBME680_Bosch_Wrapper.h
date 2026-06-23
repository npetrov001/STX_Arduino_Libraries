#pragma once

#include "STX_MinMaxAvgDiff_Simple.h"

#include "Bosch_BME68x_copy/bme68xLibrary.h"

/******************************************************************************
STX_2xBME680_Bosch_Wrapper

Reads two BME680 sensors using the Bosch BME68x library.
Supports sensors at 0x76 and 0x77.

Temperature returned in Fahrenheit.
Pressure returned in Pascals.
Humidity returned in %RH.
Gas returned in Ohms.
******************************************************************************/

class STX_2xBME680_Bosch_Wrapper
{
public:

    bool Start(bool require_both = true);

    void SetMaxDiffTolerance(
        int Tdiff = 4,
        int Hdiff = 15,
        int Pdiff = 100,
        int Gdiff = 5000);

    void Read();

    bool DiffToleranceOK_Last();
    bool DiffToleranceOK_Total();

    float T_Avg();
    float H_Avg();
    float P_Avg();
    float G_Avg();

    bool GasValid();

protected:

    uint32_t _startSensor(Bme68x& sensor, uint8_t addr);
    uint8_t  _readChipID(uint8_t addr);

    void _readSensor(
        uint32_t id,
        Bme68x& sensor,
        float& T,
        float& H,
        float& P,
        float& G);

    float _avg(float v1, float v2);
    bool _diff_ok(float v, float maxv);

public:

    bool require_both_sensors = true;

    uint32_t bosch_id1 = 0;
    uint32_t bosch_id2 = 0;

    float T1 = -404;
    float H1 = -404;
    float P1 = -404;

    float T2 = -404;
    float H2 = -404;
    float P2 = -404;

    float G1 = -404;
    float G2 = -404;

    int MaxTdiff = 4;
    int MaxHdiff = 15;
    int MaxPdiff = 100;
    int MaxGdiff = 5000;

    STX_MinMaxAvgDiff_Simple Tdiff;
    STX_MinMaxAvgDiff_Simple Hdiff;
    STX_MinMaxAvgDiff_Simple Pdiff;
    STX_MinMaxAvgDiff_Simple Gdiff;

    uint32_t StartMs = 0;

    // default 5 minutes
    uint32_t GasWarmupMs = 300000;

public:

    Bme68x sensor1;
    Bme68x sensor2;
};
