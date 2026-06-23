rem CREATES DIRECTORY JUNCTIONS FROM ARDUINO LIBRARIES DIR TO THIS SOURCE DIR FOR 2xBME680 LIBRARIES

mklink /J "%USERPROFILE%\Documents\Arduino\libraries\STX_2xBME680_Adafruit_Wrapper" STX_2xBME680_Adafruit_Wrapper
mklink /J "%USERPROFILE%\Documents\Arduino\libraries\STX_2xBME680_Bosch_Wrapper" STX_2xBME680_Bosch_Wrapper
mklink /J "%USERPROFILE%\Documents\Arduino\libraries\STX_2xBME680_Zanshin_Wrapper" STX_2xBME680_Zanshin_Wrapper
