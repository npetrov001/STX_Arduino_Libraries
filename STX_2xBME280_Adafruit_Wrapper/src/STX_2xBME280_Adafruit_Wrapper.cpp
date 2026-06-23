
#include <Wire.h>              // I2C connection

#include "STX_2xBME280_Adafruit_Wrapper.h"

// return TRUE if sensors started OK
// require_both = TRUE  - return FALSE if both sensors don't start
// require_both = FALSE - return TRUE if only one sensor starts
bool STX_2xBME280_Adafruit_Wrapper::Start(bool require_both /*= true*/)
{
    this->require_both_sensors = require_both;
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
void STX_2xBME280_Adafruit_Wrapper::SetMaxDiffTolerance(int Tdiff /*= 4*/, int Hdiff /*= 15*/, int Pdiff /*= 100*/)
{
    this->MaxTdiff = Tdiff;
    this->MaxHdiff = Hdiff;
    this->MaxPdiff = Pdiff;
}

// read values
void STX_2xBME280_Adafruit_Wrapper::Read()
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
bool STX_2xBME280_Adafruit_Wrapper::DiffToleranceOK_Last()
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
bool STX_2xBME280_Adafruit_Wrapper::DiffToleranceOK_Total()
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
float STX_2xBME280_Adafruit_Wrapper::T_Avg() { return _avg(T1, T2); }
float STX_2xBME280_Adafruit_Wrapper::H_Avg() { return _avg(H1, H2); }
float STX_2xBME280_Adafruit_Wrapper::P_Avg() { return _avg(P1, P2); }


// return ID or 0 if sensor did not start
// print error on serial if wrong sensor (BME280 & BME680 have same I2C ID yet different chip IDs)
uint32_t STX_2xBME280_Adafruit_Wrapper::_startSensor(Adafruit_BME280& sensor, uint8_t addr)
{
    bool started_ok = sensor.begin(addr);
    
    if (!started_ok)
    {
        uint8_t chip_id = (uint8_t)sensor.sensorID();
        if (chip_id == 0x61)
        {
            Serial.printf("ERROR: BME680 detected at addr=0x%X. Use 2xBME680 Wrapper instead.\r\n", addr);
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
    
    // set oversampling & filter to ignore intermediate spikes
    if (id != 0)
    {
        // values are 1, 2, 4, 8, 16
        sensor.setSampling(
            Adafruit_BME280::MODE_NORMAL,
            Adafruit_BME280::SAMPLING_X8,
            Adafruit_BME280::SAMPLING_X8,
            Adafruit_BME280::SAMPLING_X8,
            Adafruit_BME280::FILTER_X8,
            Adafruit_BME280::STANDBY_MS_0_5);
    }
    
    return id;
}

uint32_t STX_2xBME280_Adafruit_Wrapper::_readBoschID(uint8_t addr)
{
    Wire.beginTransmission(addr);
    Wire.write(0x83);
    Wire.endTransmission();

    uint32_t value = 0;
    uint8_t* buf = (uint8_t*)&value; // point buf to value to store 1 byte at a time

    uint8_t len = 4;
    uint8_t i = 0;

    uint32_t ms_start = millis();

    Wire.requestFrom(addr, len);
    while ((Wire.available()) && (i < len))  // slave may send less than requested
    {
        *buf = Wire.read(); // receive a byte as character
        buf++;
        i++;
        
        // don't check for ID longer than 2 sec
        uint32_t ms_spent = millis() - ms_start;
        if (ms_spent > 2000)
        {
            value = 0;
            break;
        }
    }

    return value;
}


void STX_2xBME280_Adafruit_Wrapper::_readSensor(uint32_t id, Adafruit_BME280& sensor, float& T, float& H, float& P)
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
        float t = sensor.readTemperature();
        if (isnan(t))
        {
            T = -404;
            H = -404;
            P = -404;
        }
        else
        {
            T = t * 1.8 + 32;
            P = sensor.readPressure();
            if (isnan(P)) P = -404;
            H = sensor.readHumidity();
            if (isnan(H)) H = -404;
        }
    }
}

float STX_2xBME280_Adafruit_Wrapper::_avg(float v1, float v2)
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

bool STX_2xBME280_Adafruit_Wrapper::_diff_ok(float v, float maxv)
{
    if (v < 0) v = -v;
    return v < maxv;
}
