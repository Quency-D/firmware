#pragma once

#ifdef __cplusplus

#include <stdint.h>

enum class HeltecV4ExpansionBoard : uint8_t {
    NONE,
    V07,
    V202,
    UNKNOWN,
};

enum class HeltecV4TftAdapter : uint8_t {
    TP_V04,
    TOUCH_TFT_V02,
};

enum class HeltecV4Display : uint8_t {
    OLED,
    TFT_TP_V04,
    TFT_TOUCH_TFT_V02,
};

struct HeltecV4I2cRoute {
    int wireSda;
    int wireScl;
    int wire1Sda;
    int wire1Scl;
};

struct HeltecV4SdPins {
    int sck;
    int mosi;
    int miso;
    int cs;
};

struct HeltecV4GnssPins {
    int enable;
    int reset;
    int standby;
};

HeltecV4ExpansionBoard classifyHeltecV4Expansion(bool gpio3, bool gpio15, bool gpio16, bool gpio45);
HeltecV4ExpansionBoard resolveHeltecV4Expansion(HeltecV4ExpansionBoard detected);
HeltecV4TftAdapter classifyHeltecV4TftAdapter(bool gpio21);
bool isHeltecV4SdAvailable(HeltecV4ExpansionBoard board, HeltecV4Display display);
HeltecV4SdPins getHeltecV4SdPins(HeltecV4ExpansionBoard board);
HeltecV4SdPins getHeltecV4SdPins(HeltecV4ExpansionBoard board, HeltecV4Display display);
HeltecV4GnssPins getHeltecV4GnssPins(HeltecV4ExpansionBoard board);
HeltecV4I2cRoute getHeltecV4I2cRoute(HeltecV4ExpansionBoard board, HeltecV4Display display);

#if defined(HELTEC_V4)

void detectHeltecV4Hardware();
void logHeltecV4Hardware();
void beginHeltecV4ExpansionI2C();
void reconcileHeltecV4ExpansionBuzzerConfig();

HeltecV4ExpansionBoard getHeltecV4ExpansionBoard();
bool heltecV4ExpansionPresent();
bool heltecV4ExpansionIsV202();
bool heltecV4ExpansionSdAvailable();
bool heltecV4ExpansionUsesWire();
bool heltecV4ExpansionUsesWire1();

int heltecV4ExpansionWireSda();
int heltecV4ExpansionWireScl();
int heltecV4ExpansionWire1Sda();
int heltecV4ExpansionWire1Scl();
int heltecV4ExpansionI2cSda();
int heltecV4ExpansionI2cScl();
int heltecV4ExpansionI2cSda1();
int heltecV4ExpansionI2cScl1();
int heltecV4ExpansionAltButtonPin();
bool heltecV4ExpansionAltButtonActiveLow();
int heltecV4ExpansionBuzzerPin();
int heltecV4ExpansionGnssEnablePin();
int heltecV4ExpansionGnssResetPin();
int heltecV4ExpansionGnssStandbyPin();
int heltecV4ExpansionSdSck();
int heltecV4ExpansionSdMosi();
int heltecV4ExpansionSdMiso();
int heltecV4ExpansionSdCs();

#define PIN_GPS_EN heltecV4ExpansionGnssEnablePin()
#define PIN_GPS_RESET heltecV4ExpansionGnssResetPin()
#define PIN_GPS_STANDBY heltecV4ExpansionGnssStandbyPin()

#if defined(HELTEC_V4_TFT)
HeltecV4TftAdapter getHeltecV4TftAdapter();
int heltecV4ExpansionTftSck();
int heltecV4ExpansionTftMosi();
int heltecV4ExpansionTftMiso();
int heltecV4ExpansionTftDc();
int heltecV4ExpansionTftCs();
int heltecV4ExpansionTftBacklight();
int heltecV4ExpansionTftReset();
bool heltecV4ExpansionTftThreeWire();
int heltecV4ExpansionTouchSda();
int heltecV4ExpansionTouchScl();
int heltecV4ExpansionTouchReset();
#endif

#define I2C_SDA heltecV4ExpansionWireSda()
#define I2C_SCL heltecV4ExpansionWireScl()
#define I2C_SDA1 heltecV4ExpansionWire1Sda()
#define I2C_SCL1 heltecV4ExpansionWire1Scl()

#define PIN_BUTTON2 heltecV4ExpansionAltButtonPin()
#define ALT_BUTTON_ACTIVE_LOW heltecV4ExpansionAltButtonActiveLow()
#define PIN_BUZZER heltecV4ExpansionBuzzerPin()
#define USE_PIN_BUZZER PIN_BUZZER

#if defined(HELTEC_V4_TFT)
#define LGFX_SPI_3WIRE heltecV4ExpansionTftThreeWire()
#define SPI_3_WIRE heltecV4ExpansionTftThreeWire()
#define LGFX_PIN_SCK heltecV4ExpansionTftSck()
#define LGFX_PIN_MOSI heltecV4ExpansionTftMosi()
#define LGFX_PIN_MISO heltecV4ExpansionTftMiso()
#define LGFX_PIN_DC heltecV4ExpansionTftDc()
#define LGFX_PIN_CS heltecV4ExpansionTftCs()
#define LGFX_PIN_BL heltecV4ExpansionTftBacklight()
#define LGFX_PIN_RST heltecV4ExpansionTftReset()

#define TOUCH_SDA_PIN heltecV4ExpansionTouchSda()
#define TOUCH_SCL_PIN heltecV4ExpansionTouchScl()
#define TOUCH_RST_PIN heltecV4ExpansionTouchReset()
#define TOUCH_I2C_PORT 0
#define SCREEN_TOUCH_RST TOUCH_RST_PIN
#endif

#define HAS_SDCARD 1
#define SDCARD_USE_SPI1
#define SDCARD_USER_SPI_BEGIN
#define SPI_MOSI heltecV4ExpansionSdMosi()
#define SPI_SCK heltecV4ExpansionSdSck()
#define SPI_MISO heltecV4ExpansionSdMiso()
#define SPI_CS heltecV4ExpansionSdCs()
#define SDCARD_CS SPI_CS
#ifndef SD_SPI_FREQUENCY
#define SD_SPI_FREQUENCY 40000000U
#endif

#endif

#endif
