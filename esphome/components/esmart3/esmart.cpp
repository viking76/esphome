#include "esmart.h"
#include "esphome/core/log.h"

namespace esphome {
namespace esmart {

static const char *TAG = "esmart";

static const uint8_t ESMART_CMD_READ_IN_REGISTERS = 0x04;
static const uint8_t ESMART_REGISTER_COUNT = 10;  // 10x 16-bit registers

void ESMART::on_modbus_data(const std::vector<uint8_t> &data) {
  if (data.size() < 20) {
    ESP_LOGW(TAG, "Invalid size for Esmart AC!");
    return;
  }

  // See https://github.com/esphome/feature-requests/issues/49#issuecomment-538636809
  //  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
  // 01 04 14 08 D1 00 6C 00 00 00 F4 00 00 00 26 00 00 01 F4 00 64 00 00 51 34
  // Id Cc Sz Volt- Current---- Power------ Energy----- Frequ PFact Alarm Crc--
  //           0     2           6          10          14    16

  auto esmart_get_16bit = [&](size_t i) -> uint16_t {
    return (uint16_t(data[i + 0]) << 8) | (uint16_t(data[i + 1]) << 0);
  };
  auto esmart_get_32bit = [&](size_t i) -> uint32_t {
    return (uint32_t(esmart_get_16bit(i + 2)) << 16) | (uint32_t(esmart_get_16bit(i + 0)) << 0);
  };

  uint16_t raw_chg = esmart_get_16bit(0);
  float chg = raw_chg / 10.0f;  // max 6553.5 V
    
  uint16_t raw_pv_volt = esmart_get_16bit(1);
  float pv_volt = raw_pv_volt / 10.0f;  // max 6553.5 V
      
  uint16_t raw_bat_volt = esmart_get_16bit(2);
  float bat_volt = raw_bat_volt / 10.0f;  // max 6553.5 V
    
  uint16_t raw_charging_current = esmart_get_16bit(3);
  float charging_current = raw_charging_current / 10.0f;  // max 6553.5 V

  //uint32_t raw_current = esmart_get_32bit(2);
  //float current = raw_current / 1000.0f;  // max 4294967.295 A

  //uint32_t raw_active_power = esmart_get_32bit(6);
  //float active_power = raw_active_power / 10.0f;  // max 429496729.5 W

  float active_energy = static_cast<float>(esmart_get_32bit(10));

  uint16_t raw_frequency = esmart_get_16bit(14);
  float frequency = raw_frequency / 10.0f;

  uint16_t raw_power_factor = esmart_get_16bit(16);
  float power_factor = raw_power_factor / 100.0f;

  ESP_LOGD(TAG, "ESMART AC: V=%.1f V, I=%.3f A, P=%.1f W, E=%.1f Wh, F=%.1f Hz, PF=%.2f", voltage, current, active_power,
           active_energy, frequency, power_factor);
  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(voltage);
  if (this->current_sensor_ != nullptr)
    this->current_sensor_->publish_state(current);
  if (this->power_sensor_ != nullptr)
    this->power_sensor_->publish_state(active_power);
  if (this->energy_sensor_ != nullptr)
    this->energy_sensor_->publish_state(active_energy);
  if (this->frequency_sensor_ != nullptr)
    this->frequency_sensor_->publish_state(frequency);
  if (this->power_factor_sensor_ != nullptr)
    this->power_factor_sensor_->publish_state(power_factor);
}

void ESMART::update() { this->send(ESMART_CMD_READ_IN_REGISTERS, 0, ESMART_REGISTER_COUNT); }
void ESMART::dump_config() {
  ESP_LOGCONFIG(TAG, "ESMART:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%02X", this->address_);
  LOG_SENSOR("", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("", "Current", this->current_sensor_);
  LOG_SENSOR("", "Power", this->power_sensor_);
  LOG_SENSOR("", "Energy", this->energy_sensor_);
  LOG_SENSOR("", "Frequency", this->frequency_sensor_);
  LOG_SENSOR("", "Power Factor", this->power_factor_sensor_);
}

}  // namespace esmart
}  // namespace esphome
