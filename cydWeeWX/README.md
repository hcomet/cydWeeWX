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
    
  * Pre-configure configuration portal items.
  * By default the cydWeeWX will concatenate a 4 digit random value to the end of the Access Point base password resulting in _cydWeeWx####_. This is done on each boot of the device. Commenting out this line will prevent the random value concatenation resulting in _cydWeeWX_ being the password all the time.  
  
    ```c
    #define WIFI_MANAGER_ENABLE_PASSWORD_RANDOM
    ```  
   
  * The default Hostname, Access Point SSID and Base Password may be changed with the following lines:  

    ```c
    #define CYD_WWX_HOSTNAME "cydWeeWX" 
    #define CYD_WWX_WM_AP_NAME CYD_WWX_HOSTNAME
    #define CYD_WWX_WM_AP_PASSWORD CYD_WWX_WM_AP_NAME
    ```

  * WOKWi Simulation setup is found [here](../WOKWi/README.md).
  
### TSP-eSPI ***User_Setup.h***

A ***User_Setup.h*** file that matches your CYD must be placed in the Arduino TFT_eSPI library folder. The one provided in Brian Lough's github [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) repository generally works. It can be found at [User_Setup.h](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/User_Setup.h). Read the [CYD Setup](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md) instructions. 

If you are having display issues then you may have the wrong driver defined. Classic CYD devices seem to work well with either _ILI9341_DRIVER _ or _ILI9341_2_DRIVER _. My CYD had the combination USB2 and USB3 ports and worked well with _ST7789_DRIVER_.

### LVGL ***lv_conf.h***

## WeeWX-JSON Extension Installation

## Build and Run

1. After configuring values in the ***cydWeeWXDefines.h***, using the Arduino IDE build and upload the firmware to your ESP32-CYD.
2. Follow the [steps](../README.md/#configuration-portal-steps) to set up your WiFi connection and WeeWX URL.
3. 