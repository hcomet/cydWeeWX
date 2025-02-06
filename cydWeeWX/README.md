# How to Build and Install cydWeeWX
## Option 1: Just install the Firmware

If you have either an [original CYD or CYD2USB](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display?tab=readme-ov-file#how-do-i-know-if-a-display-is-a-cyd) then you can skip the hassle of setting up the Arduino IDE and building the firmware from source code. Use the web based firmware [flash tool](https://hcomet.github.io/cydWeeWX/cydWeeWXFlash.html).

If you do not have an original CYD or CYD2USB then you will likely need to go to option 2 and build the cydWeeWX from source code.

## Option 2: Build it Yourself

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

The cydWeeWX firmware may be built without any changes to the default settings in the ***cydWeeWXDefines.h*** file. However, there are a couple of customizations that may be made if needed.

* Hostname: The default Hostname, Access Point SSID and Base Password may be changed with the following lines:  
  ```c
  #define CYD_WWX_HOSTNAME "cydWeeWX" 
  #define CYD_WWX_WM_AP_NAME CYD_WWX_HOSTNAME
  #define CYD_WWX_WM_AP_PASSWORD CYD_WWX_WM_AP_NAME
  ```
* WiFi Access Point Security: By default cydWeeWX will concatenate a 4 digit random value to the end of the Access Point base password. The base password is the hostname which with the default hostname of _cydWeeWX_ results in a password format of _cydWeeWx####_. This is done on each boot of the device. Commenting out this line will prevent the random value concatenation resulting in the same password, the hostname, being used all the time.  
  ```c
  #define WIFI_MANAGER_ENABLE_PASSWORD_RANDOM
  ```  
* LCD Backlight Control: If backlight control is desired then the User_Setup.h ([discussed below](#tsp-espi-user_setuph)) needs to be modified. The line defining the backlight control pin needs to be commented out as follows:
    ```c
    //#define TFT_BL 
    ```  
    The following section in the ***cydWeeWXDefines.h*** will then become active and may be modified.
    ```c
    // If TFT_BL not defined in the User_Setup.h then set it up here.
    // If you want to adjust the brightness of the LCD then you must comment out "#define TFT_BL" in the User_Setup.h
    #ifndef TFT_BL
    #define CYD_WWX_BL_PIN 21                           // Change if Pin 21 is not the backlight Pin on your CYD
    #define CYD_WWX_LDR_PIN 34                          // Change if Pin 34 is not the LDR Pin on your CYD
    #define CYD_WEEWX_TFT_BACKLIGHT_ON HIGH             // Level to turn ON back-light (HIGH or LOW)
    #define CYD_WWX_BL_TIMER_8_BIT  8                   // use 8 bit precision for LEDC timer (255 brightness levels)
    #define CYD_WWX_BL_BASE_FREQ 5000                   // use 5000 Hz as a LEDC base frequency
    #define CYD_WWX_BL_BRIGHTNESS 100                   // Default Backlight brightness (out of 255)
    #define CYD_WWX_BL_MAX_BRIGHTNESS 255               // Maximum brightness value (out of 255)
    #define CYD_WWX_BL_MIN_BRIGHTNESS 60                // Minimum brightness value (out of 255)
    // Note: LDR values go to 4096 with lower values representing brighter ambient lighting
    #define CYD_WWX_BL_LOW_THRESHOLD  100               // Low LDR reading threshold to set to Max brightness
    #define CYD_WWX_BL_HIGH_THRESHOLD  1000             // High LDR reading threshold to set to Min brightness
    #endif // TFT_BL
    ```
    The LDR(Light Dependent Resistor) is used to sense the ambient light and then adjust the LCD backlight to be stronger in brighter conditions.

* WOKWi Simulation: A build for WOKWi Simulation maybe enabled using the details found [here](../WOKWi/README.md). The default is to disable WOKWi builds since they will not work properly on the physical cydWeeWX. The following lines control WOKWi build enablement:
  ```c
  // **************************************************************************************************
  // To run on the Wokwi simulator
  // **************************************************************************************************
  //#define CYD_WWX_RUN_ON_WOKWI                      // Uncomment to build for WOKWi simulation
  //#define CYD_WWX_WOKWI_ENTER_AP_AT_BOOT            // Uncomment to force AP entry at boot
  //#define CYD_WWX_WOKWI_SHOW_ERROR_STATE            // Uncomment to enter error state on Config Portal exit
  #define CYD_WWX_WOKWI_TRIGGER_PIN 26                // Trigger Pin as defined in the diagram.json
  #define CYD_WWX_WOKWI_AP_SSID "Wokwi-GUEST"         // WOKWi WiFi open access point SSID
  #define CYD_WWX_WOKWI_AP_PASSWORD ""                // WOKWi WiFi open access point Password
  ```
  
### TSP-eSPI ***User_Setup.h***

A ***User_Setup.h*** file that matches your CYD must be placed in the Arduino TFT_eSPI library folder. Brian Lough's github [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) repository describes two basic versions that generally qualify as 'real' Cheap-Yellow-Displays. One is for CYD boards with a single USB port, [User_Setup.h](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/User_Setup.h). The second is for CYD boards with two USB ports (CYD2USB), [User_Setup.h](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/CYD2USB/User_Setup.h). Also be sure to read the general [CYD Setup](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md) instructions. 

### LVGL ***lv_conf.h***

The LVGL library needs a ***lv_conf.h*** file set up for your device. There is one included with the cydWeeWX source code, [lv_conf.h](./lv_conf.h). You can leave it in the sketch folder. If you already have one in your ***Arduino/libraries*** folder you may need to replace it with the cydWeeWX version of the file.

## WeeWX-JSON Extension Installation

The cydWeeWX device makes queries to your WeeWX server to pick up current weather data for your weather station. If you do not have a WeeWX server then please got to the [WeeWX web site](https://www.weewx.com/) to see how to download and install WeeWX. 

The WeeWX server also needs to be configured to support weewx-json report generation. Follow these [instructions](../WeeWX/README.md) to complete the configuration.

## Build and Run

1. After completing the steps above, use the Arduino IDE to build and upload the firmware to your ESP32-CYD.
2. Follow these [steps](../README.md/#configuration-portal-steps) to set up your WiFi connection and WeeWX URL on the cydWeeWX device.
3. Your ESP32-CYD should now be a working cydWeeWX.