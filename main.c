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
    unsigned int flag;
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
    { 320, 480, 0x00000004, },
    { 640, 480, 0x00000006, },
    { 352, 480, 0x00000005, },
    { 704, 480, 0x00000007, },
    { 0xffffffff, 0xffffffff, 0xffffffff, } // end marker
};

// stdlib function prototypes to keep compiler happy
void* memcpy(void *dst, const void *src, unsigned int len);
void *memset(void *s, int c, unsigned int n);

volatile unsigned short* VDP2_D0R = (unsigned short*)0x25F80000;
volatile unsigned short* VDP2_D0W = (unsigned short*)0x25F80004;
volatile unsigned short* VDP2_D1C = (unsigned short*)0x25f80028;
volatile unsigned short* VDP2_D1R = (unsigned short*)0x25f80020;

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

    val5 = (*SMPC_PDR1 & 0xF) |
            (val3 & 0xf) << 4 |
            (val2 & 0xf) << 8 |
            ((val1 & 0x8 << 0xc)) ^ 0x8FFF;

    val4 = *outData;

    outData[0] = val5;
    outData[1] = val4;
    outData[2] = val5 & (val5 ^ val4);

    return;
}

void printMenu(int param_1)
{

}


void jo_main(void)
{
    unsigned short smpcData[3];

    jo_core_init(JO_COLOR_Black);
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

    // printMenu()


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
