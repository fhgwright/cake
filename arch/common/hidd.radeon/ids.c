/*
    Copyright � 2006, The AROS Development Team. All rights reserved.
    $Id$

    Desc: List of ATI cards supported by this driver
    Lang: English
*/

#include "ids.h"

/* List of DeviceID's of supported nvidia cards */
const struct ATIDevice support[] = {
    { 0x1002, 0x5144, RADEON,   RADEONInit,    FALSE },
    { 0x1002, 0x5145, RADEON,   RADEONInit,    FALSE },
    { 0x1002, 0x5146, RADEON,   RADEONInit,    FALSE },
    { 0x1002, 0x5147, RADEON,   RADEONInit,    FALSE },
    { 0x1002, 0x4C57, RADEON,   RADEONInit,    FALSE },
    { 0x1002, 0x4C58, RADEON,   RADEONInit,    FALSE },
    { 0x1002, 0x4C59, RV100,    RADEONInit,    FALSE },
    { 0x1002, 0x4C5A, RV100,    RADEONInit,    FALSE },
    { 0x1002, 0x5159, RV100,    RADEONInit,    FALSE },
    { 0x1002, 0x515A, RV100,    RADEONInit,    FALSE },
    { 0x1002, 0x4136, RS100,    RADEONInit,    FALSE },
    { 0x1002, 0x4336, RS100,    RADEONInit,    FALSE },
    { 0x1002, 0x4137, RS200,    RADEONInit,    FALSE },
    { 0x1002, 0x4337, RS200,    RADEONInit,    FALSE },
    { 0x1002, 0x4237, RS200,    RADEONInit,    FALSE },
    { 0x1002, 0x4437, RS250,    RADEONInit,    FALSE },
    { 0x1002, 0x5148, R200,     RADEONInit,    FALSE },
    { 0x1002, 0x514C, R200,     RADEONInit,    FALSE },
    { 0x1002, 0x514D, R200,     RADEONInit,    FALSE },
    { 0x1002, 0x4242, R200,     RADEONInit,    FALSE },
    { 0x1002, 0x4243, R200,     RADEONInit,    FALSE },
    { 0x1002, 0x5157, RV200,    RADEONInit,    FALSE },
    { 0x1002, 0x5158, RV200,    RADEONInit,    FALSE },
    { 0x1002, 0x4966, RV250,    RADEONInit,    FALSE },
    { 0x1002, 0x4967, RV250,    RADEONInit,    FALSE },
    { 0x1002, 0x4C64, RV250,    RADEONInit,    FALSE },
    { 0x1002, 0x4C66, RV250,    RADEONInit,    FALSE },
    { 0x1002, 0x4C67, RV250,    RADEONInit,    FALSE },
    { 0x1002, 0x5834, RS300,    RADEONInit,    FALSE },
    { 0x1002, 0x5835, RS300,    RADEONInit,    FALSE },
    { 0x1002, 0x7834, RS350,    RADEONInit,    FALSE },
    { 0x1002, 0x7835, RS350,    RADEONInit,    FALSE },
    { 0x1002, 0x5960, RV280,    RADEONInit,    FALSE },
    { 0x1002, 0x5961, RV280,    RADEONInit,    FALSE },
    { 0x1002, 0x5962, RV280,    RADEONInit,    FALSE },
    { 0x1002, 0x5964, RV280,    RADEONInit,    FALSE },
    { 0x1002, 0x5C61, RV280,    RADEONInit,    FALSE },
    { 0x1002, 0x5C63, RV280,    RADEONInit,    FALSE },
    { 0x1002, 0x4144, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4145, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4146, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4147, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4E44, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4E45, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4E46, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4E47, R300,     RADEONInit,    FALSE },
    { 0x1002, 0x4150, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4151, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4152, RV360,    RADEONInit,    FALSE },
    { 0x1002, 0x4153, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4154, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4156, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4E50, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4E51, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4E52, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4E53, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4E54, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4E56, RV350,    RADEONInit,    FALSE },
    { 0x1002, 0x4148, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x4149, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x414A, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x414B, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x4E48, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x4E49, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x4E4A, R350,     RADEONInit,    FALSE },
    { 0x1002, 0x4E4B, R360,     RADEONInit,    FALSE },
    { 0x1002, 0x3E50, RV380,    RADEONInit,    FALSE },
    { 0x1002, 0x3E54, RV380,    RADEONInit,    FALSE },
    { 0x1002, 0x3150, RV380,    RADEONInit,    FALSE },
    { 0x1002, 0x3154, RV380,    RADEONInit,    FALSE },
    { 0x1002, 0x5B60, RV370,    RADEONInit,    FALSE },
    { 0x1002, 0x5B62, RV370,    RADEONInit,    FALSE },
    { 0x1002, 0x5B64, RV370,    RADEONInit,    FALSE },
    { 0x1002, 0x5B65, RV370,    RADEONInit,    FALSE },
    { 0x1002, 0x5460, RV370,    RADEONInit,    FALSE },
    { 0x1002, 0x5464, RV370,    RADEONInit,    FALSE },
    { 0x1002, 0x4A48, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A49, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A4A, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A4B, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A4C, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A4D, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A4E, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x4A50, R420,     RADEONInit,    FALSE },
    { 0x1002, 0x5548, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x5549, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x554A, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x554B, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x5551, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x5552, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x5554, R423,     RADEONInit,    FALSE },
    { 0x1002, 0x5D57, R423,     RADEONInit,    FALSE } ,

    { 0x0000, 0x0000, }
};