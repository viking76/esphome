#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/modbus/modbus.h"

namespace esphome {
namespace esmart {

class ESMART : public PollingComponent, public modbus::ModbusDevice {
 public:
  void set_chg_sensor(sensor::Sensor *chg_sensor) { chg_sensor_ = chg_sensor; }
  void set_pv_volt_sensor(sensor::Sensor *pv_volt_sensor) { pv_volt_sensor_ = pv_volt_sensor; }
  void set_bat_volt_sensor(sensor::Sensor *bat_volt_sensor) { bat_volt_sensor_ = bat_volt_sensor; }
  void set_charging_current_sensor(sensor::Sensor *charging_current_sensor) { charging_current_sensor_ = charging_current_sensor; }

  void update() override;

  void on_modbus_data(const std::vector<uint8_t> &data) override;

  void dump_config() override;

 protected:
  sensor::Sensor *chg_sensor_;
  sensor::Sensor *pv_volt_sensor_;
  sensor::Sensor *bat_volt_sensor_;
  sensor::Sensor *charging_current_sensor_;
};

}  // namespace esmart
}  // namespace esphome
