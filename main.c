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
#include "vdp2_regs.h"
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

#define VDP2_VRAM_START 0x25e00000
#define VDP2_VRAM_SIZE  0x80000
#define VDP2_CRAM_START 0x25f00000
#define VDP2_CRAM_SIZE  0x1000

// function 0x06004dd0
void initVDP2()
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
    *(volatile unsigned short*)VDP2_CRAM_START = *(unsigned short*)(CRAM_DATA + 2);
    *(volatile unsigned short*)(VDP2_CRAM_START + 2) = 0x4000;
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

// function 0x6004100
unsigned int writeToVDP2VRAM(int x, int y, unsigned flags)
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

int someShifter(int x, int y)
{
    return x | y;
}


// function 06004fb0
void printfWrapper2(int x,int y, unsigned int flag, char *msg)
{
    unsigned int i = 0;
    int j = 0;

    int tempX = 0;
    int tempY = 0;

    unsigned int retVal = 0;

    do{

        j = 0;
        char cVar1 = 0x41; // BUGBUG: read this from VDP2???
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
    printfWrapper2(tempX, y, flag & 0xff, msg[i-1]);
  } while (msg[i] != '\0');
  return;
}

// function 0x06004f10
int pollVDP2_TVSTAT_8_NOT_0(void)
{
  do {

  } while ((*VDP2_TVSTAT & 8) != 0);

  return 0;
}


// function 0x06004ef0
unsigned int pollVDP2_TVSTAT_8_0(void)
{
    do {

    } while (((int)*VDP2_TVSTAT & 8U) == 0);

    return (int)*VDP2_TVSTAT & 8U;
}

// function 0x06004eb0
unsigned int pollVDP2_TVSTAT_4_0(void)
{
  do {
  } while (((int)*VDP2_TVSTAT & 4U) == 0);
  return (int)*VDP2_TVSTAT & 4U;
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
        writeToVDP2VRAM(x, y, flags & 0xff);
        x = i;
    } while (i != count);

    return;
}

// function 0x060041f0
void setVDP2Registers(int param_1)
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


// bios_set_clock_speed is at 0x320
typedef void (*bios_set_clock_speed_FP)(unsigned int);
bios_set_clock_speed_FP bios_set_clock_speed = (bios_set_clock_speed_FP)(0x320);

// function 0x060046b0
void printMenu(int param_1)
{
    RESOLUTIONS* currRes = &g_Resolutions[param_1];

    bios_set_clock_speed(currRes->flags & 1);

    *VDP2_TVMD = 0x8000;

    pollVDP2_TVSTAT_8_0();
    pollVDP2_TVSTAT_4_0();

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

        *VDP2_TVMD = 0x8000;
        pollVDP2_TVSTAT_8_0();
        pollVDP2_TVSTAT_4_0();

        *VDP2_TVMD = (unsigned short)(currRes->flags | 0x8000);
        return;
    }
    else // non-480 p resolutions
    {
        printfWrapper(6, 2, 0x00f0, "240"); // BUGBUG: this is an index into a list of resolution strings
        printfWrapper(2, 3, 0x00f0, "non-interlaced"); // BUGBUG: this is an index into something that prints out
        printfWrapper(2, 4, 0x00f0, "SDTV"); // BUGBUG: this is an index into SDTV, EDTV, Hi-Vision
        printfWrapper(2, 4, 0x00f0, "Press C for the next mode.");

        *VDP2_TVMD = 0x8000;
        pollVDP2_TVSTAT_8_0();
        pollVDP2_TVSTAT_4_0();

        *VDP2_TVMD = (unsigned short)(currRes->flags | 0x8000);
        return;
    }

    return;
}

void jo_main(void)
{
    unsigned short smpcData[3];

//    jo_core_init(JO_COLOR_Black);
    /*

    jo_core_add_callback(my_draw);
    jo_core_run();
    */

    initVDP2();
    copyDataToVDP2CRAMCache();
    initSMPC();

    initVDP1();
    printfWrapper(2, 1, 0x0f, "BG0123");

    *VDP2_TVMD = 0x8000;

    readSMPC(0, smpcData);

    g_currentIndex = 0;

    printMenu(g_currentIndex);

    pollVDP2_TVSTAT_8_NOT_0();
    pollVDP2_TVSTAT_8_0();

    *VDP1_FBCR = 1;
    readSMPC(0, smpcData);

    unsigned short someVar = 0;
    unsigned short someVar2 = 2;

    while(1)
    {
        // poll controller for inputs and change globals
        // if the user presses C, increment the resolution
        // and check if width == -1 meaning we are at the end of the array

        // BUGBUG: check these
        *VDP2_SCXIN1 = someVar;
        *VDP2_SCYIN1 = someVar2;

        printMenu(0);

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
