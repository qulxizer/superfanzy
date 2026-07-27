#include "include/fans.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_pm.h"

#include "hal/ledc_types.h"
#include "soc/clk_tree_defs.h"
#include "soc/gpio_num.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
const char *TAG = "SF_FANS";
const sf_fan_timer_t tim[] = {{
    .ledc_mode = LEDC_LOW_SPEED_MODE,
    .ledc_duty_res = LEDC_TIMER_10_BIT,
    .ledc_clk_src = LEDC_AUTO_CLK,
    .ledc_timer = LEDC_TIMER_0,
    .freq_hz = 25000,
}};
sf_fan_t g_fans[] = {
    {
        .id = 1,
        .gpio_num = GPIO_NUM_2,
        .topic_control = "/fanctl/control/fan/1/PWM",
        .duty = 0,
        .ledc_channel = LEDC_CHANNEL_0,
        .tim = tim[0],
    },
    {
        .id = 2,
        .gpio_num = GPIO_NUM_4,
        .topic_control = "/fanctl/control/fan/2/PWM",
        .duty = 0,
        .tim = tim[0],
    },
    {
        .id = 3,
        .gpio_num = GPIO_NUM_16,
        .topic_control = "/fanctl/control/fan/3/PWM",
        .duty = 0,
        .tim = tim[0],
    },
    {
        .id = 4,
        .gpio_num = GPIO_NUM_17,
        .topic_control = "/fanctl/control/fan/4/PWM",
        .duty = 0,
        .tim = tim[0],
    },
};

sf_fan_error_t sf_fans_get_fan(char *topic, size_t topic_len, sf_fan_t *fan) {
  if (fan == NULL) {
    return SF_FAN_ERR_NULL;
  }
  for (int i = 0; i >= 0; i++) {

    if (strlen(g_fans[i].topic_control) == topic_len &&
        strncmp(g_fans[i].topic_control, topic, topic_len) == 0) {
      *fan = g_fans[i];
      return SF_FAN_OK;
    }
  }
  return SF_FAN_ERR_FAN_NOT_FOUND;
}

static sf_fan_error_t sf_fans_config_fan_timer(const sf_fan_timer_t *tim) {
  if (tim == NULL) {
    return SF_FAN_ERR_NULL;
  }
  ledc_timer_config_t ledc_timer = {
      .speed_mode = tim->ledc_mode,
      .duty_resolution = tim->ledc_duty_res,
      .timer_num = tim->ledc_timer,
      .clk_cfg = tim->ledc_clk_src,
      .freq_hz = tim->freq_hz,
  };
  esp_err_t e = ledc_timer_config(&ledc_timer);
  if (e != ESP_OK) {
    ESP_LOGE(TAG, "Failed to config tim:%d channel, %s", tim->ledc_timer,
             esp_err_to_name(e));
    return SF_FAN_ERR_FAILED_TO_CONFIG_TIM;
  }

  return SF_FAN_OK;
}

sf_fan_error_t sf_fans_config_fan_timers(void) {
  int tim_count = sizeof(tim) / sizeof(sf_fan_timer_t);
  for (int i = 0; i < tim_count; i++) {
    sf_fan_error_t e = sf_fans_config_fan_timer(&tim[i]);
    if (e != SF_FAN_OK) {
      // ESP_LOGE(TAG, "Failed to config timer:%d", tim->ledc_timer);
      return e;
    }
  }
  return SF_FAN_OK;
}

static sf_fan_error_t sf_fans_config_fan_channel(sf_fan_t *fan) {
  if (fan == NULL) {
    return SF_FAN_ERR_NULL;
  }
  ledc_channel_config_t ledc_channel = {
      .speed_mode = fan->tim.ledc_mode,
      .channel = fan->ledc_channel,
      .timer_sel = fan->tim.ledc_timer,
      .gpio_num = fan->gpio_num,
      .duty = fan->duty,
      .hpoint = 0,
  };
  esp_err_t e = ledc_channel_config(&ledc_channel);
  if (e != ESP_OK) {
    ESP_LOGE(TAG, "Failed to config fan:%d channel, %s", fan->id,
             esp_err_to_name(e));
    return SF_FAN_ERR_FAILED_TO_CONFIG_CHANNEL;
  }
  return SF_FAN_OK;
}

sf_fan_error_t sf_fans_config_fan_channels(void) {
  int fan_count = sizeof(g_fans) / sizeof(sf_fan_t);
  for (int i = 0; i < fan_count; i++) {
    sf_fan_error_t e = sf_fans_config_fan_channel(&g_fans[i]);
    if (e != SF_FAN_OK) {
      // ESP_LOGE(TAG, "Failed to config fan:%d channel", g_fans[i].id);
      return e;
    }
  }
  return SF_FAN_OK;
}

sf_fan_error_t sf_fans_set_duty(sf_fan_t *fan, uint16_t duty) {
  fan->duty = duty;
  esp_err_t e = ledc_set_duty(fan->tim.ledc_mode, fan->ledc_channel, fan->duty);
  if (e != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set duty for fan:%d channel, %s", fan->id,
             esp_err_to_name(e));
    return SF_FAN_ERR_FAILED_TO_SET_DUTY;
  }

  e = ledc_update_duty(fan->tim.ledc_mode, fan->ledc_channel);
  if (e != ESP_OK) {
    ESP_LOGE(TAG, "Failed to update duty for fan:%d channel, %s", fan->id,
             esp_err_to_name(e));
    return SF_FAN_ERR_FAILED_TO_UPDATE_DUTY;
  }

  ESP_LOGI(TAG, "fan:%d, duty:%d", fan->id, fan->duty);
  return SF_FAN_OK;
}

sf_fan_error_t sf_fans_init(void) {
  sf_fan_error_t e = sf_fans_config_fan_timers();
  if (e != SF_FAN_OK) {
    return e;
  }
  e = sf_fans_config_fan_channels();
  if (e != SF_FAN_OK) {
    return e;
  }
  return SF_FAN_OK;
}
#define NUM_FANS (sizeof(g_fans) / sizeof(g_fans[0]))
