# esp-mqtt-ir

I decide to make something like universal remote and this is my take on IR blaster with ESP8266.

It works with my AC (Daikin - you can find more information about it

This firmware expect IR commands in JSON format, which can be formed also from lircd.conf file:
{
  "freq": "38000", // carrier frequency
  "one": "", // 398 1300
  "zero": "", // 398 451
   "const": "", // optional - useful for remotes like Daikin
   "before": "", // header + plead - if exists
   "data": "", // data to be send
   "after": "" // ptrail + gap - if exists
}

Settings for WIFI, MQTT and OTA are in Makefile, or you can set them via env:

```DEVICE=test WIFI_SSID=Test make```

You must set DEVICE since it is used both OTA and MQTT.

You should send data to MQTT_TOPIC_PREFIX

Settings can be received on MQTT_PREFIX"/settings/reply", after send something to MQTT_PREFIX"/settings".

If you want to use this project unmodified you must have at least 16Mbit flash (2MB).

Schematics is something like: http://alexba.in/blog/2013/06/08/open-source-universal-remote-parts-and-pictures/

To reboot ESP8266 just send something to MQTT_PREFIX"restart".

To perform OTA update, first compile rom0.bin and rom1.bin. Put them on web server which can be accessed by http://OTA_HOST:OTA_PORT/OTA_PATH. For example:
```
OTA_HOST="192.168.1.1"
OTA_PORT=80
OTA_PATH="/firmware/"
```

Then just send someting to MQTT_PREFIX"update". After 10-15 seconds update will be done. 

There is no version control of bin files. Update is performed every time no matter if it is old ot new bin file.

If you use BOOT_CONFIG_CHKSUM and BOOT_IROM_CHKSUM (and you should - see warning bellow) and update failed device will return with old bin. You can check which bin is loaded by check settings and see rom:0 (for example). After update succes it will be rom:1. Else it will be rom:0 again, so you must perform update again.

You need:
* rboot boot loader: https://github.com/raburton/rboot
* esptool2: https://github.com/raburton/esptool2

**WARNING:** rboot must be compiled with BOOT_CONFIG_CHKSUM and BOOT_IROM_CHKSUM in rboot.h or it will not boot.

You can remove -iromchksum from FW_USER_ARGS in Makefile and use default settings but OTA update will be unrealible - corrupt roms & etc.

If BOOT_CONFIG_CHKSUM and BOOT_IROM_CHKSUM are enabled then rBoot wi-iromchksumll recover from last booted rom and OTA update is much more stable.

This code was tested with esp-open-sdk (SDK 1.4.0). Flash size 1MB (8Mbit) or more.
