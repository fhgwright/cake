#ifdef USE_CLASS_CHIPSMARTCARD_COLORS
const ULONG Class_ChipSmartCard_colors[96] =
{
	0x96969696,0x96969696,0x96969696,
	0x2d2d2d2d,0x28282828,0x9e9e9e9e,
	0x00000000,0x65656565,0x9a9a9a9a,
	0x35353535,0x75757575,0xaaaaaaaa,
	0x65656565,0x8a8a8a8a,0xbabababa,
	0x0c0c0c0c,0x61616161,0xffffffff,
	0x24242424,0x5d5d5d5d,0x24242424,
	0x35353535,0x8a8a8a8a,0x35353535,
	0x86868686,0xb2b2b2b2,0x3d3d3d3d,
	0x0c0c0c0c,0xe3e3e3e3,0x00000000,
	0x4d4d4d4d,0x9e9e9e9e,0x8e8e8e8e,
	0x82828282,0x00000000,0x00000000,
	0xdfdfdfdf,0x35353535,0x35353535,
	0xdbdbdbdb,0x65656565,0x39393939,
	0xdbdbdbdb,0x8e8e8e8e,0x41414141,
	0xdfdfdfdf,0xbabababa,0x45454545,
	0xefefefef,0xe7e7e7e7,0x14141414,
	0x82828282,0x61616161,0x4d4d4d4d,
	0xa6a6a6a6,0x7e7e7e7e,0x61616161,
	0xcacacaca,0x9a9a9a9a,0x75757575,
	0x9a9a9a9a,0x55555555,0xaaaaaaaa,
	0xffffffff,0x00000000,0xffffffff,
	0xffffffff,0xffffffff,0xffffffff,
	0xdfdfdfdf,0xdfdfdfdf,0xdfdfdfdf,
	0xcacacaca,0xcacacaca,0xcacacaca,
	0xbabababa,0xbabababa,0xbabababa,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0x8a8a8a8a,0x8a8a8a8a,0x8a8a8a8a,
	0x65656565,0x65656565,0x65656565,
	0x4d4d4d4d,0x4d4d4d4d,0x4d4d4d4d,
	0x3c3c3c3c,0x3c3c3c3c,0x3b3b3b3b,
	0x00000000,0x00000000,0x00000000,
};
#endif

#define CLASS_CHIPSMARTCARD_WIDTH        16
#define CLASS_CHIPSMARTCARD_HEIGHT       16
#define CLASS_CHIPSMARTCARD_DEPTH         5
#define CLASS_CHIPSMARTCARD_COMPRESSION   1
#define CLASS_CHIPSMARTCARD_MASKING       2

#ifdef USE_CLASS_CHIPSMARTCARD_HEADER
const struct BitMapHeader Class_ChipSmartCard_header =
{ 16,16,88,45,5,2,1,0,0,1,1,800,600 };
#endif

#ifdef USE_CLASS_CHIPSMARTCARD_BODY
const UBYTE Class_ChipSmartCard_body[233] = {
0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0x01,0x06,0x80,0x01,0x07,
0xc0,0x01,0x07,0xc8,0xff,0x38,0x01,0x3f,0xf8,0x01,0x2b,0x4c,0x01,0x2f,0xe4,
0x01,0x2f,0xe8,0x01,0x10,0x1c,0x01,0x3f,0xfc,0x01,0x2e,0x84,0x01,0x2f,0xec,
0x01,0x2f,0xe8,0x01,0x10,0x1c,0x01,0x3f,0xfc,0x01,0x25,0x24,0x01,0x2f,0xec,
0x01,0x2f,0xe8,0x01,0x10,0x1c,0x01,0x3f,0xfc,0x01,0x2a,0x8c,0x01,0x2f,0xec,
0x01,0x2f,0xe8,0x01,0x10,0x1c,0x01,0x3f,0xfc,0x01,0x24,0x2c,0x01,0x2f,0xec,
0x01,0x2f,0xe8,0x01,0x10,0x1c,0x01,0x3f,0xfc,0x01,0x20,0x0c,0x01,0x20,0x0c,
0x01,0x3f,0xf8,0x01,0x1f,0xfc,0x01,0x3f,0xfc,0x01,0x2c,0x0c,0x01,0x23,0x4c,
0x01,0x20,0xb8,0x01,0x1f,0x4c,0x01,0x3f,0x4c,0x01,0x3f,0xfc,0x01,0x3f,0xfc,
0x01,0x20,0x08,0x01,0x00,0x0c,0x01,0x20,0x0c,0x01,0x3f,0xfc,0x01,0x3f,0xfc,
0x01,0x20,0x08,0x01,0x00,0x0c,0x01,0x20,0x0c,0x01,0x3f,0xfc,0x01,0x3f,0xfc,
0x01,0x20,0x08,0x01,0x00,0x0c,0x01,0x20,0x0c,0x01,0x35,0x5c,0x01,0x3a,0xbc,
0x01,0x25,0x48,0x01,0x05,0x4c,0x01,0x25,0x4c,0x01,0x1a,0xac,0x01,0x0f,0xec,
0x01,0x1f,0xf8,0x01,0x3f,0xfc,0x01,0x30,0x1c,0x01,0x0a,0xac,0x01,0x3f,0xfc,
0x01,0x1f,0xf8,0x01,0x3f,0xfc,0x01,0x30,0x1c,0x01,0x1f,0xf8,0x01,0x1f,0xf8,
0xff,0x00,0x01,0x1f,0xf8,0x01,0x1f,0xf8, };
#endif