#include "waterlevel.h"

WaterLevel::WaterLevel(adc1_channel_t channel)
    : m_channel(channel)
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &m_characteristics);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(m_channel, ADC_ATTEN_DB_11));
}

uint32_t WaterLevel::voltage()
{
    return m_voltage;
}

Level WaterLevel::level()
{
    read();
    if (m_voltage < 1000) {
        return EMPTY;
    } else if (m_voltage >= 1000 && m_voltage < 1400) {
        return LOW;
    } else if (m_voltage >= 1400 && m_voltage < 1600) {
        return MEDIUM;
    } else {
        return FULL;
    }
}

void WaterLevel::read()
{
    m_voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(m_channel), &m_characteristics);
}