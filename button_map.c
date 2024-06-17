#include "button_map.h"

void bm_map_wiimote(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	/* Mapping tables */
	const uint16_t *wiimote_button_map,
	/* Outputs */
	uint16_t *wiimote_buttons)
{
	for (int i = 0; i < num_buttons; i++) {
		if (buttons & 1)
			*wiimote_buttons |= wiimote_button_map[i];
		buttons >>= 1;
	}
}

void bm_map_nunchuk(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	uint16_t ax, uint16_t ay, uint16_t az,
	/* Mapping tables */
	const uint8_t *nunchuk_button_map,
	const uint8_t *nunchuk_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_nunchuk_t *nunchuk)
{
	uint8_t nunchuk_buttons = 0;
	uint8_t nunchuk_analog_axis[BM_NUNCHUK_ANALOG_AXIS__NUM] = {0};

	for (int i = 0; i < num_buttons; i++) {
		if (buttons & 1)
			nunchuk_buttons |= nunchuk_button_map[i];
		buttons >>= 1;
	}

	for (int i = 0; i < num_analog_axis; i++) {
		if (nunchuk_analog_axis_map[i])
			nunchuk_analog_axis[nunchuk_analog_axis_map[i] - 1] = analog_axis[i];
	}

	bm_nunchuk_format(nunchuk, nunchuk_buttons, nunchuk_analog_axis, ax, ay, az);
}

void bm_map_classic(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *classic_button_map,
	const uint8_t *classic_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_classic_t *classic)
{
	uint16_t classic_buttons = 0;
	uint8_t classic_analog_axis[BM_CLASSIC_ANALOG_AXIS__NUM] = {0};

	for (int i = 0; i < num_buttons; i++) {
		if (buttons & 1)
			classic_buttons |= classic_button_map[i];
		buttons >>= 1;
	}

	for (int i = 0; i < num_analog_axis; i++) {
		if (classic_analog_axis_map[i])
			classic_analog_axis[classic_analog_axis_map[i] - 1] = analog_axis[i];
	}

	bm_classic_format(classic, classic_buttons, classic_analog_axis);
}

void bm_map_guitar(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *guitar_button_map,
	const uint8_t *guitar_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_guitar_t *guitar)
{
	uint16_t guitar_buttons = 0;
	uint8_t guitar_analog_axis[BM_GUITAR_ANALOG_AXIS__NUM] = {0};

	for (int i = 0; i < num_buttons; i++) {
		if (buttons & 1)
			guitar_buttons |= guitar_button_map[i];
		buttons >>= 1;
	}

	for (int i = 0; i < num_analog_axis; i++) {
		if (guitar_analog_axis_map[i])
			guitar_analog_axis[guitar_analog_axis_map[i] - 1] = analog_axis[i];
	}

	bm_guitar_format(guitar, guitar_buttons, guitar_analog_axis);
}

void bm_map_turntable(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *turntable_button_map,
	const uint8_t *turntable_analog_axis_map,
	/* Outputs */
	struct wiimote_extension_data_format_turntable_t *turntable)
{
	uint16_t turntable_buttons = 0;
	uint8_t turntable_analog_axis[BM_TURNTABLE_ANALOG_AXIS__NUM] = {0};

	for (int i = 0; i < num_buttons; i++) {
		if (buttons & 1)
			turntable_buttons |= turntable_button_map[i];
		buttons >>= 1;
	}

	for (int i = 0; i < num_analog_axis; i++) {
		if (turntable_analog_axis_map[i])
			turntable_analog_axis[turntable_analog_axis_map[i] - 1] = analog_axis[i];
	}

	bm_turntable_format(turntable, turntable_buttons, turntable_analog_axis);
}

void bm_map_drum(
	/* Inputs */
	int num_buttons, uint32_t buttons,
	int num_analog_axis, const uint8_t *analog_axis,
	/* Mapping tables */
	const uint16_t *drum_button_map,
	/* Outputs */
	struct wiimote_extension_data_format_drum_t *guitar)
{
	uint16_t drum_buttons = 0;

	for (int i = 0; i < num_buttons; i++) {
		if (buttons & 1)
			drum_buttons |= drum_button_map[i];
		buttons >>= 1;
	}

	bm_drum_format(guitar, drum_buttons, analog_axis);
}