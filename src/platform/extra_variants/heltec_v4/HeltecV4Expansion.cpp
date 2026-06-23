#include "HeltecV4Expansion.h"

HeltecV4ExpansionBoard classifyHeltecV4Expansion(bool gpio3, bool gpio15, bool gpio16, bool gpio45)
{
    const uint8_t highDisplayPins = gpio15 + gpio16 + gpio45;
    if (!gpio3 && highDisplayPins == 0) {
        return HeltecV4ExpansionBoard::NONE;
    }
    if (gpio3 && highDisplayPins == 0) {
        return HeltecV4ExpansionBoard::V07;
    }
    if (!gpio3 && highDisplayPins >= 2) {
        return HeltecV4ExpansionBoard::V202;
    }
    return HeltecV4ExpansionBoard::UNKNOWN;
}

HeltecV4ExpansionBoard resolveHeltecV4Expansion(HeltecV4ExpansionBoard detected)
{
    return detected == HeltecV4ExpansionBoard::UNKNOWN ? HeltecV4ExpansionBoard::V07 : detected;
}

HeltecV4TftAdapter classifyHeltecV4TftAdapter(bool gpio21)
{
    return gpio21 ? HeltecV4TftAdapter::TOUCH_TFT_V02 : HeltecV4TftAdapter::TP_V04;
}

bool isHeltecV4SdAvailable(HeltecV4ExpansionBoard board, HeltecV4Display display)
{
    if (resolveHeltecV4Expansion(board) != HeltecV4ExpansionBoard::V202) {
        return false;
    }
    return display != HeltecV4Display::TFT_TP_V04;
}

HeltecV4SdPins getHeltecV4SdPins(HeltecV4ExpansionBoard board)
{
    return resolveHeltecV4Expansion(board) == HeltecV4ExpansionBoard::V202 ? HeltecV4SdPins{16, 15, 45, 3}
                                                                           : HeltecV4SdPins{-1, -1, -1, -1};
}

HeltecV4SdPins getHeltecV4SdPins(HeltecV4ExpansionBoard board, HeltecV4Display display)
{
    return isHeltecV4SdAvailable(board, display) ? getHeltecV4SdPins(board) : HeltecV4SdPins{-1, -1, -1, -1};
}

HeltecV4GnssPins getHeltecV4GnssPins(HeltecV4ExpansionBoard board)
{
    const auto resolvedBoard = resolveHeltecV4Expansion(board);
    if (resolvedBoard == HeltecV4ExpansionBoard::NONE) {
        return {-1, -1, -1};
    }
    if (resolvedBoard == HeltecV4ExpansionBoard::V202) {
        return {42, -1, -1};
    }
    return {34, 42, 40};
}

HeltecV4I2cRoute getHeltecV4I2cRoute(HeltecV4ExpansionBoard board, HeltecV4Display display)
{
    const auto resolvedBoard = resolveHeltecV4Expansion(board);
    if (display == HeltecV4Display::OLED) {
        return resolvedBoard == HeltecV4ExpansionBoard::V07 ? HeltecV4I2cRoute{17, 18, 4, 3} : HeltecV4I2cRoute{17, 18, -1, -1};
    }

    const bool newTft = display == HeltecV4Display::TFT_TOUCH_TFT_V02;
    const HeltecV4I2cRoute noBase = newTft ? HeltecV4I2cRoute{17, 18, -1, -1} : HeltecV4I2cRoute{47, 48, -1, -1};
    if (resolvedBoard == HeltecV4ExpansionBoard::NONE) {
        return noBase;
    }

    if (newTft) {
        return resolvedBoard == HeltecV4ExpansionBoard::V07 ? HeltecV4I2cRoute{17, 18, 4, 3} : HeltecV4I2cRoute{17, 18, -1, -1};
    }

    return resolvedBoard == HeltecV4ExpansionBoard::V07 ? HeltecV4I2cRoute{47, 48, 4, 3} : HeltecV4I2cRoute{47, 48, 17, 18};
}

#if defined(HELTEC_V4)

#include "configuration.h"
#include "mesh/NodeDB.h"
#include <Arduino.h>
#include <Wire.h>

namespace
{
struct BasePins {
    int sensorSda;
    int sensorScl;
    int altButton;
    bool altButtonActiveLow;
    int buzzer;
};

#if defined(HELTEC_V4_TFT)
struct TftPins {
    int tftSck;
    int tftMosi;
    int tftMiso;
    int tftDc;
    int tftCs;
    int tftBacklight;
    int tftReset;
    bool tftThreeWire;
    int touchSda;
    int touchScl;
    int touchReset;
};
#endif

constexpr BasePins noBase = {-1, -1, -1, true, 0};
constexpr BasePins v07Base = {4, 3, 35, true, 6};
constexpr BasePins v202Base = {17, 18, 46, false, 4};

#if defined(HELTEC_V4_TFT)
constexpr TftPins tpV04 = {17, 33, -1, 16, 15, 21, 18, true, 47, 48, 44};
constexpr TftPins touchTftV02 = {16, 15, 45, 48, 47, 44, 21, false, 17, 18, -1};
#endif

constexpr uint8_t detectPins[] = {3, 15, 16, 45};
HeltecV4ExpansionBoard detectedBoard = HeltecV4ExpansionBoard::V07;
#if defined(HELTEC_V4_TFT)
HeltecV4TftAdapter detectedTftAdapter = HeltecV4TftAdapter::TP_V04;
#endif
uint8_t detectedLevels = 0;

const BasePins &basePins()
{
    const auto resolvedBoard = resolveHeltecV4Expansion(detectedBoard);
    if (resolvedBoard == HeltecV4ExpansionBoard::NONE) {
        return noBase;
    }
    if (resolvedBoard == HeltecV4ExpansionBoard::V202) {
        return v202Base;
    }
    return v07Base;
}

#if defined(HELTEC_V4_TFT)
const TftPins &tftPins()
{
    return detectedTftAdapter == HeltecV4TftAdapter::TOUCH_TFT_V02 ? touchTftV02 : tpV04;
}
#endif

HeltecV4Display currentDisplay()
{
#if defined(HELTEC_V4_TFT)
    return detectedTftAdapter == HeltecV4TftAdapter::TOUCH_TFT_V02 ? HeltecV4Display::TFT_TOUCH_TFT_V02
                                                                   : HeltecV4Display::TFT_TP_V04;
#else
    return HeltecV4Display::OLED;
#endif
}

HeltecV4I2cRoute i2cRoute()
{
    return getHeltecV4I2cRoute(detectedBoard, currentDisplay());
}

bool samplePin(uint8_t pin)
{
    uint8_t highSamples = 0;
    for (uint8_t sample = 0; sample < 5; sample++) {
        highSamples += digitalRead(pin) == HIGH;
        delay(5);
    }
    return highSamples >= 3;
}

void idleV202SdBus()
{
    if (resolveHeltecV4Expansion(detectedBoard) != HeltecV4ExpansionBoard::V202) {
        return;
    }

    const auto sdPins = getHeltecV4SdPins(detectedBoard);
    if (sdPins.cs >= 0) {
        pinMode(sdPins.cs, OUTPUT);
        digitalWrite(sdPins.cs, HIGH);
    }

#if defined(HELTEC_V4_TFT)
    const auto displayPins = tftPins();
    if (displayPins.tftCs >= 0) {
        pinMode(displayPins.tftCs, OUTPUT);
        digitalWrite(displayPins.tftCs, HIGH);
    }
#endif
}
} // namespace

void detectHeltecV4Hardware()
{
    detectedLevels = 0;
    pinMode(VEXT_ENABLE, OUTPUT);
    digitalWrite(VEXT_ENABLE, VEXT_ON_VALUE);
    delay(20);

    for (const auto pin : detectPins) {
        pinMode(pin, INPUT_PULLDOWN);
    }

    bool levels[4];
    for (uint8_t i = 0; i < 4; i++) {
        levels[i] = samplePin(detectPins[i]);
        detectedLevels |= levels[i] << i;
    }
    detectedBoard = classifyHeltecV4Expansion(levels[0], levels[1], levels[2], levels[3]);

    for (const auto pin : detectPins) {
        pinMode(pin, INPUT);
    }

#if defined(HELTEC_V4_TFT)
    pinMode(21, INPUT);
    detectedTftAdapter = classifyHeltecV4TftAdapter(samplePin(21));
#endif
    idleV202SdBus();
}

void logHeltecV4Hardware()
{
    switch (detectedBoard) {
    case HeltecV4ExpansionBoard::NONE:
        LOG_INFO("Heltec V4 base plate: none (GPIO 45/16/15/3 = %x)", static_cast<unsigned>(detectedLevels));
        break;
    case HeltecV4ExpansionBoard::V07:
        LOG_INFO("Heltec V4 base plate: V0.7 (GPIO 45/16/15/3 = %x)", static_cast<unsigned>(detectedLevels));
        break;
    case HeltecV4ExpansionBoard::V202:
        LOG_INFO("Heltec V4 base plate: V2.02 (GPIO 45/16/15/3 = %x)", static_cast<unsigned>(detectedLevels));
        break;
    case HeltecV4ExpansionBoard::UNKNOWN:
        LOG_WARN("Heltec V4 base plate: unknown GPIO combination %x; using V0.7", static_cast<unsigned>(detectedLevels));
        break;
    }

#if defined(HELTEC_V4_TFT)
    LOG_INFO("Heltec V4 TFT adapter: %s (GPIO21 = %d)",
             detectedTftAdapter == HeltecV4TftAdapter::TOUCH_TFT_V02 ? "TouchTFT V0.2" : "TP V0.4",
             detectedTftAdapter == HeltecV4TftAdapter::TOUCH_TFT_V02);
    if (!heltecV4ExpansionSdAvailable() && detectedBoard == HeltecV4ExpansionBoard::V202) {
        LOG_WARN("Heltec V4 SD disabled: V2.02 SD MOSI uses GPIO15, which conflicts with TP V0.4 TFT CS");
    }
#endif
}

void beginHeltecV4ExpansionI2C()
{
    logHeltecV4Hardware();
    if (heltecV4ExpansionUsesWire()) {
        Wire.begin(i2cRoute().wireSda, i2cRoute().wireScl);
    }
    if (heltecV4ExpansionUsesWire1()) {
        Wire1.begin(i2cRoute().wire1Sda, i2cRoute().wire1Scl);
    }
}

void reconcileHeltecV4ExpansionBuzzerConfig()
{
    const uint8_t selectedPin = basePins().buzzer;
    if (config.device.buzzer_gpio == 0 || config.device.buzzer_gpio == v07Base.buzzer ||
        config.device.buzzer_gpio == v202Base.buzzer) {
        config.device.buzzer_gpio = selectedPin;
    }
    if (moduleConfig.external_notification.output_buzzer == 0 ||
        moduleConfig.external_notification.output_buzzer == v07Base.buzzer ||
        moduleConfig.external_notification.output_buzzer == v202Base.buzzer) {
        moduleConfig.external_notification.output_buzzer = selectedPin;
    }
}

HeltecV4ExpansionBoard getHeltecV4ExpansionBoard()
{
    return detectedBoard;
}

#if defined(HELTEC_V4_TFT)
HeltecV4TftAdapter getHeltecV4TftAdapter()
{
    return detectedTftAdapter;
}
#endif

bool heltecV4ExpansionPresent()
{
    return detectedBoard != HeltecV4ExpansionBoard::NONE;
}

bool heltecV4ExpansionIsV202()
{
    return detectedBoard == HeltecV4ExpansionBoard::V202;
}

bool heltecV4ExpansionSdAvailable()
{
    return isHeltecV4SdAvailable(detectedBoard, currentDisplay());
}

bool heltecV4ExpansionUsesWire()
{
    return i2cRoute().wireSda >= 0;
}

bool heltecV4ExpansionUsesWire1()
{
    return i2cRoute().wire1Sda >= 0;
}

int heltecV4ExpansionWireSda()
{
    return i2cRoute().wireSda;
}

int heltecV4ExpansionWireScl()
{
    return i2cRoute().wireScl;
}

int heltecV4ExpansionWire1Sda()
{
    return i2cRoute().wire1Sda;
}

int heltecV4ExpansionWire1Scl()
{
    return i2cRoute().wire1Scl;
}

int heltecV4ExpansionI2cSda()
{
    return heltecV4ExpansionWireSda();
}

int heltecV4ExpansionI2cScl()
{
    return heltecV4ExpansionWireScl();
}

int heltecV4ExpansionI2cSda1()
{
    return heltecV4ExpansionWire1Sda();
}

int heltecV4ExpansionI2cScl1()
{
    return heltecV4ExpansionWire1Scl();
}

int heltecV4ExpansionAltButtonPin()
{
    return basePins().altButton;
}

bool heltecV4ExpansionAltButtonActiveLow()
{
    return basePins().altButtonActiveLow;
}

int heltecV4ExpansionBuzzerPin()
{
    return basePins().buzzer;
}

int heltecV4ExpansionGnssEnablePin()
{
    return getHeltecV4GnssPins(detectedBoard).enable;
}

int heltecV4ExpansionGnssResetPin()
{
    return getHeltecV4GnssPins(detectedBoard).reset;
}

int heltecV4ExpansionGnssStandbyPin()
{
    return getHeltecV4GnssPins(detectedBoard).standby;
}

#if defined(HELTEC_V4_TFT)
int heltecV4ExpansionTftSck()
{
    return tftPins().tftSck;
}

int heltecV4ExpansionTftMosi()
{
    return tftPins().tftMosi;
}

int heltecV4ExpansionTftMiso()
{
    return tftPins().tftMiso;
}

int heltecV4ExpansionTftDc()
{
    return tftPins().tftDc;
}

int heltecV4ExpansionTftCs()
{
    return tftPins().tftCs;
}

int heltecV4ExpansionTftBacklight()
{
    return tftPins().tftBacklight;
}

int heltecV4ExpansionTftReset()
{
    return tftPins().tftReset;
}

bool heltecV4ExpansionTftThreeWire()
{
    return tftPins().tftThreeWire;
}

int heltecV4ExpansionTouchSda()
{
    return tftPins().touchSda;
}

int heltecV4ExpansionTouchScl()
{
    return tftPins().touchScl;
}

int heltecV4ExpansionTouchReset()
{
    return tftPins().touchReset;
}
#endif

int heltecV4ExpansionSdSck()
{
    return getHeltecV4SdPins(detectedBoard, currentDisplay()).sck;
}

int heltecV4ExpansionSdMosi()
{
    return getHeltecV4SdPins(detectedBoard, currentDisplay()).mosi;
}

int heltecV4ExpansionSdMiso()
{
    return getHeltecV4SdPins(detectedBoard, currentDisplay()).miso;
}

int heltecV4ExpansionSdCs()
{
    return getHeltecV4SdPins(detectedBoard, currentDisplay()).cs;
}

#endif
