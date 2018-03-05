
#include "ir.h"
#include <string.h>
#include <osapi.h>
#include "c_types.h"
#include "gpio.h"
#include "debug.h"
#include "config.h"
#include "dht22.h"

IR_MSG irMsg;

void ICACHE_FLASH_ATTR to_bin(int value, int bitsCount, char *output) {
	int i;

	output[bitsCount] = '\0';
	for (i = bitsCount - 1; i >= 0; --i, value >>= 1)
	{
		output[i] = (value & 1) + '0';
	}
}

int in_ir_send = 0;
void ICACHE_FLASH_ATTR ir_send()
{
	int t, c, p, cycle, i = 0;
	int opu = irMsg.one_pulse / (1000000 / irMsg.freq);
	int opa = irMsg.one_pause;
	int zpu = irMsg.zero_pulse / (1000000 / irMsg.freq);
	int zpa = irMsg.zero_pause;
	int ir_freq;
	char *token, *endtoken;
	char bin[10];

	INFO("one*: %d %d\r\n", opu, opa);
	INFO("zero*: %d %d\r\n", zpu, zpa);

	cycle = ((1000000 / irMsg.freq) / 2);
	INFO("cycle: %d\r\n", cycle);

	ets_wdt_disable();
	ets_intr_lock();

	// before
	for (t = 0; t < irMsg.before_len; t++)
	{
		if (t % 2 == 0)
		{
			//INFO("before pulse: %d\r\n", irMsg.before[t]);
			p = irMsg.before[t] / (1000000 / irMsg.freq);
			for (c = 0; c < p; c++)
			{
				GPIO_OUTPUT_SET(IR_PIN, 1);
				os_delay_us(cycle);
				GPIO_OUTPUT_SET(IR_PIN, 0);
				os_delay_us(cycle);
			}
		} else {
			//INFO("before pause: %d\r\n", irMsg.before[t]);
			GPIO_OUTPUT_SET(IR_PIN, 0);
			os_delay_us(irMsg.before[t]);
		}
	}

	// data
	for (t = 0; t < irMsg.data_len; t++)
	{
		if (irMsg.data[t] > 500) {
			c = (irMsg.data[t] - 500 - 1) * 2;
			p = irMsg.subst[c] / (1000000 / irMsg.freq);
			//INFO("subst pulse: %d\r\n", irMsg.subst[c]);
			for (i = 0; i < p; i++)
			{
				GPIO_OUTPUT_SET(IR_PIN, 1);
				os_delay_us(cycle);
				GPIO_OUTPUT_SET(IR_PIN, 0);
				os_delay_us(cycle);
			}
			p = irMsg.subst[c + 1];
			//INFO("subst pause: %d\r\n", irMsg.subst[c + 1]);
			GPIO_OUTPUT_SET(IR_PIN, 0);
			os_delay_us(p);
		} else {
			to_bin(irMsg.data[t], 8, bin);
			//INFO("data: %d %s\r\n", t, bin);

			for (c = 0; c < 8; c++)
			{
				if (bin[c] == '0') {
					//INFO("data zero pulse: %d\r\n", irMsg.zero_pulse);
					for (i = 0; i < zpu; i++)
					{
						GPIO_OUTPUT_SET(IR_PIN, 1);
						os_delay_us(cycle);
						GPIO_OUTPUT_SET(IR_PIN, 0);
						os_delay_us(cycle);
					}
					//INFO("data zero pause: %d\r\n", irMsg.zero_pause);
					GPIO_OUTPUT_SET(IR_PIN, 0);
					os_delay_us(zpa);
				} else {
					//INFO("data one pulse: %d\r\n", irMsg.one_pulse);
					for (i = 0; i < opu; i++)
					{
						GPIO_OUTPUT_SET(IR_PIN, 1);
						os_delay_us(cycle);
						GPIO_OUTPUT_SET(IR_PIN, 0);
						os_delay_us(cycle);
					}
					//INFO("data one pause: %d\r\n", irMsg.one_pause);
					GPIO_OUTPUT_SET(IR_PIN, 0);
					os_delay_us(opa);
				}
			}
		}
	}

	// after
	for (t = 0; t < irMsg.after_len; t++)
	{
		if (t % 2 == 0)
		{
			//INFO("after pulse: %d\r\n", irMsg.after[t]);
			p = irMsg.after[t] / (1000000 / irMsg.freq);
			for (c = 0; c < p; c++)
			{
				GPIO_OUTPUT_SET(IR_PIN, 1);
				os_delay_us(cycle);
				GPIO_OUTPUT_SET(IR_PIN, 0);
				os_delay_us(cycle);
			}
		} else {
			//INFO("after pause: %d\r\n", irMsg.after[t]);
			GPIO_OUTPUT_SET(IR_PIN, 0);
			os_delay_us(irMsg.after[t]);
		}
	}

	ets_intr_unlock();
	ets_wdt_enable();

	return;

	// daikin
	char dataBuf[5000] = "37683 449 449 452 444 451 25300 3397 1725 398 1300 398 451 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 1300 398 451 398 1300 398 1300 398 451 398 1300 398 1300 398 1300 398 1300 398 1300 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 451 398 1300 398 451 398 451 398 451 398 1300 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 1300 398 1300 398 451 398 1300 398 451 398 1300 398 1300 398 29456 3397 1725 398 1300 398 451 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 1300 398 451 398 1300 398 1300 398 451 398 1300 398 1300 398 1300 398 1300 398 1300 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 1300 398 451 398 1300 398 1300 398 451 398 1300 398 1300 398 1300 398 451 398 1300 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 451 398 451 398 451 398 29456 3397 1725 398 1300 398 451 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 1300 398 451 398 1300 398 1300 398 451 398 1300 398 1300 398 1300 398 1300 398 1300 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 451 398 451 398 1300 398 451 398 451 398 1300 398 1300 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 1300 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 1300 398 451 398 451 398 1300 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 451 398 1300 398 451 398 1300 398 1300 398 1300 398 1300 398 451 398 99142 398\0";
	// chunlan
	//char dataBuf[5000] = "38000 8380 4500 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 1750 500 683 500 683 500 1750 500 683 500 683 500 1750 500 1750 500 1750 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 683 500 1750 500 1750 500 683 500 683 500 683 500 683 500 2000";

	if (in_ir_send) return ;

	ets_wdt_disable();
	ets_intr_lock();

	token = strtok_r(dataBuf, " ", &endtoken);
	while (token != NULL) {
		if (i == 0) {
			ir_freq = atoi(token);
			cycle = ((1000000 / ir_freq) / 2);
			//INFO("freq: %d\r\n", ir_freq);
			//INFO("cycle: %d\r\n", cycle);
		} else {
			//buf[i - 1] = atoi(token);
			//INFO("b: %d\t%d\r\n", i - 1, buf[0]);
			if (i % 2 == 0)
			{
				p = atoi(token);
				GPIO_OUTPUT_SET(IR_PIN, 0);
				os_delay_us(p);
				//INFO("space: %d\r\n", p);
			} else {
				p = atoi(token) / (1000000 / ir_freq);
				for (c = 0; c < p; c++)
				{
					GPIO_OUTPUT_SET(IR_PIN, 1);
					os_delay_us(cycle);
					GPIO_OUTPUT_SET(IR_PIN, 0);
					os_delay_us(cycle);
				}
				//INFO("pulse: %d\r\n", atoi(token));
			}
		}

		token = strtok_r(NULL, " ", &endtoken);
		i++;
	}
	
	ets_intr_unlock();
	ets_wdt_enable();
	
return;

	cycle = ((1000000 / ir_freq) / 2);

	in_ir_send = 1;

	//ets_wdt_disable();
	//ets_intr_lock();

	//for (c = 0; c < i - 1; c = c + 2)
	for (c = 0; c < 100; c = c + 2)
	{
		//INFO("I: %d = %d\r\n", c, 0);//, buf[c + 1]);
		/*for (t = 0; t < buf[c] / (1000000 / ir_freq); t++)
		{
			GPIO_OUTPUT_SET(IR_PIN, 1);
			os_delay_us(cycle);
			GPIO_OUTPUT_SET(IR_PIN, 0);
			os_delay_us(cycle);
		}
		GPIO_OUTPUT_SET(IR_PIN, 0);
		os_delay_us(buf[c + 1]);*/
	}

	//ets_intr_unlock();
	//ets_wdt_enable();

	in_ir_send = 0;

	/*
	int lead_pulse = 4510 / (1000000 / ir_freq);
	int lead_space = 4441;

	int pulse_pulse = 573 / (1000000 / ir_freq);
	int pulse_space = 1671;

	int space_pulse = 573 / (1000000 / ir_freq);
	int space_space = 562;

	int end = 579 / (1000000 / ir_freq);
	
	//int cycle = ((1000000 / ir_freq) / 2);

	if (in_ir_send) return ;

	// try to parse and set
	// shift 1 byte left - remove leading {
	//char *token, *endtoken;
	//char cmd[20], val[5000];
	//char ir_cmd[80000];
	for (t = 1; t < os_strlen(dataBuf); t++) dataBuf[t - 1] = dataBuf[t];
	dataBuf[os_strlen(dataBuf) - 2] = '\0'; // remove trailing }
	token = strtok_r(dataBuf, ",", &endtoken);
	while (token != NULL) {
		int i = 0;
		char *tmp, *endtmp;
		tmp = strtok_r(token, ":", &endtmp);
		while (tmp != NULL) {
			// trim "
			for (t = 1; t < os_strlen(tmp); t++) tmp[t - 1] = tmp[t];
			tmp[os_strlen(tmp) - 2] = '\0';
			if (i == 0) os_strcpy(cmd, tmp);
			if (i == 1) os_strcpy(val, tmp);

			tmp = strtok_r(NULL, ":", &endtmp);
			i++;
		}

		if (os_strcmp(cmd, "freq") == 0) {
			ir_freq = atoi(val);
		} else
		if (os_strcmp(cmd, "cmd") == 0) {
		}
		token = strtok_r(NULL, ",", &endtoken);
	}

	in_ir_send = 1;

	ets_wdt_disable();
	ets_intr_lock();

	// lead
	for (t = 0; t < lead_pulse; t++)
	{
		GPIO_OUTPUT_SET(IR_PIN, 1);
		os_delay_us(cycle);
		GPIO_OUTPUT_SET(IR_PIN, 0);
		os_delay_us(cycle);
	}
	GPIO_OUTPUT_SET(IR_PIN, 0);
	os_delay_us(lead_space);

	// info
	//os_strcpy(ir_cmd, "11100000111000000101001010101101");
	//os_strcpy(ir_cmd, "11100000111000001110001000011101");
	for (c = 0; c < 72; c++)
	{
		if (ir_cmd[c] == '0') {
			for (t = 0; t < space_pulse; t++)
			{
				GPIO_OUTPUT_SET(IR_PIN, 1);
				os_delay_us(cycle);
				GPIO_OUTPUT_SET(IR_PIN, 0);
				os_delay_us(cycle);
			}
			GPIO_OUTPUT_SET(IR_PIN, 0);
			os_delay_us(space_space);
		}
		if (ir_cmd[c] == '1') {
			for (t = 0; t < pulse_pulse; t++)
			{
				GPIO_OUTPUT_SET(IR_PIN, 1);
				os_delay_us(cycle);
				GPIO_OUTPUT_SET(IR_PIN, 0);
				os_delay_us(cycle);
			}
			GPIO_OUTPUT_SET(IR_PIN, 0);
			os_delay_us(pulse_space);
		}
	}

	// end
	for (t = 0; t < end; t++)
	{
		GPIO_OUTPUT_SET(IR_PIN, 1);
		os_delay_us(cycle);
		GPIO_OUTPUT_SET(IR_PIN, 0);
		os_delay_us(cycle);
	}
	GPIO_OUTPUT_SET(IR_PIN, 0);

	ets_intr_unlock();
	ets_wdt_enable();

	in_ir_send = 0;
	*/

	//INFO("IR SEND: %s\r\n", ir_cmd);
}
