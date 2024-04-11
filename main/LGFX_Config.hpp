#ifndef LGFX_CONFIG_H
#define LGFX_CONFIG_H

#include "LovyanGFX.hpp"


class LGFX : public lgfx::LGFX_Device
{

    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance; // SPIバスのインスタンス
    lgfx::Light_PWM _light_instance;

public:
    LGFX(void)
    {
        {
            auto cfg = _bus_instance.config();
            
            //SPI Configuration
            cfg.spi_host = SPI2_HOST; //ESP32-S3 SPI2_HOST
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.freq_read = 16000000;
            cfg.spi_3wire = true;
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            //SPI Pin Configuration
            cfg.pin_sclk = 36; // SPI SCLK
            cfg.pin_mosi = 35; // SPI MOSI
            cfg.pin_miso = 37; // SPI MISO
            cfg.pin_dc = 40;   // SPI DC

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            
            //Panel Configuration
            cfg.pin_cs = 42;   // CS
            cfg.pin_rst = 41;  // RST
            cfg.pin_busy = -1; // BUSY (-1 = disable)

            cfg.panel_width = 135;
            cfg.panel_height = 240;
            cfg.offset_x = 52;
            cfg.offset_y = 40;
            cfg.offset_rotation = 1;
            cfg.dummy_read_pixel = 8;
            cfg.dummy_read_bits = 1;
            cfg.readable = false;
            cfg.invert = true;
            cfg.rgb_order = false;
            cfg.dlen_16bit = false;
            cfg.bus_shared = true;
            _panel_instance.config(cfg);
        }

        {
            auto cfg = _light_instance.config();
            
            //Backlight Configuration
            cfg.pin_bl = 45;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;

            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};
#endif // LGFX_CONFIG_H