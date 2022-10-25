# DMX512 Master using ESP32 RMT feature

A code example of DMX512 transmitter for ESP32 microcontroller.

## Why RMT

This code doesn't use Hardware Serial, but RMT - Remote Control feature, implemented in ESP32.
It is like "Make everything better" magic button. It has many advantages:

- Can use any GPIO
- 2-3 channels (so 2 or 3 DMX Universes!)
- Does not interrupt serial connection to PC

## Code explanation

Function `rmt_dmx_init` sets up RMT channel with required frequency of 1 MHz, enabled default/idle state and selected pin number.

    void rmt_dmx_init() {
        rmt_config_t config = RMT_DEFAULT_CONFIG_TX(RMT_GPIO, RMT_CH);

        config.rmt_mode = RMT_MODE_TX;
        config.tx_config.idle_output_en = true; //DMX requires HIGH state when not used
        config.tx_config.idle_level = RMT_IDLE_LEVEL_HIGH;
        config.clk_div = 80; //80 MHz / 80 -> 1 us perios

        ESP_ERROR_CHECK(rmt_config(&config));
        ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
    }

Function `rmt_dmx_send` creates special `items` struct, where values and duraions of pulses are stored, and compiles them into DMX frames. 

    void rmt_dmx_send(byte *channels, short n) {
        rmt_item32_t items[6+5*n];
        items[0] = {100, 0, 8, 1}; //start sequence, BREAK (>=88 us), Mark After Break (8 us)
        ... [DMX packet structure]
        ESP_ERROR_CHECK(rmt_write_items(RMT_CH, items, sizeof(items) / sizeof(items[0]), true));
    }

You can also modify `rmt_write_items` method to be asynchronous (non-blocking), changing last argument to false.

    ESP_ERROR_CHECK(rmt_write_items(RMT_CH, items, sizeof(items) / sizeof(items[0]), false));

This code is written for Arduino IDE but uses ESP-IDF api (v.4.4.2), so be free to modify it for ESP-IDF.

## Usage example in loop

I have LED PAR fixture which uses 8 DMX channels and it is set to A001 channel. I wrote some math code to emulate red/blue flashing lights.

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

As you can see, it is as simple as this.

## About

Feel free to use this example in your projects. c:

