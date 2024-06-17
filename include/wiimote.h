#ifndef WIIMOTE_H
#define WIIMOTE_H
#include "defines.h"
#include <stdint.h>
#include <stdbool.h>
/* Source: HID_010_SPC_PFL/1.0 (official HID specification) and Dolphin emulator */
#define HID_TYPE_HANDSHAKE	0
#define HID_TYPE_SET_REPORT	5
#define HID_TYPE_DATA		0xA
#define HID_HANDSHAKE_SUCCESS	0
#define HID_PARAM_INPUT		1
#define HID_PARAM_OUTPUT	2

/* Wiimote definitions */
#define WIIMOTE_HCI_CLASS_0	0x00
#define WIIMOTE_HCI_CLASS_1	0x04
#define WIIMOTE_HCI_CLASS_2	0x48

#define WIIMOTE_REMOTE_FEATURES ((uint8_t[]){0xBC, 0x02, 0x04, 0x38, 0x08, 0x00, 0x00, 0x00})

#define WIIMOTE_LMP_VERSION	0x2
#define WIIMOTE_LMP_SUBVERSION	0x229
/* Broadcom Corporation */
#define WIIMOTE_MANUFACTURER_ID	0x000F

#define WII_REQUEST_MTU		185
#define WIIMOTE_MAX_PAYLOAD	23

/* Wiimote -> Host */
#define INPUT_REPORT_ID_STATUS		0x20
#define INPUT_REPORT_ID_READ_DATA_REPLY	0x21
#define INPUT_REPORT_ID_ACK		0x22
/* Not a real value on the wiimote, just a state to disable reports */
#define INPUT_REPORT_ID_REPORT_DISABLED	0x00
#define INPUT_REPORT_ID_BTN		0x30
#define INPUT_REPORT_ID_BTN_ACC		0x31
#define INPUT_REPORT_ID_BTN_EXP8	0x32
#define INPUT_REPORT_ID_BTN_ACC_IR	0x33
#define INPUT_REPORT_ID_BTN_EXP19	0x34
#define INPUT_REPORT_ID_BTN_ACC_EXP	0x35
#define INPUT_REPORT_ID_BTN_IR_EXP	0x36
#define INPUT_REPORT_ID_BTN_ACC_IR_EXP	0x37
#define INPUT_REPORT_ID_EXP21		0x3d

/* Host -> Wiimote */
#define OUTPUT_REPORT_ID_RUMBLE		0x10
#define OUTPUT_REPORT_ID_LED		0x11
#define OUTPUT_REPORT_ID_REPORT_MODE	0x12
#define OUTPUT_REPORT_ID_IR_ENABLE	0x13
#define OUTPUT_REPORT_ID_SPEAKER_ENABLE	0x14
#define OUTPUT_REPORT_ID_STATUS 	0x15
#define OUTPUT_REPORT_ID_WRITE_DATA 	0x16
#define OUTPUT_REPORT_ID_READ_DATA	0x17
#define OUTPUT_REPORT_ID_SPEAKER_DATA	0x18
#define OUTPUT_REPORT_ID_SPEAKER_MUTE	0x19
#define OUTPUT_REPORT_ID_IR_ENABLE2	0x1a

/* Error codes */
#define ERROR_CODE_SUCCESS		0
#define ERROR_CODE_BUSY			4
#define ERROR_CODE_INVALID_SPACE	6
#define ERROR_CODE_NACK			7
#define ERROR_CODE_INVALID_ADDRESS	8

/* Address spaces */
#define ADDRESS_SPACE_EEPROM		0x00
#define ADDRESS_SPACE_I2C_BUS		0x01
#define ADDRESS_SPACE_I2C_BUS_ALT	0x02

/* I2C addresses */
#define EEPROM_I2C_ADDR		0x50
#define EXTENSION_I2C_ADDR	0x52
#define CAMERA_I2C_ADDR		0x58

/* Memory sizes */
#define EEPROM_FREE_SIZE	0x1700

/* Offsets in Wiimote memory */
#define WIIMOTE_EXP_MEM_CALIBR	0x20
#define WIIMOTE_EXP_ID		0xFA

/* Wiimote button codes */
#define WIIMOTE_BUTTON_TWO		0x0001
#define WIIMOTE_BUTTON_ONE		0x0002
#define WIIMOTE_BUTTON_B		0x0004
#define WIIMOTE_BUTTON_A		0x0008
#define WIIMOTE_BUTTON_MINUS		0x0010
#define WIIMOTE_BUTTON_ZACCEL_BIT6	0x0020
#define WIIMOTE_BUTTON_ZACCEL_BIT7	0x0040
#define WIIMOTE_BUTTON_HOME		0x0080
#define WIIMOTE_BUTTON_LEFT		0x0100
#define WIIMOTE_BUTTON_RIGHT		0x0200
#define WIIMOTE_BUTTON_DOWN		0x0400
#define WIIMOTE_BUTTON_UP		0x0800
#define WIIMOTE_BUTTON_PLUS		0x1000
#define WIIMOTE_BUTTON_ZACCEL_BIT4	0x2000
#define WIIMOTE_BUTTON_ZACCEL_BIT5	0x4000
#define WIIMOTE_BUTTON_UNKNOWN		0x8000
#define WIIMOTE_BUTTON_ALL		0x1F9F

/* Nunchuk button codes */
#define NUNCHUK_BUTTON_Z	0x01
#define NUNCHUK_BUTTON_C	0x02

/* Classic Controller button codes */
#define CLASSIC_CTRL_BUTTON_UP		0x0001
#define CLASSIC_CTRL_BUTTON_LEFT	0x0002
#define CLASSIC_CTRL_BUTTON_ZR		0x0004
#define CLASSIC_CTRL_BUTTON_X		0x0008
#define CLASSIC_CTRL_BUTTON_A		0x0010
#define CLASSIC_CTRL_BUTTON_Y		0x0020
#define CLASSIC_CTRL_BUTTON_B		0x0040
#define CLASSIC_CTRL_BUTTON_ZL		0x0080
#define CLASSIC_CTRL_BUTTON_FULL_R	0x0200
#define CLASSIC_CTRL_BUTTON_PLUS	0x0400
#define CLASSIC_CTRL_BUTTON_HOME	0x0800
#define CLASSIC_CTRL_BUTTON_MINUS	0x1000
#define CLASSIC_CTRL_BUTTON_FULL_L	0x2000
#define CLASSIC_CTRL_BUTTON_DOWN	0x4000
#define CLASSIC_CTRL_BUTTON_RIGHT	0x8000
#define CLASSIC_CTRL_BUTTON_ALL		0xFEFF

/* Guitar button codes */
#define GUITAR_CTRL_BUTTON_STRUM_UP	0x0001
#define GUITAR_CTRL_BUTTON_YELLOW	0x0008
#define GUITAR_CTRL_BUTTON_GREEN	0x0010
#define GUITAR_CTRL_BUTTON_BLUE		0x0020
#define GUITAR_CTRL_BUTTON_RED		0x0040
#define GUITAR_CTRL_BUTTON_ORANGE	0x0080
#define GUITAR_CTRL_BUTTON_PLUS		0x0400
#define GUITAR_CTRL_BUTTON_MINUS	0x1000
#define GUITAR_CTRL_BUTTON_STRUM_DOWN	0x4000
#define GUITAR_CTRL_BUTTON_ALL		0xFEFF

/* Drum button codes */
#define DRUM_CTRL_BUTTON_UP			0x0001
#define DRUM_CTRL_BUTTON_LEFT		0x0002
#define DRUM_CTRL_BUTTON_YELLOW		0x0008
#define DRUM_CTRL_BUTTON_GREEN		0x0010
#define DRUM_CTRL_BUTTON_BLUE		0x0020
#define DRUM_CTRL_BUTTON_RED		0x0040
#define DRUM_CTRL_BUTTON_ORANGE		0x0080
#define DRUM_CTRL_BUTTON_PLUS		0x0400
#define DRUM_CTRL_BUTTON_MINUS		0x1000
#define DRUM_CTRL_BUTTON_DOWN		0x4000
#define DRUM_CTRL_BUTTON_RIGHT		0x8000
#define DRUM_CTRL_BUTTON_ALL		0xFEFF

/* Turntable button codes */
#define TURNTABLE_CTRL_BUTTON_LEFT_GREEN	0x0008
#define TURNTABLE_CTRL_BUTTON_LEFT_RED		0x2000
#define TURNTABLE_CTRL_BUTTON_LEFT_BLUE		0x0080
#define TURNTABLE_CTRL_BUTTON_RIGHT_GREEN	0x0020
#define TURNTABLE_CTRL_BUTTON_RIGHT_RED		0x0200
#define TURNTABLE_CTRL_BUTTON_RIGHT_BLUE	0x0004
#define TURNTABLE_CTRL_BUTTON_EUPHORIA		0x0010
#define TURNTABLE_CTRL_BUTTON_PLUS			0x0400
#define TURNTABLE_CTRL_BUTTON_MINUS			0x1000
#define TURNTABLE_CTRL_BUTTON_ALL			0xFEFF

/* Acceleromter configuration */
#define ACCEL_ZERO_G	(0x80 << 2)
#define ACCEL_ONE_G	(0x9A << 2)

/* IR data modes */
#define IR_MODE_BASIC		1
#define IR_MODE_EXTENDED	3
#define IR_MODE_FULL		5

/* IR configuration */
#define IR_MAX_DOTS	4
#define IR_LOW_X	0x7F
#define IR_LOW_Y	0x5D
#define IR_HIGH_X	0x380
#define IR_HIGH_Y	0x2A2
#define IR_CENTER_X	((IR_HIGH_X + IR_LOW_X) >> 1)
#define IR_CENTER_Y	((IR_HIGH_Y + IR_LOW_Y) >> 1)
#define IR_HORIZONTAL_OFFSET	64
#define IR_VERTICAL_OFFSET	110
#define IR_DOT_SIZE	4
#define IR_DOT_CENTER_MIN_X (IR_LOW_X  + IR_HORIZONTAL_OFFSET)
#define IR_DOT_CENTER_MAX_X (IR_HIGH_X - IR_HORIZONTAL_OFFSET)
#define IR_DOT_CENTER_MIN_Y (IR_LOW_Y  + IR_VERTICAL_OFFSET)
#define IR_DOT_CENTER_MAX_Y (IR_HIGH_Y - IR_VERTICAL_OFFSET)

/* Input reports (Wiimote -> Host) */

struct wiimote_input_report_ack_t {
	uint16_t buttons;
	uint8_t rpt_id;
	uint8_t error_code;
} ATTRIBUTE_PACKED;

struct wiimote_input_report_status_t {
	uint16_t buttons;
	uint8_t leds : 4;
	uint8_t ir : 1;
	uint8_t speaker : 1;
	uint8_t extension : 1;
	uint8_t battery_low : 1;
	uint8_t padding2[2];
	uint8_t battery;
} ATTRIBUTE_PACKED;

struct wiimote_input_report_read_data_t {
	uint16_t buttons;
	uint8_t size_minus_one : 4;
	uint8_t error : 4;
	// big endian:
	uint16_t address;
	uint8_t data[16];
} ATTRIBUTE_PACKED;

/* Output reports (Host -> Wiimote) */

struct wiimote_output_report_led_t {
	uint8_t leds : 4;
	uint8_t : 2;
	uint8_t ack : 1;
	uint8_t rumble : 1;
} ATTRIBUTE_PACKED;

struct wiimote_output_report_enable_feature_t {
	uint8_t : 5;
	// Enable/disable certain feature.
	uint8_t enable : 1;
	// Respond with an ack.
	uint8_t ack : 1;
	uint8_t rumble : 1;
} ATTRIBUTE_PACKED;

struct wiimote_output_report_mode_t {
	uint8_t : 5;
	uint8_t continuous : 1;
	uint8_t ack : 1;
	uint8_t rumble : 1;
	uint8_t mode;
} ATTRIBUTE_PACKED;

struct wiimote_output_report_write_data_t {
	uint8_t : 4;
	uint8_t space : 2;
	uint8_t : 1;
	uint8_t rumble : 1;
	// Used only for register space (i2c bus) (7-bits):
	uint8_t slave_address : 7;
	// A real wiimote ignores the i2c read/write bit.
	uint8_t i2c_rw_ignored : 1;
	// big endian:
	uint16_t address;
	uint8_t size;
	uint8_t data[16];
} ATTRIBUTE_PACKED;

struct wiimote_output_report_read_data_t {
	uint8_t : 4;
	uint8_t space : 2;
	uint8_t : 1;
	uint8_t rumble : 1;
	// Used only for register space (i2c bus) (7-bits):
	uint8_t slave_address : 7;
	// A real wiimote ignores the i2c read/write bit.
	uint8_t i2c_rw_ignored : 1;
	// big endian:
	uint16_t address;
	uint16_t size;
} ATTRIBUTE_PACKED;

/* IR camera */

#define CAMERA_DATA_BYTES	36

struct wiimote_ir_camera_registers_t {
	// Contains sensitivity and other unknown data
	// TODO: Does disabling the camera peripheral reset the mode or sensitivity?
	uint8_t sensitivity_block1[9];
	uint8_t unk_0x09[17];

	// addr: 0x1a
	uint8_t sensitivity_block2[2];
	uint8_t unk_0x1c[20];

	// addr: 0x30
	uint8_t enable_object_tracking;
	uint8_t unk_0x31[2];

	// addr: 0x33
	uint8_t mode;
	uint8_t unk_0x34[3];

	// addr: 0x37
	uint8_t camera_data[CAMERA_DATA_BYTES];
	uint8_t unk_0x5b[165];
} ATTRIBUTE_PACKED;

struct ir_dot_t {
	uint16_t x, y;
};

/* Extensions */

#define CONTROLLER_DATA_BYTES	21

struct wiimote_extension_data_format_nunchuk_t {
	// joystick x, y
	uint8_t jx;
	uint8_t jy;
	// accelerometer
	uint8_t ax;
	uint8_t ay;
	uint8_t az;
	union {
		uint8_t hex;
		struct {
			// LSBs of accelerometer
			uint8_t acc_z_lsb : 2;
			uint8_t acc_y_lsb : 2;
			uint8_t acc_x_lsb : 2;
			uint8_t c : 1;
			uint8_t z : 1;
		};
	} bt;
};

struct wiimote_extension_data_format_classic_t {
	uint8_t rx3 : 2; // byte 0
	uint8_t lx : 6;

	uint8_t rx2 : 2; // byte 1
	uint8_t ly : 6;

	uint8_t rx1 : 1;
	uint8_t lt2 : 2;
	uint8_t ry : 5;

	uint8_t lt1 : 3;
	uint8_t rt : 5;

	union { // byte 4, 5
		uint16_t hex;
		struct {
			uint8_t dpad_right : 1;
			uint8_t dpad_down : 1;
			uint8_t lt : 1;  // left trigger
			uint8_t minus : 1;
			uint8_t home : 1;
			uint8_t plus : 1;
			uint8_t rt : 1;  // right trigger
			uint8_t : 1;

			uint8_t zl : 1;  // left z button
			uint8_t b : 1;
			uint8_t y : 1;
			uint8_t a : 1;
			uint8_t x : 1;
			uint8_t zr : 1;
			uint8_t dpad_left : 1;
			uint8_t dpad_up : 1;
		};
	} bt;
};

struct wiimote_extension_data_format_turntable_t {
	uint8_t rtt3 : 2;  // byte 0
	uint8_t sx : 6;

	uint8_t rtt2 : 2;  // byte 1
	uint8_t sy : 6;

	uint8_t rtt1 : 1;
	uint8_t effects1 : 2;
	uint8_t crossfade : 4;
	uint8_t rtt5 : 1;

	uint8_t effects2 : 3;
	uint8_t ltt1 : 5;

	union {  // byte 4, 5
		uint16_t hex;
		struct {
			uint8_t : 1;
			uint8_t : 1;
			uint8_t left_red : 1;
			uint8_t minus : 1;
			uint8_t : 1;
			uint8_t plus : 1;
			uint8_t right_red : 1;
			uint8_t ltt5 : 1;

			uint8_t left_blue : 1;
			uint8_t  : 1;
			uint8_t right_green : 1;
			uint8_t euphoria : 1;
			uint8_t left_green : 1;
			uint8_t right_blue : 1;
			uint8_t : 1;
			uint8_t : 1;
		};
	} bt;
};

struct wiimote_extension_data_format_guitar_t {
	uint8_t : 2;  // byte 0
	uint8_t sx : 6;

	uint8_t : 2; 
	uint8_t sy : 6; // byte 1

	uint8_t : 3;	// byte 2
	uint8_t tb : 5; 

	uint8_t : 3;	   // byte 3
	uint8_t wb : 5;

	union {  // byte 4, 5
		uint16_t hex;
		struct {
			uint8_t : 1;
			uint8_t strum_down : 1;
			uint8_t : 1;
			uint8_t minus : 1;
			uint8_t : 1;
			uint8_t plus : 1;
			uint8_t : 1;
			uint8_t : 1;

			uint8_t orange : 1;
			uint8_t red : 1;
			uint8_t blue : 1;
			uint8_t green : 1;
			uint8_t yellow : 1;
			uint8_t : 1;
			uint8_t : 1;
			uint8_t strum_up : 1;
		};
	} bt;
};

struct wiimote_extension_data_format_drum_t {
	bool : 1;  // byte 0
	bool : 1;
	uint8_t sx : 6;

	bool : 1;  // byte 1
	bool : 1;
	uint8_t sy : 6;

	bool hhp : 1;  // byte 2
	bool has_velocity : 1;
	uint8_t velocity_type : 5;
	uint8_t : 1;

	uint8_t velocity : 3;  // byte 3
	uint8_t extra : 4; // needs to be set to 0b0110
	uint8_t : 1;

	union {  // byte 4, 5
		uint16_t hex;
		struct {
			uint8_t : 3;
			uint8_t minus : 1;
			uint8_t : 1;
			uint8_t plus : 1;
			uint8_t : 2;

			uint8_t orange : 1;
			uint8_t red : 1;
			uint8_t blue : 1;
			uint8_t green : 1;
			uint8_t yellow : 1;
			uint8_t kick : 1;
			uint8_t : 2;
		};
	} bt;
};

union wiimote_extension_data_t {
	struct wiimote_extension_data_format_nunchuk_t nunchuk;
	struct wiimote_extension_data_format_classic_t classic;
	struct wiimote_extension_data_format_guitar_t guitar;
	struct wiimote_extension_data_format_drum_t drum;
	struct wiimote_extension_data_format_turntable_t turntable;
};

#define ENCRYPTION_ENABLED 0xaa

struct wiimote_extension_registers_t {
	// 21 bytes of possible extension data
	uint8_t controller_data[CONTROLLER_DATA_BYTES];
	uint8_t unknown2[11];
	// address 0x20
	uint8_t calibration1[0x10];
	uint8_t calibration2[0x10];
	// address 0x40
	uint8_t encryption_key_data[0x10];
	uint8_t unknown4[0xA0];
	// address 0xF0
	uint8_t encryption;
	uint8_t unknown5[0x9];
	// address 0xFA
	uint8_t identifier[6];
} ATTRIBUTE_PACKED;

#define ENCRYPTION_KEY_DATA_BEGIN \
	offsetof(struct wiimote_extension_registers_t, encryption_key_data)

#define ENCRYPTION_KEY_DATA_END \
	(ENCRYPTION_KEY_DATA_BEGIN + \
	 MEMBER_SIZE(struct wiimote_extension_registers_t, encryption_key_data))

/* Extension IDs */

static const uint8_t EXT_ID_CODE_NUNCHUNK[6] 		= {0x00, 0x00, 0xa4, 0x20, 0x00, 0x00};
static const uint8_t EXP_ID_CODE_CLASSIC_CONTROLLER[6]	= {0x00, 0x00, 0xa4, 0x20, 0x01, 0x01};
static const uint8_t EXP_ID_CODE_CLASSIC_WIIU_PRO[6]		= {0x00, 0x00, 0xa4, 0x20, 0x01, 0x20};
static const uint8_t EXP_ID_CODE_GUITAR[6]			= {0x00, 0x00, 0xa4, 0x20, 0x01, 0x03};
static const uint8_t EXP_ID_CODE_DRUM[6] = {0x01, 0x00, 0xa4, 0x20, 0x01, 0x03};
static const uint8_t EXP_ID_CODE_TURNTABLE[6] = {0x03, 0x00, 0xa4, 0x20, 0x01, 0x03};
static const uint8_t EXP_ID_CODE_MOTION_PLUS[6]		= {0x00, 0x00, 0xA6, 0x20, 0x00, 0x05};

/* EEPROM */
union wiimote_usable_eeprom_data_t {
	struct {
		// addr: 0x0000
		uint8_t ir_calibration_1[11];
		uint8_t ir_calibration_2[11];
		uint8_t accel_calibration_1[10];
		uint8_t accel_calibration_2[10];
		// addr: 0x002A
		uint8_t user_data[0x0FA0];
		// addr: 0x0FCA
		uint8_t mii_data_1[0x02f0];
		uint8_t mii_data_2[0x02f0];
		// addr: 0x15AA
		uint8_t unk_1[0x0126];
		// addr: 0x16D0
		uint8_t unk_2[24];
		uint8_t unk_3[24];
	};
	uint8_t data[EEPROM_FREE_SIZE];
};

/* Helper inline functions */

static inline bool input_report_has_btn(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_EXP21:
		return false;
	default:
		return true;
	}
}

static inline uint8_t input_report_acc_size(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_BTN_ACC:
	case INPUT_REPORT_ID_BTN_ACC_IR:
	case INPUT_REPORT_ID_BTN_ACC_EXP:
	case INPUT_REPORT_ID_BTN_ACC_IR_EXP:
		return 3;
	default:
		return 0;
	}
}

static inline uint8_t input_report_acc_offset(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_BTN_ACC:
	case INPUT_REPORT_ID_BTN_ACC_IR:
	case INPUT_REPORT_ID_BTN_ACC_EXP:
	case INPUT_REPORT_ID_BTN_ACC_IR_EXP:
		return 2;
	default:
		return 0;
	}
}

static inline uint8_t input_report_ext_size(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_BTN_EXP8:
		return 8;
	case INPUT_REPORT_ID_BTN_EXP19:
		return 19;
	case INPUT_REPORT_ID_BTN_ACC_EXP:
		return 16;
	case INPUT_REPORT_ID_BTN_IR_EXP:
		return 9;
	case INPUT_REPORT_ID_BTN_ACC_IR_EXP:
		return 6;
	case INPUT_REPORT_ID_EXP21:
		return 21;
	default:
		return 0;
	}
}

static inline uint8_t input_report_ext_offset(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_BTN_EXP8:
	case INPUT_REPORT_ID_BTN_EXP19:
		return 2;
	case INPUT_REPORT_ID_BTN_ACC_EXP:
		return 5;
	case INPUT_REPORT_ID_BTN_IR_EXP:
		return 12;
	case INPUT_REPORT_ID_BTN_ACC_IR_EXP:
		return 15;
	case INPUT_REPORT_ID_EXP21:
	default:
		return 0;
	}
}

static inline uint8_t input_report_ir_size(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_BTN_ACC_IR:
		return 12;
	case INPUT_REPORT_ID_BTN_IR_EXP:
	case INPUT_REPORT_ID_BTN_ACC_IR_EXP:
		return 10;
	default:
		return 0;
	}
}

static inline uint8_t input_report_ir_offset(uint8_t rpt_id)
{
	switch (rpt_id) {
	case INPUT_REPORT_ID_BTN_ACC_IR:
	case INPUT_REPORT_ID_BTN_ACC_IR_EXP:
		return 5;
	case INPUT_REPORT_ID_BTN_IR_EXP:
		return 2;
	default:
		return 0;
	}
}

#endif
