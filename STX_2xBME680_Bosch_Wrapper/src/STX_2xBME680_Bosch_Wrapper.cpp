#include <Wire.h>

#include "STX_2xBME680_Bosch_Wrapper.h"

bool STX_2xBME680_Bosch_Wrapper::Start(bool require_both)
{
    this->require_both_sensors = require_both;

    StartMs = millis();

    bosch_id1 = _startSensor(sensor1, 0x76);
    bosch_id2 = _startSensor(sensor2, 0x77);

    if (require_both)
    {
        return (bosch_id1 && bosch_id2);
    }

    return (bosch_id1 || bosch_id2);
}

void STX_2xBME680_Bosch_Wrapper::SetMaxDiffTolerance(
    int Tdiff,
    int Hdiff,
    int Pdiff,
    int Gdiff)
{
    MaxTdiff = Tdiff;
    MaxHdiff = Hdiff;
    MaxPdiff = Pdiff;
    MaxGdiff = Gdiff;
}

void STX_2xBME680_Bosch_Wrapper::Read()
{
    _readSensor(bosch_id1, sensor1, T1, H1, P1, G1);
    _readSensor(bosch_id2, sensor2, T2, H2, P2, G2);

    if (T1 > -400 && T2 > -400)
        Tdiff.Push(T1, T2);

    if (H1 > -400 && H2 > -400)
        Hdiff.Push(H1, H2);

    if (P1 > -400 && P2 > -400)
        Pdiff.Push(P1, P2);

    if (GasValid() && G1 > 0 && G2 > 0)
        Gdiff.Push(G1, G2);
}

bool STX_2xBME680_Bosch_Wrapper::GasValid()
{
    return (millis() - StartMs) >= GasWarmupMs;
}

uint32_t STX_2xBME680_Bosch_Wrapper::_startSensor(
    Bme68x& sensor,
    uint8_t addr)
{
    sensor.begin(addr, Wire);

    if (sensor.checkStatus() == BME68X_ERROR)
    {
        uint8_t chip_id = _readChipID(addr);

        if (chip_id == 0x60)
        {
            Serial.printf(
                "ERROR: BME280 detected at addr=0x%X. "
                "Use 2xBME280 Wrapper instead.\r\n",
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
                "ERROR: Sensor not detected at addr=0x%X\r\n",
                addr);
        }

        return 0;
    }

    // equivalent settings to Adafruit wrapper
    sensor.setTPH(
        BME68X_OS_8X,
        BME68X_OS_4X,
        BME68X_OS_2X);

    sensor.setFilter(BME68X_FILTER_SIZE_3);

    // enable heater immediately
    sensor.setHeaterProf(320, 150);

    return sensor.getUniqueId();
}

uint8_t STX_2xBME680_Bosch_Wrapper::_readChipID(uint8_t addr)
{
    Wire.beginTransmission(addr);
    Wire.write(0xD0);
    Wire.endTransmission();

    if (Wire.requestFrom(addr, (uint8_t)1) != 1)
    {
        return 0;
    }

    return Wire.read();
}

void STX_2xBME680_Bosch_Wrapper::_readSensor(
    uint32_t id,
    Bme68x& sensor,
    float& T,
    float& H,
    float& P,
    float& G)
{
    if (id == 0)
    {
        T = H = P = G = -404;
        return;
    }

    bme68xData data;

    sensor.setOpMode(BME68X_FORCED_MODE);

    // Adafruit: delay is sensor.getMeasDur() + sensor.getHeaterConfiguration().heatr_dur * 1000UL
    delayMicroseconds(sensor.getMeasDur() + sensor.getHeaterConfiguration().heatr_dur * 1000UL);

    if (!sensor.fetchData())
    {
        T = H = P = G = -404;
        return;
    }

    sensor.getData(data);

    T = data.temperature * 1.8f + 32.0f;
    H = data.humidity;
    P = data.pressure;
    G = data.gas_resistance;
}

bool STX_2xBME680_Bosch_Wrapper::DiffToleranceOK_Last()
{
    if (bosch_id1 == 0 || bosch_id2 == 0)
        return false;

    bool ok =
        _diff_ok(Tdiff.last_diff, MaxTdiff) &&
        _diff_ok(Hdiff.last_diff, MaxHdiff) &&
        _diff_ok(Pdiff.last_diff, MaxPdiff);

    if (GasValid())
    {
        ok = ok &&
            _diff_ok(Gdiff.last_diff, MaxGdiff);
    }

    return ok;
}

bool STX_2xBME680_Bosch_Wrapper::DiffToleranceOK_Total()
{
    if (bosch_id1 == 0 || bosch_id2 == 0)
        return false;

    bool ok =
        _diff_ok(Tdiff.min_diff, MaxTdiff) &&
        _diff_ok(Tdiff.max_diff, MaxTdiff) &&
        _diff_ok(Hdiff.min_diff, MaxHdiff) &&
        _diff_ok(Hdiff.max_diff, MaxHdiff) &&
        _diff_ok(Pdiff.min_diff, MaxPdiff) &&
        _diff_ok(Pdiff.max_diff, MaxPdiff);

    if (GasValid())
    {
        ok = ok &&
            _diff_ok(Gdiff.min_diff, MaxGdiff) &&
            _diff_ok(Gdiff.max_diff, MaxGdiff);
    }

    return ok;
}

float STX_2xBME680_Bosch_Wrapper::_avg(float v1, float v2)
{
    if (v1 < -400) return v2;
    if (v2 < -400) return v1;

    return (v1 + v2) / 2.0f;
}

bool STX_2xBME680_Bosch_Wrapper::_diff_ok(float v, float maxv)
{
    if (v < 0)
        v = -v;

    return v < maxv;
}

float STX_2xBME680_Bosch_Wrapper::T_Avg() { return _avg(T1, T2); }
float STX_2xBME680_Bosch_Wrapper::H_Avg() { return _avg(H1, H2); }
float STX_2xBME680_Bosch_Wrapper::P_Avg() { return _avg(P1, P2); }
float STX_2xBME680_Bosch_Wrapper::G_Avg() { return _avg(G1, G2); }

