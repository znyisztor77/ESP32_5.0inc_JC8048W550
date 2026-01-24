# Description
- I brought this up to make it easier to configure the Guition ESP32_5.0inc_JC8048W550 IPS capacitive display for an LVGL 9.x project.
- I tested it with LVGL 9.4 and it works great.
  
Libraries:

- #include <lvgl.h> ver. 9.4.
- #include <Arduino_GFX_Library.h> ver.1.6.4.  https://github.com/moononournation/Arduino_GFX
- #include <Wire.h>
- #include <TAMC_GT911.h>  ver. 1.0.2  https://github.com/tamctec/gt911-arduino

> [!NOTE]
> This library uses the newly introduced esp_lcd_panel interfaces. This should provide some support in the future for updates and new boards. These drivers are provided by Espressif and have already been copied and included to this library.

> [!TIP]
> If you already have a project, clone it with the `git clone --recurse-submodules`. If creating a new project, use `git submodule add https://github.com/rzeldent/platformio-espressif32-sunton.git boards` to add them to your project as a submodule.

> [!WARNING]
> After the library has been build, changes in the lv_conf.h are no longer applied because libraries are cached.
> To apply these settings, delete the `.pio` directory so the libraries will be rebuild.
