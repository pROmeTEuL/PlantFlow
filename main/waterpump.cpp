#include "waterpump.h"

WaterPump::WaterPump(gpio_num_t gpio) : m_gpio(gpio), m_status(OFF)
{
    gpio_set_direction(m_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(m_gpio, m_status);
}

void WaterPump::on()
{
    m_status = ON;
    gpio_set_level(m_gpio, m_status);
}

void WaterPump::off()
{
    m_status = OFF;
    gpio_set_level(m_gpio, m_status);
}

Status WaterPump::status()
{
    return m_status;
}