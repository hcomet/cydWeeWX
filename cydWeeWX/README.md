# Building and installing cydWeeWX

## Set Up Your Build Environment

1. Install the Arduino IDE.  
  <u>**NOTE:**</u>  Use of the latest 2.x version of the IDE is recommended since builds have only been verified with that version of the IDE.
2. Add support for ESP32 modules using the Boards Manager in the Arduino IDE.  
    * Install the boards plugin for esp32 by Espressif Systems version 3.x.
3. The CYD uses an ESP32-WROOM and any if the basic boards definitions should work. I've used "`ESP32-WROOM-DA Module`" and "`ESP32 Dev Module`" without issue.
4. The cydWeeWX uses [LVGL](https://lvgl.io/) which makes builds quite large. Make sure to select "`Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFS)`"
5. Install the following libraries if not already installed:
   * [ArduinoJson by Benoit Blanchon](https://arduinojson.org/) - Version: 7.2.1
   * [TaskScheduler by Anatoli Arkhipenko](https://github.com/arkhipenko/TaskScheduler) - Version: 3.8.5
   * [WiFiManager by tzapu](https://github.com/tzapu/WiFiManager) - Version 2.0.17
   * [lvgl by kisvegabor](https://github.com/lvgl/lvgl) - Version 9.2.2
   * [TFT_eSPI by Bodmer](https://github.com/Bodmer/TFT_eSPI) - Version 2.5.43

   <u>**NOTE:**</u> If newer versions of a library are available then only use new subversions not new major versions.

6. Download the latest firmware release from <https://github.com/hcomet/cydWeeWX/releases>  
  <u>**NOTE:**</u> After extracting the release to your file system open the ***cydWeeWX.ino*** file in the Arduino IDE. This will open the full set of source files in the IDE. Now configure the firmware before building it.
  
### Configure Firmware in the ***cydWeeWXDefines.h*** file

The cydWeeWX firmware may be built without any changes to the default settings in the ***cydWeeWXDefines.h*** file. However, there are a couple of customizations that may be made.

  1. Display Backlight.
  2. Pre-configure WiFi.
  3. Random Access Point password.
  4. Device AP and Host name.
  5. WOKWi.
  
### TSP-eSPI ***User_Setup.h***

### LVGL ***lv_conf.h***

## WeeWX-JSON Extension Installation

## Build and Run

1. After configuring values in the ***cydWeeWXDefines.h***, using the Arduino IDE build and upload the firmware to your ESP32-CYD.
2. Follow the [steps](../README.md/#configuration-portal-steps) to set up your WiFi connection and WeeWX URL.
3. 