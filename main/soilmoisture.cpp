#include "soilmoisture.h"

SoilMoisture::SoilMoisture(adc1_channel_t channel)
    : m_channel(channel)
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &m_characteristics);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
    ESP_ERROR_CHECK(adc1_config_channel_atten(m_channel, ADC_ATTEN_DB_11));
}

uint32_t SoilMoisture::voltage()
{
    read();
    return m_voltage;
}

Moisture SoilMoisture::level()
{
    if (m_voltage > 2000) {
        return DRY;
    } else if (m_voltage <= 2000 && m_voltage > 1500) {
        return MOIST;
    } else {
        return WET;
    }
}

void SoilMoisture::read()
{
    m_voltage = esp_adc_cal_raw_to_voltage(adc1_get_raw(m_channel), &m_characteristics);
}