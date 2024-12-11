#pragma once
#define IOS_ALIGN __attribute__((aligned(32)))
#define USB_INPUT_DEVICE_PRIVATE_DATA_SIZE 64
#ifndef ATTRIBUTE_ALIGN
# define ATTRIBUTE_ALIGN(v)	__attribute__((aligned(v)))
#endif
#ifndef ATTRIBUTE_PACKED
# define ATTRIBUTE_PACKED	__attribute__((packed))
#endif
#define ARRAY_SIZE(_arr)   ( sizeof(_arr) / sizeof(_arr[0]) )
#define BIT(nr)		(1ull << (nr))

#define le16toh(x) __builtin_bswap16(x)
#define htole16(x) __builtin_bswap16(x)
#define BE16(i) ((((i) & 0xFF) << 8 | ((i) >> 8) & 0xFF) & 0xFFFF)