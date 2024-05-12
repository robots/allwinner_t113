#ifndef _DE_PRIV_h_
#define _DE_PRIV_h_

#include "platform.h"

#define N2_POWER(a, power)      (((unsigned long long)a)<<power)

/* GSU configuration */
#define GSU_PHASE_NUM            16
/* bit19 to bit2 is fraction part */
#define GSU_PHASE_FRAC_BITWIDTH  18
/* bit19 to bit2 is fraction part, and bit1 to bit0 is void */
#define GSU_PHASE_FRAC_REG_SHIFT 2
/* frame buffer information fraction part bit width */
#define GSU_FB_FRAC_BITWIDTH     32


typedef struct {
	__IO uint32_t SCLK_GATE;
	__IO uint32_t HCLK_GATE;
	__IO uint32_t AHB_RESET;
	__IO uint32_t SCLK_DIV;
	__IO uint32_t DE2TCON_MUX;
} DE_Top_TypeDef;


typedef struct {
	__IO uint32_t CTL;
	__IO uint32_t STS;
	__IO uint32_t DBUFFER;
	__IO uint32_t SIZE;
} DE_MUX_GLB_TypeDef;

typedef struct {
	__IO uint32_t FILLCOLOR_CTL;
	struct {
		__IO uint32_t FILL_COLOR;
		__IO uint32_t CH_ISIZE;
		__IO uint32_t CH_OFFSET;
		__IO uint32_t dummy;
	} PIPE[5];
	__IO uint32_t res1[11];
	__IO uint32_t CH_RTCTL;
	__IO uint32_t PREMUL_CTL;
	__IO uint32_t BK_COLOR;
	__IO uint32_t SIZE;
	__IO uint32_t CTL[4];
	__IO uint32_t res2[4];
	__IO uint32_t KEY_CTL;
	__IO uint32_t KEY_CON;
	__IO uint32_t res3[2];
	__IO uint32_t KEY_MAX[4];
	__IO uint32_t res4[4];
	__IO uint32_t KEY_MIN[4];
	__IO uint32_t res5[4];
	__IO uint32_t OUT_COLOR;
} DE_MUX_BLD_TypeDef;


typedef struct {
	struct {
		__IO uint32_t ATTCTL;
		__IO uint32_t MBSIZE;
		__IO uint32_t COOR;
		__IO uint32_t PITCH0;
		__IO uint32_t PITCH1;
		__IO uint32_t PITCH2;
		__IO uint32_t TOP_LADD0;
		__IO uint32_t TOP_LADD1;
		__IO uint32_t TOP_LADD2;
		__IO uint32_t BOT_LADD0;
		__IO uint32_t BOT_LADD1;
		__IO uint32_t BOT_LADD2;
	} LAYER[4];
	__IO uint32_t FILL_COLOR[4];
	__IO uint32_t TOP_HADD0;
	__IO uint32_t TOP_HADD1;
	__IO uint32_t TOP_HADD2;
	__IO uint32_t BOT_HADD0;
	__IO uint32_t BOT_HADD1;
	__IO uint32_t BOT_HADD2;
	__IO uint32_t SIZE;
	__IO uint32_t res;
	__IO uint32_t HDS_CTL0;
	__IO uint32_t HDS_CTL1;
	__IO uint32_t VDS_CTL0;
	__IO uint32_t VDS_CTL1;
} DE_MUX_OVL_V_TypeDef;

typedef struct {
	struct {
		__IO uint32_t ATTCTL;
		__IO uint32_t MBSIZE;
		__IO uint32_t COOR;
		__IO uint32_t PITCH;
		__IO uint32_t TOP_LADD;
		__IO uint32_t BOT_LADD;
		__IO uint32_t FILL_COLOR;
		__IO uint32_t res;
	} LAYER[4];
	__IO uint32_t TOP_HADD;
	__IO uint32_t BOT_HADD;
	__IO uint32_t SIZE;
} DE_MUX_OVL_UI_TypeDef;

typedef struct {
	__IO uint32_t CTRL_REG;
	__IO uint32_t res0;
	__IO uint32_t STATUS_REG;
	__IO uint32_t FIELD_CTRL_REG;
	__IO uint32_t BIST_REG;
	__IO uint32_t res1[11];
	__IO uint32_t OUTSIZE_REG;
	__IO uint32_t res2[15];
	__IO uint32_t INSIZE_REG;
	__IO uint32_t res3;
	__IO uint32_t HSTEP_REG;
	__IO uint32_t VSTEP_REG;
	__IO uint32_t HPHASE_REG;
	__IO uint32_t res4;
	__IO uint32_t VPHASE0_REG;
	__IO uint32_t VPHASE1_REG;
	__IO uint32_t res5[88];
	__IO uint32_t HCOEF_REG[16];
} DE_GSU_TypeDef;

// adaptive saturation enhancement
typedef struct {
	__IO uint32_t CTRL_REG;
	__IO uint32_t SIZE_REG;
	__IO uint32_t WIN0_REG;
	__IO uint32_t WIN1_REG;
	__IO uint32_t WEIGHT;
} DE_ASE_TypeDef;

// black and white stretch
typedef struct {
	__IO uint32_t GCTRL_REG;
	__IO uint32_t SIZE_REG;
	__IO uint32_t WIN0_REG;
	__IO uint32_t WIN1_REG;
	__IO uint32_t resv[4];
	__IO uint32_t THR0_REG;
	__IO uint32_t THR1_REG;
	__IO uint32_t SLP0_REG;
	__IO uint32_t SLP1_REG;
} DE_BWS_TypeDef;

// csc(?)
typedef struct {
	__IO uint32_t BYPASS_REG;
	__IO uint32_t resv[3];
	__IO uint32_t COEFF0_REG[3];
	__IO uint32_t CONST0_REG;
	__IO uint32_t COEFF1_REG[3];
	__IO uint32_t CONST1_REG;
	__IO uint32_t COEFF2_REG[3];
	__IO uint32_t CONST2_REG;
	__IO uint32_t GLB_ALPHA_REG;
} DE_CSC_TypeDef;

// dynamic range control
typedef struct {
	__IO uint32_t GNECTL_REG;
	__IO uint32_t SIZE_REG;
	__IO uint32_t CTL_REG;
	__IO uint32_t SET_REG;
	__IO uint32_t WP0_REG;
	__IO uint32_t WP1_REG;
	// TODO: much more
} DE_DRC_TypeDef;

// fancy color curvature change
typedef struct {
	__IO uint32_t FCC_CTL_REG;
	__IO uint32_t FCC_INPUT_SIZE_REG;
	__IO uint32_t FCC_OUTPUT_WIN0_REG;
	__IO uint32_t FCC_OUTPUT_WIN1_REG;
	__IO uint32_t FCC_HUE_RANGE_REG[6];
	__IO uint32_t res1[2];
	__IO uint32_t FCC_LOCAL_GAIN_REG[6];
	// TODO: much more
} DE_FCC_TypeDef;

// fresh and contrast enhancement
typedef struct {
	__IO uint32_t GCTRL_REG;
	__IO uint32_t FCE_SIZE_REG;
	__IO uint32_t FCE_WIN0_REG;
	__IO uint32_t FCE_WIN1_REG;
	__IO uint32_t LCE_GAIN_REG;
	// TODO: much more
} DE_FCE_TypeDef;

typedef struct {
	__IO uint32_t CTRL_REG;
	__IO uint32_t STATUS_REG;
	__IO uint32_t FIELD_CTRL_REG;
	__IO uint32_t BIST_REG;
	__IO uint32_t res1[11];
	__IO uint32_t OUTSIZE_REG;
	// TODO: much more
} DE_UIS_TypeDef;

// luminance transient improvement
typedef struct {
	__IO uint32_t EN;
	// TODO: much more
} DE_LTI_TypeDef;

// luma peaking
typedef struct {
	__IO uint32_t CTRL_REG;
	// TODO: much more
} DE_PEAK_TypeDef;

// video scaler
typedef struct {
	__IO uint32_t CTRL_REG;
	// TODO: much more
} DE_VSU_TypeDef;

// copy and rotation
typedef struct {
	__IO uint32_t GLB_CTL;
	// TODO: much more
} DE_CRS_TypeDef;

// realtime writeback
typedef struct {
	__IO uint32_t GCTRL_REG;
	// TODO: much more
} DE_RTWB_TypeDef;

typedef enum {
	LAY_FBFMT_ARGB_8888,
	LAY_FBFMT_ABGR_8888,
	LAY_FBFMT_RGBA_8888,
	LAY_FBFMT_BGRA_8888,
	LAY_FBFMT_XRGB_8888,
	LAY_FBFMT_XBRG_8888,
	LAY_FBFMT_RGBX_8888,
	LAY_FBFMT_BGRX_8888,
	LAY_FBFMT_RGB_888,
	LAY_FBFMT_BGR_888,
	LAY_FBFMT_RGB_565,
	LAY_FBFMT_BGR_565,
	LAY_FBFMT_ARGB_4444,
	LAY_FBFMT_ABGR_4444,
	LAY_FBFMT_RGBA_4444,
	LAY_FBFMT_BGRA_4444,
	LAY_FBFMT_ARGB_1555,
	LAY_FBFMT_ABGR_1555,
	LAY_FBFMT_RGBA_5551,
	LAY_FBFMT_BGRA_5551,
	LAY_FBFMT_last,
} LAY_FBFMT;
#define DE_TOP_BASE DE_BASE
#define DE_MUX_BASE (DE_BASE + 0x100000)

#define DE_MUX_GLB_BASE      (DE_MUX_BASE)
#define DE_MUX_BLD_BASE      (DE_MUX_BASE + 0x1000 + 0 * 0x1000)
#define DE_MUX_OVL_V_BASE    (DE_MUX_BASE + 0x1000 + 1 * 0x1000)
#define DE_MUX_OVL_UI1_BASE  (DE_MUX_BASE + 0x1000 + 2 * 0x1000)
#define DE_MUX_OVL_UI2_BASE  (DE_MUX_BASE + 0x1000 + 3 * 0x1000)
#define DE_MUX_OVL_UI3_BASE  (DE_MUX_BASE + 0x1000 + 4 * 0x1000)
#define DE_MUX_VSU_BASE      (DE_MUX_BASE + 0x20000)
#define DE_MUX_GSU1_BASE     (DE_MUX_BASE + 0x40000)
#define DE_MUX_GSU2_BASE     (DE_MUX_BASE + 0x50000)
#define DE_MUX_GSU3_BASE     (DE_MUX_BASE + 0x60000)

#define DE_MUX_FCE_BASE      (DE_MUX_BASE + 0xA0000)
#define DE_MUX_BWS_BASE      (DE_MUX_BASE + 0xA2000)
#define DE_MUX_LTI_BASE      (DE_MUX_BASE + 0xA4000)
#define DE_MUX_PEAK_BASE     (DE_MUX_BASE + 0xA6000)
#define DE_MUX_ASE_BASE      (DE_MUX_BASE + 0xA8000)
#define DE_MUX_FCC_BASE      (DE_MUX_BASE + 0xAA000)
#define DE_MUX_DCSC_BASE     (DE_MUX_BASE + 0xB0000)

#define DE_TOP               ((DE_Top_TypeDef *)DE_BASE)
#define DE_MUX_GLB           ((DE_MUX_GLB_TypeDef *)DE_MUX_GLB_BASE)
#define DE_MUX_BLD           ((DE_MUX_BLD_TypeDef *)DE_MUX_BLD_BASE)
#define DE_MUX_OVL_V         ((DE_MUX_OVL_V_TypeDef *)DE_MUX_OVL_V_BASE)
#define DE_MUX_OVL_UI1       ((DE_MUX_OVL_UI_TypeDef *)DE_MUX_OVL_UI1_BASE)
#define DE_MUX_OVL_UI2       ((DE_MUX_OVL_UI_TypeDef *)DE_MUX_OVL_UI2_BASE)
#define DE_MUX_OVL_UI3       ((DE_MUX_OVL_UI_TypeDef *)DE_MUX_OVL_UI3_BASE)
#define DE_MUX_VSU           ((DE_VSU_TypeDef *)DE_MUX_VSU_BASE)
#define DE_MUX_GSU1          ((DE_GSU_TypeDef *)DE_MUX_GSU1_BASE)
#define DE_MUX_GSU2          ((DE_GSU_TypeDef *)DE_MUX_GSU2_BASE)
#define DE_MUX_GSU3          ((DE_GSU_TypeDef *)DE_MUX_GSU3_BASE)
#define DE_MUX_FCE           ((DE_FCE_TypeDef *)DE_MUX_FCE_BASE)
#define DE_MUX_BWS           ((DE_BWS_TypeDef *)DE_MUX_BWS_BASE)
#define DE_MUX_LTI           ((DE_LTI_TypeDef *)DE_MUX_LTI_BASE)
#define DE_MUX_PEAK          ((DE_PEAK_TypeDef *)DE_MUX_PEAK_BASE)
#define DE_MUX_ASE           ((DE_ASE_TypeDef *)DE_MUX_ASE_BASE)
#define DE_MUX_FCC           ((DE_FCC_TypeDef *)DE_MUX_FCC_BASE)
#define DE_MUX_DCSC          ((DE_CSC_TypeDef *)DE_MUX_DCSC_BASE)

#endif
