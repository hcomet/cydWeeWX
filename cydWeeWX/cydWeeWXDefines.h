// **********************************************************************************
// ** Defines included in cydWeeWX project
// ** Details at https://github.com/hcomet/cydWeeWX
// ** (c) Copyright Stephen Hillier 2024. All Rights Reserved.
// **********************************************************************************

#ifndef CYD_WEEWX_DEFINES
#define CYD_WEEWX_DEFINES

// **************************************************************************************************
// LVGL Display items
// **************************************************************************************************
#define LV_LVGL_H_INCLUDE_SIMPLE

#define CYD_WWX_SCREEN_WIDTH 240                    // Display width
#define CYD_WWX_SCREEN_HEIGHT 320                   // Display height
#define CYD_WWX_ROTATE_SCREEN LV_DISPLAY_ROTATION_90    // Display rotation to landscape
#define CYD_WWX_DRAW_BUF_SIZE (CYD_WWX_SCREEN_WIDTH * CYD_WWX_SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))    // Drawing buffer size

// lv timer 
#define CYD_WWX_WEEWX_LV_TIMER 20000                // 20 seconds switching WeeWX readings grid
#define CYD_WWX_WIFI_MANAGER_LV_TIMER 1000          // 1 second to keep portal timer updated.

// LV RGB Colors
#define CYD_WWX_NIGHT_TEXT_COLOR 0xFAFAD2
#define CYD_WWX_NIGHT_BG_COLOR 0x707060
#define CYD_WWX_DAY_TEXT_COLOR 0xFFFFFF
#define CYD_WWX_HEADER_TEXT_COLOR 0x3F51B5
#define CYD_WWX_DAY_BG_COLOR 0x0000CD
#define CYD_WWX_ERROR_TEXT_COLOR 0xFF0000
#define CYD_WWX_WEEWX_BG_COLOR 0xF5F5E0
#define CYD_WWX_WIFI_MANAGER_BG_COLOR 0x000000
#define CYD_WWX_WIFI_MANAGER_TEXT_COLOR 0xFFEB3B

// Pin to control backlight
#define CYD_WWX_BL_PIN 21                           // Change if Pin 21 is not the backlight Pin on your CYD
#define CYD_WWX_BL_TIMER_8_BIT  8                   // use 8 bit precision for LEDC timer (255 brightness levels)
#define CYD_WWX_BL_BASE_FREQ 5000                   // use 5000 Hz as a LEDC base frequency
#define CYD_WWX_BL_BRIGHTNESS 100                   // Default backlight brightness (out of 255)

// **************************************************************************************************
// Task Scheduler related items
// **************************************************************************************************
#define CYD_WWX_GET_WEEWX_UPDATE_EVERY 120000       // Query WeeWX Server every 2 minutes 
#define CYD_WWX_GET_OPENMETEO_UPDATE_EVERY 300000   // 5 minutes - Open-meteo data updates every 15 mins. Stay less than 10,000 calls per day.
#define CYD_WWX_CALL_LVGL_HANDLER_EVERY 5           // Refresh LVGL display every 5 msec
#define CYD_WWX_CHECK_WM_TRIGGER_PIN_EVERY 100      // Check trigger pin every 100 msec
#define CYD_WWX_PROCESS_WM_EVERY 10                 // process Configuration Portal activity every 10 msec
#define CYD_WWX_ONE_SECOND_TIMER 1000               // One second task timer - DO NOT CHANGE

// **************************************************************************************************
// urls for data retrieval
// **************************************************************************************************

#define CYD_WWX_OPEN_METEO_URL "http://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s&current=is_day,weather_code&daily=sunrise,sunset&forecast_days=1"  // Open-Meteo URL (DO NOT CHANGE)
#define CYD_WWX_WEEWX_URL "http://yourWeeWx.server.local/"  // This ia an example. May be changed here or through the Management Portal
#define CYD_WWX_WEEWX_JSON_DATA_FILE "cyd_weewx.json"       // WeeWX JSON file name - DO NOT CHANGE
#define CYD_WWX_WEEWX_URL_FIELD_LENGTH 64                   // Configuration Portal field length for WeeWX server URL

#define CYD_WWX_ERROR_STATE_CODE 1000               // Forces WMO Icon to error icon
#define CYD_WWX_ERROR_WAIT_TO_REBOOT 300            // wait until reboot when in error state - in seconds

// **************************************************************************************************
// WiFi Manager items
// **************************************************************************************************
#define CYD_WWX_HOSTNAME "cydWeeWX"                 // Default device hostname
#define CYD_WWX_WM_AP_NAME CYD_WWX_HOSTNAME         // Device SSID when in AP mode
#define CYD_WWX_WM_AP_PASSWORD CYD_WWX_WM_AP_NAME   // Device AP password 
#define WIFI_MANAGER_ENABLE_PASSWORD_RANDOM         // if defined will add a 4 digit random value to the password on every system boot
#define CYD_WWX_WM_TITLE "Configuration Portal"     // Title to use in the Configuration Portal
#define CYD_WWX_WM_TRIGGER_PIN 0                    // Trigger pin to enter/leave Configuration Portal mode (BOOT pin on CYD)
#define CYD_WWX_WM_TIMEOUT 300                      // Configuration Portal timeout in seconds (5 minutes)
#define CYD_WWX_WM_TRIGGER_PIN_HOLD_COUNT 20        // Pin must be held down this value times CYD_WWX_CHECK_WM_TRIGGER_PIN_EVERY in msec

// **************************************************************************************************
// Message template strings - DO NOT CHANGE
// **************************************************************************************************
#define CYD_WWX_WM_AP_SCREEN_MESSAGE "Configuration Portal is Active.\nCurrently in AP Mode.\n\nSSID: %s\nPassword: %s\n\nGo to: 191.168.4.1\nand configure cydWeeWX."
#define CYD_WWX_WM_ST_SCREEN_MESSAGE "Configuration Portal is Active.\nCurrently in Station Mode.\nPortal IP Address: %s."
#define CYD_WWX_WM_TIMER_MESSAGE "Portal closes in %02d:%02d"

// **************************************************************************************************
// WeeWX URL Stored in EEPROM Preferences
// **************************************************************************************************
#define CYD_WWX_PREFERENCES_NAMESPACE "cydWeeWX"    // Preferences Namespace
#define CYD_WWX_PREFERENCES_KEY "WEEWX_JSON_URL"    // Preferences Key name
#define CYD_WWX_PREFERENCES_RW false                // Open Preferences Read/Write
#define CYD_WWX_PREFERENCES_RO true                 // Open Preferences Read Only

// **************************************************************************************************
// To run on the Wokwi simulator
// **************************************************************************************************
//#define CYD_WWX_RUN_ON_WOKWI                      // Uncomment to build for WOKWi simulation
//#define CYD_WWX_WOKWI_ENTER_AP_AT_BOOT            // Uncomment to force AP entry at boot
//#define CYD_WWX_WOKWI_SHOW_ERROR_STATE            // Uncomment to enter error state on Config Portal exit
#define CYD_WWX_WOKWI_TRIGGER_PIN 26                // Trigger Pin as defined in the diagram.json
#define CYD_WWX_WOKWI_AP_SSID "Wokwi-GUEST"         // WOKWi WiFi open access point SSID
#define CYD_WWX_WOKWI_AP_PASSWORD ""                // WOKWi WiFi open access point Password

// **************************************************************************************************
// Logging Macros
// **************************************************************************************************
#define CYD_WEEWX_LOG_OUTPUT Serial                 // Stream to send Logs to - Serial is default
//#define CYD_WEEWX_ENABLE_DEBUG_LOG                // Comment out to disable Debug logging
#define CYD_WEEWX_ENABLE_INFO_LOG                   // Comment out to disable Information logging
#define CYD_WEEWX_ENABLE_ERROR_LOG                  // Comment out to disable Error logging
#if defined CYD_WEEWX_ENABLE_DEBUG_LOG || defined CYD_WEEWX_ENABLE_DEBUG_LOG || defined CYD_WEEWX_ENABLE_ERROR_LOG
template <class T>
inline Print &operator<<(Print &obj, T arg)
{
    obj.print(arg);
    return obj;
}
#endif  // CYD_WEEWX_ENABLE_DEBUG_LOG || CYD_WEEWX_ENABLE_INFO_LOG || CYD_WEEWX_ENABLE_ERROR_LOG
#ifdef CYD_WEEWX_ENABLE_DEBUG_LOG
#define LOG_DEBUG(svc, content) CYD_WEEWX_LOG_OUTPUT << "DEBUG: <" << svc << "> " << content << "\r\n"
#else
#define LOG_DEBUG(svc, content) 
#endif  // CYD_WEEWX_ENABLE_DEBUG_LOG
#ifdef CYD_WEEWX_ENABLE_INFO_LOG
#define LOG_INFO(svc, content) CYD_WEEWX_LOG_OUTPUT << "INFO: <" << svc << "> " << content << "\r\n"
#else
#define LOG_INFO(svc, content) 
#endif  // CYD_WEEWX_ENABLE_INFO_LOG
#ifdef CYD_WEEWX_ENABLE_ERROR_LOG
#define LOG_ERROR(svc, content) CYD_WEEWX_LOG_OUTPUT << "ERROR: <" << svc << "> " << content << "\r\n"
#else
#define LOG_ERROR(svc, content) 
#endif  // CYD_WEEWX_ENABLE_ERROR_LOG

#endif // CYD_WEEWX_DEFINES