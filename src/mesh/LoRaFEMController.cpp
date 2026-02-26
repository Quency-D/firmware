#if HAS_LORA_FEM
#include "LoRaFEMController.h"

#if defined(ARCH_ESP32)
#include <driver/rtc_io.h>
#include <esp_sleep.h>
#endif

LoRaFEMController loraFEMController;

void LoRaFEMController::init(void)
{
    setLnaCanControl(false);
#if defined(ARCH_ESP32)
    rtc_gpio_hold_dis((gpio_num_t)LORA_PA_POWER);
    rtc_gpio_hold_dis((gpio_num_t)LORA_GC1109_PA_EN);
    rtc_gpio_hold_dis((gpio_num_t)LORA_GC1109_PA_TX_EN);
#ifdef LORA_KCT8103L_PA_CSD
    rtc_gpio_hold_dis((gpio_num_t)LORA_KCT8103L_PA_CSD);
    rtc_gpio_hold_dis((gpio_num_t)LORA_KCT8103L_PA_CTX);
#endif

    pinMode(LORA_PA_POWER, OUTPUT);
    digitalWrite(LORA_PA_POWER, HIGH);
    delay(1);

#ifdef LORA_KCT8103L_PA_CSD
    // Auto-detect FEM type via default pull-up/pull-down on shared GPIO.
    // LORA_KCT8103L_PA_CSD and LORA_GC1109_PA_EN share the same physical pin.
    pinMode(LORA_KCT8103L_PA_CSD, INPUT);
    delay(1);
    if (digitalRead(LORA_KCT8103L_PA_CSD) == HIGH) {
        // FEM is KCT8103L
        fem_type = KCT8103L_PA;
        pinMode(LORA_KCT8103L_PA_CSD, OUTPUT);
        digitalWrite(LORA_KCT8103L_PA_CSD, HIGH);
        pinMode(LORA_KCT8103L_PA_CTX, OUTPUT);
        digitalWrite(LORA_KCT8103L_PA_CTX, HIGH);
        setLnaCanControl(true);
    } else {
        // FEM is GC1109
        fem_type = GC1109_PA;
        pinMode(LORA_GC1109_PA_EN, OUTPUT);
        digitalWrite(LORA_GC1109_PA_EN, HIGH);
        pinMode(LORA_GC1109_PA_TX_EN, OUTPUT);
        digitalWrite(LORA_GC1109_PA_TX_EN, LOW);
    }
#else
    // Board has GC1109 only (no auto-detection needed)
    fem_type = GC1109_PA;
    pinMode(LORA_GC1109_PA_EN, OUTPUT);
    digitalWrite(LORA_GC1109_PA_EN, HIGH);
    pinMode(LORA_GC1109_PA_TX_EN, OUTPUT);
    digitalWrite(LORA_GC1109_PA_TX_EN, LOW);
#endif
#endif
}

void LoRaFEMController::setSleepModeEnable(void)
{
#if defined(ARCH_ESP32)
    if (fem_type == GC1109_PA) {
        /*
         * Do not switch the power on and off frequently.
         * After turning off PA_EN, the power consumption drops to uA level.
         */
        digitalWrite(LORA_GC1109_PA_EN, LOW);
        digitalWrite(LORA_GC1109_PA_TX_EN, LOW);
    }
#ifdef LORA_KCT8103L_PA_CSD
    else if (fem_type == KCT8103L_PA) {
        digitalWrite(LORA_KCT8103L_PA_CSD, LOW);
    }
#endif
#endif
}

void LoRaFEMController::setTxModeEnable(void)
{
#if defined(ARCH_ESP32)
    if (fem_type == GC1109_PA) {
        digitalWrite(LORA_GC1109_PA_EN, HIGH);
        digitalWrite(LORA_GC1109_PA_TX_EN, HIGH);
    }
#ifdef LORA_KCT8103L_PA_CSD
    else if (fem_type == KCT8103L_PA) {
        digitalWrite(LORA_KCT8103L_PA_CSD, HIGH);
        digitalWrite(LORA_KCT8103L_PA_CTX, HIGH);
    }
#endif
#endif
}

void LoRaFEMController::setRxModeEnable(void)
{
#if defined(ARCH_ESP32)
    if (fem_type == GC1109_PA) {
        digitalWrite(LORA_GC1109_PA_EN, HIGH);
        digitalWrite(LORA_GC1109_PA_TX_EN, LOW);
    }
#ifdef LORA_KCT8103L_PA_CSD
    else if (fem_type == KCT8103L_PA) {
        digitalWrite(LORA_KCT8103L_PA_CSD, HIGH);
        if (lna_enabled) {
            digitalWrite(LORA_KCT8103L_PA_CTX, LOW);
        } else {
            digitalWrite(LORA_KCT8103L_PA_CTX, HIGH);
        }
    }
#endif
#endif
}

void LoRaFEMController::setRxModeEnableWhenMCUSleep(void)
{
#if defined(ARCH_ESP32)
    // Keep FEM powered during deep sleep so LNA remains active for RX wake.
    // Latch with RTC hold so the state survives deep sleep.
    digitalWrite(LORA_PA_POWER, HIGH);
    rtc_gpio_hold_en((gpio_num_t)LORA_PA_POWER);
    if (fem_type == GC1109_PA) {
        digitalWrite(LORA_GC1109_PA_EN, HIGH);
        rtc_gpio_hold_en((gpio_num_t)LORA_GC1109_PA_EN);
        gpio_pulldown_en((gpio_num_t)LORA_GC1109_PA_TX_EN);
    }
#ifdef LORA_KCT8103L_PA_CSD
    else if (fem_type == KCT8103L_PA) {
        digitalWrite(LORA_KCT8103L_PA_CSD, HIGH);
        rtc_gpio_hold_en((gpio_num_t)LORA_KCT8103L_PA_CSD);
        if (lna_enabled) {
            digitalWrite(LORA_KCT8103L_PA_CTX, LOW);
        } else {
            digitalWrite(LORA_KCT8103L_PA_CTX, HIGH);
        }
        rtc_gpio_hold_en((gpio_num_t)LORA_KCT8103L_PA_CTX);
    }
#endif
#endif
}

void LoRaFEMController::setLNAEnable(bool enabled)
{
    lna_enabled = enabled;
}

int8_t LoRaFEMController::powerConversion(int8_t loraOutputPower)
{
    const uint16_t gc1109_tx_gain[] = {11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 10, 10, 9, 9, 8, 7};
    const uint16_t kct8103l_tx_gain[] = {13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 12, 12, 11, 11, 10, 9, 8, 7};
    const uint16_t *tx_gain;
    uint16_t tx_gain_num;
    if (fem_type == GC1109_PA) {
        tx_gain = gc1109_tx_gain;
        tx_gain_num = sizeof(gc1109_tx_gain) / sizeof(gc1109_tx_gain[0]);
    } else if (fem_type == KCT8103L_PA) {
        tx_gain = kct8103l_tx_gain;
        tx_gain_num = sizeof(kct8103l_tx_gain) / sizeof(kct8103l_tx_gain[0]);
    } else {
        return loraOutputPower;
    }
    for (int radio_dbm = 0; radio_dbm < tx_gain_num; radio_dbm++) {
        if (((radio_dbm + tx_gain[radio_dbm]) > loraOutputPower) ||
            ((radio_dbm == (tx_gain_num - 1)) && ((radio_dbm + tx_gain[radio_dbm]) <= loraOutputPower))) {
            LOG_INFO("Requested Tx power: %d dBm; Device LoRa Tx gain: %d dB", loraOutputPower, tx_gain[radio_dbm]);
            loraOutputPower -= tx_gain[radio_dbm];
            break;
        }
    }
    return loraOutputPower;
}

#endif
