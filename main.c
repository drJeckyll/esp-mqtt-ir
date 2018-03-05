#include <c_types.h>
#include <ets_sys.h>
#include <osapi.h>
#include <gpio.h>
#include <os_type.h>
#include <user_interface.h>
#include <mem.h>

#include "main.h"
#include "config.h"
#include "uart.h"
#include "wifi.h"
#include "mqtt.h"
#include "ota.h"
#include "debug.h"

#include "ir.h"
#include "dht22.h"

MQTT_Client mqttClient;

void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status)
{
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	} else {
		MQTT_Disconnect(&mqttClient);
	}
}

void mqttSendSettings(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;

	char buff[100] = "";
	float * r= readDHT();
	int temp = (int)(r[0] * 100);
	int humid = (int)(r[1] * 100);

	os_sprintf(buff, "{\"temp\":%d,\"humid\":%d,\"light\":%d,\"rom\":%d,\"device\":\"%s\"}",
		temp,
		humid,
		system_adc_read(),
		rboot_get_current_rom(),
		DEVICE
	);
	MQTT_Publish(client, MQTT_TOPIC_SETTINGS_REPLY, buff, os_strlen(buff), 0, 0);
}

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Connected\r\n");

	MQTT_Subscribe(client, MQTT_TOPIC_IR, 0);
	MQTT_Subscribe(client, MQTT_TOPIC_SETTINGS, 0);
	MQTT_Subscribe(client, MQTT_TOPIC_UPDATE, 0);
	MQTT_Subscribe(client, MQTT_TOPIC_RESTART, 0);

	mqttSendSettings(args);
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args)
{
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

int a = 0;
void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	char *topicBuf = (char*)os_zalloc(topic_len + 1),
	     *dataBuf  = (char*)os_zalloc(data_len + 1);

	MQTT_Client* client = (MQTT_Client*)args;

	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;

	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;

	INFO("Receive topic: %s, data: %s\r\n", topicBuf, dataBuf);

	if (os_strcmp(topicBuf, MQTT_TOPIC_IR) == 0)
	{
		char *token, *endtoken;
		int t, apply = 0;
		char cmd[20], val[500];
		for (t = 1; t < os_strlen(dataBuf); t++) dataBuf[t - 1] = dataBuf[t]; // remove leading {
		dataBuf[os_strlen(dataBuf) - 2] = '\0'; // remove trailing }
		token = strtok_r(dataBuf, ",", &endtoken);
		while (token != NULL) {
			int i = 0;
			char *tmp, *endtmp;
			tmp = strtok_r(token, ":", &endtmp);
			while (tmp != NULL) {
				for (t = 1; t < os_strlen(tmp); t++) tmp[t - 1] = tmp[t]; // remove leading "
				tmp[os_strlen(tmp) - 2] = '\0'; // remove trailing "
				if (i == 0) os_strcpy(cmd, tmp);
				if (i == 1) os_strcpy(val, tmp);

				tmp = strtok_r(NULL, ":", &endtmp);
				i++;
			}
				
			if (os_strcmp(cmd, "freq") == 0) {
				irMsg.freq = atoi(val);
				INFO("freq: %d\r\n", irMsg.freq);
			} else
			if (os_strcmp(cmd, "one") == 0) {
				i  = 0;
				tmp = strtok_r(val, " ", &endtmp);
				while (tmp != NULL) {
					if (i == 0) irMsg.one_pulse = atoi(tmp);
					if (i == 1) irMsg.one_pause = atoi(tmp);

					tmp = strtok_r(NULL, " ", &endtmp);
					i++;
				}
				INFO("one: %d %d\r\n", irMsg.one_pulse, irMsg.one_pause);
			} else
			if (os_strcmp(cmd, "zero") == 0) {
				i  = 0;
				tmp = strtok_r(val, " ", &endtmp);
				while (tmp != NULL) {
					if (i == 0) irMsg.zero_pulse = atoi(tmp);
					if (i == 1) irMsg.zero_pause = atoi(tmp);

					tmp = strtok_r(NULL, " ", &endtmp);
					i++;
				}
				INFO("zero: %d %d\r\n", irMsg.zero_pulse, irMsg.zero_pause);
			} else
			if (os_strcmp(cmd, "before") == 0) {
				i  = 0;
				tmp = strtok_r(val, " ", &endtmp);
				INFO("before:");
				while (tmp != NULL) {
					irMsg.before[i] = atoi(tmp);
					INFO(" %d", irMsg.before[i]);

					tmp = strtok_r(NULL, " ", &endtmp);
					i++;
				}
				irMsg.before_len = i;
				INFO(" = %d\r\n", irMsg.before_len);
			} else
			if (os_strcmp(cmd, "after") == 0) {
				i  = 0;
				tmp = strtok_r(val, " ", &endtmp);
				INFO("after:");
				while (tmp != NULL) {
					irMsg.after[i] = atoi(tmp);
					INFO(" %d", irMsg.after[i]);

					tmp = strtok_r(NULL, " ", &endtmp);
					i++;
				}
				irMsg.after_len = i;
				INFO(" = %d\r\n", irMsg.after_len);
			} else
			if (os_strcmp(cmd, "const") == 0) {
				i  = 0;
				tmp = strtok_r(val, " ", &endtmp);
				INFO("subst:");
				while (tmp != NULL) {
					irMsg.subst[i] = atoi(tmp);
					INFO(" %d", irMsg.subst[i]);

					tmp = strtok_r(NULL, " ", &endtmp);
					i++;
				}
				irMsg.subst_len = i;
				INFO(" = %d\r\n", irMsg.subst_len);
			} else
			if (os_strcmp(cmd, "data") == 0) {
				i  = 0;
				tmp = strtok_r(val, " ", &endtmp);
				INFO("data:");
				while (tmp != NULL) {
					if (tmp[0] == '$') {
						irMsg.data[i] = 500 + (tmp[1] - 0x30);
					} else {
						irMsg.data[i] = strtoul(tmp, NULL, 16);
					}
					INFO(" %d", irMsg.data[i]);

					tmp = strtok_r(NULL, " ", &endtmp);
					i++;
				}
				irMsg.data_len = i;
				INFO(" = %d\r\n", irMsg.data_len);
			}

			token = strtok_r(NULL, ",", &endtoken);
		}

		ir_send();

	} else
	if (os_strcmp(topicBuf, MQTT_TOPIC_UPDATE) == 0)
	{
		OtaUpdate();
	} else
	if (os_strcmp(topicBuf, MQTT_TOPIC_RESTART) == 0)
	{
		system_restart();
	} else 
	if (os_strcmp(topicBuf, MQTT_TOPIC_SETTINGS) == 0)
	{
		mqttSendSettings(args);
	}

	os_free(topicBuf);
	os_free(dataBuf);
}

void ICACHE_FLASH_ATTR
user_init()
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);

	INFO("Starting up...\r\n");

	INFO("Loading config...\r\n");
	CFG_Load();

	gpio_init();

	INFO("Initializing MQTT...\r\n");
	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

	INFO("Connect to WIFI...\r\n");
	WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);

	// ir
	INFO("Setup IR...\r\n");
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	gpio_output_set(0, BIT4, BIT4, 0);

	ioInit();

	// DHT22 init
	DHTInit();

	INFO("Startup completed. Now running from rom %d...\r\n", rboot_get_current_rom());
}
