#ifndef XIAO_BATTERY_H
#define XIAO_BATTERY_H

#include <zboss_api.h>

extern zb_zcl_attr_t battery_power_config_attr_list[];

int battery_init(void);
void battery_start(void);

#endif /* XIAO_BATTERY_H */
