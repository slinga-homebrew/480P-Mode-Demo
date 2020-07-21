// This is a decompilation attempt of Charles MacDonald's 480p Mode Demo
// The original ISO is available here: https://segaxtreme.net/threads/progressive-hires-test-demo.15539/

// Jo Engine
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

// libyaul
/*
* Copyright (c) 2012-2019 Israel Jacquez
* See LICENSE for details.
*
* Israel Jacquez <mrkotfw@gmail.com>
*/


#include <jo/jo.h>
#include "vdp2_regs.h"
#include "vdp2_cram_data.h"

#define UNUSED(x) (void)(x)

typedef struct _RESOLUTIONS {
    unsigned int width;
    unsigned int height;
    unsigned int flags;

    // flag is really a union:
    // - bit 0 is passed to bios_set_clock_speed() to run at a faster clock speed
    // - bit 2 is 480p progressive flag
    // - bits 0-2 are combined again as an index into tvModes
    // - bits 6-7 are an index into interlaceModes

} RESOLUTIONS, *PRESOLUTIONS;

const char* tvModes[] =
{
    "SDTV",
    "SDTV",
    "SDTV",
    "SDTV",
    "EDTV (VGA), progressive scan",
    "Hi-Vision, progessive scan",
    "EDTV (VGA), progressive scan",
    "Hi-Vision, progessive scan"
};

const char* interlaceModes[] =
{
    "Non-interlaced",
    "---", // not used??
    "Single-density interlaced",
    "Double density interlaced"
};

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
    { 320, 480, 0x00000004, }, // EDTV (VGA), progressive scan
    { 640, 480, 0x00000006, }, // EDTV (VGA), progressive scan
    { 352, 480, 0x00000005, }, // Hi-Vision, progressive scan
    { 704, 480, 0x00000007, }, // Hi-Vision, progressive scan
    { 0xffffffff, 0xffffffff, 0xffffffff, } // end marker
};

// stdlib function prototypes to keep compiler happy
//void* memcpy(void *dst, const void *src, unsigned int len);
//void *memset(void *s, int c, unsigned int n);

// access to VRAM must be 2 byte aligned
// caller must make sure len is 2 byte aligned
void __attribute__ ((noinline)) my_memcpy(volatile unsigned short* dst, const unsigned short* src, unsigned int len)
{
    for(unsigned int i = 0; i < len; i += 2)
    {
        dst[i/2] = src[i/2];
    }
}

// access to VRAM must be 2 byte aligned
// caller must make sure len is 2 byte aligned
void __attribute__ ((noinline)) my_memset(volatile unsigned short* s, int c, unsigned int n)
{
    for(unsigned int i = 0; i < n; i += 2)
    {
        s[i/2] = (unsigned short)c;
    }
}

#define VDP2_VRAM_START 0x25e00000
#define VDP2_VRAM_SIZE  0x80000
#define VDP2_CRAM_START 0x25f00000
#define VDP2_CRAM_SIZE  0x1000

// function 0x06004dd0
void __attribute__ ((noinline)) initVDP2()
{
    //
    // set VDP2 registers
    //
    *VDP2_TVMD = 0;
    *VDP2_RAMCTL = *VDP2_RAMCTL & 0xCFFF;

    *VDP2_MPOFN = 0;
    *VDP2_CHCTLA = 0x1e;

    *VDP2_SCXIN0 = 0;
    *VDP2_SCXDN0 = 0;
    *VDP2_SCYIN0 = 0;
    *VDP2_BGON = 1;

    //
    // zero VDP2 VRAM and CRAM cache
    //

    // in the original disassembly the code zeros two byte aligned
    // - iteration one 0x40000
    // - iteration two 0x0800
    // I doubled both iterations and used a memset instead

    my_memset((void*)VDP2_VRAM_START, 0, VDP2_VRAM_SIZE);
    my_memset((void*)VDP2_CRAM_START, 0, VDP2_CRAM_SIZE);
}

// function 0x06004f70
void __attribute__ ((noinline)) copyDataToVDP2CRAMCache()
{
    my_memcpy((void*)VDP2_CRAM_START, (void*)CRAM_DATA, sizeof(CRAM_DATA));

    *(volatile unsigned short*)VDP2_CRAM_START = *(unsigned short*)(CRAM_DATA + 2);
    *(volatile unsigned short*)(VDP2_CRAM_START + 2) = 0x4000;
}

volatile unsigned char* SMPC_DDR1 = (void*)0x20100079;
volatile unsigned char* SMPC_DDR2 = (void*)0x2010007b;
volatile unsigned char* SMPC_IOSEL1 = (void*)0x2010007d;
volatile unsigned char* SMPC_EXLE1 = (void*)0x2010007f;

// function 0x06005280
void __attribute__ ((noinline)) initSMPC()
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
void __attribute__ ((noinline)) initVDP1()
{
    unsigned short pattern = 0x0101;

    *VDP2_PRISA = pattern;
    *VDP2_PRISB = pattern;
    *VDP2_PRISC = pattern;
    *VDP2_PRISD = pattern;

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
#define VDP1_VRAM_SIZE  0x40

unsigned short g_VDP1_Global = 0;
unsigned int g_currentResolutionIndex = 0;

// function 0x06004aa0
void __attribute__ ((noinline)) initVDP1VRAMCache()
{
    jo_memset((void*)VDP1_VRAM_START, 0xFF, VDP1_VRAM_SIZE);

    *(unsigned short*)VDP1_VRAM_START = 0x4000;

    // BUGBUG: some global is set to 1 here?
    g_VDP1_Global = 1;
}

// function 06004ad0
void __attribute__ ((noinline)) setSomethingVDP1VRAMCache()
{
    *(unsigned short*)((unsigned char*)(VDP1_VRAM_START) + (g_VDP1_Global & 0xffff) * 20) = 0x8000;
    g_VDP1_Global++;
}

// function 0x6004100
unsigned int __attribute__ ((noinline)) writeToVDP2VRAM(int x, int y, unsigned flags)
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

int __attribute__ ((noinline)) someShifter(int x, int y)
{
    return x >> y;
}

// function 06004fb0
void __attribute__ ((noinline)) drawCharacter(int x,int y, unsigned int flag, char msg)
{
    unsigned int i = 0;
    int j = 0;

    int tempX = 0;
    int tempY = 0;

    unsigned int retVal = 0;

    do{

        j = 0;
        char cVar1 = characterData[(msg*8) | i];
        tempY = (y * 8) + i;

        do {
            while(retVal = someShifter(0x0080, j), (retVal & (int)cVar1  & 0xFFU) == 0) {
                tempX = j + (x*8);

                j = j + 1;

                writeToVDP2VRAM(tempX, tempY, flag & 0x0f);

                if(j == 8)
                    goto LOOP_EXIT;
            }

            tempX = j + (x * 8);
            j = j + 1;
            writeToVDP2VRAM(tempX, tempY, (flag & 0xff) >> 4);

        }while(j != 8);

    LOOP_EXIT:
        i = i + 1;
        if(i == 8) {
            return;
        }

    }while(true);
}

// function 06005060
void __attribute__ ((noinline)) drawString(int x,int y, unsigned int flag, const char *msg)
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
        drawCharacter(tempX, y, flag & 0xff, msg[i-1]);
    } while (msg[i] != '\0');

    return;
}

// function 0x06004f10
int __attribute__ ((noinline)) pollVDP2_TVSTAT_8_NOT_0(void)
{
    do {

    } while ((*VDP2_TVSTAT & 8) != 0);

    return 0;
}

// function 0x06004ef0
unsigned int __attribute__ ((noinline)) pollVDP2_TVSTAT_8_0(void)
{
    do {

    } while (((int)*VDP2_TVSTAT & 8U) == 0);

    return (int)*VDP2_TVSTAT & 8U;
}

// function 0x06004eb0
unsigned int __attribute__ ((noinline)) pollVDP2_TVSTAT_4_0(void)
{
do {
} while (((int)*VDP2_TVSTAT & 4U) == 0);
return (int)*VDP2_TVSTAT & 4U;
}

// function 0x06004b60
void __attribute__ ((noinline)) setVDP1VRAM(unsigned short a, unsigned short b)
{
    volatile unsigned short* vdp1_vram = (volatile unsigned short*)((unsigned char*)VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 9;
    vdp1_vram[1] = 0;
    vdp1_vram[10] = a;
    vdp1_vram[0xb] = b;

    g_VDP1_Global++;
}

// function 0x06004ba0
void __attribute__ ((noinline)) setVDP1VRAM_2(unsigned short a, unsigned short b, unsigned short c, unsigned short d)
{
    volatile unsigned short* vdp1_vram = (volatile unsigned short*)((unsigned char*)VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 8;
    vdp1_vram[1] = 0;
    vdp1_vram[6] = a;
    vdp1_vram[7] = b;
    vdp1_vram[10] = c;
    vdp1_vram[0xb] = d;

    g_VDP1_Global++;
}

// function 0x06004be0
void __attribute__ ((noinline)) setVDP1VRAM_3(unsigned short a, unsigned short b)
{
    volatile unsigned short* vdp1_vram = (unsigned short*)((unsigned char*)VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 10;
    vdp1_vram[1] = 0;
    vdp1_vram[6] = a;
    vdp1_vram[7] = b;

    g_VDP1_Global++;
}

// function 0x06004c20
void __attribute__ ((noinline))  setVDP1VRAM_4(unsigned short param_1,unsigned short param_2,unsigned short param_3,unsigned short param_4,unsigned short param_5, unsigned short param_6,unsigned short param_7,unsigned short param_8,unsigned short param_9)
{
    volatile unsigned short* vdp1_vram = (volatile unsigned short*)(VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

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
void __attribute__ ((noinline)) setVDP1VRAM_5()
{
    volatile unsigned short* vdp1_vram = (volatile unsigned short*)(VDP1_VRAM_START + (g_VDP1_Global & 0xFFFF)*0x20);

    vdp1_vram[0] = 0x8000;

    g_VDP1_Global++;
}

volatile unsigned char* SMPC_PDR1 = (void*)0x20100075;

// function 0x060052b0
void __attribute__ ((noinline)) readSMPC(unsigned int unused, unsigned short* outData)
{
    unsigned char val1, val2, val3;
    unsigned short val4;
    unsigned short val5;

    UNUSED(unused);

    *SMPC_PDR1 = 0x60;
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

    val5 = ((short)(*SMPC_PDR1 & 0xF) |
            (val3 & 0xf) << 4 |
            (val2 & 0xf) << 8 |
            ((val1 & 0x8 << 0xc))) ^ 0x8FFF;

    val4 = *outData;

    outData[0] = val5;
    outData[1] = val4;
    outData[2] = val5 & (val5 ^ val4);

    return;
}

// function 0x6004150
void __attribute__ ((noinline)) drawVerticalLines(int x, int y, int count, unsigned int flags)
{
    int i = 0;

    if (count <= y) {
        return;
    }

    do {
        i = y + 1;
        writeToVDP2VRAM(x, y, flags & 0xff);
        y = i;
    } while (i != count);

    return;
}

// function 0x60041a0
void __attribute__ ((noinline)) drawHorizontalLines(int x, int y, int count, unsigned int flags)
{
    int i = 0;

    if (count <= x) {
        return;
    }

    do {
        i = x + 1;
        writeToVDP2VRAM(x, y, flags & 0xff);
        x = i;
    } while (i != count);

    return;
}

// function 0x060041f0
void __attribute__ ((noinline)) setVDP2Registers(int param_1)
{
    RESOLUTIONS* currRes = &g_Resolutions[param_1];

    *VDP2_TVMD = 0;
    *VDP2_BGON = 0;
    *VDP2_RAMCTL = 0;

    *VDP2_CYCA0L = 0xEEEEEEEE;
    *VDP2_CYCA1L = 0xEEEEEEEE;
    *VDP2_CYCB0L = 0xEEEEEEEE;
    *VDP2_CYCB1L = 0xEEEEEEEE;

    // BUGBUG: why is this size double??
    // Is it wrong here or earlier??
    my_memset((void*)VDP2_VRAM_START, 0, 0x100000);

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

            drawHorizontalLines(0, k, currRes->width - 1, 7);
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
            drawVerticalLines(j << 4, 0, currRes->height - 1, 7);
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

    drawVerticalLines(0, 0, currRes->height, 0xf);
    drawVerticalLines(currRes->width - 1, 0, currRes->height, 0xf);
    drawHorizontalLines(0, 0, currRes->width, 0xf);
    drawHorizontalLines(0, currRes->height -1, currRes->width, 0xf);

    *VDP2_BGON = 1;
    *VDP2_RAMCTL = 0;
    *VDP2_CYCA0L = 0x45454545;
    *VDP2_CYCA1L = 0x45454545;
    *VDP2_CYCB0L = 0xEEEEEEEE;
    *VDP2_CYCB1L = 0xEEEEEEEE;
    *VDP2_CHCTLA = 0x0e0e;
    *VDP2_CHCTLB = 0;

    *VDP2_PNCN0 = 0;
    *VDP2_PNCN1 = 0;
    *VDP2_PNCN2 = 0;
    *VDP2_PNCN3 = 0;
    *VDP2_PLSZ = 0;
    *VDP2_MPOFN = 0;
    *VDP2_MPOFR = 0;
    *VDP2_MPABN0 = 0;
    *VDP2_MPCDN0 = 0;
    *VDP2_MPABN1 = 0;
    *VDP2_MPCDN1 = 0;
    *VDP2_MPABN2 = 0;
    *VDP2_MPCDN2 = 0;
    *VDP2_MPABN3 = 0;
    *VDP2_MPCDN3 = 0;
    *VDP2_MPABRB = 0;
    *VDP2_MPCDRB = 0;
    *VDP2_MPEFRB = 0;
    *VDP2_MPGHRB = 0;
    *VDP2_MPIJRB = 0;
    *VDP2_MPKLRB = 0;
    *VDP2_MPMNRB = 0;
    *VDP2_MPOPRB = 0;
    *VDP2_MPABRA = 0;
    *VDP2_MPCDRA = 0;
    *VDP2_MPEFRA = 0;
    *VDP2_MPGHRA = 0;
    *VDP2_MPIJRA = 0;
    *VDP2_MPKLRA = 0;
    *VDP2_MPMNRA = 0;
    *VDP2_MPOPRA = 0;
    *VDP2_BMPNA = 0;
    *VDP2_BMPNB = 0;
    *VDP2_OVPNRA = 0;
    *VDP2_OVPNRB = 0;
    *VDP2_MZCTL = 0;
    *VDP2_SCXIN0 = 0;
    *VDP2_SCXDN0 = 0;
    *VDP2_SCYIN0 = 0;
    *VDP2_SCYDN0 = 0;
    *VDP2_SCXIN1 = 0;
    *VDP2_SCXDN1 = 0;
    *VDP2_SCYIN1 = 0;
    *VDP2_SCYDN1 = 0;
    *VDP2_SCXN2 = 0;
    *VDP2_SCYN2 = 0;
    *VDP2_SCXN3 = 0;
    *VDP2_SCYN3 = 0;

    *VDP2_ZMCTL = 0x0101;
    *VDP2_ZMXIN0 = 1;
    *VDP2_ZMXDN0 = 0;
    *VDP2_ZMYIN0 = 1;
    *VDP2_ZMYDN0 = 0;
    *VDP2_ZMXIN1 = 1;
    *VDP2_ZMXDN1 = 0;
    *VDP2_ZMYIN1 = 1;
    *VDP2_ZMYDN0 = 0;
    *VDP2_SCRCTL = 0;

    // longs
    *VDP2_VCSTAU = 0;
    *VDP2_LSTA0U = 0;
    *VDP2_LSTA1U = 0;
    *VDP2_RPTAU = 0;
    *VDP2_BKTAU = 0;
    *VDP2_LCTAU = 0;
    *VDP2_LWTA0U = 0;
    *VDP2_LWTA0U = 0;
    *VDP2_LSTA0U = 0;
    *VDP2_LSTA1U = 0;
    *VDP2_VCSTAU = 0;
    *VDP2_RPTAU = 0;
    *VDP2_LCTAU = 0;
    *VDP2_BKTAU = 0;
    *VDP2_LWTA0U = 0;
    *VDP2_LWTA1U = 0;

    // back to WORDs again
    *VDP2_RPRCTL = 0;
    *VDP2_RPMD = 0;
    *VDP2_KTCTL = 0;
    *VDP2_KTAOF = 0;
    *VDP2_WPSX0 = 0;
    *VDP2_WPSY0 = 0;
    *VDP2_WPEX0 = 0;
    *VDP2_WPEY0 = 0;
    *VDP2_WPSX1 = 0;
    *VDP2_WPSY1 = 0;
    *VDP2_WPEX1 = 0;
    *VDP2_WPEY1 = 0;
    *VDP2_WCTLA = 0;
    *VDP2_WCTLB = 0;
    *VDP2_WCTLC = 0;
    *VDP2_WCTLD = 0;

    *VDP2_SPCTL = 0x20;
    *VDP2_PRISA = 0x0101;
    *VDP2_PRISB = 0x0101;
    *VDP2_PRISC = 0x0101;
    *VDP2_PRISD = 0x0101;

    *VDP2_CCRSA = 0;
    *VDP2_CCRSB = 0;
    *VDP2_CCRSC = 0;
    *VDP2_CCRSD = 0;
    *VDP2_CRAOFA = 0;
    *VDP2_CRAOFB = 0;
    *VDP2_SFSEL = 0;
    *VDP2_SFCODE = 0;

    *VDP2_PRINA = 0x0707;

    *VDP2_PRINB = 0;
    *VDP2_PRIR = 0;
    *VDP2_SFPRMD = 0;
    *VDP2_LNCLEN = 0;
    *VDP2_CCCTL = 0;
    *VDP2_CCRNA = 0;
    *VDP2_CCRNB = 0;
    *VDP2_CCRR = 0;
    *VDP2_CCRLB = 0;
    *VDP2_SFCCMD = 0;
    *VDP2_CLOFEN = 0;
    *VDP2_CLOFSL = 0;
    *VDP2_COAR = 0;
    *VDP2_COAG = 0;
    *VDP2_COAB = 0;
    *VDP2_COBR = 0;
    *VDP2_COBG = 0;
    *VDP2_COBB = 0;
    *VDP2_SDCTL = 0;

    // 480p special case
    if(currRes->flags - 6 < 2)
    {
        *VDP2_BGON = 3;
        *VDP2_ZMXIN0 = 2;
        *VDP2_ZMXDN0 = 0;
        *VDP2_ZMXIN1 = 2;
        *VDP2_ZMXDN1 = 0;
        *VDP2_SCXIN0 = 0;
        *VDP2_SCXDN0 = 0;
        *VDP2_SCXIN1 = 1;
        *VDP2_SCXDN1 = 0;
        *VDP2_SCYIN1 = 0;
        *VDP2_SCYDN1 = 0;
    }

    // write data to VDP2 CRAM Cache
    // BUGBUG: this was done inlined, but reusing a function I already had
    copyDataToVDP2CRAMCache();
}

// taken from libyaul
void bios_clock_speed_chg(unsigned char mode)
{
    register unsigned int *bios_address;
    bios_address = (unsigned int *)0x06000320;

    ((void (*)(unsigned int))*bios_address)(mode);
}

// function 0x060046b0
void __attribute__ ((noinline)) changeResolution(int index)
{
    char buffer[16] = {0};

    RESOLUTIONS* currRes = &g_Resolutions[index];

    bios_clock_speed_chg(currRes->flags & 1);
    bios_clock_speed_chg(currRes->flags & 1);

    *VDP2_TVMD = 0x8000;

    pollVDP2_TVSTAT_8_0();
    pollVDP2_TVSTAT_4_0();

    setVDP2Registers(index);

    int flags = currRes->flags;

    int interlaceModeIndex = (flags >> 6) & 3;

    drawString(2, 2, 0x00f0, "---x---");

    int tvModeIndex = (flags & 7);

    // print width of the string
    sprintf(buffer, "%d", currRes->width);
    drawString(2, 2, 0x00f0, buffer);

    // 480p resolutions
    if((flags & 0x4) != 0)
    {
        sprintf(buffer, "%d", currRes->height);
        drawString(6, 2, 0x00f0, buffer);
        drawString(2, 3, 0x00f0, interlaceModes[interlaceModeIndex]);
        drawString(2, 4, 0x00f0, tvModes[tvModeIndex]);
        drawString(2, 6, 0x00f0, "Press C for the next mode.");

        drawString(2, 7, 0x00f0, "Original code by Charles MacDonald. Decompiled by Slinga.");

        *VDP2_TVMD = 0x8000;
        pollVDP2_TVSTAT_8_0();
        pollVDP2_TVSTAT_4_0();

        *VDP2_TVMD = (unsigned short)(currRes->flags | 0x8000);
        return;
    }
    else // non-480 p resolutions
    {
        sprintf(buffer, "%d", currRes->height);
        drawString(6, 2, 0x00f0, buffer);
        drawString(2, 3, 0x00f0, interlaceModes[interlaceModeIndex]);
        drawString(2, 4, 0x00f0, tvModes[tvModeIndex]);
        drawString(2, 6, 0x00f0, "Press C for the next mode.");

        drawString(2, 8, 0x00f0, "Original code by Charles MacDonald");
        drawString(2, 9, 0x00f0, "Decompiled by Slinga");

        *VDP2_TVMD = 0x8000;
        pollVDP2_TVSTAT_8_0();
        pollVDP2_TVSTAT_4_0();

        *VDP2_TVMD = (unsigned short)(currRes->flags | 0x8000);
        return;
    }

    return;
}

void __attribute__ ((noinline)) jo_main(void)
{
    unsigned short smpcData[3] = {0};

    // initialized VDP1 and VDP2
    initVDP2();
    copyDataToVDP2CRAMCache();
    initSMPC();
    initVDP1();

    // this is in the disassembly but doesn't actually display anything
    // because it's not in the loop before
    // drawString(2, 4, 0xF0, "BG0123");

    *VDP2_TVMD = 0x8000;

    readSMPC(0, smpcData);

    g_currentResolutionIndex = 0;

    changeResolution(g_currentResolutionIndex);

    pollVDP2_TVSTAT_8_NOT_0();
    pollVDP2_TVSTAT_8_0();

    *VDP1_FBCR = 1;
    readSMPC(0, smpcData);

    unsigned short SCXIN1 = 0;
    unsigned short SCYIN1 = 0;

    // while controller 1 doesn't hit start
    while(((smpcData[0] >> 0xb) & 1) == 0)
    {
        // poll controller for inputs and change globals
        // and check if width == -1 meaning we are at the end of the array
        if(((smpcData[2] >> 0x9) & 1) != 0)
        {
            // if the user presses C, increment the resolution
            int tempResIndex = g_currentResolutionIndex + 1;

            // sanity check it
            if(g_Resolutions[tempResIndex].width == 0xFFFFFFFF)
            {
                g_currentResolutionIndex = 0;
            }
            else
            {
                g_currentResolutionIndex = tempResIndex;
            }
            changeResolution(g_currentResolutionIndex);
        }

        // the decompilation also checks 4 other buttons to increment/decrement the SCXIN1/SCYINI
        if ((smpcData[2] & 0x40) != 0) {
            SCXIN1 = SCXIN1 - 1;
        }
        if ((smpcData[2] & 0x80) != 0) {
            SCXIN1 = SCXIN1 + 1;
        }
        if ((smpcData[2] & 0x10) != 0) {
            SCYIN1 = SCYIN1 - 1;
        }
        if ((smpcData[2] & 0x20) != 0) {
            SCYIN1 = SCYIN1 + 1;
        }

        *VDP2_SCXIN1 = SCXIN1;
        *VDP2_SCYIN1 = SCYIN1;

        initVDP1VRAMCache();
        setVDP1VRAM(0xA0, 0x70);
        setVDP1VRAM_2(0, 0, 0xA0, 0x70);
        setVDP1VRAM_3(0, 0);

        // set some global to 0??

        setVDP1VRAM_4(0x9f, 0, 0x9f, 0x6f, 0, 0x6f, 0, 0, 0xfc1f);
        setSomethingVDP1VRAMCache();

        pollVDP2_TVSTAT_8_NOT_0();
        pollVDP2_TVSTAT_8_0();

        *VDP1_FBCR = 1;

        readSMPC(0, smpcData);

    }

    // these functions aren't called until the user hits start so I don't think they are necessary to RE
    // 0x06005570
    // 0x06004e50
    // 0x60052a0
}
