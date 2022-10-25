#include "driver/rmt.h"

#define RMT_CH RMT_CHANNEL_0 //depends on esp, look at docs
#define RMT_GPIO GPIO_NUM_10 //any GPIO

void setup() 
{
  Serial.begin(115200);
  rmt_dmx_init();
  Serial.println("setup successfull");
}

void rmt_dmx_init() {
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(RMT_GPIO, RMT_CH);

  config.rmt_mode = RMT_MODE_TX;
  config.tx_config.idle_output_en = true; //DMX requires HIGH state when not used
  config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
  config.clk_div = 80; //80 MHz / 80 -> 1 us perios

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

void rmt_dmx_send(byte *channels, short n) {
  rmt_item32_t items[6+5*n];
  items[0] = {100, 0, 8, 1}; //start sequence, BREAK (>=88 us), Mark After Break (8 us)
  items[1] = {4, 0, 4, 0}; //start code for lighting: 0x00
  items[2] = {4, 0, 4, 0}; //message format: {0, LSB, ..., MSB, 1, 1}
  items[3] = {4, 0, 4, 0}; //so {0, 0,0,0,0,0,0,0,0, 1, 1}
  items[4] = {4, 0, 4, 0};
  items[5] = {4, 0, 8, 1};
  
  for (int i = 0; i < n; i++) {
    byte ch = *(channels+i); //message format: {0, LSB, ..., MSB, 1, 1}
    items[6 + i*5 + 0] = {4, 0, 4, bitRead(ch, 0)};
    items[6 + i*5 + 1] = {4, bitRead(ch, 1), 4, bitRead(ch, 2)};
    items[6 + i*5 + 2] = {4, bitRead(ch, 3), 4, bitRead(ch, 4)};
    items[6 + i*5 + 3] = {4, bitRead(ch, 5), 4, bitRead(ch, 6)};
    items[6 + i*5 + 4] = {4, bitRead(ch, 7), 8, 1};
  }
  
  ESP_ERROR_CHECK(rmt_write_items(RMT_CH, items, sizeof(items) / sizeof(items[0]), true));
}

//brightness, r, g, b, w, flash, function, speed
byte channels[] = {255, 0, 0 ,0, 0, 0, 0, 0};
void loop() 
{
  short t1 = (millis()>>1) % 512;
  short t3 = (t1 + 384) % 512;
  channels[1] = max(64, (t1 > 255 ? 511 - t1 : t1));
  channels[3] = max(64, (t3 > 255 ? 511 - t3 : t3));
  channels[4] = t1%128 > 82 ? 50 : 0;
  rmt_dmx_send(channels, 8);
  delay(15);
}
