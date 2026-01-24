# Description
- I brought this up to make it easier to configure the Guition ESP32_5.0inc_JC8048W550 IPS capacitive display for an LVGL 9.x project.
- I tested it with LVGL 9.4 and it works great.
  
Libraries:

- #include <lvgl.h> ver. 9.4. https://docs.lvgl.io/9.4/
- #include <Arduino_GFX_Library.h> ver.1.6.4.  https://github.com/moononournation/Arduino_GFX
- #include <Wire.h> communication with I2C devices (GT911 Capacitive touch panel)
- #include <TAMC_GT911.h>  ver. 1.0.2  https://github.com/tamctec/gt911-arduino

> [!NOTE]
> The storage contains a folder called configuration files. The files in the folder are configuration files. These files are required for the display and touch panel to start.

- The lv.conf.h in the folder should be placed in the main folder of the arduino libraries
- The files in the databus folder must be overwritten with the files in the GFX_Library_for_Arduino databus folder.
```
/home/_YOURUSERNAME_/Arduino/libraries/GFX_Library_for_Arduino/src/databus
```
> [!IMPORTANT]
> It is important that only the two files are overwritten, the rest remain in the folder.


