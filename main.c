/*
** Jo Sega Saturn Engine
** Copyright (c) 2012-2017, Johannes Fetz (johannesfetz@gmail.com)
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Johannes Fetz nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL Johannes Fetz BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <jo/jo.h>
#include "vdp2_cram_data.h"

typedef struct _RESOLUTIONS {
    unsigned int width;
    unsigned int height;
    unsigned int flags;

    // flag is really a union:
    // - lowest bit is passed to bios_set_clock_speed()

} RESOLUTIONS, *PRESOLUTIONS;

// global array at 0x060057a4
RESOLUTIONS g_Resolutions[] = {
    { 320, 224, 0x00000000, },
    { 320, 240, 0x00000010, },
    { 320, 256, 0x00000020, },
    { 320, 448, 0x000000c0, },
    { 320, 480, 0x000000d0, },
    { 320, 512, 0x000000e0, },
    { 640, 224, 0x00000002, },
    { 640, 240, 0x00000012, },
    { 640, 256, 0x00000022, },
    { 640, 448, 0x000000c2, },
    { 640, 480, 0x000000d2, },
    { 640, 512, 0x000000e2, },
    { 352, 224, 0x00000001, },
    { 352, 240, 0x00000011, },
    { 352, 256, 0x00000021, },
    { 352, 448, 0x000000c1, },
    { 352, 480, 0x000000d1, },
    { 352, 512, 0x000000e1, },
    { 704, 224, 0x00000003, },
    { 704, 240, 0x00000013, },
    { 704, 256, 0x00000023, },
    { 704, 448, 0x000000c3, },
    { 704, 480, 0x000000d3, },
    { 704, 512, 0x000000e3, },
    { 320, 480, 0x00000004, }, // SDTV
    { 640, 480, 0x00000006, }, // EDTV (VGA), progressive SCAN
    { 352, 480, 0x00000005, }, // NULL??
    { 704, 480, 0x00000007, },
    { 0xffffffff, 0xffffffff, 0xffffffff, } // end marker
};

// stdlib function prototypes to keep compiler happy
void* memcpy(void *dst, const void *src, unsigned int len);
void *memset(void *s, int c, unsigned int n);

volatile unsigned short* VDP2_D0R = (unsigned short*)0x25F80000;
volatile unsigned short* VDP2_D0W = (unsigned short*)0x25F80004;
volatile unsigned short* VDP2_D1R = (unsigned short*)0x25f80020;
volatile unsigned short* VDP2_D1C = (unsigned short*)0x25f80028;
volatile unsigned short* VDP2_D1AD = (unsigned short*)0x25f8002C;

volatile unsigned short* VDP2_D1EN = (unsigned short*)0x25f80030;
volatile unsigned short* VDP2_D1MD = (unsigned short*)0x25f80034;
volatile unsigned short* VDP2_D2R = (unsigned short*)0x25f80040;
volatile unsigned short* VDP2_D2W = (unsigned short*)0x25f80044;
volatile unsigned short* VDP2_D2C = (unsigned short*)0x25f80048;
volatile unsigned short* VDP2_D2AD = (unsigned short*)0x25f8004C;

volatile unsigned short* VDP2_D2EN = (unsigned short*)0x25f80050;
volatile unsigned short* VDP2_D2MD = (unsigned short*)0x25f80054;
volatile unsigned short* VDP2_DSTP = (unsigned short*)0x25f80060;

volatile unsigned short* VDP2_PPAF = (unsigned short*)0x25f80080;
volatile unsigned short* VDP2_PPD = (unsigned short*)0x25f80084;


#define VDP2_VRAM_START 0x25e00000
#define VDP2_VRAM_SIZE  0x40000
#define VDP2_CRAM_START 0x25f00000
#define VDP2_CRAM_SIZE  0x800

// function 0x06004dd0
void initVDP2()
{
    //
    // set VDP2 registers
    //
    *VDP2_D0R = 0;

    // BUGBUG what's this VDP reg?
    *(unsigned short*)(0x25f8000e) = *(unsigned short*)(0x25f8000e) & 0xCFFF;

    // BUGBUG what's this VDP2 reg?
    *(unsigned short*)0x25f8003c = 0;

    *VDP2_D1C = 0x1e;

    // BUGBUG what's this VDP2 reg?
    *(unsigned short*)0x25f80070 = 0;
    *(unsigned short*)0x25f80072 = 0;
    *(unsigned short*)0x25f80074 = 0;

    *VDP2_D1R = 1;

    //
    // zero VDP2 VRAM and CRAM cache
    //
    memset((void*)VDP2_VRAM_START, 0, VDP2_VRAM_SIZE);
    memset((void*)VDP2_CRAM_START, 0, VDP2_CRAM_SIZE);
}

// function 0x06004f70
void copyDataToVDP2CRAMCache()
{
    memcpy((void*)VDP2_CRAM_START, (void*)CRAM_DATA, sizeof(CRAM_DATA));

    // BUGBUG:
    // set a pointer in the start of CRAM???
    // this decompilation doesn't make much sense
    *(unsigned short*)VDP2_CRAM_START = *(unsigned short*)(CRAM_DATA + 1);
    *(unsigned short*)(VDP2_CRAM_START + 1) = 0x4000;
}

volatile unsigned char* SMPC_DDR1 = (void*)0x20100079;
volatile unsigned char* SMPC_DDR2 = (void*)0x2010007b;
volatile unsigned char* SMPC_IOSEL1 = (void*)0x2010007d;
volatile unsigned char* SMPC_EXLE1 = (void*)0x2010007f;

// function 0x06005280
void initSMPC()
{
    *SMPC_DDR1 = 0x60;
    *SMPC_DDR2 = 0x60;
    *SMPC_IOSEL1 = 3;
    *SMPC_EXLE1 = 0;


    return;
}

volatile unsigned short* VDP1_TVMR = (unsigned short*)0x25d00000;
volatile unsigned short* VDP1_FBCR = (unsigned short*)0x25d00002;
volatile unsigned short* VDP1_PTMR = (unsigned short*)0x25d00004;
volatile unsigned short* VDP1_EWDR = (unsigned short*)0x25d00006;
volatile unsigned short* VDP1_EWLR = (unsigned short*)0x25d00008;
volatile unsigned short* VDP1_EWRR = (unsigned short*)0x25d0000a;

void initVDP1VRAMCache();
void setSomethingVDP1VRAMCache();

// function 0x06004b00
void initVDP1()
{
    unsigned short pattern = 0x0101;

    // BUGBUG: what are these registers
    *(unsigned short*)0x25f80070 = pattern;
    *(unsigned short*)0x25f80072 = pattern;
    *(unsigned short*)0x25f80074 = pattern;
    *(unsigned short*)0x25f80076 = pattern;

    *VDP1_TVMR = 0;
    *VDP1_FBCR = 0;
    *VDP1_PTMR = 2;
    *VDP1_EWDR = 0x9CE7;
    *VDP1_EWLR = 0;
    *VDP1_EWRR = 0x7FFF;

    initVDP1VRAMCache();
    setSomethingVDP1VRAMCache();
}

#define VDP1_VRAM_START 0x25c00000
#define VDP1_VRAM_SIZE  0x20

unsigned short g_VDP1_Global = 0;
unsigned int g_currentIndex = 0;

// function 0x06004aa0
void initVDP1VRAMCache()
{
    memset((void*)VDP1_VRAM_START, 0xFF, VDP1_VRAM_SIZE);

    *(unsigned short*)VDP1_VRAM_START = 0x4000;

    // BUGBUG: some global is set to 1 here?
    g_VDP1_Global = 1;
}

// function 06004ad0
void setSomethingVDP1VRAMCache()
{
    *(unsigned short*)((unsigned char*)(VDP1_VRAM_START) + (g_VDP1_Global & 0xffff) * 20) = 0x8000;
    g_VDP1_Global++;
}

void my_draw(void)
{
    jo_printf(2, 2, "test");
    jo_printf(2, 2, "hello world");
}

// function 06005060
void printfWrapper(int x,int y, unsigned int flag,char *msg)
{
  int tempX;
  int i;

  if (*msg == '\0')
  {
    return;
  }

  i = 0;
  do {
    tempX = x + i;
    i = i + 1;
    //printfWrapper2(tempX, y, flag & 0xff, msg[i-1]);
  } while (msg[i] != '\0');
  return;
}

// function 0x06004ef0
int pollVDP2_D0W_8(void)
{
  do {

  } while ((*VDP2_D0W & 8) == 0);

  return 0;
}


// function 0x06004ef0
unsigned int pollVDP2_D0W_8_2(void)
{
    do {

    } while (((int)*VDP2_D0W & 8U) == 0);

    return (int)*VDP2_D0W & 8U;
}

// function 0x06004eb0
unsigned int pollVDP2_D0W_4(void)
{
  do {
  } while (((int)*VDP2_D0W  & 4U) == 0);
  return (int)*VDP2_D0W  & 4U;
}

// function 0x06004b60
void setVDP1VRAM(unsigned short a, unsigned short b)
{
    unsigned short* vdp1_vram = ((unsigned char*)VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 9;
    vdp1_vram[1] = 0;
    vdp1_vram[10] = a;
    vdp1_vram[0xb] = b;

    g_VDP1_Global++;
}

// function 0x06004ba0
void setVDP1VRAM_2(unsigned short a, unsigned short b, unsigned short c, unsigned short d)
{
    unsigned short* vdp1_vram = ((unsigned char*)VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 8;
    vdp1_vram[1] = 0;
    vdp1_vram[6] = a;
    vdp1_vram[7] = b;
    vdp1_vram[10] = c;
    vdp1_vram[0xb] = d;

    g_VDP1_Global++;
}

// function 0x06004be0
void setVDP1VRAM_3(unsigned short a, unsigned short b)
{
    unsigned short* vdp1_vram = ((unsigned char*)VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 10;
    vdp1_vram[1] = 0;
    vdp1_vram[6] = a;
    vdp1_vram[7] = b;

    g_VDP1_Global++;
}

// function 0x06004c20
void setVDP1VRAM_4(unsigned short param_1,unsigned short param_2,unsigned short param_3,unsigned short param_4,unsigned short param_5, unsigned short param_6,unsigned short param_7,unsigned short param_8,unsigned short param_9)
{
    unsigned short* vdp1_vram = (VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[1] = 5;

    // BUGBUG: decompilation says this value is taken from a global:
    // but it always appears to be 0 so I'm leaving it out
    //vdp1_vram[2] = *(unsigned short *)PTR_DAT_06004c98 | 0xc0;
    vdp1_vram[2] = 0xc0;

    vdp1_vram[3] = param_9;
    vdp1_vram[6] = param_1;
    vdp1_vram[7] = param_2;
    vdp1_vram[8] = param_3;
    vdp1_vram[9] = param_4;
    vdp1_vram[10] = param_5;
    vdp1_vram[0xb] = param_6;
    vdp1_vram[0xc] = param_7;
    vdp1_vram[0xd] = param_8;

    g_VDP1_Global++;
}

// function 0x06004ad0
void setVDP1VRAM_5()
{
    unsigned short* vdp1_vram = (VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 0x8000;

    g_VDP1_Global++;
}

volatile unsigned char* SMPC_PDR1 = (void*)0x20100075;

// function 0x060052b0
void readSMPC(unsigned int unused, unsigned short* outData)
{
    *SMPC_PDR1 = 0x60;
    unsigned char val1, val2, val3;
    unsigned short val4;
    unsigned short val5;

    // BUGBUG: won't the compiler just optimize this out??
    for(int i = 0x10; i > 0; i--)
    {

    }

    val1 = *SMPC_PDR1;
    *SMPC_PDR1= 0x40;

    // BUGBUG: won't the compiler just optimize this out??
    for(int i = 0x10; i > 0; i--)
    {

    }

    val2 = *SMPC_PDR1;
    *SMPC_PDR1= 0x20;

    // BUGBUG: won't the compiler just optimize this out??
    for(int i = 0x10; i > 0; i--)
    {

    }

    val3 = *SMPC_PDR1;
    *SMPC_PDR1= 0x0;

    // BUGBUG: won't the compiler just optimize this out??
    for(int i = 0x10; i > 0; i--)
    {

    }

    val5 = ((*SMPC_PDR1 & 0xF) |
            (val3 & 0xf) << 4 |
            (val2 & 0xf) << 8 |
            ((val1 & 0x8 << 0xc))) ^ 0x8FFF;

    val4 = *outData;

    outData[0] = val5;
    outData[1] = val4;
    outData[2] = val5 & (val5 ^ val4);

    return;
}

// function 0x6004100
unsigned int writeTOVDP2VRAM(int x, int y, unsigned flags)
{
    unsigned int temp1;
    unsigned int temp2;


    unsigned char* vramByte = (unsigned char*)VDP2_VRAM_START + (x >> 1 | y * 0x200);

    if((x & 1) != 0)
    {
        temp1 = (int)((char)(*vramByte & 0xf0) | (flags & 0xf));
        *vramByte = (unsigned char)temp1;
        return temp1;
    }

    temp2 = ((flags & 0xff)) << 4 & 0xf0;
    temp1 = (int)((char)(*vramByte & 0xf));
    *vramByte = (unsigned char)temp1 | (unsigned char)temp2;

    return temp1 | temp2;
}

void printfWrapper5(int x, int y, int count, unsigned int flags)
{
    // BUGBUG: not impl
    return;

}

// function 0x60041a0
void printfWrapper6(int x, int y, int count, unsigned int flags)
{
    int i = 0;

    if (count <= x) {
        return;
    }

    do {
        i = x + 1;
        writeTOVDP2VRAM(x, y, flags & 0xff);
        x = i;
    } while (i != count);

    return;
}


volatile unsigned int* VDP2_D0EN = (void*)0x25f80010;
volatile unsigned int* VDP2_D0MD = (void*)0x25f80014;
volatile unsigned short* VDP2_AREF = (void*)0x25f800b8;
volatile unsigned short* VDP2_T0C = (void*)0x25f80090;
volatile unsigned short* VDP2_T1S = (void*)0x25f80094;
volatile unsigned int* VDP2_IST = (void*)0x25f800a4;
volatile unsigned int* VDP2_AIACK = (void*)0x25f800a8;
volatile unsigned short* VDP2_T1MD = (void*)0x25f80098;
volatile unsigned short* VDP2_DSTA = (void*)0x25f8007C;
volatile unsigned short* VDP2_PDA = (void*)0x25f80088;
volatile unsigned int* VDP2_IMS = (void*)0x25f800A0;
volatile unsigned short* VDP2_ASR0 = (void*)0x25f800b0;
volatile unsigned short* VDP2_ASR1 = (void*)0x25f800b4;
volatile unsigned short* VDP2_D1W = (void*)0x25f80024;

// function 0x060041f0
void setVDP2Registers(int param_1)
    {
    RESOLUTIONS* currRes = &g_Resolutions[param_1];

    *VDP2_D0R = 0;
    *VDP2_D1R = 0;

    // BUGBUG: what's this registers
    *(volatile unsigned short*)0x25f8000e = 0;

    *VDP2_D0EN = 0xEEEEEEEE;
    *VDP2_D0MD = 0xEEEEEEEE;
    *(volatile unsigned int*)0x25f80018 = 0xEEEEEEEE;
    *(volatile unsigned int*)0x25f8001C = 0xEEEEEEEE;


    // BUGBUG: why is this size double??
    // Is it wrong here or earlier??
    memset((void*)VDP2_VRAM_START, 0, 512*1024);

    int tempHeight = currRes->height;

    // BUGBUG: how can this be negative?? all heights are positive except for the sentinel -1
    // which never should get here
    if(tempHeight < 0)
    {
        tempHeight += 0xf;
    }

    // BUGBUG: how can this be negative??
    if((tempHeight >> 4) < 1)
    {
        tempHeight = currRes->width;
    }
    else
    {
        int j = 0;
        int k = 0;

        do{

            printfWrapper6(0, k, currRes->width - 1, 7);
            tempHeight = currRes->height;

            j = j + 1;
            k = k + 0x10;

            if(tempHeight < 0)
            {
                tempHeight += 0xf;
            }
        }while(j < (int)tempHeight >> 4);

        tempHeight = currRes->width;
    }

    if(tempHeight < 0)
    {
        tempHeight += 0xf;
    }

    if(0 < (tempHeight >> 4))
    {
        int j = 0;
        while(true)
        {
            printfWrapper5(j << 4, 0, currRes->height - 1, 7);
            int tempWidth = currRes->width;

            j++;

            if(tempWidth < 0)
            {
                tempWidth += 0xf;
            }

            if((tempWidth >> 4) <= j)
            {
                break;
            }
        }

        tempHeight = currRes->height;
    }

    printfWrapper5(0, 0, currRes->height, 0xf);
    printfWrapper5(currRes->width - 1, 0, currRes->height, 0xf);
    printfWrapper6(0, 0, currRes->width, 0xf);
    printfWrapper6(0, currRes->height -1, currRes->width, 0xf);

    *VDP2_D1R = 1;
    *(volatile unsigned short*)0x25f8000e = 0;
    *VDP2_D0EN = 0x45454545;
    *VDP2_D0MD = 0x45454545;
    *(volatile unsigned int*)0x25f80018 = 0xEEEEEEEE;
    *(volatile unsigned int*)0x25f8001C = 0xEEEEEEEE;
    *VDP2_D1C = 0x0e0e;

    *(volatile unsigned short*)0x25f8002a = 0;
    *VDP2_D1EN = 0;
    *(volatile unsigned short*)0x25f80032 = 0;
    *VDP2_D1MD = 0;
    *(volatile unsigned short*)0x25f80036 = 0;
    *(volatile unsigned short*)0x25f8003a = 0;
    *(volatile unsigned short*)0x25f8003c = 0;
    *(volatile unsigned short*)0x25f8003e = 0;
    *VDP2_D2R = 0;
    *(volatile unsigned short*)0x25f80042 = 0;
    *VDP2_D2W = 0;
    *(volatile unsigned short*)0x25f80046 = 0;
    *VDP2_D2C = 0;
    *(volatile unsigned short*)0x25f8004a = 0;
    *VDP2_D2AD = 0;
    *(volatile unsigned short*)0x25f8004e = 0;
    *VDP2_DSTP = 0;
    *(volatile unsigned short*)0x25f80062 = 0;
    *(volatile unsigned short*)0x25f80064 = 0;
    *(volatile unsigned short*)0x25f80066 = 0;
    *(volatile unsigned short*)0x25f80068 = 0;
    *(volatile unsigned short*)0x25f8006a = 0;
    *(volatile unsigned short*)0x25f8006c = 0;
    *(volatile unsigned short*)0x25f8006e = 0;
    *VDP2_D2EN = 0;
    *(volatile unsigned short*)0x25f80052 = 0;
    *VDP2_D2MD = 0;
    *(volatile unsigned short*)0x25f80056 = 0;
    *(volatile unsigned short*)0x25f80058 = 0;
    *(volatile unsigned short*)0x25f8005a = 0;
    *(volatile unsigned short*)0x25f8005c = 0;
    *(volatile unsigned short*)0x25f8005e = 0;
    *VDP2_D1AD = 0;
    *(volatile unsigned short*)0x25f8002e = 0;

    *VDP2_AREF = 0;
    *(volatile unsigned short*)0x25f8002e = 0;
    *(volatile unsigned short*)0x25f80022 = 0;

    *(volatile unsigned short*)0x25f80070 = 0;
    *(volatile unsigned short*)0x25f80072 = 0;
    *(volatile unsigned short*)0x25f80074 = 0;

    *(volatile unsigned short*)0x25f80076 = 0;
    *VDP2_PPAF = 0;
    *(volatile unsigned short*)0x25f80082 = 0;
    *VDP2_PPD = 0;
    *(volatile unsigned short*)0x25f80086 = 0;
    *VDP2_T0C = 0;
    *(volatile unsigned short*)0x25f80092 = 0;
    *VDP2_T1S = 0;
    *(volatile unsigned short*)0x25f80096 = 0;
    *VDP2_T1MD = 0x0101;
    *(volatile unsigned short*)0x25f80078 = 1;
    *(volatile unsigned short*)0x25f8007a = 0;
    *VDP2_DSTA = 1;
    *(volatile unsigned short*)0x25f8007e = 0;
    *VDP2_PDA = 1;
    *(volatile unsigned short*)0x25f8008a = 0;
    *VDP2_PPD = 1;
    *(volatile unsigned short*)0x25f8008e = 0;
    *(volatile unsigned short*)0x25f8009a = 0;

    *(volatile unsigned int*)0x25f8009c = 0;
    *VDP2_IMS = 0; // BUGBUG: unsigned int
    *VDP2_IST = 0;// BUGBUG: unsigned int
    *(volatile unsigned int*)0x25f800bc = 0;
    *(volatile unsigned int*)0x25f800ac = 0;
    *VDP2_AIACK = 0; // BUGBUG: unsigned int
    *(volatile unsigned int*)0x25f800d8 = 0; // BUGBUG why is this done twice??
    *(volatile unsigned int*)0x25f800d8 = 0;
    *VDP2_IMS = 0; // BUGBUG: unsigned int

    *VDP2_IST = 0; // BUGBUG: unsigned int
    *(volatile unsigned int*)0x25f8009c = 0;
    *(volatile unsigned int*)0x25f800bc = 0;
    *VDP2_AIACK = 0; // BUGBUG: unsigned int
    *(volatile unsigned int*)0x25f800ac = 0;
    *(volatile unsigned int*)0x25f800d8 = 0;
    *(volatile unsigned int*)0x25f800dc = 0;

    *(volatile unsigned short*)0x25f800b2 = 0;
    *VDP2_ASR0 = 0;
    *VDP2_ASR1 = 0;
    *(volatile unsigned short*)0x25f800b6 = 0;
    *(volatile unsigned short*)0x25f800c0 = 0;
    *(volatile unsigned short*)0x25f800c2 = 0;
    *(volatile unsigned short*)0x25f800c4 = 0;
    *(volatile unsigned short*)0x25f800c6 = 0;
    *(volatile unsigned short*)0x25f800c8 = 0;
    *(volatile unsigned short*)0x25f800ca = 0;
    *(volatile unsigned short*)0x25f800cc = 0;
    *(volatile unsigned short*)0x25f800ce = 0;
    *(volatile unsigned short*)0x25f800d0 = 0;
    *(volatile unsigned short*)0x25f800d2 = 0;
    *(volatile unsigned short*)0x25f800d4 = 0;
    *(volatile unsigned short*)0x25f800d6 = 0;
    *(volatile unsigned short*)0x25f800e0 = 0;

    *(volatile unsigned short*)0x25f800f0 = 0x0101;
    *(volatile unsigned short*)0x25f800f2 = 0x0101;
    *(volatile unsigned short*)0x25f800f4 = 0x0101;
    *(volatile unsigned short*)0x25f800f6 = 0x0101;

    *(volatile unsigned short*)0x25f80100 = 0;
    *(volatile unsigned short*)0x25f80102 = 0;
    *(volatile unsigned short*)0x25f80104 = 0;
    *(volatile unsigned short*)0x25f80106 = 0;
    *(volatile unsigned short*)0x25f800e4 = 0;
    *(volatile unsigned short*)0x25f800e6 = 0;

    *VDP2_D1W = 0;
    *(volatile unsigned short*)0x25f80026 = 0;
    *(volatile unsigned short*)0x25f800f8 = 0x0707;

    *(volatile unsigned short*)0x25f800fa = 0;
    *(volatile unsigned short*)0x25f800fc = 0;
    *(volatile unsigned short*)0x25f800ea = 0;
    *(volatile unsigned short*)0x25f800e8 = 0;
    *(volatile unsigned short*)0x25f800ec = 0;
    *(volatile unsigned short*)0x25f80108 = 0;
    *(volatile unsigned short*)0x25f8010a = 0;
    *(volatile unsigned short*)0x25f8010c = 0;
    *(volatile unsigned short*)0x25f8010e = 0;
    *(volatile unsigned short*)0x25f800ee = 0;
    *(volatile unsigned short*)0x25f80110 = 0;
    *(volatile unsigned short*)0x25f80112 = 0;
    *(volatile unsigned short*)0x25f80114 = 0;
    *(volatile unsigned short*)0x25f80116 = 0;
    *(volatile unsigned short*)0x25f80118 = 0;
    *(volatile unsigned short*)0x25f8011a = 0;
    *(volatile unsigned short*)0x25f8011c = 0;
    *(volatile unsigned short*)0x25f8011e = 0;
    *(volatile unsigned short*)0x25f800e2 = 0;


    // 480p special case
    if(currRes->flags - 6 < 2)
    {
        *VDP2_D1R = 3;
        *(volatile unsigned short*)0x25f80078 = 2;
        *(volatile unsigned short*)0x25f8007a = 0;
        *VDP2_PDA = 2;
        *(volatile unsigned short*)0x25f8008a = 0;
        *(volatile unsigned short*)0x25f80070 = 0;
        *(volatile unsigned short*)0x25f80072 = 0;
        *VDP2_PPAF = 1;
        *(volatile unsigned short*)0x25f80082 = 0;
        *VDP2_PPD = 0;
        *(volatile unsigned short*)0x25f80086 = 0;
    }

    // write data to VDP2 CRAM Cache
    // BUGBUG: this was done inlined, but reusing a fucntion I already had
    copyDataToVDP2CRAMCache();



}


// bios_set_clock_speed is at 0x320
typedef void (*bios_set_clock_speed_FP)(unsigned int);
bios_set_clock_speed_FP bios_set_clock_speed = (bios_set_clock_speed_FP)(0x320);

// function 0x060046b0
void printMenu(int param_1)
{
    RESOLUTIONS* currRes = &g_Resolutions[param_1];

    bios_set_clock_speed(currRes->flags & 1);

    *VDP2_D0R = 0x8000;

    pollVDP2_D0W_8_2();
    pollVDP2_D0W_4();

    setVDP2Registers(param_1);

    int flags = currRes->flags;

    int someField = (flags >> 6) & 3;

    printfWrapper(2, 2, 0x00f0, "---x---");

    int someField2 = (flags & 7) * 4;

    // print width of the string
    printfWrapper(2, 2, 0x00f0, "320"); // BUGBUG: this is an index into something that prints out the widths

    // 480p resolutions
    if((flags & 0x4) != 0)
    {
        printfWrapper(6, 2, 0x00f0, "480");
        printfWrapper(2, 3, 0x00f0, "non-interlaced"); // BUGBUG: this is an index into something that prints out

        printfWrapper(2, 4, 0x00f0, "SDTV"); // BUGBUG: this is an index into SDTV, EDTV, Hi-Vision
        printfWrapper(2, 4, 0x00f0, "Press C for the next mode.");

        *VDP2_D0R = 0x8000;
        pollVDP2_D0W_8_2();
        pollVDP2_D0W_4();

        *VDP2_D0R = (unsigned short)(currRes->flags | 0x8000);
        return;
    }
    else // non-480 p resolutions
    {
        printfWrapper(6, 2, 0x00f0, "240"); // BUGBUG: this is an index into a list of resolution strings
        printfWrapper(2, 3, 0x00f0, "non-interlaced"); // BUGBUG: this is an index into something that prints out
        printfWrapper(2, 4, 0x00f0, "SDTV"); // BUGBUG: this is an index into SDTV, EDTV, Hi-Vision
        printfWrapper(2, 4, 0x00f0, "Press C for the next mode.");

        *VDP2_D0R = 0x8000;
        pollVDP2_D0W_8_2();
        pollVDP2_D0W_4();

        *VDP2_D0R = (unsigned short)(currRes->flags | 0x8000);
        return;
    }

    return;
}

void jo_main(void)
{
    unsigned short smpcData[3];

    //jo_core_init(JO_COLOR_Black);
    /*

    jo_core_add_callback(my_draw);
    jo_core_run();
    */

    initVDP2();
    copyDataToVDP2CRAMCache();
    initSMPC(); // not implemented

    initVDP1();
    printfWrapper(2, 1, 0x0f, "BG0123");

    *VDP2_D0R = 0x8000;

    readSMPC(0, smpcData);

    g_currentIndex = 0;

    printMenu(g_currentIndex);

    pollVDP2_D0W_8();
    pollVDP2_D0W_8_2();

    *VDP1_FBCR = 1;
    readSMPC(0, smpcData);

    unsigned short someVar = 0;
    unsigned short someVar2 = 2;

    while(1)
    {
        // poll controller for inputs and change globals
        // if the user presses C, increment the resolution
        // and check if width == -1 meaning we are at the end of the array

        *VDP2_PPAF = someVar;
        *VDP2_PPD = someVar2;

        printMenu(0);

        initVDP1VRAMCache();
        setVDP1VRAM(0xA0, 0x70);
        setVDP1VRAM_2(0, 0, 0xA0, 0x70);
        setVDP1VRAM_3(0, 0);

        // set some global to 0??

        setVDP1VRAM_4(0x9f, 0, 0x9f, 0x6f, 0, 0x6f, 0, 0, 0xfc1f);
        setVDP1VRAM_5();

        pollVDP2_D0W_8();
        pollVDP2_D0W_8_2();

        *VDP1_FBCR = 1;

        readSMPC(0, smpcData);

    }
}
