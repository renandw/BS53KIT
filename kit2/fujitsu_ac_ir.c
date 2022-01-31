#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "fujitsu_ac_ir.h"
#include <ir/ir.h>
#include <ir/generic.h>


#define countof(x) (sizeof(x) / sizeof(*x))

static const char* ac_cmd_string(ac_cmd command) {
        static char unknown[5];
        switch (command) {
        case ac_cmd_turn_on: return "turn on";
        case ac_cmd_turn_off: return "turn off";
        default: {
                snprintf(unknown, sizeof(unknown), "0x%02x", command & 0xff);
                return unknown;
        }
        }
}

static const char* ac_mode_string(ac_mode mode) {
        static char* strings[] = {"auto", "cool", "dry", "fan", "heat"};
        static char unknown[5];
        if (mode > countof(strings)) {
                snprintf(unknown, sizeof(unknown), "0x%02x", mode & 0xff);
                return unknown;
        }

        return strings[mode];
}

static const char* ac_fan_string(ac_fan fan) {
        static char* strings[] = {"auto", "high", "med", "low"};
        static char unknown[5];
        if (fan > countof(strings)) {
                snprintf(unknown, sizeof(unknown), "0x%02x", fan & 0xff);
                return unknown;
        }
        return strings[fan];
}

static const char* ac_swing_string(ac_swing swing) {
        static char* strings[] = {"off", "vert"};
        static char unknown[5];
        if (swing > countof(strings)) {
                snprintf(unknown, sizeof(unknown), "0x%02x", swing & 0xff);
                return unknown;
        }
        return strings[swing];
}

static void print_state(const char *prompt, fujitsu_ac_state_t *state) {
        printf(
                "%s: command=%s mode=%s fan=%s swing=%s temperature=%d\n",
                prompt,
                ac_cmd_string(state->command),
                ac_mode_string(state->mode),
                ac_fan_string(state->fan),
                ac_swing_string(state->swing),
                state->temperature
                );
}



static fujitsu_ac_model model;
static ir_generic_config_t fujitsu_ac_ir_config = {
        .header_mark = 9100,
        .header_space = -4500,

        .bit1_mark = 500,
        .bit1_space = -1650,

        .bit0_mark = 500,
        .bit0_space = -500,

        .footer_mark = 400,
        .footer_space = -8000,

        .tolerance = 20,
};


void fujitsu_ac_ir_tx_init(fujitsu_ac_model ac_model) {
        ir_tx_init();
        model = ac_model;
}

int fujitsu_ac_ir_send(fujitsu_ac_state_t *state) {
        uint8_t cmd[13];
        size_t cmd_size = 13;

        cmd[0] = 0xC3;
        cmd[1] = state->swing | ((state->temperature - 8) << 3);
        cmd[2] = 0xe0;
        cmd[3] = 0x10;;
        cmd[4] = state->fan;
        cmd[5] = 0x00;
        cmd[6] = state->mode;
        cmd[7] = 0x00;
        cmd[8] = 0x00;
        cmd[9] = state->command;
        cmd[10] = 0x00;
        cmd[11] = 0x08; // timer off values

        uint8_t checksum = 0;

        switch (model) {
        case fujitsu_ac_model_ARRAH2E:
                for(uint8_t i = 0; i < 12; i++) {
                        checksum += cmd[i];
                }
                cmd[12] = checksum;
                break;
        case fujitsu_ac_model_ARDB1:
                for(uint8_t i = 0; i < 12; i++) {
                        checksum += cmd[i];
                }
                cmd[12] = checksum;
                break;
        }


        return ir_generic_send(&fujitsu_ac_ir_config, cmd, cmd_size);
}


typedef struct {
        ir_decoder_t decoder;
        ir_decoder_t *generic_decoder;
} fujitsu_ac_ir_decoder_t;


static int fujitsu_ac_ir_decoder_decode(fujitsu_ac_ir_decoder_t *decoder,
                                        int16_t *pulses, uint16_t pulse_count,
                                        void *decode_buffer, uint16_t decode_buffer_size)
{
/*    if (decode_buffer_size < sizeof(fujitsu_ac_state_t))
        return -2;

    fujitsu_ac_state_t *state = decode_buffer;

    uint8_t cmd[13];
    int cmd_size = decoder->generic_decoder->decode(
        decoder->generic_decoder, pulses, pulse_count, cmd, sizeof(cmd)
    );
    if (cmd_size <= 0)
        return cmd_size;

    if (cmd_size < 6)
        return -1;

    if (cmd[0] != 0xC3)
        return -1;

    if (cmd[2] != 0xe0)
        return -1;

    if (cmd[3] != 0x10)
        return -1;

    if (cmd[7] != 0x00)
        return -1;

    if (cmd[8] != 0x00)
        return -1;

    switch (cmd[5]) {
    case ac_cmd_turn_off:
        if ((cmd_size == 7) && (cmd[6] != (~cmd[5] & 0xff))) {
            return -1;
        } else if (cmd_size > 7) {
            return -1;
        }

        cmd[5] = 0x00;

        break;

        state->command = cmd[9] && 0xf;
        state->temperature = AC_MIN_TEMPERATURE + (cmd[1] >> 3);
        state->mode = cmd[6] & 0xf;
        state->fan = cmd[4] & 0xf;
        state->swing = cmd[1] >> 4;

        break;
    }


    print_state("Decoded state", state);

    return sizeof(fujitsu_ac_state_t);
   }
 */
}


static void fujitsu_ac_ir_decoder_free(fujitsu_ac_ir_decoder_t *decoder) {
        decoder->generic_decoder->free(decoder->generic_decoder);
        free(decoder);
}


ir_decoder_t *fujitsu_ac_ir_make_decoder() {
        fujitsu_ac_ir_decoder_t *decoder = malloc(sizeof(fujitsu_ac_ir_decoder_t));
        if (!decoder)
                return NULL;

        decoder->generic_decoder = ir_generic_make_decoder(&fujitsu_ac_ir_config);
        if (!decoder->generic_decoder) {
                free(decoder);
                return NULL;
        }

        decoder->decoder.decode = (ir_decoder_decode_t) fujitsu_ac_ir_decoder_decode;
        decoder->decoder.free = (ir_decoder_free_t) fujitsu_ac_ir_decoder_free;

        return (ir_decoder_t*) decoder;
}
