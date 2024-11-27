# cydWeeWX
Simple WeeWX Weather Station Display for the Cheap Yellow Display [(CYD)](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display).

<p align="center">
  <img src="./images/cydWeeWX.gif" />
</p>

The cydWeeWX is designed for the ESP32-Cheap-Yellow-Display (CYD) which provides a  320 x 240 2.8" LCD display with a touch screen and embedded ESP32. The CYD can usually be found for $10 to $20 from [AliExpress](https://www.aliexpress.com/w/wholesale-esp32-cheap-yellow-display.html?spm=a2g0o.productlist.auto_suggest.2.379fEl3aEl3atZ).

Functionality is fairly simple with a single main display screen and no touch capability implemented. All information except for the WMO Icon and description are pulled from your WeeWX server. The WMO code used for the WMO Icon and Description is pulled from [Open-Meteo](https://open-meteo.com/) using the latitude and longitude from WeeWX. 

The main display shows current weather data including:

* A header with the location, latitude, longitude and last WeeWX weather update date and time.
* WMO icon and weather description from Open-Meteo. Open-Meteo updates this every 15 minutes with the free API service.
* Outdoor and indoor temperature with a trend indicator for the outdoor temperature.
* Outdoor and indoor humidity with a trend indicator for the outdoor humidity.
* Barometric pressure with trend indicator.
* Rain rate with trend indicator.
* Wind speed with trend indicator and wind direction indicator.
* Wind gusts with trend indicator and wind direction indicator.
* Sunrise and Sunset.
* Moonrise and Moonset.
* Moon phase.

All units shown are based on the units provided by the WeeWX data query. Similarly the time is shown assuming the WeeWX query provides local time. The WeeWX data query relies on the installation of [WeeWX-JSON extension](https://github.com/teeks99/weewx-json) modified to also generate a custom JSON report with a new report template, [cyd_weewx.json.tmpl](./WeeWX/cyd_weewx.json.tmpl).

The cydWeeWX device makes use of Arduino [WiFi Manager](https://github.com/tzapu/WiFiManager) library to implement a configuration portal. The configuration portal allows you to set up the WiFi configuration as well as input the URL for your WeeWX server.
