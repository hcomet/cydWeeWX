// **********************************************************************************
// ** cydWeeWX - main
// ** A simple CYD (Cheap Yellow Display) weather information display. Information is
// ** collected from a WeeWX weather station server and displayed on the CYD.
// **
// ** Project details: https://github.com/hcomet/cydWeeWX
// ** WeeWX information: https://www.weewx.com/
// ** CYD Information: https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display
// **
// ** Also required for this project:
// ** WeeWX JSON Extension: https://github.com/teeks99/weewx-json
// ** Weather Icons Font: https://erikflowers.github.io/weather-icons/
// ** DejaVu Font: https://dejavu-fonts.github.io/
// ** Arduino LVGL: https://lvgl.io/
// ** ArduinoJSON: https://arduinojson.org/
// **
// **********************************************************************************
// ** cydWeeWX is licensed under the MIT License 
// ** https://github.com/hcomet/cydWeeWX?tab=MIT-1-ov-file#readme
// **
// ** (c) Copyright 2024. All Rights Reserved.
// **********************************************************************************

#include <Arduino.h>
#include "cydWeeWXDefines.h"
#include "weatherIconsDefines.h"
#include "lv_conf.h"
#include <lvgl.h>
#include <WiFiManager.h>
#include <TFT_eSPI.h>
#include "wmoIcons_64c.h"
#include "weathericons_22c.h"
#include "dejaVuSansCondensed_18c.h"
#include <Preferences.h>
#include <esp_timer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h>

int brightness = CYD_WWX_BL_BRIGHTNESS;

// WiFi Manager Config
#define WM_TRIGGER_PIN 0
WiFiManager wm; // global wm instance
WiFiManagerParameter * wmWeeWXUrl; // global param ( for non blocking w params )
bool wifiManagerActive = false;
uint32_t wifiManagerActiveTime = 0;
String cydWeeWXPassword = String(CYD_WWX_WM_AP_PASSWORD);

// WeeWX server url and preferences storage
String cydWeeWXUrl = String(CYD_WWX_WEEWX_URL);
Preferences cydWeeWXPreference;
bool saveCydWeeWXConfigNow = false;

// Error State tracking and Label text
bool cydWeeWXErrorState = false;
String errorDescription = String();

// String Variables for WiFiManager LVGL Label text
String wifiManagerMessage = String();
String wifiManagerTimer = String();

// String Variables for WeeWX LVGL Label text
String location = String();
String latitude = String();
String longitude = String();
String weatherDescription = String();
String screenHeader = String();
String lastUpdateTime = String();

String sunrise = String();
String sunset = String();
String moonrise = String();
String moonset = String();
String moonPhase = String();
String iconMoonPhase = String();

String iconTemperature = String(WI_THERMOMETER);
String temperature = String();
String insideTemperature = String();
String trendTemperature = String();
String unitsTemperature = String();

String iconHumidity = String(WI_HUMIDITY);
String humidity = String();
String insideHumidity = String();
String trendHumidity = String();
String unitsHumidity = String();

String iconWindGust = String(WI_WIND_GUST);
String windGust = String();
String trendWindGust = String();
String unitsWindGust = String();
String windGustDirection = String();

String iconWind = String(WI_WIND);
String wind = String();
String trendWind = String();
String unitsWind = String();
String windDirection = String();

String iconPressure = String(WI_BAROMETER);
String pressure = String();
String trendPressure = String();
String unitsPressure = String();

String iconRainRate = String(WI_RAINDROPS);
String rainRate = String();
String trendRainRate = String();
String unitsRainRate = String();

// Weather WMO Icon related variables
bool isDay = true;
int weatherCode = 0;

bool whichReadingsToShow = true;

namespace sensortype {
enum sensor
  {
    TEMPERATURE = 0, 
    HUMIDITY,
    WIND, 
    WIND_GUST, 
    PRESSURE,
    RAIN_RATE, 
    MAX_SENSORS 
  };
}

struct trendLimits {
  float  lowLimit;
  float  highLimit;
};

// WeeWX trends are over the last hour. Set low and high limits for rate of change arrows
const trendLimits trendLimitsForSensors[sensortype::MAX_SENSORS] = {
  {.lowLimit=0.25, .highLimit=2.0},    // TEMPERATURE degrees C per hour
  {.lowLimit=1.0, .highLimit=5.0},    // HUMIDITY % per hour
  {.lowLimit=0.5, .highLimit=5.0},      // WIND km/h per hour
  {.lowLimit=0.5, .highLimit=5.0},      // WIND_GUST km/h per hour
  {.lowLimit=1.0, .highLimit=5.0},     // PRESSURE mpa per hour
  {.lowLimit=0.5, .highLimit=3.0}     // RAIN_RATE mm/h per hour
};

// ******************************
// LVGL related variables
// ******************************
//
lv_display_t * cydWeeWXDisp;

enum class displayname {
      WEEWX_MAIN = 0,
      WIFI_MANAGER_MAIN,
      MAX_DISPLAYNAME_TYPES
};
displayname currentActiveDisplay = displayname::MAX_DISPLAYNAME_TYPES;  // used as no active display

uint32_t draw_buf[CYD_WWX_DRAW_BUF_SIZE / 4];

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// LVGL Styles
lv_style_t  displayStyle;
static lv_style_t myDayStyle;
static lv_style_t myNightStyle;
lv_style_t gridStyle;
lv_style_t cellStyle;

// LGVL Label Text Values
static lv_obj_t * textLabelWifiManagerMessage;
static lv_obj_t * textLabelWifiManagerTimer;
static lv_obj_t * textLabelWeatherDescription;
static lv_obj_t * textLabelScreenHeader;
static lv_obj_t * textLabelLocation;
static lv_obj_t * textLabelTemperature;
static lv_obj_t * textLabelInsideTemperature;
static lv_obj_t * textLabelHumidity;
static lv_obj_t * textLabelInsideHumidity;
static lv_obj_t * textLabelReadingsGrid32;
static lv_obj_t * textLabelReadingsGrid42;
static lv_obj_t * textLabelSunrise;
static lv_obj_t * textLabelSunset;
static lv_obj_t * textLabelMoonrise;
static lv_obj_t * textLabelMoonset;
static lv_obj_t * textLabelMoonPhase;


// LGVL Label Text Units
static lv_obj_t * textLabelUnitsTemperature;
static lv_obj_t * textLabelUnitsHumidity;
static lv_obj_t * textLabelReadingsGrid34;
static lv_obj_t * textLabelReadingsGrid44;

// LGVL Label Text Trend
static lv_obj_t * textLabelTrendTemperature;
static lv_obj_t * textLabelTrendHumidity;
static lv_obj_t * textLabelReadingsGrid33;
static lv_obj_t * textLabelReadingsGrid43;

// LGVL Label Text Icons
static lv_obj_t * textLabelIconWMO;
static lv_obj_t * textLabelIconTemperature;
static lv_obj_t * textLabelIconHumidity;
static lv_obj_t * textLabelReadingsGrid31;
static lv_obj_t * textLabelReadingsGrid35;
static lv_obj_t * textLabelReadingsGrid41;
static lv_obj_t * textLabelReadingsGrid45;
static lv_obj_t * textLabelIconInside;
static lv_obj_t * textLabelIconOutside;
static lv_obj_t * textLabelIconInsideTemperature;
static lv_obj_t * textLabelIconInsideHumidity;
static lv_obj_t * textLabelIconSunrise;
static lv_obj_t * textLabelIconSunset;
static lv_obj_t * textLabelIconMoonrise;
static lv_obj_t * textLabelIconMoonset;
static lv_obj_t * textLabelIconMoonPhase;

// LVGL Other Objects
static lv_obj_t * weatherIconBox;

// Weather readings Grid
static int32_t sensorReadingsGridColumnDsc[] = {17,65,15,60,25, LV_GRID_TEMPLATE_LAST};   /* 2 columns with 100- and 400-px width */
static int32_t sensorReadingsGridRowDsc[] = {25, 25, 25, 25, 25, LV_GRID_TEMPLATE_LAST}; /* 3 100-px tall rows */
static lv_obj_t * sensorReadingsGrid;

// Almanac readings Grid
static int32_t almanacReadingsGridColumnDsc[] = {30,120,30,120, LV_GRID_TEMPLATE_LAST};   /* 2 columns with 100- and 400-px width */
static int32_t almanacReadingsGridRowDsc[] = {25, 25, 25, LV_GRID_TEMPLATE_LAST}; /* 3 100-px tall rows */
static lv_obj_t * almanacReadingsGrid;

// TaskScheduler items
void tLvglHandlerCB() {
  
  lv_task_handler();  // let the GUI do its work
}

void tWeeWXUpdateCB() {
  getWeeWXData();
}

void tOpenMeteoUpdateCB() {
  getOpenMeteoData();
}

void tWifiMessageUpdateCB() {
  setWifiMessage();
}

void tCheckWifiManagerPinCB();
void tProcessWifiManagerCB();

void tTimerWifiManagerDisableCB() {
  wifiManagerActiveTime = 0;
}

void tTimerWifiManagerCB() {
  wifiManagerActiveTime += 1;
  setWifiMessage();
  
  if (wifiManagerActiveTime >= CYD_WWX_WM_TIMEOUT) {
    wm.stopConfigPortal();
  }
}


Task tLvglHandler(CYD_WWX_CALL_LVGL_HANDLER_EVERY, TASK_FOREVER, &tLvglHandlerCB);
Task tWeeWXUpdate(CYD_WWX_GET_WEEWX_UPDATE_EVERY, TASK_FOREVER, &tWeeWXUpdateCB);
Task tOpenMeteoUpdate(CYD_WWX_GET_OPENMETEO_UPDATE_EVERY, TASK_FOREVER, &tOpenMeteoUpdateCB);
Task tCheckWifiManagerPin(CYD_WWX_CHECK_WM_PIN_EVERY, TASK_FOREVER, &tCheckWifiManagerPinCB);
Task tProcessWifiManager(CYD_WWX_PROCESS_WM_EVERY, TASK_FOREVER, &tProcessWifiManagerCB);
Task tTimerWifiManager(CYD_WWX_ONE_SECOND_WM_TIMER, TASK_FOREVER, &tTimerWifiManagerCB, NULL, NULL, NULL, tTimerWifiManagerDisableCB);

Scheduler cydScheduler;

void displayCleanup( void )
{
  if (currentActiveDisplay == displayname::WEEWX_MAIN ) {
    lv_style_reset( &displayStyle );
    lv_style_reset( &myDayStyle );
    lv_style_reset( &myNightStyle );
    lv_style_reset( &gridStyle );
    lv_style_reset( &cellStyle );
  }
}

void displayReInit( displayname whichDisplay )
{
  lv_obj_clean ( lv_scr_act() ); // Clean objects from current screen.
  lv_obj_invalidate( lv_scr_act() ); // Invalidate objects for redraw.
  lv_refr_now( cydWeeWXDisp ); // Update display immediately.
  displayCleanup(); // Cleanup screen resources.
  lv_disp_remove( cydWeeWXDisp ); // Cleanup display registration.
  tWeeWXUpdate.disable();
  tOpenMeteoUpdate.disable();
  tLvglHandler.disable();
  lv_deinit(); // Cleanup LVGL resources.
  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);
  lv_tick_set_cb(cydWeeWXTickCB);
  tLvglHandler.enable();

  // Initialize the TFT display using the TFT_eSPI library
  cydWeeWXDisp = lv_tft_espi_create(CYD_WWX_SCREEN_WIDTH, CYD_WWX_SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(cydWeeWXDisp, LV_DISPLAY_ROTATION_90);

  if (whichDisplay == displayname::WEEWX_MAIN) {
      tWeeWXUpdate.enable();
      tOpenMeteoUpdate.enable();
      cydWeeWXErrorState = false;
      getWeeWXData();
      getOpenMeteoData();
      createMainWeeWXGui();
  } else if (whichDisplay == displayname::WIFI_MANAGER_MAIN) {
      setWifiMessage();
      createMainWifiManagerGui();
  }
}

void wifiConfigCB() {

    displayReInit(displayname::WEEWX_MAIN);
}

int wifiManagerPinCount = 0;
void tCheckWifiManagerPinCB() {
  if (!digitalRead(WM_TRIGGER_PIN)) {
    wifiManagerPinCount += 1;
  } else{
    wifiManagerPinCount = 0;
  }

  LOG_DEBUG("tCheckWifiManagerPinCB", "Trigger pin held for " << wifiManagerPinCount << "cycles");

  if (wifiManagerPinCount > CYD_WWX_WM_PIN_HOLD_COUNT) {
    LOG_DEBUG("tCheckWifiManagerPinCB", "Trigger pin pressed long enough.");
    wifiManagerPinCount = 0;

    wm.setConfigPortalTimeout(CYD_WWX_WM_TIMEOUT);
    wm.setSaveConfigCallback(wifiConfigCB);
    wm.setBreakAfterConfig(true);
    wm.setConfigPortalBlocking(false);

    tProcessWifiManager.enable();
    tTimerWifiManager.enable();
    tCheckWifiManagerPin.disable();
    if(!wm.startConfigPortal(CYD_WWX_WM_AP_NAME, cydWeeWXPassword.c_str())) {
      LOG_INFO("tCheckWifiManagerPinCB", "Failed to connect or hit timeout");
    } else {   
      LOG_INFO("tCheckWifiManagerPinCB", "WiFi connected.");
    }

    displayReInit(displayname::WIFI_MANAGER_MAIN);
  }
}

void tProcessWifiManagerCB() {
  
  if (wm.getConfigPortalActive()) {
    wm.process();
  } else {
    LOG_DEBUG("tProcessWifiManagerCB", "WM Portal no longer active.");
    if (saveCydWeeWXConfigNow) {
      saveCydWeeWxConfig();
    }
    if (WiFi.isConnected()) {
      tProcessWifiManager.disable();
      tTimerWifiManager.disable();
      tCheckWifiManagerPin.enable();
      displayReInit(displayname::WEEWX_MAIN);
    } else {
      ESP.restart();
    }
  }

}

void setSensorTrend( float trend, sensortype::sensor type) {
  String trendString = String();

  LOG_DEBUG("setSensorTrend", "Sensor <" << type << "> trend: " << trend << " vs " << trendLimitsForSensors[type].lowLimit << " to " << trendLimitsForSensors[type].highLimit);
  
  if (abs(trend) >= trendLimitsForSensors[type].highLimit) {
    if (trend >= 0) {
      trendString = String(WI_DIRECTION_UP);
      LOG_DEBUG("setSensorTrend", "UP");
    } else {
      trendString = String(WI_DIRECTION_DOWN);
      LOG_DEBUG("setSensorTrend", "DOWN");
    } 
  } else if (abs(trend) >= trendLimitsForSensors[type].lowLimit) {
    if (trend >= 0) {
      trendString = String(WI_DIRECTION_UP_RIGHT);
      LOG_DEBUG("setSensorTrend", "UP_RIGHT");
    } else {
      trendString = String(WI_DIRECTION_DOWN_RIGHT);
      LOG_DEBUG("setSensorTrend", "DOWN_RIGHT");
    } 
  }

  switch (type) {
  case sensortype::TEMPERATURE:
    trendTemperature = trendString;
    break;
  case sensortype::HUMIDITY:
    trendHumidity = trendString;
    break;
  case sensortype::WIND_GUST:
    trendWindGust = trendString;
    break;
  case sensortype::PRESSURE:
    trendPressure = trendString;
    break;
  case sensortype::RAIN_RATE:
    trendRainRate = trendString;
    break;
  default:
    break;
  }

}

static void timer_cb(lv_timer_t * timer) {
  LV_UNUSED(timer);

  if (currentActiveDisplay == displayname::WEEWX_MAIN) {
    if (isDay)
    {
      lv_obj_remove_style(weatherIconBox, &myNightStyle, 0);
      lv_obj_remove_style(weatherIconBox, &myDayStyle, 0);
      lv_obj_add_style(weatherIconBox, &myDayStyle, 0);
      lv_obj_set_style_text_color((lv_obj_t*) textLabelIconWMO, lv_color_hex(CYD_WWX_DAY_TEXT_COLOR), 0);
      lv_obj_set_style_text_color((lv_obj_t*) textLabelWeatherDescription, lv_color_hex(CYD_WWX_DAY_TEXT_COLOR), 0);
    }
    else
    {
      lv_obj_remove_style(weatherIconBox, &myNightStyle, 0);
      lv_obj_remove_style(weatherIconBox, &myDayStyle, 0);
      lv_obj_add_style(weatherIconBox, &myNightStyle, 0);
      lv_obj_set_style_text_color((lv_obj_t*) textLabelIconWMO, lv_color_hex(CYD_WWX_NIGHT_TEXT_COLOR), 0);
      lv_obj_set_style_text_color((lv_obj_t*) textLabelWeatherDescription, lv_color_hex(CYD_WWX_NIGHT_TEXT_COLOR), 0);
    }

    if (cydWeeWXErrorState) {
      setWmoIconAndDescription(CYD_WWX_ERROR_STATE_CODE);
      lv_obj_set_style_text_color((lv_obj_t*) textLabelWeatherDescription, lv_color_hex(CYD_WWX_ERROR_TEXT_COLOR), 0);
      lv_label_set_text(textLabelWeatherDescription, errorDescription.c_str());
      lv_obj_set_style_text_color((lv_obj_t*) textLabelScreenHeader, lv_color_hex(CYD_WWX_ERROR_TEXT_COLOR), 0);
      lv_label_set_text(textLabelScreenHeader, errorDescription.c_str());
    } else {
      setWmoIconAndDescription(weatherCode);
      lv_label_set_text(textLabelWeatherDescription, weatherDescription.c_str());
      lv_label_set_text(textLabelScreenHeader, screenHeader.c_str());
    }
    
    lv_label_set_text(textLabelTemperature, temperature.c_str());
    lv_label_set_text(textLabelTrendTemperature, trendTemperature.c_str());
    lv_label_set_text(textLabelInsideTemperature, insideTemperature.c_str());
    lv_label_set_text(textLabelUnitsTemperature, unitsTemperature.c_str());
    lv_label_set_text(textLabelHumidity, humidity.c_str());
    lv_label_set_text(textLabelTrendHumidity, trendHumidity.c_str());
    lv_label_set_text(textLabelInsideHumidity, insideHumidity.c_str());
    lv_label_set_text(textLabelUnitsHumidity, unitsHumidity.c_str());
    whichReadingsToShow = !whichReadingsToShow;
    if (whichReadingsToShow) {
      lv_label_set_text(textLabelReadingsGrid31, iconWind.c_str());
      lv_label_set_text(textLabelReadingsGrid32, wind.c_str());
      lv_label_set_text(textLabelReadingsGrid33, trendWind.c_str());
      lv_label_set_text(textLabelReadingsGrid34, unitsWind.c_str());
      lv_label_set_text(textLabelReadingsGrid35, windDirection.c_str());
      lv_label_set_text(textLabelReadingsGrid41, iconWindGust.c_str());
      lv_label_set_text(textLabelReadingsGrid42, windGust.c_str());
      lv_label_set_text(textLabelReadingsGrid43, trendWindGust.c_str());
      lv_label_set_text(textLabelReadingsGrid44, unitsWindGust.c_str());
      lv_label_set_text(textLabelReadingsGrid45, windGustDirection.c_str());
    } else {
      
      lv_label_set_text(textLabelReadingsGrid31, iconPressure.c_str());
      lv_label_set_text(textLabelReadingsGrid32, pressure.c_str());
      lv_label_set_text(textLabelReadingsGrid33, trendPressure.c_str());
      lv_label_set_text(textLabelReadingsGrid34, unitsPressure.c_str());
      lv_label_set_text(textLabelReadingsGrid35, "");
      lv_label_set_text(textLabelReadingsGrid41, iconRainRate.c_str());
      lv_label_set_text(textLabelReadingsGrid42, rainRate.c_str());
      lv_label_set_text(textLabelReadingsGrid43, trendRainRate.c_str());
      lv_label_set_text(textLabelReadingsGrid44, unitsRainRate.c_str());
      lv_label_set_text(textLabelReadingsGrid45, "");
    }
    lv_label_set_text(textLabelSunrise, sunrise.c_str());
    lv_label_set_text(textLabelSunset, sunset.c_str());
    lv_label_set_text(textLabelMoonrise, moonrise.c_str());
    lv_label_set_text(textLabelMoonset, moonset.c_str());
    lv_label_set_text(textLabelMoonPhase, moonPhase.c_str());
    lv_label_set_text(textLabelIconMoonPhase, iconMoonPhase.c_str());

  } else if (currentActiveDisplay == displayname::WIFI_MANAGER_MAIN) {
    setWifiMessage();
    lv_label_set_text(textLabelWifiManagerTimer, wifiManagerTimer.c_str());
  } else {
    LOG_ERROR("timer_cb", "No current display defined.");
  }

}

void createMainWifiManagerGui(void) {

  currentActiveDisplay = displayname::WIFI_MANAGER_MAIN;
  
  lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
  lv_style_init(&displayStyle);
  lv_style_set_bg_color(&displayStyle, lv_color_hex(CYD_WWX_WIFI_MANAGER_BG_COLOR)); 
  lv_obj_add_style(lv_screen_active(), &displayStyle, 0);

  textLabelWifiManagerMessage = lv_label_create(lv_screen_active());
  lv_label_set_text(textLabelWifiManagerMessage, wifiManagerMessage.c_str());
  lv_obj_align(textLabelWifiManagerMessage, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelWifiManagerMessage, &lv_font_montserrat_16, 0);
  lv_label_set_long_mode(textLabelWifiManagerMessage, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color((lv_obj_t*) textLabelWifiManagerMessage, lv_color_hex(CYD_WWX_WIFI_MANAGER_TEXT_COLOR), 0);
  lv_obj_set_width(textLabelWifiManagerMessage, 320);
  lv_obj_set_style_text_align(textLabelWifiManagerMessage, LV_TEXT_ALIGN_CENTER, 0);

  textLabelWifiManagerTimer = lv_label_create(lv_screen_active());
  lv_label_set_text(textLabelWifiManagerTimer, wifiManagerTimer.c_str());
  lv_obj_align(textLabelWifiManagerTimer, LV_ALIGN_CENTER, 0, 50);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelWifiManagerTimer, &lv_font_montserrat_16, 0);
  lv_label_set_long_mode(textLabelWifiManagerTimer, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color((lv_obj_t*) textLabelWifiManagerTimer, lv_color_hex(CYD_WWX_WIFI_MANAGER_TEXT_COLOR), 0);
  lv_obj_set_width(textLabelWifiManagerTimer, 320);
  lv_obj_set_style_text_align(textLabelWifiManagerTimer, LV_TEXT_ALIGN_CENTER, 0);

  lv_timer_t * timer = lv_timer_create(timer_cb, CYD_WWX_WIFI_MANAGER_LV_TIMER, NULL);
  lv_timer_ready(timer);

}

void createMainWeeWXGui(void) {

  currentActiveDisplay = displayname::WEEWX_MAIN;

  LV_FONT_DECLARE(wmoIcons_64c);
  LV_FONT_DECLARE(weathericons_22c);
  LV_FONT_DECLARE(dejaVuSansCondensed_18c);

  
  lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
  lv_style_init(&displayStyle);
  lv_style_set_bg_color(&displayStyle, lv_color_hex(CYD_WWX_WEEWX_BG_COLOR)); 
  lv_obj_add_style(lv_screen_active(), &displayStyle, 0);

  // Grid and Cell styles
  lv_style_init(&gridStyle);
  lv_style_set_pad_row(&gridStyle, 0);
  lv_style_set_pad_column(&gridStyle, 0);
  lv_style_set_pad_all(&gridStyle, 0);
  lv_style_set_bg_color(&gridStyle, lv_obj_get_style_bg_color(lv_screen_active(),0));
  lv_style_set_border_color(&gridStyle, lv_obj_get_style_bg_color(lv_screen_active(),0));

  lv_style_init(&cellStyle);
  lv_style_set_pad_all(&cellStyle, 0);

  // Day & Night Styles
  lv_style_init(&myNightStyle);
  lv_style_set_bg_color(&myNightStyle, lv_color_hex(CYD_WWX_NIGHT_BG_COLOR)); // Medium Grey
  lv_style_set_border_color(&myNightStyle, lv_color_hex(CYD_WWX_NIGHT_BG_COLOR));
  
  lv_style_init(&myDayStyle);
  lv_style_set_bg_color(&myDayStyle, lv_color_hex(CYD_WWX_DAY_BG_COLOR)); // Medium blue
  lv_style_set_border_color(&myDayStyle, lv_color_hex(CYD_WWX_DAY_BG_COLOR));

  // Weather Icon Box
  weatherIconBox = lv_obj_create(lv_scr_act());
  lv_obj_set_scrollbar_mode(weatherIconBox, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_size(weatherIconBox , 120, 120);
  lv_obj_set_pos(weatherIconBox , 5, 35);
  lv_obj_add_style(weatherIconBox, &myDayStyle, 0); 

  // Current weather icon
  textLabelIconWMO = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconWMO, &wmoIcons_64c, 0);
  lv_obj_set_parent(textLabelIconWMO, weatherIconBox);
  lv_obj_align(textLabelIconWMO, LV_ALIGN_CENTER, 0, -5);
  lv_obj_set_style_text_align(textLabelIconWMO, LV_TEXT_ALIGN_CENTER, 0);
  
  setWmoIconAndDescription(weatherCode);
  
  // Weather Description
  textLabelWeatherDescription = lv_label_create(lv_screen_active());
  lv_label_set_text(textLabelWeatherDescription, weatherDescription.c_str());
  lv_obj_set_parent(textLabelWeatherDescription, weatherIconBox);
  lv_obj_align(textLabelWeatherDescription, LV_ALIGN_BOTTOM_MID, 0, 10);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelWeatherDescription, &lv_font_montserrat_16, 0);
  lv_label_set_long_mode(textLabelWeatherDescription, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(textLabelWeatherDescription, 110);
  lv_obj_set_style_text_align(textLabelWeatherDescription, LV_TEXT_ALIGN_CENTER, 0);

  // Screen header
  textLabelScreenHeader = lv_label_create(lv_screen_active());
  lv_label_set_text(textLabelScreenHeader, screenHeader.c_str());
  lv_obj_align(textLabelScreenHeader, LV_ALIGN_CENTER, 0, -105);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelScreenHeader, &lv_font_montserrat_22, 0);
  lv_label_set_long_mode(textLabelScreenHeader, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_obj_set_width(textLabelScreenHeader, 320);
  lv_obj_set_style_text_align(textLabelScreenHeader, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color((lv_obj_t*) textLabelScreenHeader, lv_color_hex(CYD_WWX_HEADER_TEXT_COLOR), 0);

  // Sensor readings grid
  sensorReadingsGrid = lv_obj_create(lv_screen_active());
  lv_obj_set_grid_dsc_array(sensorReadingsGrid, sensorReadingsGridColumnDsc, sensorReadingsGridRowDsc);
  lv_obj_set_size(sensorReadingsGrid , 190, 200);
  lv_obj_set_pos(sensorReadingsGrid , 130, 30);
  lv_obj_add_style(sensorReadingsGrid, &gridStyle, 0);

  // Sensor grid column icons  - Outside Temp/Humidity first then Inside
  textLabelIconOutside = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconOutside, LV_GRID_ALIGN_CENTER, 1, 2, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconOutside, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelIconOutside, &cellStyle, 0);
  lv_label_set_text(textLabelIconOutside, String(LV_SYMBOL_IMAGE).c_str());

  textLabelIconInside = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconInside, LV_GRID_ALIGN_CENTER, 3, 2, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconInside, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelIconInside, &cellStyle, 0);
  lv_label_set_text(textLabelIconInside, String(LV_SYMBOL_HOME).c_str());

  // Temperature Outside then Inside
  textLabelIconTemperature = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconTemperature, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconTemperature, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconTemperature, &cellStyle, 0);
  lv_label_set_text(textLabelIconTemperature, iconTemperature.c_str());

  textLabelTemperature = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelTemperature, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelTemperature, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelTemperature, &cellStyle, 0);
  lv_label_set_text(textLabelTemperature, temperature.c_str());

  textLabelTrendTemperature = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelTrendTemperature, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelTrendTemperature, &weathericons_22c, 0);
  lv_obj_add_style(textLabelTrendTemperature, &cellStyle, 0);
  lv_label_set_text(textLabelTrendTemperature, trendTemperature.c_str());

  textLabelInsideTemperature = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelInsideTemperature, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelInsideTemperature, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelInsideTemperature, &cellStyle, 0);
  lv_label_set_text(textLabelInsideTemperature, insideTemperature.c_str());

  textLabelUnitsTemperature = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelUnitsTemperature, LV_GRID_ALIGN_START, 4, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelUnitsTemperature, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelUnitsTemperature, &cellStyle, 0);
  lv_label_set_text(textLabelUnitsTemperature, unitsTemperature.c_str());

  // Humidity Outside then Inside
  textLabelIconHumidity = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconHumidity, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconHumidity, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconHumidity, &cellStyle, 0);
  lv_label_set_text(textLabelIconHumidity, iconHumidity.c_str());
  
  textLabelHumidity = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelHumidity, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelHumidity, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelHumidity, &cellStyle, 0);
  lv_label_set_text(textLabelHumidity, humidity.c_str());

  textLabelTrendHumidity = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelTrendHumidity, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelTrendHumidity, &weathericons_22c, 0);
  lv_obj_add_style(textLabelTrendHumidity, &cellStyle, 0);
  lv_label_set_text(textLabelTrendHumidity, trendHumidity.c_str());

  textLabelInsideHumidity = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelInsideHumidity, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelInsideHumidity, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelInsideHumidity, &cellStyle, 0);
  lv_label_set_text(textLabelInsideHumidity, insideHumidity.c_str());
  
  textLabelUnitsHumidity = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelUnitsHumidity, LV_GRID_ALIGN_START, 4, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelUnitsHumidity, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelUnitsHumidity, &cellStyle, 0);
  lv_label_set_text(textLabelUnitsHumidity, unitsHumidity.c_str());

  // Sensor readings grid row 3 start with  Wind 
  textLabelReadingsGrid31 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid31, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid31, &weathericons_22c, 0);
  lv_obj_add_style(textLabelReadingsGrid31, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid31, iconWind.c_str());
  
  textLabelReadingsGrid32 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid32, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid32, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelReadingsGrid32, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid32, wind.c_str());

  textLabelReadingsGrid33 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid33, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid33, &weathericons_22c, 0);
  lv_obj_add_style(textLabelReadingsGrid33, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid33, trendWind.c_str());
  
  textLabelReadingsGrid34 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid34, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid34, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelReadingsGrid34, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid34, unitsWind.c_str());

  textLabelReadingsGrid35 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid35, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid35, &weathericons_22c, 0);
  lv_obj_add_style(textLabelReadingsGrid35, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid35, windDirection.c_str());
  
  // Sensor readings grid row 4 start with Wind Gust
  textLabelReadingsGrid41 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid41, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid41, &weathericons_22c, 0);
  lv_obj_add_style(textLabelReadingsGrid41, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid41, iconWindGust.c_str());
  
  textLabelReadingsGrid42 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid42, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid42, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelReadingsGrid42, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid42, windGust.c_str());

  textLabelReadingsGrid43 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid43, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid43, &weathericons_22c, 0);
  lv_obj_add_style(textLabelReadingsGrid43, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid43, trendWindGust.c_str());
  
  textLabelReadingsGrid44 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid44, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid44, &lv_font_montserrat_22, 0);
  lv_obj_add_style(textLabelReadingsGrid44, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid44, unitsWindGust.c_str());

  textLabelReadingsGrid45 = lv_label_create(sensorReadingsGrid);
  lv_obj_set_grid_cell(textLabelReadingsGrid45, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 4, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelReadingsGrid45, &weathericons_22c, 0);
  lv_obj_add_style(textLabelReadingsGrid45, &cellStyle, 0);
  lv_label_set_text(textLabelReadingsGrid45, windGustDirection.c_str());
  
  // Almanac readings grid
  almanacReadingsGrid = lv_obj_create(lv_screen_active());
  lv_obj_set_grid_dsc_array(almanacReadingsGrid, almanacReadingsGridColumnDsc, almanacReadingsGridRowDsc);
  lv_obj_set_size(almanacReadingsGrid , 320, 100);
  lv_obj_set_pos(almanacReadingsGrid , 30, 160);
  lv_obj_add_style(almanacReadingsGrid, &gridStyle, 0);

  // Sunrise 
  textLabelIconSunrise = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconSunrise, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconSunrise, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconSunrise, &cellStyle, 0);
  lv_label_set_text(textLabelIconSunrise, String(WI_SUNRISE).c_str());
  
  textLabelSunrise = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelSunrise, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelSunrise, &dejaVuSansCondensed_18c, 0);
  lv_obj_add_style(textLabelSunrise, &cellStyle, 0);
  lv_label_set_text(textLabelSunrise, sunrise.c_str());
  
  // Sunset
  textLabelIconSunset = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconSunset, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconSunset, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconSunset, &cellStyle, 0);
  lv_label_set_text(textLabelIconSunset, String(WI_SUNSET).c_str());
    
  textLabelSunset = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelSunset, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelSunset, &dejaVuSansCondensed_18c, 0);
  lv_obj_add_style(textLabelSunset, &cellStyle, 0);
  lv_label_set_text(textLabelSunset, sunset.c_str());
  
  // Moonrise
  textLabelIconMoonrise = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconMoonrise, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconMoonrise, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconMoonrise, &cellStyle, 0);
  lv_label_set_text(textLabelIconMoonrise, String(WI_MOONRISE).c_str());
  
  textLabelMoonrise = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelMoonrise, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelMoonrise, &dejaVuSansCondensed_18c, 0);
  lv_obj_add_style(textLabelMoonrise, &cellStyle, 0);
  lv_label_set_text(textLabelMoonrise, moonrise.c_str());
 
  // Moonset
  textLabelIconMoonset = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconMoonset, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconMoonset, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconMoonset, &cellStyle, 0);
  lv_label_set_text(textLabelIconMoonset, String(WI_MOONSET).c_str());
  
  textLabelMoonset = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelMoonset, LV_GRID_ALIGN_START, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelMoonset, &dejaVuSansCondensed_18c, 0);
  lv_obj_add_style(textLabelMoonset, &cellStyle, 0);
  lv_label_set_text(textLabelMoonset, moonset.c_str());
  
  // Moon Phase
  textLabelIconMoonPhase = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelIconMoonPhase, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelIconMoonPhase, &weathericons_22c, 0);
  lv_obj_add_style(textLabelIconMoonPhase, &cellStyle, 0);
  lv_label_set_text(textLabelIconMoonPhase, iconMoonPhase.c_str());
  
  textLabelMoonPhase = lv_label_create(almanacReadingsGrid);
  lv_obj_set_grid_cell(textLabelMoonPhase, LV_GRID_ALIGN_START, 1, 3, LV_GRID_ALIGN_CENTER, 2, 1);
  lv_obj_set_style_text_font((lv_obj_t*) textLabelMoonPhase, &lv_font_montserrat_16, 0);
  lv_obj_add_style(textLabelMoonPhase, &cellStyle, 0);
  lv_label_set_text(textLabelMoonPhase, moonPhase.c_str());

  lv_timer_t * timer = lv_timer_create(timer_cb, CYD_WWX_WEEWX_LV_TIMER, NULL);
  lv_timer_ready(timer);
}

/*
  WMO Weather interpretation codes (WW)- Code	Description
  0	Clear sky
  1, 2, 3	Mainly clear, partly cloudy, and overcast
  45, 48	Fog and depositing rime fog
  51, 53, 55	Drizzle: Light, moderate, and dense intensity
  56, 57	Freezing Drizzle: Light and dense intensity
  61, 63, 65	Rain: Slight, moderate and heavy intensity
  66, 67	Freezing Rain: Light and heavy intensity
  71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
  77	Snow grains
  80, 81, 82	Rain showers: Slight, moderate, and violent
  85, 86	Snow showers slight and heavy
  95 *	Thunderstorm: Slight or moderate
  96, 99 *	Thunderstorm with slight and heavy hail
*/
void setWmoIconAndDescription(int code) {
  
  switch (code) {
    
    case 0:
      if(isDay) 
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SUNNY).c_str());
      else 
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_CLEAR).c_str());
      weatherDescription = "CLEAR SKY";
      break;
    case 1: 
      if(isDay) 
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SUNNY_OVERCAST).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_PARTLY_CLOUDY).c_str());
      weatherDescription = "MAINLY CLEAR";
      break;
    case 2: 
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_CLOUDY).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_CLOUDY).c_str());
      weatherDescription = "PARTLY CLOUDY";
      break;
    case 3:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_CLOUDY).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_CLOUDY).c_str());
      weatherDescription = "OVERCAST";
      break;
    case 45:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_FOG).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_FOG).c_str());
      weatherDescription = "FOG";
      break;
    case 48:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_FOG).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_FOG).c_str());
      weatherDescription = "DEPOSITING RIME FOG";
      break;
    case 51:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SPRINKLE).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SPRINKLE).c_str());
      weatherDescription = "DRIZZLE LIGHT INTENSITY";
      break;
    case 53:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_RAIN).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_RAIN).c_str());
      weatherDescription = "DRIZZLE MODERATE INTENSITY";
      break;
    case 55:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SHOWERS).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SHOWERS).c_str()); 
      weatherDescription = "DRIZZLE DENSE INTENSITY";
      break;
    case 56:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_RAIN_MIX).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_RAIN_MIX).c_str());
      weatherDescription = "FREEZING DRIZZLE LIGHT";
      break;
    case 57:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SLEET).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SLEET).c_str());
      weatherDescription = "FREEZING DRIZZLE DENSE";
      break;
    case 61:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SPRINKLE).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SPRINKLE).c_str());
      weatherDescription = "RAIN SLIGHT INTENSITY";
      break;
    case 63:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_RAIN).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_RAIN).c_str());
      weatherDescription = "RAIN MODERATE INTENSITY";
      break;
    case 65:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_STORM_SHOWERS).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_STORM_SHOWERS).c_str());
      weatherDescription = "RAIN HEAVY INTENSITY";
      break;
    case 66:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SLEET).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SLEET).c_str());
      weatherDescription = "FREEZING RAIN LIGHT INTENSITY";
      break;
    case 67:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SLEET_STORM).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SLEET_STORM).c_str());
      weatherDescription = "FREEZING RAIN HEAVY INTENSITY";
      break;
    case 71:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SNOW).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SNOW).c_str());
      weatherDescription = "SNOW FALL SLIGHT INTENSITY";
      break;
    case 73:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SNOW).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SNOW).c_str());
      weatherDescription = "SNOW FALL MODERATE INTENSITY";
      break;
    case 75:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SNOW_WIND).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SNOW_WIND).c_str());
      weatherDescription = "SNOW FALL HEAVY INTENSITY";
      break;
    case 77:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SNOW).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SNOW).c_str());
      weatherDescription = "SNOW GRAINS";
      break;
    case 80:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_RAIN).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_RAIN).c_str());
      weatherDescription = "RAIN SHOWERS SLIGHT";
      break;
    case 81:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_RAIN).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_RAIN).c_str());
      weatherDescription = "RAIN SHOWERS MODERATE";
      break;
    case 82:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_STORM_SHOWERS).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_STORM_SHOWERS).c_str());
      weatherDescription = "RAIN SHOWERS VIOLENT";
      break;
    case 85:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SNOW).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SNOW).c_str());
      weatherDescription = "SNOW SHOWERS SLIGHT";
      break;
    case 86:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_SNOW).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_SNOW).c_str());
      weatherDescription = "SNOW SHOWERS HEAVY";
      break;
    case 95:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_THUNDERSTORM).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_THUNDERSTORM).c_str());
      weatherDescription = "THUNDERSTORM";
      break;
    case 96:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_HAIL).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_HAIL).c_str());
      weatherDescription = "THUNDERSTORM SLIGHT HAIL";
      break;
    case 99:
      if (isDay)
        lv_label_set_text(textLabelIconWMO, String(WI_DAY_HAIL).c_str());
      else
        lv_label_set_text(textLabelIconWMO, String(WI_NIGHT_HAIL).c_str());
      weatherDescription = "THUNDERSTORM HEAVY HAIL";
      break;
    case CYD_WWX_ERROR_STATE_CODE: 
      lv_label_set_text(textLabelIconWMO, String(WI_ERROR).c_str());
      weatherDescription = String("cydWeeWX in Error State");
      break;
    default: 
      lv_label_set_text(textLabelIconWMO, String(WI_NA).c_str());
      weatherDescription = String("WMO CODE <" + String(code) + "> NOT FOUND");
      break;
  }
}

String getWindDirectionString(double direction)
{
  String directionString = String();

  if (direction == 0 )
    directionString = String();
  else if ((direction > (360-22.5)) || (direction <= 22.5))
    directionString = String( WI_WIND_DIRECTION_360 );
  else if ((direction > 22.5) && (direction <= (45+22.5)))
    directionString = String( WI_WIND_DIRECTION_45 );
  else if ((direction > (45+22.5)) && (direction <= (90+22.5)))
    directionString = String( WI_WIND_DIRECTION_90 );
  else if ((direction > (90+22.5)) && (direction <= (135+22.5)))
    directionString = String( WI_WIND_DIRECTION_135 );
  else if ((direction > (135+22.5)) && (direction <= (180+22.5)))
    directionString = String( WI_WIND_DIRECTION_180 );
  else if ((direction > (180+22.5)) && (direction <= (225+22.5)))
    directionString = String( WI_WIND_DIRECTION_225 );
  else if ((direction > (225+22.5)) && (direction <= (270+22.5)))
    directionString = String( WI_WIND_DIRECTION_270 );
  else if ((direction > (270+22.5)) && (direction <= (315+22.5)))
    directionString = String( WI_WIND_DIRECTION_315 );
  else
    directionString = String();

  LOG_DEBUG("getWindDirectionString", "Windirection: " << direction << " Symbol: " << directionString);
  return directionString;
}

void setMoonPhaseString(int phasePercent, bool isWaxing)
{
  if (phasePercent > 95) {
    iconMoonPhase = String(WI_MOON_FULL);
    moonPhase = String("Full Moon at " + String(phasePercent) + "%");
  } else if (phasePercent > 88) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_GIBBOUS_6);
      moonPhase = String("Waxing Gibbous at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_GIBBOUS_1);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 81) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_GIBBOUS_5);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_GIBBOUS_2);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 74) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_GIBBOUS_4);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_GIBBOUS_3);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 67) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_GIBBOUS_3);
      moonPhase = String("Waxing Gibbous at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_GIBBOUS_4);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 60) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_GIBBOUS_2);
      moonPhase = String("Waxing Gibbous at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_GIBBOUS_5);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 53) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_GIBBOUS_1);
      moonPhase = String("Waxing Gibbous at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_GIBBOUS_6);
      moonPhase = String("Waning Gibbous at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 46) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_FIRST_QUARTER);
      moonPhase = String("First Quarter at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_THIRD_QUARTER);
      moonPhase = String("Third Quarter at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 39) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_CRESCENT_6);
      moonPhase = String("Waxing Crescent at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_CRESCENT_1);
      moonPhase = String("Waning Crescent at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 32) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_CRESCENT_5);
      moonPhase = String("Waxing Crescent at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_CRESCENT_2);
      moonPhase = String("Waning Crescent at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 25) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_CRESCENT_4);
      moonPhase = String("Waxing Crescent at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_CRESCENT_3);
      moonPhase = String("Waning Crescent at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 18) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_CRESCENT_3);
      moonPhase = String("Waxing Crescent at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_CRESCENT_4);
      moonPhase = String("Waning Crescent at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 11) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_CRESCENT_2);
      moonPhase = String("Waxing Crescent at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_CRESCENT_5);
      moonPhase = String("Waning Crescent at " + String(phasePercent) + "%");
    }
  } else if (phasePercent > 4) {
    if (isWaxing) {
      iconMoonPhase = String(WI_MOON_WAXING_CRESCENT_1);
      moonPhase = String("Waxing Crescent at " + String(phasePercent) + "%");
    } else {
      iconMoonPhase = String(WI_MOON_WANING_CRESCENT_6);
      moonPhase = String("Waning Crescent at " + String(phasePercent) + "%");
    }
  } else {
    iconMoonPhase = String(WI_MOON_NEW);
    moonPhase = String("New Moon at " + String(phasePercent) + "%");
  }
}

void setWifiMessage() {
  if (wm.getConfigPortalActive()) {
    char buf[256] = {};
    if (WiFi.status() == WL_CONNECTED) {
      sprintf(buf, CYD_WWX_WM_ST_SCREEN_MESSAGE, WiFi.localIP().toString().c_str());
      wifiManagerMessage = String(buf);
      memset(buf, '\0', strlen(buf));
      sprintf(buf, CYD_WWX_WM_TIMER_MESSAGE, (int)((CYD_WWX_WM_TIMEOUT - wifiManagerActiveTime)/60), (int)((CYD_WWX_WM_TIMEOUT - wifiManagerActiveTime)%60));
      wifiManagerTimer = String( buf );
    } else {
      sprintf(buf, CYD_WWX_WM_AP_SCREEN_MESSAGE, wm.getConfigPortalSSID().c_str(), cydWeeWXPassword.c_str());
      wifiManagerMessage = String(buf);
      memset(buf, '\0', strlen(buf));
      sprintf(buf, CYD_WWX_WM_TIMER_MESSAGE, (int)((CYD_WWX_WM_TIMEOUT - wifiManagerActiveTime)/60), (int)((CYD_WWX_WM_TIMEOUT - wifiManagerActiveTime)%60));
      wifiManagerTimer = String( buf );
    }
  } else {
    wifiManagerMessage = String("No idea what's happening. " + wm.getConfigPortalSSID() + " <" + String(wm.getConfigPortalActive()) + "> <" + String(wm.getWebPortalActive()) + ">");
  }
}

void getOpenMeteoData() {
  if (cydWeeWXErrorState) {
    LOG_INFO("getOpenMeteoData", "In error state, skipping GET processing.");
    return;
  }
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Construct the API endpoint
    char urlBuf[256] = {};
    
    if (!((latitude.isEmpty()) || (longitude.isEmpty()))) {
      sprintf(urlBuf, CYD_WWX_OPEN_METEO_URL, latitude, longitude);
      http.begin(String(urlBuf));
      int httpCode = http.GET(); // Make the GET request

      if (httpCode > 0) {
        String om_payload = http.getString();

        LOG_DEBUG("getOpenMeteoData","Request information:");
        LOG_DEBUG("getOpenMeteoData", om_payload);

        // Parse the JSON to extract the time
        JsonDocument om_doc;
        DeserializationError error = deserializeJson(om_doc, om_payload);
        if (!error) 
        {
          weatherCode = om_doc["current"]["weather_code"];
        } else {
          LOG_ERROR("getOpenMeteoData", "deserializeJson() OpenMeteo Response failed: " << error.c_str());
          cydWeeWXErrorState = true;
          errorDescription = String("Deserialization error, cannot retrieve WMO icon.");
        }
      } else {
        LOG_ERROR("getOpenMeteoData", "GET request failed, error: " << http.errorToString(httpCode).c_str());
        cydWeeWXErrorState = true;
        errorDescription = String("Open-Meteo GET request failed, error: " + http.errorToString(httpCode));
      }

      http.end(); // Close connection
    } else {
      LOG_ERROR("getOpenMeteoData", "Latitude or longitude is blank, cannot retrieve WMO icon.");
      cydWeeWXErrorState = true;
      errorDescription = String("Latitude or longitude is blank, cannot retrieve WMO icon.");
    }
  } else {
    LOG_ERROR("getOpenMeteoData", "Not connected to Wi-Fi");
    cydWeeWXErrorState = true;
    errorDescription = String("Not connected to Wi-Fi");

  }
}
void getWeeWXData() {
  if (cydWeeWXErrorState) {
    LOG_INFO("getWeeWXData", "In error state, skipping GET processing.");
    return;
  }
  LOG_DEBUG("getWeeWXData", "getWeeWXData:");
  if (WiFi.status() == WL_CONNECTED) {
    String weeWXJsonUrl = String(cydWeeWXUrl + CYD_WWX_WEEWX_JSON_DATA_FILE);
    HTTPClient http;
    LOG_DEBUG("getWeeWXData", "      Request WeeWX Data from: " << weeWXJsonUrl.c_str());
    http.begin(String(weeWXJsonUrl));
    int httpCode = http.GET(); // Make the GET request

    if ((httpCode > 0) && (httpCode != 404)) {
      // Check for the response
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        LOG_DEBUG("getWeeWXData", "Request information:");
        LOG_DEBUG("getWeeWXData", payload);
        // Parse the JSON to extract the time
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          const char* datetime = doc["generation"]["time"];
          float tempTemperature = doc["current"]["temperature"]["value"];
          float temperatureTrend = doc["current"]["temperature trend"]["value"];
          float tempHumidity = doc["current"]["humidity"]["value"];
          float humidityTrend = doc["current"]["humidity trend"]["value"];
          float tempInsideTemperature = doc["current"]["inside temperature"]["value"];
          float tempInsideHumidity = doc["current"]["inside humidity"]["value"];
          double tempLat = doc["station"]["latitude"];
          double tempLong = doc["station"]["longitude"];
          double tempWind = doc["current"]["wind speed"]["value"];
          float windTrend = doc["current"]["wind speed trend"]["value"];
          double tempWindGust = doc["current"]["wind gust"]["value"];
          float windGustTrend = doc["current"]["wind gust trend"]["value"];
          double tempWindDir = doc["current"]["wind direction"]["value"];
          double tempPressure = doc["current"]["barometer"]["value"];
          double pressureTrend = doc["current"]["barometer trend"]["value"];
          double tempRainRate = doc["current"]["rain rate"]["value"];
          double rainRateTrend = doc["current"]["rain rate trend"]["value"];
          int moonPhasePercent = doc["almanac"]["moon fullness"];
          bool moonWaxing = doc["almanac"]["moon waxing"];
          char tbuf[32] = {};

          LOG_DEBUG("getWeeWXData", "Time: " << datetime);
          LOG_DEBUG("getWeeWXData", "Temperature: " << tempTemperature);
          LOG_DEBUG("getWeeWXData", "Temperature trend: " << temperatureTrend);
          LOG_DEBUG("getWeeWXData", "Humidity: " << tempHumidity);
          LOG_DEBUG("getWeeWXData", "Humidity Trend: " << humidityTrend);
          LOG_DEBUG("getWeeWXData", "Inside Temperature: " << tempInsideTemperature);
          LOG_DEBUG("getWeeWXData", "Inside Humidity: " << insideHumidity);
          LOG_DEBUG("getWeeWXData", "Wind Speed: " << tempWind);
          LOG_DEBUG("getWeeWXData", "Wind trend: " << windTrend);
          LOG_DEBUG("getWeeWXData", "Wind Gust: " << tempWindGust);
          LOG_DEBUG("getWeeWXData", "Wind Gust trend: " << windGustTrend);
          LOG_DEBUG("getWeeWXData", "Wind Direction: " << tempWindDir);
          LOG_DEBUG("getWeeWXData", "Pressure: " << tempPressure);
          LOG_DEBUG("getWeeWXData", "Pressure Trend: " << pressureTrend);
          LOG_DEBUG("getWeeWXData", "Rain Rate: " << tempRainRate);
          LOG_DEBUG("getWeeWXData", "Rain Rate Trend: " << rainRateTrend);
          LOG_DEBUG("getWeeWXData", "Moon Phase %: " << moonPhasePercent);
          LOG_DEBUG("getWeeWXData", "Longitude: " << tempLong);
          LOG_DEBUG("getWeeWXData", "Latitude: " << tempLat);

          LOG_DEBUG("getWeeWXData", "      WeeWX Data received.");
          sprintf(tbuf, "%6.1f", tempTemperature);
          temperature = String(tbuf);
          unitsTemperature = String(doc["current"]["temperature"]["units"]);
          setSensorTrend( temperatureTrend, sensortype::TEMPERATURE);

          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%6.1f", tempInsideTemperature);
          insideTemperature = String(tbuf);
           
          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%2.0f", tempHumidity);
          humidity = String(tbuf);
          unitsHumidity = String(doc["current"]["humidity"]["units"]);

          setSensorTrend( humidityTrend, sensortype::HUMIDITY);

          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%2.0f", tempInsideHumidity);
          insideHumidity = String(tbuf);

          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%6.1f", tempWind);;
          wind = String(tbuf);
          unitsWind = String(doc["current"]["wind speed"]["units"]);

          setSensorTrend( windTrend, sensortype::WIND);

          windDirection = getWindDirectionString(tempWindDir);

          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%6.1f", tempWindGust);;
          windGust = String(tbuf);
          unitsWindGust = String(doc["current"]["wind gust"]["units"]);

          setSensorTrend( windGustTrend, sensortype::WIND_GUST);

          windGustDirection = getWindDirectionString(tempWindDir);

          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%4.0f", tempPressure);
          pressure = String(tbuf);
          unitsPressure = String(doc["current"]["barometer"]["units"]);

          setSensorTrend( pressureTrend, sensortype::PRESSURE);

          memset(tbuf, '\0', strlen(tbuf));
          sprintf(tbuf, "%4.0f", tempRainRate);
          rainRate = String(tbuf);
          unitsRainRate = String(doc["current"]["rain rate"]["units"]);

          setSensorTrend( rainRateTrend, sensortype::RAIN_RATE);

          // Split the datetime into date and time
          String datetime_str = String(datetime);
          int splitIndex = datetime_str.indexOf('T');

          location = String(doc["station"]["location"]);

          memset(tbuf,'\0', strlen(tbuf));
          sprintf(tbuf, "%.3f", tempLat);
          latitude = String(tbuf);
          
          memset(tbuf,'\0', strlen(tbuf));
          sprintf(tbuf, "%.3f", tempLong);
          longitude = String(tbuf);
          
          screenHeader = String(location + " (Lat:" + latitude + ", Lon:" + longitude + ") - "
                        + datetime_str.substring(0, splitIndex) + " @" + datetime_str.substring(splitIndex + 1, splitIndex + 6));

          // Almanac items
          sunrise = String(doc["almanac"]["sunrise"]);
          sunset = String(doc["almanac"]["sunset"]);
          moonrise = String(doc["almanac"]["moonrise"]);
          moonset = String(doc["almanac"]["moonset"]);
          isDay = ((int)doc["almanac"]["is day"] == 1);

          LOG_DEBUG("getWeeWXData", "Sunrise: " << sunrise);
          LOG_DEBUG("getWeeWXData", "Sunset: " << sunset);
          LOG_DEBUG("getWeeWXData", "Moonrise: " << moonrise);
          LOG_DEBUG("getWeeWXData", "Moonset: " << moonset);

          setMoonPhaseString( moonPhasePercent, moonWaxing);

        } else {

          LOG_ERROR("getWeeWXData", "deserializeJson() failed: " << error.c_str());

          cydWeeWXErrorState = true;
          errorDescription = String("WeeWX data deserializeJson() failed: " + String(error.c_str()));
        }
        
      }
    } else {
      LOG_ERROR("getWeeWXData", "GET request failed, error: " << httpCode << " - " << http.errorToString(httpCode).c_str());
      cydWeeWXErrorState = true;
      errorDescription = String("WeeWX GET request failed, error: " + String(http.errorToString(httpCode).c_str()));
    }
    http.end(); // Close connection
  } else {
    LOG_ERROR("getWeeWXData", "Not connected to Wi-Fi");
    cydWeeWXErrorState = true;
    errorDescription = String("Not connected to Wi-Fi");
  }
}

uint32_t cydWeeWXTickCB() {
  return (esp_timer_get_time() / 1000LL);
}

void loadCydWeeWXConfig() {

  cydWeeWXPreference.begin(CYD_WWX_PREFERENCES_NAMESPACE, CYD_WWX_PREFERENCES_RO);

  if (!cydWeeWXPreference.isKey(CYD_WWX_PREFERENCES_KEY)) {
    cydWeeWXPreference.end();
    cydWeeWXPreference.begin(CYD_WWX_PREFERENCES_NAMESPACE, CYD_WWX_PREFERENCES_RW);
    cydWeeWXPreference.putString(CYD_WWX_PREFERENCES_KEY, cydWeeWXUrl);
    cydWeeWXPreference.end();
    cydWeeWXPreference.begin(CYD_WWX_PREFERENCES_NAMESPACE, CYD_WWX_PREFERENCES_RO); 
  }

  cydWeeWXUrl = cydWeeWXPreference.getString(CYD_WWX_PREFERENCES_KEY);
  LOG_DEBUG("loadCydWeeWXConfig", "Load config from preferences: " << cydWeeWXUrl.c_str());
  cydWeeWXPreference.end();
}

void saveCydWeeWxConfig() {
  char buf[128] = {};
  saveCydWeeWXConfigNow = false;

  strlcpy(buf, wmWeeWXUrl->getValue(), sizeof(buf));
  cydWeeWXUrl = String(buf);

  // Add "/" to url if its missing
  if (!cydWeeWXUrl.endsWith("/")) {
    cydWeeWXUrl = String(cydWeeWXUrl + "/");
  }
  LOG_DEBUG("saveCydWeeWxConfig","Save config to preferences: " << cydWeeWXUrl.c_str());
  cydWeeWXPreference.begin(CYD_WWX_PREFERENCES_NAMESPACE, CYD_WWX_PREFERENCES_RW);
  cydWeeWXPreference.putString(CYD_WWX_PREFERENCES_KEY, cydWeeWXUrl);
  cydWeeWXPreference.end();
}

void saveCydWeeWxConfigCB() {
  
  saveCydWeeWXConfigNow = true;
  LOG_DEBUG("saveCydWeeWxConfigCB", "saveCydWeeWXConfigNow = true;");
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  delay(3000);
  Serial.setDebugOutput(true); 
  LOG_INFO("setup", "Starting"); 
  LOG_INFO("setup", LVGL_Arduino);

  // Load WeeWX URL from Preferences
  loadCydWeeWXConfig();

  // Set up WiFi Manager
  if (!ledcAttach(CYD_WWX_BL_PIN, CYD_WWX_BL_BASE_FREQ, CYD_WWX_BL_TIMER_8_BIT)) {
    LOG_ERROR("setup", "ledcAttach failure for pin: " << CYD_WWX_BL_PIN);
  } else {
    pinMode(WM_TRIGGER_PIN, INPUT);  // Pin to detect to activate WiFi Manager Portal
  }

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP  
  
  wmWeeWXUrl = new WiFiManagerParameter("URL", "WeeWX URL", cydWeeWXUrl.c_str(), CYD_WWX_WEEWX_URL_FIELD_LENGTH);
  
  wm.addParameter(wmWeeWXUrl);
  wm.setSaveParamsCallback(saveCydWeeWxConfigCB);
  wm.setHostname(CYD_WWX_HOSTNAME);
  wm.setWiFiAutoReconnect(true);
  wm.setTitle(CYD_WWX_WM_TITLE);
  
  std::vector<const char *> menu = {"param","wifi","info","sep", "update", "restart","exit"};
  wm.setMenu(menu);

  wm.setConfigPortalTimeout(CYD_WWX_WM_TIMEOUT); // auto close configportal after n seconds
  
  wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails
  
  wm.setConfigPortalBlocking(false);
  wm.setHostname(CYD_WWX_HOSTNAME);

  #ifdef WIFI_MANAGER_ENABLE_PASSWORD_RANDOM
  char buf[32] = {};
  cydWeeWXPassword = String(cydWeeWXPassword + "%04d");
  sprintf(buf, cydWeeWXPassword.c_str(), random(10000));
  cydWeeWXPassword = String(buf);
  #endif

  LOG_INFO("setup", "Starting WiFi Manager AP: " << CYD_WWX_WM_AP_NAME << " Password: " << cydWeeWXPassword);

  if(!wm.autoConnect(CYD_WWX_WM_AP_NAME, cydWeeWXPassword.c_str())) {
    LOG_INFO("setup", "Failed to connect or hit timeout");
  } else {   
    LOG_INFO("setup", "WiFi Connected");
  }


  // Start LVGL
  lv_init();
  // Register print function for debugging
  lv_log_register_print_cb(log_print);
  lv_tick_set_cb(cydWeeWXTickCB);

  // Initialize the TFT display using the TFT_eSPI library
  cydWeeWXDisp = lv_tft_espi_create(CYD_WWX_SCREEN_WIDTH, CYD_WWX_SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(cydWeeWXDisp, CYD_WWX_ROTATE_SCREEN);

  if (!ledcWrite(CYD_WWX_BL_PIN, brightness)) {
    LOG_ERROR("setup", "Backlight could not be set on pin: " << CYD_WWX_BL_PIN);
  }

  // Set up Task Scheduler
  cydScheduler.init();
  cydScheduler.addTask(tLvglHandler);
  cydScheduler.addTask(tWeeWXUpdate);
  cydScheduler.addTask(tOpenMeteoUpdate);
  cydScheduler.addTask(tCheckWifiManagerPin);
  cydScheduler.addTask(tProcessWifiManager);
  cydScheduler.addTask(tTimerWifiManager);
  

  if (WiFi.status() == WL_CONNECTED) {
    LOG_INFO("setup", "Wifi is connected.");
    
    createMainWeeWXGui();
    getWeeWXData();
    getOpenMeteoData();
    
    tWeeWXUpdate.enable();
    tOpenMeteoUpdate.enable();
    tCheckWifiManagerPin.enable();

  } else {
    LOG_INFO("setup", "Wifi is NOT connected.");
    setWifiMessage();
    createMainWifiManagerGui();

    tProcessWifiManager.enable();
    tTimerWifiManager.enable();
  }

  
  tLvglHandler.enable();

  // Set up scheduler and tasks
  
  cydScheduler.startNow();

  LOG_INFO("setup", "Exiting setup.");
}

void loop() {
  // Only need task scheduler in the loop
  cydScheduler.execute();

}