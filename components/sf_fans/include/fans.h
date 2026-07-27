#ifndef SF_FANS_CLIENT
#define SF_FANS_CLIENT

#include "hal/ledc_types.h"
#include "soc/gpio_num.h"
#include <stdint.h>

typedef struct {
  ledc_timer_bit_t ledc_duty_res;
  ledc_clk_src_t ledc_clk_src;
  ledc_timer_t ledc_timer;
  ledc_mode_t ledc_mode;
  uint32_t freq_hz; // 25k mostly but just in case

} sf_fan_timer_t;

typedef struct {
  const char *topic_control;
  gpio_num_t gpio_num;
  uint8_t id;
  uint32_t duty; // 0-100%
  // uint32_t freq_hz; // 25k mostly but just in case
  // uint32_t rpm; : TODO

  // LEDC Config
  //
  ledc_channel_t ledc_channel;
  sf_fan_timer_t tim;
} sf_fan_t;

typedef enum {
  SF_FAN_OK,
  SF_FAN_ERR_NULL,
  SF_FAN_ERR_FAN_NOT_FOUND,
  SF_FAN_ERR_FAILED_TO_CONFIG_TIM,
  SF_FAN_ERR_FAILED_TO_CONFIG_CHANNEL,
  SF_FAN_ERR_FAILED_TO_SET_DUTY,
  SF_FAN_ERR_FAILED_TO_UPDATE_DUTY,

} sf_fan_error_t;

sf_fan_error_t sf_fans_get_fan(char *topic, size_t topic_len, sf_fan_t *fan);
sf_fan_error_t sf_fans_set_duty(sf_fan_t *fan, uint16_t duty);
sf_fan_error_t sf_fans_config_fan_timers(void);
sf_fan_error_t sf_fans_config_fan_channels(void);
sf_fan_error_t sf_fans_init(void);

#endif // !SF_FANS_CLIENT
