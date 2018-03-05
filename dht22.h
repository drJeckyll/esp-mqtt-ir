
#define DHT_PIN	2
#define DHT11 0

void ICACHE_FLASH_ATTR DHT(void);

float * ICACHE_FLASH_ATTR readDHT(void);

void DHTInit(void);
