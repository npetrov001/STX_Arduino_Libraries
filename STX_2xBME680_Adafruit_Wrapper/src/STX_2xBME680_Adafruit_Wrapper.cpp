#include <Wire.h>

#include "STX_2xBME680_Adafruit_Wrapper.h"

// return TRUE if sensors started OK
// require_both = TRUE  - return FALSE if both sensors don't start
// require_both = FALSE - return TRUE if only one sensor starts
bool STX_2xBME680_Adafruit_Wrapper::Start(bool require_both)
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

// set maximum tolerances for Diff between temperature, humidity, pressure & gas
void STX_2xBME680_Adafruit_Wrapper::SetMaxDiffTolerance(
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

// read values
void STX_2xBME680_Adafruit_Wrapper::Read()
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

// return TRUE once gas sensor warmup period has elapsed
bool STX_2xBME680_Adafruit_Wrapper::GasValid()
{
    return (millis() - StartMs) >= GasWarmupMs;
}

// return ID or 0 if sensor did not start
// print error on serial if wrong sensor
// BME280 and BME680 use same I2C addresses but different chip IDs
uint32_t STX_2xBME680_Adafruit_Wrapper::_startSensor(
    Adafruit_BME680& sensor,
    uint8_t addr)
{
    bool started_ok = sensor.begin(addr);

    if (!started_ok)
    {
        uint8_t chip_id = _readChipID(addr);

        if (chip_id == 0x60)
        {
            Serial.printf("ERROR: BME280 detected at addr=0x%X.  Use 2xBME280 Wrapper instead.\r\n", addr);
        }
        else if (chip_id != 0)
        {
            Serial.printf("ERROR: Unsupported chip ID 0x%X at addr=0x%X\r\n", chip_id, addr);
        }
        else
        {
            Serial.printf("ERROR: Sensor not detected at addr=0x%X.\r\n", addr);
        }

        return 0;
    }

    uint32_t id = _readBoschID(addr);

    // values are 1,2,4,8,16
    sensor.setTemperatureOversampling(BME680_OS_8X);

    // humidity changes slower, 2X is enough
    sensor.setHumidityOversampling(BME680_OS_2X);

    // pressure changes slower, 4X is enough
    sensor.setPressureOversampling(BME680_OS_4X);

    sensor.setIIRFilterSize(BME680_FILTER_SIZE_3);

    // enable heater immediately so gas sensor starts stabilizing
    sensor.setGasHeater(320, 150);

    return id;
}

// read chip ID register
uint8_t STX_2xBME680_Adafruit_Wrapper::_readChipID(uint8_t addr)
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

// read Bosch factory-programmed fingerprint
uint32_t STX_2xBME680_Adafruit_Wrapper::_readBoschID(uint8_t addr)
{
    Wire.beginTransmission(addr);
    Wire.write(0x83);
    Wire.endTransmission();

    uint32_t value = 0;
    uint8_t* buf = (uint8_t*)&value;

    uint8_t len = 4;
    uint8_t i = 0;

    uint32_t ms_start = millis();

    Wire.requestFrom(addr, len);

    while ((Wire.available()) && (i < len))
    {
        *buf = Wire.read();
        buf++;
        i++;

        uint32_t ms_spent = millis() - ms_start;

        // don't check for ID longer than 2 sec
        if (ms_spent > 2000)
        {
            value = 0;
            break;
        }
    }

    return value;
}

// perform one complete measurement cycle
// BME680 measures temperature, humidity, pressure and gas together
void STX_2xBME680_Adafruit_Wrapper::_readSensor(
    uint32_t id,
    Adafruit_BME680& sensor,
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

    if (!sensor.performReading())
    {
        T = H = P = G = -404;
        return;
    }

    T = sensor.temperature * 1.8f + 32.0f;
    H = sensor.humidity;
    P = sensor.pressure;
    G = sensor.gas_resistance; // will return values, however usable values after GasValid() = true
}

bool STX_2xBME680_Adafruit_Wrapper::DiffToleranceOK_Last()
{
    if (bosch_id1 == 0 || bosch_id2 == 0)
    {
        return false;
    }

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

bool STX_2xBME680_Adafruit_Wrapper::DiffToleranceOK_Total()
{
    if (bosch_id1 == 0 || bosch_id2 == 0)
    {
        return false;
    }

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

float STX_2xBME680_Adafruit_Wrapper::_avg(float v1, float v2)
{
    if (v1 < -400)
        return v2;

    if (v2 < -400)
        return v1;

    return (v1 + v2) / 2.0f;
}

bool STX_2xBME680_Adafruit_Wrapper::_diff_ok(float v, float maxv)
{
    if (v < 0)
        v = -v;

    return v < maxv;
}

float STX_2xBME680_Adafruit_Wrapper::T_Avg() { return _avg(T1, T2); }
float STX_2xBME680_Adafruit_Wrapper::H_Avg() { return _avg(H1, H2); }
float STX_2xBME680_Adafruit_Wrapper::P_Avg() { return _avg(P1, P2); }
float STX_2xBME680_Adafruit_Wrapper::G_Avg() { return _avg(G1, G2); }
