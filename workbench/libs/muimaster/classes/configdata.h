/*
    Copyright � 2002, The AROS Development Team. 
    All rights reserved.
    
    $Id$
*/

#ifndef _MUI_CLASSES_CONFIGDATA_H
#define _MUI_CLASSES_CONFIGDATA_H

#define MUIC_Configdata "Configdata.mui"

/* The config items for MUIM_GetConfigItem */
#define MUICFG_Invalid                (-1L)
#define MUICFG_Frame_Drag             0x18
#define MUICFG_Font_Normal            0x1e
#define MUICFG_Font_List              0x1f
#define MUICFG_Font_Tiny              0x20
#define MUICFG_Font_Fixed             0x21
#define MUICFG_Font_Title             0x22
#define MUICFG_Font_Big	              0x23
#define MUICFG_PublicScreen           0x24
#define MUICFG_Frame_Button           0x2b
#define MUICFG_Frame_ImageButton      0x2c
#define MUICFG_Frame_Text             0x2d
#define MUICFG_Frame_String           0x2e
#define MUICFG_Frame_ReadList         0x2f
#define MUICFG_Frame_InputList        0x30
#define MUICFG_Frame_Prop             0x31
#define MUICFG_Frame_Gauge            0x32
#define MUICFG_Frame_Group            0x33
#define MUICFG_Frame_PopUp            0x34
#define MUICFG_Frame_Virtual          0x35
#define MUICFG_Frame_Slider           0x36
#define MUICFG_Background_Window      0x37
#define MUICFG_Background_Requester   0x38
#define MUICFG_Background_Button      0x39
#define MUICFG_Background_List        0x3a
#define MUICFG_Background_Text        0x3b
#define MUICFG_Background_Prop        0x3c
#define MUICFG_Background_PopUp       0x3d
#define MUICFG_Background_Selected    0x3e
#define MUICFG_Background_ListCursor  0x3f
#define MUICFG_Background_ListSelect  0x40
#define MUICFG_Background_ListSelCur  0x41
#define MUICFG_Image_ArrowUp          0x42
#define MUICFG_Image_ArrowDown        0x43
#define MUICFG_Image_ArrowLeft        0x44
#define MUICFG_Image_ArrowRight       0x45
#define MUICFG_Image_CheckMark        0x46
#define MUICFG_Image_RadioButton      0x47
#define MUICFG_Image_Cycle            0x48
#define MUICFG_Image_PopUp            0x49
#define MUICFG_Image_PopFile          0x4a
#define MUICFG_Image_PopDrawer        0x4b
#define MUICFG_Image_PropKnob         0x4c
#define MUICFG_Image_Drawer           0x4d
#define MUICFG_Image_HardDisk         0x4e
#define MUICFG_Image_Disk             0x4f
#define MUICFG_Image_Chip             0x50
#define MUICFG_Image_Volume           0x51
#define MUICFG_Image_Network          0x52
#define MUICFG_Image_Assign           0x53
#define MUICFG_Background_Register    0x54
#define MUICFG_Image_TapePlay         0x55
#define MUICFG_Image_TapePlayBack     0x56
#define MUICFG_Image_TapePause        0x57
#define MUICFG_Image_TapeStop         0x58
#define MUICFG_Image_TapeRecord       0x59
#define MUICFG_Background_Framed      0x5a
#define MUICFG_Background_Slider      0x5b
#define MUICFG_Background_SliderKnob  0x5c
#define MUICFG_Image_TapeUp           0x5d
#define MUICFG_Image_TapeDown         0x5e
#define MUICFG_Font_Button            0x80
#define MUICFG_Font_Knob              0x88
#define MUICFG_Frame_Knob             0x90
#define MUICFG_Background_Page        0x95
#define MUICFG_Background_ReadList    0x96

#define MUIA_Configdata_Application (TAG_USER|0x10203453) /* ZV1: PRIV i.g  Object * */
#define MUIA_Configdata_ZunePrefs   (TAG_USER|0x10203454) /* ZV1: PRIV .g.  struct ZunePrefsNew * */

extern const struct __MUIBuiltinClass _MUI_Configdata_desc; /* PRIV */

#endif
