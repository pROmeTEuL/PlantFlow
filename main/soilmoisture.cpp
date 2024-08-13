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
    return m_voltage;
}

uint32_t SoilMoisture::raw()
{
    return m_raw;
}

Moisture SoilMoisture::level()
{
    read();
    /// WIP:
    if (m_voltage < 1000) {
        return DRY;
    } else if (m_voltage >= 1000 && m_voltage < 1400) {
        return MOIST;
    } else {
        return WET;
    }
}

void SoilMoisture::read()
{
    m_raw = adc1_get_raw(m_channel);
    m_voltage = esp_adc_cal_raw_to_voltage(m_raw, &m_characteristics);
}