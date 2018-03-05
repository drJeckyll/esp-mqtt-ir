#include "os_type.h"

#define IR_PIN 5

typedef struct{
	uint32_t freq;
	uint32_t one_pulse;
	uint32_t one_pause;
	uint32_t zero_pulse;
	uint32_t zero_pause;
	uint32_t before[10];
	uint32_t before_len;
	uint32_t after[10];
	uint32_t after_len;
	uint32_t data[100];
	uint32_t data_len;
	uint32_t subst[20];
	uint32_t subst_len;
} IR_MSG;
extern IR_MSG irMsg;

void ir_send();
