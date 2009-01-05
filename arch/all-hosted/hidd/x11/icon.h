/*  GIMP header image file format (INDEXED)  */

static unsigned int width = 40;
static unsigned int height = 40;

/*  Call this macro repeatedly.  After each use, the pixel data can be extracted  */

#define HEADER_PIXEL(data,pixel) {\
  pixel[0] = header_data_cmap[(unsigned char)data[0]][0]; \
  pixel[1] = header_data_cmap[(unsigned char)data[0]][1]; \
  pixel[2] = header_data_cmap[(unsigned char)data[0]][2]; \
  data ++; }

static char header_data_cmap[256][3] = {
	{ 22, 26, 28},
	{ 19, 29, 29},
	{ 21, 28, 33},
	{ 17, 30, 35},
	{ 24, 28, 30},
	{ 22, 29, 35},
	{ 18, 31, 36},
	{ 23, 29, 35},
	{ 25, 29, 31},
	{ 19, 33, 38},
	{ 27, 31, 33},
	{ 22, 35, 40},
	{ 20, 38, 47},
	{ 24, 37, 42},
	{ 31, 35, 37},
	{ 21, 39, 48},
	{ 33, 36, 38},
	{ 22, 40, 49},
	{ 33, 37, 39},
	{ 24, 41, 51},
	{ 34, 38, 40},
	{ 20, 43, 57},
	{ 24, 42, 52},
	{ 26, 44, 53},
	{ 23, 46, 60},
	{ 22, 48, 57},
	{ 25, 47, 61},
	{ 26, 48, 62},
	{ 40, 44, 46},
	{ 26, 49, 63},
	{ 22, 51, 70},
	{ 26, 52, 60},
	{ 44, 48, 50},
	{ 26, 54, 73},
	{ 27, 55, 74},
	{ 30, 56, 65},
	{ 47, 51, 53},
	{ 21, 60, 77},
	{ 49, 53, 55},
	{ 25, 60, 84},
	{ 30, 61, 74},
	{ 33, 61, 80},
	{ 52, 56, 58},
	{ 28, 63, 87},
	{ 29, 66, 84},
	{ 54, 58, 61},
	{ 26, 68, 97},
	{ 23, 72, 89},
	{ 26, 71, 94},
	{ 37, 68, 81},
	{ 31, 72,101},
	{ 60, 65, 67},
	{ 34, 75, 87},
	{ 25, 76,110},
	{ 32, 76, 99},
	{ 34, 75,104},
	{ 32, 79, 96},
	{ 27, 80,108},
	{ 21, 85,118},
	{ 30, 85,107},
	{ 68, 72, 74},
	{ 26, 85,124},
	{ 43, 83, 95},
	{ 35, 83,118},
	{ 69, 74, 76},
	{ 47, 83,101},
	{ 38, 85,120},
	{ 72, 76, 78},
	{ 31, 88,128},
	{ 74, 78, 80},
	{ 76, 80, 82},
	{ 27, 95,140},
	{ 17,102,146},
	{ 38, 96,130},
	{ 42, 98,115},
	{ 32, 98,143},
	{ 31,100,139},
	{ 55, 94,107},
	{ 46, 98,121},
	{  9,110,160},
	{ 35,100,145},
	{ 28,107,127},
	{ 82, 87, 89},
	{ 53, 98,116},
	{ 26,106,151},
	{ 34,108,134},
	{ 16,112,162},
	{ 88, 92, 94},
	{ 32,110,155},
	{ 44,110,126},
	{ 38,111,137},
	{ 89, 93, 95},
	{ 23,115,165},
	{ 93, 97,100},
	{ 29,118,168},
	{ 31,119,169},
	{ 33,120,170},
	{ 51,120,129},
	{ 39,123,149},
	{ 44,123,155},
	{ 48,125,140},
	{ 26,129,179},
	{ 39,126,170},
	{ 45,128,154},
	{103,107,110},
	{ 27,135,172},
	{ 33,132,182},
	{ 18,140,189},
	{ 35,133,183},
	{ 47,133,152},
	{ 86,117,132},
	{ 21,141,190},
	{ 38,135,185},
	{ 72,126,143},
	{ 51,136,155},
	{ 40,136,186},
	{112,116,119},
	{ 30,144,193},
	{ 47,141,166},
	{ 87,126,140},
	{ 97,124,133},
	{ 71,135,151},
	{ 38,149,172},
	{115,120,122},
	{ 44,145,182},
	{ 35,147,196},
	{ 47,143,187},
	{ 35,150,192},
	{118,123,125},
	{ 22,158,200},
	{ 38,151,194},
	{ 51,147,191},
	{ 63,146,166},
	{ 40,153,195},
	{121,126,128},
	{ 43,154,197},
	{123,128,131},
	{ 59,154,173},
	{ 47,156,199},
	{125,130,132},
	{ 35,167,202},
	{128,133,136},
	{ 37,168,203},
	{ 51,162,199},
	{131,136,139},
	{ 40,170,205},
	{ 52,167,196},
	{ 43,171,206},
	{134,139,142},
	{ 45,172,208},
	{ 30,179,214},
	{ 54,172,195},
	{124,147,151},
	{138,143,146},
	{107,155,167},
	{ 73,170,182},
	{ 52,176,212},
	{133,148,154},
	{ 52,181,209},
	{ 54,182,210},
	{136,151,157},
	{ 65,180,204},
	{ 57,183,212},
	{ 73,178,196},
	{ 83,175,194},
	{146,151,153},
	{140,155,162},
	{ 47,192,219},
	{137,156,167},
	{ 61,187,215},
	{ 72,185,209},
	{ 64,188,217},
	{ 51,194,221},
	{ 65,189,218},
	{ 53,195,223},
	{120,169,181},
	{ 55,196,224},
	{154,159,162},
	{ 57,198,225},
	{ 60,199,227},
	{ 73,195,224},
	{160,165,168},
	{ 74,200,222},
	{139,175,183},
	{ 75,201,223},
	{163,168,171},
	{ 77,202,224},
	{ 79,204,226},
	{150,177,187},
	{ 70,211,231},
	{ 81,206,228},
	{ 73,212,233},
	{170,175,177},
	{154,181,192},
	{ 74,214,234},
	{ 86,210,232},
	{164,180,186},
	{ 88,216,231},
	{178,183,186},
	{181,186,189},
	{108,221,238},
	{130,213,227},
	{110,223,240},
	{190,193,189},
	{111,224,241},
	{174,202,212},
	{191,196,198},
	{121,226,237},
	{197,200,196},
	{195,200,203},
	{200,202,199},
	{136,230,243},
	{201,206,208},
	{139,233,245},
	{206,208,204},
	{146,233,240},
	{166,225,236},
	{149,232,246},
	{207,212,215},
	{156,234,242},
	{213,215,212},
	{168,236,246},
	{213,218,220},
	{216,218,214},
	{169,238,247},
	{171,239,249},
	{221,223,220},
	{221,227,229},
	{188,242,248},
	{190,244,250},
	{228,230,227},
	{198,244,251},
	{199,245,252},
	{229,234,237},
	{232,234,231},
	{204,246,247},
	{235,238,234},
	{213,247,250},
	{216,249,252},
	{239,241,238},
	{223,248,253},
	{241,243,240},
	{225,249,254},
	{226,251,255},
	{230,251,250},
	{247,249,246},
	{239,253,253},
	{248,250,247},
	{241,254,255},
	{250,252,249},
	{251,253,250},
	{248,254,255},
	{249,255,255},
	{252,255,251},
	{254,255,252},
	{255,255,255}
	};
static char header_data[] = {
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,250,245,245,245,245,249,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,241,209,177,134,93,60,36,
	16,0,0,0,0,0,0,16,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,247,206,139,60,8,0,0,0,0,0,0,
	0,0,0,0,0,0,0,16,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,214,123,28,0,1,6,11,17,22,21,21,21,23,
	22,17,12,9,2,0,0,36,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	250,198,69,0,0,3,15,24,26,27,27,27,27,27,27,27,
	27,27,27,26,3,0,0,60,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,208,
	64,0,0,9,22,6,24,30,77,119,157,160,166,168,160,152,
	110,77,30,26,7,0,0,93,
	254,254,254,254,254,254,254,254,254,254,254,254,254,239,116,4,
	0,3,19,26,49,120,205,246,251,251,251,251,251,251,251,251,
	251,251,188,26,3,0,0,134,
	254,254,254,254,254,254,254,254,254,254,254,254,210,38,0,0,
	11,24,65,193,244,242,242,246,246,246,246,246,246,246,248,248,
	248,196,27,26,1,0,0,181,
	254,254,254,254,254,254,254,254,254,254,254,177,10,0,0,12,
	40,175,238,242,238,228,229,242,242,242,242,242,242,242,242,244,
	183,27,27,24,0,0,0,214,
	254,254,254,254,254,254,254,254,254,254,139,0,0,2,15,83,
	216,228,228,228,228,211,211,231,231,231,232,232,232,235,237,154,
	26,27,27,21,0,0,16,245,
	254,254,254,254,254,254,254,254,254,128,0,0,0,12,121,213,
	213,213,217,219,215,191,195,219,224,224,224,224,224,224,113,33,
	34,34,33,17,0,0,82,254,
	254,254,254,254,254,254,254,254,141,0,0,0,17,137,202,200,
	202,202,204,207,202,158,159,200,211,211,213,213,201,78,39,43,
	43,43,43,12,0,0,165,254,
	254,254,254,254,254,254,254,181,0,0,0,15,137,191,191,191,
	197,191,197,197,184,117,131,180,200,200,202,164,57,50,50,50,
	50,50,46,7,0,4,226,254,
	254,254,254,254,254,254,212,10,0,0,11,118,178,178,182,184,
	186,186,187,190,146,84,84,146,191,182,99,63,61,66,61,68,
	63,66,41,0,0,70,254,254,
	254,254,254,254,254,239,42,0,0,3,90,171,171,171,171,173,
	167,172,172,178,99,55,53,99,124,75,75,75,80,72,80,80,
	80,71,17,0,0,192,254,254,
	254,254,254,254,254,116,0,0,0,39,88,124,147,150,150,158,
	158,159,159,159,54,26,12,33,86,92,92,94,95,95,95,96,
	96,58,0,0,42,247,254,254,
	254,254,254,254,208,4,0,0,17,79,95,94,94,101,126,127,
	127,133,127,102,2,0,0,33,101,108,106,108,112,112,112,112,
	101,17,0,0,185,254,254,254,
	254,254,254,249,60,0,0,0,76,112,112,112,112,115,107,111,
	107,111,111,73,0,0,0,37,117,127,125,127,133,133,133,127,
	48,0,0,67,247,254,254,254,
	254,254,254,192,0,0,0,37,125,133,133,133,133,138,138,138,
	129,129,129,48,0,0,0,44,143,140,140,142,142,142,142,85,
	0,0,16,223,254,254,254,254,
	254,254,249,51,0,0,1,105,140,140,142,142,145,145,147,149,
	147,149,142,11,0,0,0,59,150,150,158,158,150,158,98,3,
	0,4,181,254,254,254,254,254,
	254,254,198,0,0,0,47,156,158,158,150,158,159,159,159,159,
	159,159,103,0,0,0,0,81,171,171,171,173,171,98,3,0,
	0,134,254,254,254,254,254,254,
	254,233,45,0,0,0,56,171,173,167,167,167,172,172,172,172,
	172,172,52,0,0,0,0,109,182,182,178,172,74,0,0,0,
	123,249,254,254,254,254,254,254,
	226,51,0,0,0,0,7,178,182,182,182,184,184,186,186,186,
	186,161,2,0,0,0,0,151,190,190,122,35,0,0,4,148,
	250,254,254,254,254,254,254,254,
	177,104,91,10,0,0,0,114,170,191,191,191,195,191,195,191,
	191,89,0,0,0,0,12,170,100,31,0,0,0,32,198,254,
	254,254,254,254,254,254,254,254,
	254,254,254,234,165,51,0,0,0,25,62,97,132,155,163,170,
	161,13,0,0,0,0,1,0,0,0,0,10,128,236,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,223,153,67,8,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,28,134,227,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,239,199,144,87,42,4,0,
	0,0,0,0,0,16,69,139,203,247,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,247,230,
	220,218,218,222,236,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,254,
	254,254,254,254,254,254,254,254
	};