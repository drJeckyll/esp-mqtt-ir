#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define CFG_HOLDER	0x00FF55A1	/* Change this value to load default configurations */
#define CFG_LOCATION 0x7c		/* After first rom - you can use this or next one */
//#define CFG_LOCATION 0xfc		/* After second rom - this will be aresed with blank4.bin after flashing - don't use it - system params according Espressif */

#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 /*second*/
#define QUEUE_BUFFER_SIZE		 		2048

/* for SSL */
#define CLIENT_SSL_ENABLE

#define MQTT_TOPIC_IR				MQTT_PREFIX

#define MQTT_TOPIC_UPDATE			MQTT_PREFIX"/update"

#define MQTT_TOPIC_RESTART			MQTT_PREFIX"/restart"

#define MQTT_TOPIC_SETTINGS			MQTT_PREFIX"/settings"
#define MQTT_TOPIC_SETTINGS_REPLY	MQTT_PREFIX"/settings/reply"

//#define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
#define PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/

#endif
