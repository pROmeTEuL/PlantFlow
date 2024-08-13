#include "brightnesslevel.h"

BrightnessLevel::BrightnessLevel(adc1_channel_t channel, gpio_num_t gpio)
    : m_channel(channel)
    , m_gpio(gpio)
{
    gpio_set_direction(m_gpio, GPIO_MODE_INPUT);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &m_characteristics);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(m_channel, ADC_ATTEN_DB_11));
}

uint32_t BrightnessLevel::voltage()
{
    return m_voltage;
}

Brightness BrightnessLevel::level()
{
    read();
    if (m_isDark)
        return DARK;
    else if (m_voltage < 150)
        return BRIGHT;
    else
        return DIM;
}

void BrightnessLevel::read()
{
    m_voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(m_channel), &m_characteristics);
    m_isDark = gpio_get_level(m_gpio);
}