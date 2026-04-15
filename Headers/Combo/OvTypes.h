#pragma once

#define OV_NONE         0x00
#define OV_CHEST        0x01
#define OV_COLLECTIBLE  0x02
#define OV_NPC          0x03
#define OV_GS           0x04
#define OV_SF           0x05
#define OV_COW          0x06
#define OV_SHOP         0x07
#define OV_SCRUB        0x08
#define OV_SR           0x09
#define OV_FISH         0x0a

#define OV_XFLAG0       0x10
#define OV_XFLAG1       0x11
#define OV_XFLAG2       0x12
#define OV_XFLAG3       0x13
#define OV_XFLAG4       0x14
#define OV_XFLAG5       0x15
#define OV_XFLAG6       0x16
#define OV_XFLAG7       0x17
#define OV_XFLAG8       0x18
#define OV_XFLAG9       0x19
#define OV_XFLAG10      0x1a
#define OV_XFLAG11      0x1b

#define OVF_RENEW             (1 << 2)
#define OVF_PRECOND           (1 << 3)

void CorrectComboItem(struct ComboItem* Item);