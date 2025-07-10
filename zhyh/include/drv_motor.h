#ifndef __DRV_MOTOR_H__
#define __DRV_MOTOR_H__

#include  "stdbool.h"


void motor_dev_init(void);
void motor_set_state(bool state);
int get_motor_state(void);
void motor_set_state_C(bool state);
int get_motor_state_C(void);
void motor_set_state_D(bool state);
int get_motor_state_D(void);


#endif