#ifdef USE_CLASS_PRINTER_COLORS
const ULONG Class_Printer_colors[96] =
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

#define CLASS_PRINTER_WIDTH        16
#define CLASS_PRINTER_HEIGHT       16
#define CLASS_PRINTER_DEPTH         5
#define CLASS_PRINTER_COMPRESSION   1
#define CLASS_PRINTER_MASKING       2

#ifdef USE_CLASS_PRINTER_HEADER
const struct BitMapHeader Class_Printer_header =
{ 16,16,88,216,5,2,1,0,0,1,1,800,600 };
#endif

#ifdef USE_CLASS_PRINTER_BODY
const UBYTE Class_Printer_body[233] = {
0x01,0x00,0x80,0x01,0x00,0xa0,0x01,0x00,0x80,0x01,0x00,0x60,0x01,0x00,0xe0,
0x01,0x01,0x20,0x01,0x01,0x68,0x01,0x01,0x60,0x01,0x00,0x18,0x01,0x01,0xf8,
0x01,0x02,0x08,0x01,0x02,0xaa,0x01,0x02,0xa8,0x01,0x00,0x06,0x01,0x03,0xfe,
0x01,0x14,0x06,0x01,0x05,0x56,0x01,0x05,0x54,0x01,0x18,0x02,0x01,0x1f,0xfe,
0x01,0x02,0x0c,0x01,0x2a,0xac,0x01,0x22,0xa8,0x01,0x1c,0x04,0x01,0x3f,0xfc,
0x01,0x12,0x99,0x01,0x44,0xd9,0x01,0x40,0xd6,0x01,0x3f,0x0f,0x01,0x67,0xff,
0x01,0xab,0x14,0x01,0x10,0xb0,0x01,0x00,0x0f,0xff,0xff,0x01,0xf7,0xff,0x01,
0x56,0x46,0x01,0x49,0xc1,0x01,0x40,0x3f,0x01,0xbf,0xff,0xff,0xff,0x01,0xb4,
0x96,0x01,0x33,0x81,0x01,0x30,0x7f,0x01,0xcf,0xff,0xff,0xff,0x01,0x8d,0x2e,
0x01,0x0d,0x01,0x01,0x0c,0xff,0x01,0xf3,0xff,0xff,0xff,0x01,0x5f,0x18,0x01,
0x83,0x07,0x01,0x02,0xff,0x01,0xfd,0xff,0xff,0xff,0x01,0xd9,0xa4,0x01,0xe7,
0x1a,0x01,0x00,0xff,0xff,0xff,0xff,0xff,0x01,0x10,0x15,0x01,0x1e,0x69,0x01,
0x61,0xfe,0x01,0x7f,0xff,0x01,0x7f,0xff,0x01,0x2d,0x54,0x01,0x26,0xa4,0x01,
0x19,0xf8,0x01,0x3f,0xfc,0x01,0x3f,0xfc,0x01,0x0b,0x50,0x01,0x08,0x90,0x01,
0x07,0xe0,0x01,0x0f,0xf0,0x01,0x0f,0xf0,0x01,0x02,0x40,0x01,0x02,0x40,0x01,
0x01,0x80,0x01,0x03,0xc0,0x01,0x03,0xc0, };
#endif