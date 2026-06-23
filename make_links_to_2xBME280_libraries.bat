rem CREATES DIRECTORY JUNCTIONS FROM ARDUINO LIBRARIES DIR TO THIS SOURCE DIR FOR 2xBME280 LIBRARIES

mklink /J "%USERPROFILE%\Documents\Arduino\libraries\STX_2xBME280_Adafruit_Wrapper" STX_2xBME280_Adafruit_Wrapper
mklink /J "%USERPROFILE%\Documents\Arduino\libraries\STX_2xBME280_Bosch_Wrapper" STX_2xBME280_Bosch_Wrapper
mklink /J "%USERPROFILE%\Documents\Arduino\libraries\STX_2xBME280_SparkFun_Wrapper" STX_2xBME280_SparkFun_Wrapper
