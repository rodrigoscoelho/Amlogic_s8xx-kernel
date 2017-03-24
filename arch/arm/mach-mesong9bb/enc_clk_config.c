/*
 * arch/arm/mach-mesong9tv/enc_clk_config.c
 *
 * Copyright (C) 2014 Amlogic, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/cpu.h>

#include <linux/clkdev.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <plat/io.h>
#include <plat/cpufreq.h>
#include <mach/am_regs.h>
#include <mach/clock.h>
#include <mach/cpu.h>

#include <linux/amlogic/vout/vinfo.h>
#include <linux/amlogic/vout/enc_clk_config.h>
#include "hw_enc_clk_config.h"

static DEFINE_MUTEX(enc_clock_lock);
static unsigned int hpll_vco_clk = 0xffff;      // not initial value

#define check_clk_config(para)\
    if (para == -1)\
        return;

#define check_div() \
    if (div == -1)\
        return ;\
    switch (div) {\
        case 1:\
            div = 0; break;\
        case 2:\
            div = 1; break;\
        case 4:\
            div = 2; break;\
        case 6:\
            div = 3; break;\
        case 12:\
            div = 4; break;\
        default:\
            break;\
    }

#define h_delay()       \
    do {                \
        int i = 1000;   \
        while (i--);     \
    }while(0)

#define WAIT_FOR_PLL_LOCKED(reg)                        \
    do {                                                \
        unsigned int st = 0, cnt = 10;                  \
        while (cnt --) {                                 \
            msleep_interruptible(10);                   \
            st = !!(aml_read_reg32(reg) & (1 << 31));   \
            if (st) {                                    \
                printk("hpll locked\n");                \
                break;                                  \
            }                                           \
            else {  /* reset pll */                     \
                printk("hpll reseting\n");              \
                aml_set_reg32_bits(reg, 0x5, 28, 3);    \
                aml_set_reg32_bits(reg, 0x4, 28, 3);    \
            }                                           \
        }                                               \
        if (cnt < 9)                                     \
            printk(KERN_CRIT "pll[0x%x] reset %d times\n", reg, 9 - cnt);\
    } while(0);

// viu_channel_sel: 1 or 2
// viu_type_sel: 0: 0=ENCL, 1=ENCI, 2=ENCP, 3=ENCT.
int set_viu_path(unsigned viu_channel_sel, viu_type_e viu_type_sel)
{
    if ((viu_channel_sel > 2) || (viu_channel_sel == 0))
        return -1;
    printk("VPU_VIU_VENC_MUX_CTRL: 0x%x\n", aml_read_reg32(P_VPU_VIU_VENC_MUX_CTRL));
    if (viu_channel_sel == 1) {
        aml_set_reg32_bits(P_VPU_VIU_VENC_MUX_CTRL, viu_type_sel, 0, 2);
        printk("viu chan = 1\n");
    }
    else {
        //viu_channel_sel ==2
        aml_set_reg32_bits(P_VPU_VIU_VENC_MUX_CTRL, viu_type_sel, 2, 2);
        printk("viu chan = 2\n");
    }
    printk("VPU_VIU_VENC_MUX_CTRL: 0x%x\n", aml_read_reg32(P_VPU_VIU_VENC_MUX_CTRL));
    return 0;
}

static void set_hdmitx_sys_clk(void)
{
    aml_set_reg32_bits(P_HHI_HDMI_CLK_CNTL, 0, 9, 3);
    aml_set_reg32_bits(P_HHI_HDMI_CLK_CNTL, 0, 0, 7);
    aml_set_reg32_bits(P_HHI_HDMI_CLK_CNTL, 1, 8, 1);
}

static unsigned int acq_val = 0;
static void set_hpll_clk_out(unsigned clk)
{
    int i = 0;

    for (i=0;i<10;i++) {
    check_clk_config(clk);
    printk("config HPLL = %d\n", clk);
    switch (clk) {
    case 2970:
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL, 0x5000023d);
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL2, 0);
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL3, 0x135c5091);
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL4, 0x801da72c);
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL5, 0x714869c0);    //5940 0x71c86900      // 0x71486900 2970
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL6, 0x00000e55);
            aml_write_reg32(P_HHI_HDMI_PLL_CNTL, 0x4000023d);
            printk("waiting HPLL lock\n");
            WAIT_FOR_PLL_LOCKED(P_HHI_HDMI_PLL_CNTL);
            aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 1, 14, 1); // div mode
            aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 0xe00, 0, 12); // div_frac
        break;
    case 4320:
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL2, 0);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL3, 0x0d5c5091);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL4, 0x801da72c);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL5, 0x714869c0);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL6, 0x00000e55);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL, 0x0000022d);
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL, 0x5, 28, 3);  //reset hpll
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL, 0x4, 28, 3);
        WAIT_FOR_PLL_LOCKED(P_HHI_HDMI_PLL_CNTL);
        break;
    case 2448:
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL2, 0);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL3, 0x0d5c5091);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL4, 0x801da72c);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL5, 0x714869c0);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL6, 0x00000e55);
        aml_write_reg32(P_HHI_HDMI_PLL_CNTL, 0x00000266);
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL, 0x5, 28, 3);  //reset hpll
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL, 0x4, 28, 3);
        WAIT_FOR_PLL_LOCKED(P_HHI_HDMI_PLL_CNTL);
        break;
    default:
        printk("error hpll clk: %d\n", clk);
        break;
    }

    // Step 1: close PVT_FIX_EN, enable ACQ
    mdelay(20);
    aml_write_reg32(P_HHI_HDMI_PLL_CNTL5, 0x754868c0);
    // Step 2: read ACQ
    mdelay(20);
    acq_val = (aml_read_reg32(P_HHI_HDMI_PLL_CNTL_I) >> 4) & 0xff;
    printk("acq_val1 = 0x%02x\n", acq_val);

    aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 1, 14, 1); // div mode
    mdelay(20);

    acq_val = (aml_read_reg32(P_HHI_HDMI_PLL_CNTL_I) >> 4) & 0xff;
    printk("acq_val2 = 0x%02x\n", acq_val);

    if (acq_val <= 0x8a) {
        //aml_write_reg32(P_HHI_HDMI_PLL_CNTL6, 0xe51);
        mdelay(20);
    } else {
        break;
    }
}
    printk("acq_val = 0x%02x, loop = %d.\n", acq_val, i);

/*
    acq_val = (aml_read_reg32(P_HHI_HDMI_PLL_CNTL_I) >> 4) & 0xff;
    printk("acq_val3 = 0x%02x\n", acq_val);
    if ((acq_val <= 0x8a) || (acq_val >= 0xf0)) {
        loop ++;
        if (loop > 5) {
            printk("pll set loop = %d, return\n", loop);
            return;
        } else
            set_hpll_clk_out(clk);
    } else {
        printk("config HPLL done\n");
        return ;
    }
*/
    printk("config HPLL done\n");
}

static void set_hpll_od1(unsigned div)
{
    switch (div) {
    case 1:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 0, 16, 2);
        break;
    case 2:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 1, 16, 2);
        break;
    case 4:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 2, 16, 2);
        break;
    case 8:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 3, 16, 2);
        break;
    default:
        break;
    }
}

static void set_hpll_od2(unsigned div)
{
    switch (div) {
    case 1:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 0, 22, 2);
        break;
    case 2:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 1, 22, 2);
        break;
    case 4:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 2, 22, 2);
        break;
    case 8:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 3, 22, 2);
        break;
    default:
        break;
    }
}

static void set_hpll_od3(unsigned div)
{
    switch (div) {
    case 1:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 0, 18, 2);
        break;
    case 2:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 1, 18, 2);
        break;
    case 4:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 2, 18, 2);
        break;
    case 8:
        aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2, 3, 18, 2);
        break;
    default:
        break;
    }
}

// --------------------------------------------------
//              clocks_set_vid_clk_div
// --------------------------------------------------
// wire            clk_final_en    = control[19];
// wire            clk_div1        = control[18];
// wire    [1:0]   clk_sel         = control[17:16];
// wire            set_preset      = control[15];
// wire    [14:0]  shift_preset    = control[14:0];
void clocks_set_vid_clk_div(int div_sel)
{
    int shift_val = 0;
    int shift_sel = 0;

    printk("%s[%d] div = %d\n", __func__, __LINE__, div_sel);
    // Disable the output clock
    aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 19, 1);
    aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 15, 1);

    switch (div_sel) {
    case CLK_UTIL_VID_PLL_DIV_1:      shift_val = 0xFFFF; shift_sel = 0; break;
    case CLK_UTIL_VID_PLL_DIV_2:      shift_val = 0x0aaa; shift_sel = 0; break;
    case CLK_UTIL_VID_PLL_DIV_3:      shift_val = 0x0db6; shift_sel = 0; break;
    case CLK_UTIL_VID_PLL_DIV_3p5:    shift_val = 0x36cc; shift_sel = 1; break;
    case CLK_UTIL_VID_PLL_DIV_3p75:   shift_val = 0x6666; shift_sel = 2; break;
    case CLK_UTIL_VID_PLL_DIV_4:      shift_val = 0x0ccc; shift_sel = 0; break;
    case CLK_UTIL_VID_PLL_DIV_5:      shift_val = 0x739c; shift_sel = 2; break;
    case CLK_UTIL_VID_PLL_DIV_6:      shift_val = 0x0e38; shift_sel = 0; break;
    case CLK_UTIL_VID_PLL_DIV_6p25:   shift_val = 0x0000; shift_sel = 3; break;
    case CLK_UTIL_VID_PLL_DIV_7:      shift_val = 0x3c78; shift_sel = 1; break;
    case CLK_UTIL_VID_PLL_DIV_7p5:    shift_val = 0x78f0; shift_sel = 2; break;
    case CLK_UTIL_VID_PLL_DIV_12:     shift_val = 0x0fc0; shift_sel = 0; break;
    case CLK_UTIL_VID_PLL_DIV_14:     shift_val = 0x3f80; shift_sel = 1; break;
    case CLK_UTIL_VID_PLL_DIV_15:     shift_val = 0x7f80; shift_sel = 2; break;
    case CLK_UTIL_VID_PLL_DIV_2p5:    shift_val = 0x5294; shift_sel = 2; break;
    default:
        printk("Error: clocks_set_vid_clk_div:  Invalid parameter\n");
        break;
    }

    if (shift_val == 0xffff ) {      // if divide by 1
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 1, 18, 1);
    } else {
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 16, 2);
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 15, 1);
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 0, 14);

        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, shift_sel, 16, 2);
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 1, 15, 1);
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, shift_val, 0, 15);
        aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 0, 15, 1);
    }
    // Enable the final output clock
    aml_set_reg32_bits(P_HHI_VID_PLL_CLK_DIV, 1, 19, 1);
}

static void set_vid_pll_div(unsigned div)
{
    clocks_set_vid_clk_div(div);
}

static void set_vid_clk_div(unsigned div)
{
    check_clk_config(div);
    if (div == 0)
        div = 1;
    aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 16, 3);   // select vid_pll_clk
    aml_set_reg32_bits(P_HHI_VID_CLK_DIV, div-1, 0, 8);
    aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 7, 0, 3);
}

static void set_hdmi_tx_pixel_div(unsigned div)
{
    check_div();
    aml_set_reg32_bits(P_HHI_HDMI_CLK_CNTL, div, 16, 4);
}
static void set_encp_div(unsigned div)
{
    check_div();
    aml_set_reg32_bits(P_HHI_VID_CLK_DIV, div, 24, 4);
    aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2, 1, 2, 1);   //enable gate
}

static void set_enci_div(unsigned div)
{
    check_div();
    aml_set_reg32_bits(P_HHI_VID_CLK_DIV, div, 28, 4);
    aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2, 1, 0, 1);   //enable gate
}

static void set_encl_div(unsigned div)
{
    check_div();
    aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, div, 12, 4);
    aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2, 1, 3, 1);   //enable gate
}

static void set_vdac0_div(unsigned div)
{
    check_div();
    aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, div, 28, 4); //???
    aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2, 1, 4, 1);   //enable gate
}

// mode viu_path viu_type hpll_clk_out od1 od2 od3
// vid_pll_div vid_clk_div hdmi_tx_pixel_div encp_div enci_div encl_div vdac0_div
static hw_enc_clk_val_t setting_enc_clk_val[] = {
    {VMODE_480I,           1, VIU_ENCI, 4320, 4, 4, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, -1, 2, -1, -1},
    {VMODE_480CVBS,        1, VIU_ENCI, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 22, 1, 1, 1, -1, 1},
    {VMODE_576I,           1, VIU_ENCI, 4320, 4, 4, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, -1, 2, -1, -1},
    {VMODE_576CVBS,        1, VIU_ENCI, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 22, 1, 1, 1, -1, 1},
    {VMODE_576P,           1, VIU_ENCP, 4320, 4, 4, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_480P,           1, VIU_ENCP, 4320, 4, 4, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_720P_50HZ,      1, VIU_ENCP, 2970, 4, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_720P,           1, VIU_ENCP, 2970, 4, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_1080I,          1, VIU_ENCP, 2970, 4, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_1080I_50HZ,     1, VIU_ENCP, 2970, 4, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_1080P,          1, VIU_ENCP, 2970, 1, 2, 2, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_1080P_50HZ,     1, VIU_ENCP, 2970, 1, 2, 2, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_1080P_24HZ,     1, VIU_ENCP, 2970, 2, 2, 2, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K2K_30HZ,      1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 2, 1, 1, -1, -1, -1},
    {VMODE_4K2K_25HZ,      1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 2, 1, 1, -1, -1, -1},
    {VMODE_4K2K_24HZ,      1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 2, 1, 1, -1, -1, -1},
    {VMODE_4K2K_SMPTE,     1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 2, 1, 1, -1, -1, -1},
    {VMODE_4K2K_60HZ_Y420, 1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_4K2K_60HZ,      1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K2K_50HZ_Y420, 1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_4K2K_50HZ,      1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K2K_FAKE_5G,   1, VIU_ENCP, 2448, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K2K_5G,        1, VIU_ENCP, 2448, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K1K_100HZ,     1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K1K_100HZ_Y420,1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_4K1K_120HZ,     1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K1K_120HZ_Y420,1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_4K05K_200HZ,    1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K05K_200HZ_Y420,1,VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
    {VMODE_4K05K_240HZ,    1, VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 1, 1, -1, -1, -1},
    {VMODE_4K05K_240HZ_Y420,1,VIU_ENCP, 2970, 1, 1, 1, CLK_UTIL_VID_PLL_DIV_5, 1, 2, 1, -1, -1, -1},
};

void set_vmode_clk(vmode_t mode)
{
    unsigned int val = 0;
    int i = 0;
    int j = 0;
    hw_enc_clk_val_t *p_enc =NULL;

    printk("set_vmode_clk mode is %d\n", mode);
    val = aml_read_reg32(P_HHI_HDMI_PLL_CNTL);
    if ((val >> 30) == 0x3) {
        switch (val & 0xffff) {
        case 0x23d:
            hpll_vco_clk = 2970;
            break;
        case 0x266:
            hpll_vco_clk = 2448;
            break;
        case 0x22d:
            hpll_vco_clk = 4320;
            break;
        default:
            hpll_vco_clk = 0xffff;
        }
    } else {
        hpll_vco_clk = 0xffff;
    }
    if (hpll_vco_clk != 0xffff) {
        printk("hpll already clk: %d\n", hpll_vco_clk);
    } else {
        printk("hpll already clk: -1\n");
    }

    p_enc=&setting_enc_clk_val[0];
    i = ARRAY_SIZE(setting_enc_clk_val);
    for (j = 0; j < i; j++) {
        if (mode == p_enc[j].mode)
            break;
    }
    if (j == i) {
        printk("set_vmode_clk: not valid mode %d\n", mode);
        return;
    }
    set_viu_path(p_enc[j].viu_path, p_enc[j].viu_type);
    set_hdmitx_sys_clk();
    if (hpll_vco_clk != p_enc[j].hpll_clk_out)
        set_hpll_clk_out(p_enc[j].hpll_clk_out);
    set_hpll_od1(p_enc[j].od1);
    set_hpll_od2(p_enc[j].od2);
    set_hpll_od3(p_enc[j].od3);
    set_vid_pll_div(p_enc[j].vid_pll_div);
    set_vid_clk_div(p_enc[j].vid_clk_div);
    set_hdmi_tx_pixel_div(p_enc[j].hdmi_tx_pixel_div);
    set_encp_div(p_enc[j].encp_div);
    set_enci_div(p_enc[j].enci_div);
    set_encl_div(p_enc[j].encl_div);
    set_vdac0_div(p_enc[j].vdac0_div);
}



static int sHDMI_DPLL_DATA[][3] = {
	//frequency(M)    HHI_HDMI_PLL_CNTL   HHI_HDMI_PLL_CNTL2: (bit18: OD1 is 1)
	{   399.840,         0x60000663,         0x00520f5b},
	{   378.000,         0x6000023e,         0x00920fff},
//	{  2079.000,         0x60000681,         0x00110eff},
	{  2079.000,         0x500404ad,         0x00414400},
	{   810.000,         0x60000886,         0x00120fff},
	{  1080.000,         0x6000022c,         0x00120fff},
	{  2227.500,         0x6000068b,         0x00110380},
	{  4455.000,         0x6000068b,         0x00100380},
	{  2970.000,         0x6000023d,         0x00110dff},
	{  5940.000,         0x6000023d,         0x00100dff},
	{   540.000,         0x6000022c,         0x00520fff},
	{   576.000,         0x6000022f,         0x00520fff},
	{   594.000,         0x60000462,         0x00520fff},
	{  1188.000,         0x60000462,         0x00120fff},
	{   742.500,         0x6000023d,         0x00520dff},
	{  1485.000,         0x6000023d,         0x00120dff},
	{   928.125,         0x60000674,         0x00120040},
	{  1856.250,         0x60000674,         0x00110040},
	{  1039.500,         0x60000681,         0x00120eff},
	{  2702.002,         0x60000470,         0x00110955},
	{   337.500,         0x600008e0,         0x00920fff},
	{   270.000,         0x6000022c,         0x00920fff},
	{        0,                   0,                  0}
};

int set_hdmi_dpll(int freq, int od1)
{
	int i;
	i=0;
	while (sHDMI_DPLL_DATA[i][0] != 0) {
		if (sHDMI_DPLL_DATA[i][0] == freq)
			break;
		i++;
	}

	if (sHDMI_DPLL_DATA[i][0] == 0)
		return 1;
	else {
		aml_write_reg32(P_HHI_HDMI_PLL_CNTL, sHDMI_DPLL_DATA[i][1]);
		aml_write_reg32(P_HHI_HDMI_PLL_CNTL2,sHDMI_DPLL_DATA[i][2]);
//		aml_set_reg32_bits(P_HHI_HDMI_PLL_CNTL2,od1,18,2);
		aml_write_reg32(P_HHI_HDMI_PLL_CNTL, sHDMI_DPLL_DATA[i][1] & (~(1<<28)));
	}

	printk("Wait 10us for phy_clk stable!\n");
	// delay 10uS to wait clock is stable
	udelay(10);

	return 0;
}

void set_crt_video_enc (int vIdx, int inSel, int DivN)
{
	if (vIdx == 0) //V1
	{
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0

		//delay 2uS
		udelay(2);

		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, inSel,   16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VID_CLK_DIV, (DivN-1), 0, 8); // [7:0]   - cntl_xd0

		// delay 5uS
		udelay(5);

		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0

	} else { //V2
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 0, 19, 1); //[19] -disable clk_div0

		//delay 2uS
		udelay(2);

		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, inSel,  16, 3); // [18:16] - cntl_clk_in_sel
		aml_set_reg32_bits(P_HHI_VIID_CLK_DIV, (DivN-1),0, 8); // [7:0]   - cntl_xd0

		// delay 5uS
		udelay(5);

		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL, 1, 19, 1); //[19] -enable clk_div0
	}

	//delay 5uS
	udelay(5);
}

void enable_crt_video_encl(int enable, int inSel)
{
	aml_set_reg32_bits(P_HHI_VIID_CLK_DIV,inSel,  12, 4); //encl_clk_sel:hi_viid_clk_div[15:12]

	if (inSel <= 4) //V1
		aml_set_reg32_bits(P_HHI_VID_CLK_CNTL,1, inSel, 1);
	else
		aml_set_reg32_bits(P_HHI_VIID_CLK_CNTL,1, (inSel-5),1);

	aml_set_reg32_bits(P_HHI_VID_CLK_CNTL2,enable, 3, 1); //gclk_encl_clk:hi_vid_clk_cntl2[3]

#ifndef NO_EDP_DSI
	aml_set_reg32_bits(P_VPU_MISC_CTRL, 1, 0, 1);    // vencl_clk_en_force: vpu_misc_ctrl[0]
#endif
}

