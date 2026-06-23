
#include <Wire.h>             // I2C connection

#include "STX_2xBME280_SparkFun_Wrapper.h"

// return TRUE if sensors started OK
// require_both = true:
//   both sensors must initialize
// require_both = false:
//   only one sensor must initialize
bool STX_2xBME280_SparkFun_Wrapper::Start(bool require_both /*= true*/)
{
    this->require_both_sensors = require_both;
    sensor1.reset();
    sensor2.reset();
    delay(20);
    this->bosch_id1 = _startSensor(sensor1, 0x76);
    this->bosch_id2 = _startSensor(sensor2, 0x77);
    if (require_both)
    {
        return (bosch_id1 != 0 && bosch_id2 != 0);
    }
    else
    {
        return (bosch_id1 != 0 || bosch_id2 != 0);
    }
}

// set maximum tolerances for Diff between temperature, humidity & pressure
void STX_2xBME280_SparkFun_Wrapper::SetMaxDiffTolerance(int Tdiff /*= 4*/, int Hdiff /*= 15*/, int Pdiff /*= 100*/)
{
    this->MaxTdiff = Tdiff;
    this->MaxHdiff = Hdiff;
    this->MaxPdiff = Pdiff;
}

// read values
void STX_2xBME280_SparkFun_Wrapper::Read()
{
    _readSensor(bosch_id1, sensor1, T1, H1, P1);
    _readSensor(bosch_id2, sensor2, T2, H2, P2);

    if (T1 > -400 && T2 > -400)
        Tdiff.Push(T1, T2);

    if (H1 > -400 && H2 > -400)
        Hdiff.Push(H1, H2);

    if (P1 > -400 && P2 > -400)
        Pdiff.Push(P1, P2);
}

// return TRUE if last readings are within tolerance diffs
bool STX_2xBME280_SparkFun_Wrapper::DiffToleranceOK_Last()
{
    if (bosch_id1 == 0 || bosch_id2 == 0)
    {
        // one of the sensors is not working
        return false;
    }
    else
    {
        return 
            _diff_ok(Tdiff.last_diff, MaxTdiff) &&
            _diff_ok(Hdiff.last_diff, MaxHdiff) &&
            _diff_ok(Pdiff.last_diff, MaxPdiff);
    }
}

// return TRUE if all readings since restart are within tolerance diffs
bool STX_2xBME280_SparkFun_Wrapper::DiffToleranceOK_Total()
{
    if (bosch_id1 == 0 || bosch_id2 == 0)
    {
        // one of the sensors is not working
        return false;
    }
    else
    {
        // check both - min & max diffs
        return
            _diff_ok(Tdiff.min_diff, MaxTdiff) &&
            _diff_ok(Hdiff.min_diff, MaxHdiff) &&
            _diff_ok(Pdiff.min_diff, MaxPdiff) &&
            _diff_ok(Tdiff.max_diff, MaxTdiff) &&
            _diff_ok(Hdiff.max_diff, MaxHdiff) &&
            _diff_ok(Pdiff.max_diff, MaxPdiff);
    }
}

// get average T / H / P, ignore bad sensor readings
float STX_2xBME280_SparkFun_Wrapper::T_Avg() { return _avg(T1, T2); }
float STX_2xBME280_SparkFun_Wrapper::H_Avg() { return _avg(H1, H2); }
float STX_2xBME280_SparkFun_Wrapper::P_Avg() { return _avg(P1, P2); }


// return ID or 0 if sensor did not start
// print error on serial if wrong sensor (BME280 & BME680 have same I2C ID yet different chip IDs)
uint32_t STX_2xBME280_SparkFun_Wrapper::_startSensor(
    BME280& sensor,
    uint8_t addr)
{
    sensor.setI2CAddress(addr);

    bool started_ok = sensor.beginI2C();

    if (!started_ok)
    {
        uint8_t chip_id = sensor.readRegister(BME280_CHIP_ID_REG);

        if (chip_id == 0x61)
        {
            Serial.printf(
                "ERROR: BME680 detected at addr=0x%X. "
                "Use 2xBME680 Wrapper instead.\r\n",
                addr);
        }
        else if (chip_id != 0)
        {
            Serial.printf(
                "ERROR: Unsupported chip ID 0x%X at addr=0x%X\r\n",
                chip_id,
                addr);
        }
        else
        {
            Serial.printf(
                "ERROR: Sensor not detected at addr=0x%X.\r\n",
                addr);
        }

        return 0;
    }

    uint32_t id = 0;

    sensor.readRegisterRegion((uint8_t*)&id, 0x83, 4);

    if (id != 0)
    {
        sensor.setTempOverSample(8);
        sensor.setPressureOverSample(8);
        sensor.setHumidityOverSample(8);
        sensor.setFilter(8);
    }

    return id;
}

void STX_2xBME280_SparkFun_Wrapper::_readSensor(uint32_t id, BME280& sensor, float& T, float& H, float& P)
{
    if (id == 0)
    {
        // sensor not initialized, store bad readings
        T = -404;
        H = -404;
        P = -404;
    }
    else
    {
        BME280_SensorMeasurements measurements;
        sensor.readAllMeasurements(&measurements, 1);
        T = measurements.temperature;
        H = measurements.humidity;
        P = measurements.pressure;
    }
}

float STX_2xBME280_SparkFun_Wrapper::_avg(float v1, float v2)
{
    // values < -400 are invalid
    if (v1 < -400)
    {
        return v2;
    }
    else if (v2 < -400)
    {
        return v1;
    }
    else
    {
        // both values are good
        return (v1 + v2) / 2;
    }
}

bool STX_2xBME280_SparkFun_Wrapper::_diff_ok(float v, float maxv)
{
    if (v < 0) v = -v;
    return v < maxv;
}
