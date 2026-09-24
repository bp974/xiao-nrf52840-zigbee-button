#include <errno.h>
#include <stdint.h>

#include <zephyr/devicetree.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zboss_api.h>
#include <zboss_api_addons.h>

#include "battery.h"
#include "zb_button_remote.h"

#define BATTERY_SAMPLE_INTERVAL      K_MINUTES(15)
#define BATTERY_DIVIDER_TOP_OHMS     1000U
#define BATTERY_DIVIDER_BOTTOM_OHMS  510U
#define BATTERY_SAMPLE_COUNT         8U
#define BATTERY_ENABLE_PIN           14

LOG_MODULE_REGISTER(battery, LOG_LEVEL_INF);

static const struct adc_dt_spec battery_adc =
	ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
static const struct device *gpio0 = DEVICE_DT_GET(DT_NODELABEL(gpio0));
static struct k_work_delayable battery_work;

static zb_uint8_t battery_voltage_attr =
	ZB_ZCL_POWER_CONFIG_BATTERY_VOLTAGE_INVALID;
zb_uint16_t battery_voltage_exact_attr;
static zb_uint8_t battery_size_attr = 0xff;
static zb_uint8_t battery_quantity_attr = 1;
static zb_uint8_t battery_rated_voltage_attr = 42;
static zb_uint8_t battery_alarm_mask_attr = 0;
static zb_uint8_t battery_voltage_min_threshold_attr = 0;
static zb_uint8_t battery_percentage_attr =
	ZB_ZCL_POWER_CONFIG_BATTERY_REMAINING_UNKNOWN;

/* NCS 2.6.0's extended power-config macro omits the battery number argument. */
ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(
	battery_power_config_attr_list, ZB_ZCL_POWER_CONFIG)
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID(
	&battery_voltage_attr, ),
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_SIZE_ID(
	&battery_size_attr, ),
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_QUANTITY_ID(
	&battery_quantity_attr, ),
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_RATED_VOLTAGE_ID(
	&battery_rated_voltage_attr, ),
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_ALARM_MASK_ID(
	&battery_alarm_mask_attr, ),
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_MIN_THRESHOLD_ID(
	&battery_voltage_min_threshold_attr, ),
ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID(
	&battery_percentage_attr, ),
ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST;

static zb_uint8_t voltage_to_percentage(uint32_t millivolts)
{
	static const uint16_t voltage_table[] = {
		3300, 3500, 3600, 3650, 3700, 3750,
		3800, 3850, 3900, 4000, 4100, 4200,
	};
	static const uint8_t percentage_table[] = {
		0, 5, 10, 15, 20, 30, 40, 50, 60, 75, 90, 100,
	};

	if (millivolts <= voltage_table[0]) {
		return 0;
	}
	if (millivolts >= voltage_table[ARRAY_SIZE(voltage_table) - 1]) {
		return 200;
	}

	for (size_t i = 1; i < ARRAY_SIZE(voltage_table); i++) {
		if (millivolts <= voltage_table[i]) {
			uint32_t low_mv = voltage_table[i - 1];
			uint32_t high_mv = voltage_table[i];
			uint32_t low_pct = percentage_table[i - 1];
			uint32_t high_pct = percentage_table[i];
			uint32_t percent = low_pct +
				((millivolts - low_mv) * (high_pct - low_pct)) /
				(high_mv - low_mv);

			return (zb_uint8_t)(percent * 2U);
		}
	}

	return 0;
}

static int read_battery(uint32_t *millivolts, int32_t *adc_millivolts)
{
	int16_t samples[BATTERY_SAMPLE_COUNT];
	struct adc_sequence sequence;
	struct adc_sequence_options options = {
		.extra_samplings = BATTERY_SAMPLE_COUNT - 1U,
	};
	int32_t sample_mv;
	int64_t total = 0;
	int err;

	sequence = (struct adc_sequence){
		.buffer = samples,
		.buffer_size = sizeof(samples),
		.options = &options,
	};
	adc_sequence_init_dt(&battery_adc, &sequence);

	err = adc_read_dt(&battery_adc, &sequence);
	if (err) {
		return err;
	}

	for (size_t i = 0; i < BATTERY_SAMPLE_COUNT; i++) {
		total += samples[i];
	}

	sample_mv = (int32_t)(total / BATTERY_SAMPLE_COUNT);
	err = adc_raw_to_millivolts_dt(&battery_adc, &sample_mv);
	if (err) {
		return err;
	}
	*adc_millivolts = sample_mv;

	*millivolts = (uint32_t)sample_mv * BATTERY_DIVIDER_TOP_OHMS /
		BATTERY_DIVIDER_BOTTOM_OHMS + (uint32_t)sample_mv;
	return 0;
}

static void battery_apply_zigbee(zb_bufid_t bufid)
{
	zb_zcl_status_t voltage_status;
	zb_zcl_status_t exact_voltage_status;
	zb_zcl_status_t percentage_status;

	ZVUNUSED(bufid);

	voltage_status = zb_zcl_set_attr_val(
		BUTTON_REMOTE_ENDPOINT, ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID,
		&battery_voltage_attr, ZB_FALSE);
	exact_voltage_status = zb_zcl_set_attr_val(
		BUTTON_REMOTE_ENDPOINT, ZB_ZCL_CLUSTER_ID_BASIC,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		BUTTON_REMOTE_EXACT_VOLTAGE_ATTR_ID,
		(zb_uint8_t *)&battery_voltage_exact_attr, ZB_FALSE);
	percentage_status = zb_zcl_set_attr_val(
		BUTTON_REMOTE_ENDPOINT, ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
		ZB_ZCL_CLUSTER_SERVER_ROLE,
		ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
		&battery_percentage_attr, ZB_FALSE);

	LOG_INF("Battery ZCL update: voltage=%u (%u mV), status=%d; exact=%u mV, status=%d; percentage=%u (%u%%), status=%d",
		battery_voltage_attr, battery_voltage_attr * 100U, voltage_status,
		battery_voltage_exact_attr, exact_voltage_status,
		battery_percentage_attr, battery_percentage_attr / 2U,
		percentage_status);

	if (voltage_status != RET_OK) {
		LOG_ERR("Failed to update battery voltage attribute: %d", voltage_status);
	}
	if (exact_voltage_status != RET_OK) {
		LOG_ERR("Failed to update exact battery voltage attribute: %d",
			exact_voltage_status);
	}
	if (percentage_status != RET_OK) {
		LOG_ERR("Failed to update battery percentage attribute: %d", percentage_status);
	}
}

static void battery_work_fn(struct k_work *work)
{
	uint32_t millivolts;
	int32_t adc_millivolts;
	int err;

	ARG_UNUSED(work);

	err = read_battery(&millivolts, &adc_millivolts);
	if (err) {
		LOG_ERR("Battery ADC read failed: %d", err);
	} else {
		battery_voltage_attr = (zb_uint8_t)((millivolts + 50U) / 100U);
		battery_voltage_exact_attr = (zb_uint16_t)millivolts;
		battery_percentage_attr = voltage_to_percentage(millivolts);
		LOG_INF("Battery: %u mV (ADC: %d mV, %u%%)", millivolts,
			adc_millivolts,
			battery_percentage_attr / 2U);
		ZB_SCHEDULE_APP_CALLBACK(battery_apply_zigbee, 0);
	}

	k_work_reschedule(&battery_work, BATTERY_SAMPLE_INTERVAL);
}

int battery_init(void)
{
	int err;

	if (!adc_is_ready_dt(&battery_adc)) {
		return -ENODEV;
	}
	if (!device_is_ready(gpio0)) {
		return -ENODEV;
	}
	err = adc_channel_setup_dt(&battery_adc);
	if (err) {
		return err;
	}

	/* P0.14 low enables the XIAO's battery-voltage divider. */
	err = gpio_pin_configure(gpio0, BATTERY_ENABLE_PIN, GPIO_OUTPUT_LOW);
	if (err) {
		return err;
	}

	k_work_init_delayable(&battery_work, battery_work_fn);
	return 0;
}

void battery_start(void)
{
	k_work_reschedule(&battery_work, K_NO_WAIT);
}
