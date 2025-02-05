# How to Build and Install cydWeeWX

## Set Up Your Build Environment

1. Install the Arduino IDE.  
  <u>**NOTE:**</u>  Use of the latest 2.x version of the IDE is recommended since builds have only been verified with that version of the IDE.
2. Add support for ESP32 modules using the _Boards Manager_ in the Arduino IDE.  
    * Install the boards plugin for esp32 by Espressif Systems version 3.x. Tested with Arduino core for ESP32 v3.1.1.
3. The CYD uses an ESP32-WROOM and any of the basic boards definitions should work. I've used _ESP32-WROOM-DA Module_ and _ESP32 Dev Module_ without issue.
4. The cydWeeWX uses [LVGL](https://lvgl.io/) which makes builds quite large. Make sure to select a **Partition Scheme** of _Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFS)_.
5. Install the following Arduino libraries if not already installed:
   * [ArduinoJson by Benoit Blanchon](https://arduinojson.org/) - Version: 7.3.0
   * [TaskScheduler by Anatoli Arkhipenko](https://github.com/arkhipenko/TaskScheduler) - Version: 3.8.5
   * [WiFiManager by tzapu](https://github.com/tzapu/WiFiManager) - Version 2.0.17
   * [lvgl by kisvegabor](https://github.com/lvgl/lvgl) - Version 9.2.2
   * [TFT_eSPI by Bodmer](https://github.com/Bodmer/TFT_eSPI) - Version 2.5.43

   <u>**NOTE:**</u> If newer versions of a library are available then only use new subversions not new major versions.

6. Download the latest source code release from <https://github.com/hcomet/cydWeeWX/releases>  
  <u>**NOTE:**</u> After extracting the release to your file system open the ***cydWeeWX.ino*** file in the Arduino IDE. This will open the full set of source files in the IDE. Now configure the firmware before building it.
  
### Configure Firmware in the ***cydWeeWXDefines.h*** file

The cydWeeWX firmware may be built without any changes to the default settings in the ***cydWeeWXDefines.h*** file. However, there are a couple of customizations that may be made.

  * Set the pin number for backlight control if you have one. Pin 21 seems to be common.
    ```c
    #define CYD_WWX_BL_PIN 21 
    ``` 
    You may need to remove the pin definition from your ***User_Setup.h*** for PWM blacklight control. This can be done by commenting out the following line in the User_Setup.h:

    ```c
    //#define TFT_BL   21            // LED back-light control pin
    ```

  * Change the backlight brightness level. 100 out of 255 seems to work well on my CYD but models vary.

    ```c
    #define CYD_WWX_BL_BRIGHTNESS 100
    ```  
    
  * By default cydWeeWX will concatenate a 4 digit random value to the end of the Access Point base password resulting in _cydWeeWx####_. This is done on each boot of the device. Commenting out this line will prevent the random value concatenation resulting in same password being used all the time.  
  
    ```c
    #define WIFI_MANAGER_ENABLE_PASSWORD_RANDOM
    ```  
   
  * The default Hostname, Access Point SSID and Base Password may be changed with the following lines:  

    ```c
    #define CYD_WWX_HOSTNAME "cydWeeWX" 
    #define CYD_WWX_WM_AP_NAME CYD_WWX_HOSTNAME
    #define CYD_WWX_WM_AP_PASSWORD CYD_WWX_WM_AP_NAME
    ```

  * A build for WOKWi Simulation maybe enabled using the details found [here](../WOKWi/README.md). The default is to disable WOKWi builds since they will not work properly on the physical cydWeeWX.
  
### TSP-eSPI ***User_Setup.h***

A ***User_Setup.h*** file that matches your CYD must be placed in the Arduino TFT_eSPI library folder. Brian Lough's github [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) repository provides two basic versions that generally work. One is for CYD boards with a single USB port, [User_Setup.h](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/User_Setup.h). The second is for CYD boards with two USB ports, [User_Setup.h](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/CYD2USB/User_Setup.h). Also be sure to read the general [CYD Setup](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md) instructions. 

### LVGL ***lv_conf.h***

The LVGL library needs a ***lv_conf.h*** file set up for your device. There is one included with the cydWeeWX source code, [lv_conf.h](./lv_conf.h). You can leave it in the sketch folder. If you already have one in your ***Arduino/libraries*** folder you may need to replace it with the cydWeeWX version of the file.

## WeeWX-JSON Extension Installation

The cydWeeWX device makes queries to your WeeWX server to pick up current weather data for your weather station. The WeeWX server needs to be configured to support weewx-json report generation. Follow these [instructions](../WeeWX/README.md) to complete the configuration.

## Build and Run

1. After completing the steps above, use the Arduino IDE to build and upload the firmware to your ESP32-CYD.
2. Follow the [steps](../README.md/#configuration-portal-steps) to set up your WiFi connection and WeeWX URL.
3. Your ESP32-CYD should now be a working cydWeeWX.