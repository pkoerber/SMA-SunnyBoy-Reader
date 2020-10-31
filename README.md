# SMA-SunnyBoy-Reader
Reading data from a SMA SunnyBoy solar inverter with an esp8266

## Compatibility

Tested with an SMA Sunny Boy 2.0 and a NodeMCU v3 ESP8266

## Example

See the [demo](https://github.com/pkoerber/SMA-SunnyBoy-Reader/blob/main/examples/SMAReader_Demo/SMAReader_Demo.ino)

## Initialization

```C++
SMAReader(IPAddress inverterAddress, const char* accountType, const char* passwd, byte numTries=5);
```

Constructs an SMAReader object, with 
- `inverterAddress`: the IP address of the SMA inverter
- `accountType`: one of SMAREADER_USER (user account) or SMAREADER_INSTALLER (installer account)
- `passwd`: password for the account
- `numtries`: how many times to try to connect to the inverter before giving up

`numTries` can be changed afterwards with `setNumTries`

## Functions

```C++
bool getValues(int numKeys, const String* keys, int* values);
```
Get the values corresponding to the `keys`, the results are stored in the array `values` (must be of appropriate size)
Returns `true` if successful, `false` otherwise

The following keys are predefined:

| Define | Key | Unit | Comment|
|------------------------------|
|KEY_POWER |6100_40263F00|  W| |
|KEY_ENERGY_TODAY|6400_00262200| Wh| |
|KEY_ENERGY_TOTAL|6400_00260100| Wh| |
|KEY_AC_L1_POWER|6100_40464000| W |only one if only one phase|
|KEY_AC_L1_VOLTAGE|6100_00464800|1e-2 V|only one if only one phase|
|KEY_AC_L1_CURRENT|6100_40465300| mA|only one if only one phase|
|KEY_AC_L2_POWER|6100_40464100| W|only if multiple phases|
|KEY_AC_L2_VOLTAGE|6100_00464900| 1e-2 V|only if multiple phases|
|KEY_AC_L2_CURRENT|6100_40465400| mA|only if multiple phases|
|KEY_AC_L3_POWER|6100_40464200| W|only if multiple phases|
|KEY_AC_L3_VOLTAGE|6100_00464A00| 1e-2 V|only if multiple phases|
|KEY_AC_L3_CURRENT|6100_40465500| mA |only if multiple phases|
|KEY_AC_L1L2_VOLTAGE|6100_00464B00| 1e-2 V|only if multiple phases|
|KEY_AC_L2L3_VOLTAGE|6100_00464C00| 1e-2 V|only if multiple phases|
|KEY_AC_L3L1_VOLTAGE|6100_00464D00| 1e-2 V|only if multiple phases|
|KEY_AC_FREQUENCY|6100_00465700| 1e-2 Hz | | 
|KEY_DC_POWER|6380_40251E00| W | |
|KEY_DC_VOLTAGE|6380_40451F00| 1e-2 V | |
|KEY_DC_CURRENT|6380_40452100| mA | |
|KEY_OPERATING_TIME|6400_00462E00| s | |
|KEY_FEED_IN_TIME|6400_00462F00| s | |
|KEY_ETHERNET_IP|6180_104A9A00| | String |
|KEY_WLAN_IP|6180_104AB700| | String |
|KEY_WLAN_STRENGTH|6100_004AB600| percentage | |


```C++
int getLog(uint32_t startTime, uint32_t endTime, uint32_t* values, uint32_t* timestamps=nullptr);
```

Get historic values for the total energy production with a 5-minute interval beween `startTime` and `endTime`.
These times are given as Unix timestamps (seconds after 01/01/1970, midnight UTC). This is compatible with the C++ function [mktime](http://www.cplusplus.com/reference/ctime/mktime/). See the [example code]([demo](https://github.com/pkoerber/SMA-SunnyBoy-Reader/blob/main/examples/SMAReader_Demo/SMAReader_Demo.ino).
The resulting values are stored in `values` (must be of appropriate size). If `timestamps` is not a nullptr, the corresponding timestamps are stored therein.
Returns the number of values or -1 in case of failure.





