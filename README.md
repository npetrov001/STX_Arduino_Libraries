Libraries to read Dual BME280 & BME680 sensors.

You can use these libraries with SimpleTronix 2xBME280 or 2xBME680 boards or with any 2 generic sensor boards on I2C where one board is set to default address 0x76 and one to alternative address 0x77.

The biggest reason to use 2 sensors is for increased reliability.  You can set maximum thrresholds and check if readings between the 2 sensors deviate from each other too much.
