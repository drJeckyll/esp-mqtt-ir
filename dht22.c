
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include "ets_sys.h"
#include "osapi.h"
#include "c_types.h"
#include "user_interface.h"
#include "espconn.h"
#include "mem.h"
#include "gpio.h"
#include "dht22.h"
#include "debug.h"

#define MAXTIMINGS 10000
#define BREAKTIME 20
 
  float * ICACHE_FLASH_ATTR
readDHT(void)
{
	static float r[2];
    int counter = 0;
    int laststate = 1;
    int i = 0;
    int j = 0;
    int checksum = 0;
    int bitidx = 0;
    int bits[250];

    int data[100];

    data[0] = data[1] = data[2] = data[3] = data[4] = 0;

	GPIO_OUTPUT_SET(DHT_PIN, 1);
    os_delay_us(250000);
    GPIO_OUTPUT_SET(DHT_PIN, 0);
    os_delay_us(20000);
    GPIO_OUTPUT_SET(DHT_PIN, 1);
    os_delay_us(40);
    GPIO_DIS_OUTPUT(DHT_PIN);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);

    // wait for pin to drop?
    while (GPIO_INPUT_GET(DHT_PIN) == 1 && i<100000) {
          os_delay_us(1);
          i++;
    }

	if (i == 100000) {
		r[0] = 0;
		r[1] = 0;
		INFO("GET OUT\r\n");
		return r;
	}

    // read data!

	INFO("i: %d\r\n", i);

    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while ( GPIO_INPUT_GET(DHT_PIN) == laststate) {
            counter++;
          	os_delay_us(1);
            if (counter == 1000)
                break;
        }
        laststate = GPIO_INPUT_GET(DHT_PIN);
        if (counter == 1000) break;

        bits[bitidx++] = counter;

        if (( i > 3) && (i % 2 == 0)) {
            // shove each bit into the storage bytes
            data[j / 8] <<= 1;
            if (counter > BREAKTIME)
                data[j / 8] |= 1;
            j++;
        }
    }

    for (i=3; i<bitidx; i+=2) {
        INFO("bit %d: %d\r\n", i-3, bits[i]);
        INFO("bit %d: %d (%d)\r\n", i-2, bits[i+1], bits[i+1] > BREAKTIME);
    }
    INFO("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", j, data[0], data[1], data[2], data[3], data[4]);

    float temp_p, hum_p;
    if (j >= 39) {
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
        if (data[4] == checksum) {
            // yay! checksum is valid 
            
            if (DHT11)
            {
            	hum_p = data[0];
            	temp_p = data[2];
			} else {
            	hum_p = data[0] * 256 + data[1];
            	hum_p /= 10;

            	temp_p = (data[2] & 0x7F) * 256 + data[3];
            	temp_p /= 10.0;
            	if (data[2] & 0x80)
                	temp_p *= -1;
            }
			INFO("Temp =  %d *C, Hum = %d %%\r\n", (int)(temp_p * 100), (int)(hum_p * 100));
			r[0] = temp_p;
			r[1] = hum_p;
            
        }
    }

	return r;
}

void ICACHE_FLASH_ATTR DHT(void) {
	 readDHT();
}


void DHTInit() {
    //Set GPIO2 to output mode for DHT22
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);
}
