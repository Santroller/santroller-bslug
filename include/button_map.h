#ifndef BUTTON_MAPPING_H
#define BUTTON_MAPPING_H
#include "wiimote.h"
#include <stdint.h>

#define BM_ANALOG_AXIS_INVALID	0

/* Nunchuk */
enum bm_nunchuk_analog_axis_e {
	BM_NUNCHUK_ANALOG_AXIS_X = 1,
	BM_NUNCHUK_ANALOG_AXIS_Y,
	BM_NUNCHUK_ANALOG_AXIS__NUM = BM_NUNCHUK_ANALOG_AXIS_Y
};

/* Classic controller */
enum bm_classic_analog_axis_e {
	BM_CLASSIC_ANALOG_AXIS_LEFT_X = 1,
	BM_CLASSIC_ANALOG_AXIS_LEFT_Y,
	BM_CLASSIC_ANALOG_AXIS_RIGHT_X,
	BM_CLASSIC_ANALOG_AXIS_RIGHT_Y,
	BM_CLASSIC_ANALOG_AXIS__NUM = BM_CLASSIC_ANALOG_AXIS_RIGHT_Y
};

/* Guitar controller */
enum bm_guitar_analog_axis_e {
	BM_GUITAR_ANALOG_AXIS_STICK_X = 1,
	BM_GUITAR_ANALOG_AXIS_STICK_Y,
	BM_GUITAR_ANALOG_AXIS_TAP_BAR,
	BM_GUITAR_ANALOG_AXIS_WHAMMY_BAR,
	BM_GUITAR_ANALOG_AXIS__NUM = BM_GUITAR_ANALOG_AXIS_WHAMMY_BAR
};

/* Turntable controller */
enum bm_turntable_analog_axis_e {
	BM_TURNTABLE_ANALOG_AXIS_STICK_X = 1,
	BM_TURNTABLE_ANALOG_AXIS_STICK_Y,
	BM_TURNTABLE_ANALOG_LEFT_TURNTABLE_VELOCITY,
	BM_TURNTABLE_ANALOG_RIGHT_TURNTABLE_VELOCITY,
	BM_TURNTABLE_ANALOG_CROSS_FADER,
	BM_TURNTABLE_ANALOG_EFFECTS_DIAL,
	BM_TURNTABLE_ANALOG_AXIS__NUM = BM_TURNTABLE_ANALOG_EFFECTS_DIAL
};

/* Drum controller */
enum bm_drum_analog_axis_e {
	BM_DRUM_ANALOG_AXIS_STICK_X = 1,
	BM_DRUM_ANALOG_AXIS_STICK_Y,
	BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR,
	BM_DRUM_ANALOG_AXIS_VELOCITY,
	BM_DRUM_ANALOG_AXIS__NUM = BM_DRUM_ANALOG_AXIS_VELOCITY
};

/* IR pointer emulation */
enum bm_ir_emulation_mode_e {
	BM_IR_EMULATION_MODE_NONE,
	BM_IR_EMULATION_MODE_DIRECT,
	BM_IR_EMULATION_MODE_RELATIVE_ANALOG_AXIS,
	BM_IR_EMULATION_MODE_ABSOLUTE_ANALOG_AXIS,
};

enum bm_ir_axis_e {
	BM_IR_AXIS_NONE,
	BM_IR_AXIS_X,
	BM_IR_AXIS_Y,
	BM_IR_AXIS__NUM = BM_IR_AXIS_Y,
};

struct bm_ir_emulation_state_t {
	uint16_t position[BM_IR_AXIS__NUM];
};

void bm_map_wiimote(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	/* Mapping tables */
	const uint16_t *wiimote_button_map,
	/* Outputs */
	uint16_t *wiimote_buttons);

void bm_map_nunchuk(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	uint16_t ax, uint16_t ay, uint16_t az,
	/* Mapping tables */
	const uint8_t *nunchuk_button_map,
	const uint8_t *nunchuk_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_nunchuk_t *nunchuk);

void bm_map_classic(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *classic_button_map,
	const uint8_t *classic_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_classic_t *classic);

void bm_map_guitar(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *guitar_button_map,
	const uint8_t *guitar_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_guitar_t *guitar);

void bm_map_turntable(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *turntable_button_map,
	const uint8_t *turntable_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_turntable_t *guitar);

void bm_map_drum(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *drum_button_map,
	/* Outputs */
	struct wiimote_extension_data_format_drum_t *drum);

void bm_map_ir_direct(
	/* Inputs */
	int num_coordinates, const uint16_t *x, const uint16_t *y,
	uint16_t max_x, uint16_t max_y,
	/* Outputs */
	struct ir_dot_t ir_dots[static IR_MAX_DOTS]);

void bm_map_ir_analog_axis(
	/* Inputs */
	enum bm_ir_emulation_mode_e mode,
	struct bm_ir_emulation_state_t *state,
	int num_analog_axis, const uint8_t *analog_axis,
	const uint8_t *ir_analog_axis_map,
	/* Outputs */
	struct ir_dot_t ir_dots[static IR_MAX_DOTS]);

static inline bool bm_check_switch_mapping(uint32_t buttons, bool *switch_mapping, uint32_t switch_mapping_combo)
{
	bool switch_pressed = (buttons & switch_mapping_combo) == switch_mapping_combo;
	bool ret = false;

	if (switch_pressed && !*switch_mapping)
		ret = true;

	*switch_mapping = switch_pressed;
	return ret;
}

static inline void bm_nunchuk_format(struct wiimote_extension_data_format_nunchuk_t *out,
					 uint8_t buttons, uint8_t analog_axis[static BM_NUNCHUK_ANALOG_AXIS__NUM],
					 uint16_t ax, uint16_t ay, uint16_t az)
{
	out->jx = analog_axis[BM_NUNCHUK_ANALOG_AXIS_X - 1];
	out->jy = analog_axis[BM_NUNCHUK_ANALOG_AXIS_Y - 1];
	out->ax = (ax >> 2) & 0xFF;
	out->ay = (ay >> 2) & 0xFF;
	out->az = (az >> 2) & 0xFF;
	out->bt.acc_z_lsb = az & 3;
	out->bt.acc_y_lsb = ay & 3;
	out->bt.acc_x_lsb = ax & 3;
	out->bt.c = !(buttons & NUNCHUK_BUTTON_C);
	out->bt.z = !(buttons & NUNCHUK_BUTTON_Z);
}

static inline void bm_classic_format(struct wiimote_extension_data_format_classic_t *out,
					 uint16_t buttons, uint8_t analog_axis[static BM_CLASSIC_ANALOG_AXIS__NUM])
{
	uint8_t lx = analog_axis[BM_CLASSIC_ANALOG_AXIS_LEFT_X - 1] >> 2;
	uint8_t ly = analog_axis[BM_CLASSIC_ANALOG_AXIS_LEFT_Y - 1] >> 2;
	uint8_t rx = analog_axis[BM_CLASSIC_ANALOG_AXIS_RIGHT_X - 1] >> 3;
	uint8_t ry = analog_axis[BM_CLASSIC_ANALOG_AXIS_RIGHT_Y - 1] >> 3;
	uint8_t lt = (buttons & CLASSIC_CTRL_BUTTON_FULL_L) ? 31 : 0;
	uint8_t rt = (buttons & CLASSIC_CTRL_BUTTON_FULL_R) ? 31 : 0;

	out->rx3 = (rx >> 3) & 3;
	out->lx = lx & 0x3F;
	out->rx2 = (rx >> 1) & 3;
	out->ly = ly & 0x3F;
	out->rx1 = rx & 1;
	out->lt2 = (lt >> 3) & 3;
	out->ry = ry & 0x1F;
	out->lt1 = lt & 0x7;
	out->rt = rt & 0x1F;
	out->bt.hex = (~buttons) & CLASSIC_CTRL_BUTTON_ALL;
}



static inline void bm_turntable_format(struct wiimote_extension_data_format_turntable_t *out,
					 uint16_t buttons, uint8_t analog_axis[static BM_TURNTABLE_ANALOG_AXIS__NUM])
{
	uint8_t sx = analog_axis[BM_TURNTABLE_ANALOG_AXIS_STICK_X - 1] >> 2;
	uint8_t sy = analog_axis[BM_TURNTABLE_ANALOG_AXIS_STICK_Y - 1] >> 2;
	uint8_t ltt = analog_axis[BM_TURNTABLE_ANALOG_LEFT_TURNTABLE_VELOCITY - 1] >> 3;
	uint8_t rtt = analog_axis[BM_TURNTABLE_ANALOG_RIGHT_TURNTABLE_VELOCITY - 1] >> 3;
	uint8_t cross_fader = analog_axis[BM_TURNTABLE_ANALOG_CROSS_FADER - 1] >> 4;
	uint8_t effects_dial = analog_axis[BM_TURNTABLE_ANALOG_EFFECTS_DIAL - 1] >> 3;

	out->sx = sx & 0x3F;
	out->sy = sy & 0x3F;
	out->rtt1 = rtt & 1;
	out->rtt2 = (rtt >> 1) & 3;
	out->rtt3 = (rtt >> 3) & 3;
	out->rtt5 = rtt & (1<<5);
	out->effects2 = (effects_dial >> 3) & 3;
	out->effects1 = effects_dial & 0x7;
	out->ltt1 = ltt & 0x20;
	out->crossfade = cross_fader;
	out->bt.hex = (~buttons) & TURNTABLE_CTRL_BUTTON_ALL;
	out->bt.ltt5 = ltt & (1<<5);
}

static inline void bm_guitar_format(struct wiimote_extension_data_format_guitar_t *out,
					 uint16_t buttons, const uint8_t analog_axis[static BM_GUITAR_ANALOG_AXIS__NUM])
{
	uint8_t sx = analog_axis[BM_GUITAR_ANALOG_AXIS_STICK_X - 1] >> 2;
	uint8_t sy = analog_axis[BM_GUITAR_ANALOG_AXIS_STICK_Y - 1] >> 2;
	uint8_t tb = analog_axis[BM_GUITAR_ANALOG_AXIS_TAP_BAR - 1] >> 3;
	uint8_t wb = analog_axis[BM_GUITAR_ANALOG_AXIS_WHAMMY_BAR - 1] >> 3;
	out->sx = sx & 0x3F;
	out->sy = sy & 0x3F;
	out->tb = tb & 0x1F;
	out->wb = wb & 0x1F;
	out->bt.hex = (~buttons) & GUITAR_CTRL_BUTTON_ALL;
}

static inline void bm_drum_format(struct wiimote_extension_data_format_drum_t *out,
					 uint16_t buttons, const uint8_t analog_axis[static BM_DRUM_ANALOG_AXIS__NUM])
{
	uint8_t sx = analog_axis[BM_DRUM_ANALOG_AXIS_STICK_X - 1] >> 2;
	uint8_t sy = analog_axis[BM_DRUM_ANALOG_AXIS_STICK_Y - 1] >> 2;
	uint8_t type = analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY_SELECTOR - 1];
	uint8_t velocity = analog_axis[BM_DRUM_ANALOG_AXIS_VELOCITY - 1] >> 5;
	out->sx = sx & 0x3F;
	out->sy = sy & 0x3F;
	out->velocity_type = type & 0x1f;
	out->velocity = velocity & 0x07;
	out->bt.hex = (~buttons) & DRUM_CTRL_BUTTON_ALL;
	out->extra = 0b0110;
}

static inline void bm_ir_emulation_state_reset(struct bm_ir_emulation_state_t *state)
{
	state->position[BM_IR_AXIS_X - 1] = IR_CENTER_X;
	state->position[BM_IR_AXIS_Y - 1] = IR_CENTER_Y;
}

static inline void bm_ir_dots_set_out_of_screen(struct ir_dot_t ir_dots[static IR_MAX_DOTS])
{
	for (int i = 0; i < IR_MAX_DOTS; i++)
		ir_dots[i].y = 1023;
}

#endif
