#pragma once

#include <stddef.h>
#include <ir/ir.h>


typedef enum {
    fujitsu_ac_model_ARRAH2E = 1,
    fujitsu_ac_model_ARDB1,
} fujitsu_ac_model;


typedef enum {
    ac_cmd_turn_on = 0x20,
    ac_cmd_turn_off = 0x00,
} ac_cmd;

typedef enum {
    ac_mode_auto = 0x00,
    ac_mode_cool = 0x20,
    ac_mode_dry = 0x40,
    ac_mode_fan = 0xc0,
    ac_mode_heat = 0x80,
} ac_mode;

typedef enum {
    ac_fan_auto = 0xA0,
    ac_fan_high = 0x20,
    ac_fan_med = 0x40,
    ac_fan_low = 0x60,
} ac_fan;

typedef enum {
    ac_swing_off = 0x07,
    ac_swing_vert = 0x00,
} ac_swing;


#define AC_MIN_TEMPERATURE 16
#define AC_MAX_TEMPERATURE 30



typedef struct {
  ac_swing swing;
  uint8_t temperature;
  ac_fan fan;
  ac_mode mode;
  ac_cmd command;

} fujitsu_ac_state_t;


void fujitsu_ac_ir_tx_init(fujitsu_ac_model ac_model);
int fujitsu_ac_ir_send(fujitsu_ac_state_t *state);

ir_decoder_t *fujitsu_ac_ir_make_decoder();
