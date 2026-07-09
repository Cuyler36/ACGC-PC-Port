#include "libu64/gfxprint.h"

#include "libultra/libultra.h"
#include "libc64/aprintf.h"

u8 __gfxprint_default_flags;

static void gfxprint_setup(gfxprint_t* gprint) {
  int i;
  int tile;

  /* Initialize RDP & RSP settings */
  gDPPipeSync(gprint->glistp++);
  gDPSetOtherMode(gprint->glistp++,
                  G_AD_DISABLE | G_CD_DISABLE | G_CK_NONE | G_TC_FILT |
                      G_TF_BILERP | G_TT_IA16 | G_TL_TILE | G_TD_CLAMP |
                      G_TP_NONE | G_CYC_1CYCLE | G_PM_NPRIMITIVE,
                  G_RM_XLU_SURF | G_RM_XLU_SURF2 | G_ZS_PRIM);
  gDPSetCombineMode(gprint->glistp++, G_CC_DECALRGBA, G_CC_DECALRGBA);

  /* Initialize font texture */
  gDPLoadTextureBlock_4b(gprint->glistp++, gfxprint_font, G_IM_FMT_CI, 16, 256, 0,
                         G_TX_NOMIRROR | G_TX_CLAMP, G_TX_NOMIRROR | G_TX_CLAMP,
                         0, 0, 0, 0);

  /* Load font palette */
  gDPLoadTLUT_palX(gprint->glistp++, 0, gfxprint_moji_tlut, 64);

  /* Initialize tile settings */
  for (i = 0, tile = 1; i < 3; i++, tile++) {
    gDPSetTile(gprint->glistp++, G_IM_FMT_CI, G_IM_SIZ_4b,
               ((((16) >> 1) + 7) >> 3), 0, tile * 2, tile,
               G_TX_NOMIRROR | G_TX_CLAMP, 0, 0, G_TX_NOMIRROR | G_TX_CLAMP, 0,
               0);
    gDPSetTileSize(gprint->glistp++, tile * 2, 0, 0,
                   (16 - 1) << G_TEXTURE_IMAGE_FRAC,
                   (256 - 1) << G_TEXTURE_IMAGE_FRAC);
  }

  /* Initialize font color */
  gDPSetColor(gprint->glistp++, G_SETPRIMCOLOR, gprint->color.rgba8888);
}

extern void gfxprint_color(gfxprint_t* gprint, u32 r, u32 g, u32 b, u32 a) {
  gprint->color.c.r = r;
  gprint->color.c.g = g;
  gprint->color.c.b = b;
  gprint->color.c.a = a;

  gDPPipeSync(gprint->glistp++);
  gDPSetColor(gprint->glistp++, G_SETPRIMCOLOR, gprint->color.rgba8888);
}

extern void gfxprint_locate(gfxprint_t* gprint, int x, int y) {
  gprint->position_x = gprint->offset_x + x * 4;
  gprint->position_y = gprint->offset_y + y * 4;
}

extern void gfxprint_locate8x8(struct gfxprint_obj* gprint, int x, int y) {
  gfxprint_locate(gprint, x * 8, y * 8);
}

static void gfxprint_putc1(gfxprint_t* gprint, char c) {
  int tile;
  int x0;
  int x1;

  tile = (c & 3) * 2;
  x0 = (c & 4) * 2;
  x1 = c & 0xF8;

  if (gfxprint_isChanged(gprint)) {
    gfxprint_clrChanged(gprint);
    gDPPipeSync(gprint->glistp++);
    gDPSetTextureLUT(gprint->glistp++, G_TT_IA16);
    gDPSetCycleType(gprint->glistp++, G_CYC_1CYCLE);
    gDPSetRenderMode(gprint->glistp++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetCombineMode(gprint->glistp++, G_CC_MODULATEIDECALA_PRIM,
                      G_CC_MODULATEIDECALA_PRIM);
  }

  if (gfxprint_isShadow(gprint)) {
    gDPSetColor(gprint->glistp++, G_SETPRIMCOLOR, 0);
    if (gfxprint_isHighres(gprint)) {
      gSPTextureRectangle(
          gprint->glistp++, (gprint->position_x + 4) << 1,
          (gprint->position_y + 4) << 1, (gprint->position_x + 32) << 1,
          (gprint->position_y + 32) << 1, tile, x0 << 5, x1 << 5, 512, 512);
    } else {
      gSPTextureRectangle(gprint->glistp++, gprint->position_x + 4,
                          gprint->position_y + 4, gprint->position_x + 32,
                          gprint->position_y + 32, tile, x0 << 5, x1 << 5, 1024,
                          1024);
    }
    gDPSetColor(gprint->glistp++, G_SETPRIMCOLOR, gprint->color.rgba8888);
  }

  if (gfxprint_isHighres(gprint)) {
    gSPTextureRectangle(gprint->glistp++, gprint->position_x << 1,
                        gprint->position_y << 1, (gprint->position_x + 28) << 1,
                        (gprint->position_y + 28) << 1, tile, x0 << 5, x1 << 5,
                        512, 512);
  } else {
    gSPTextureRectangle(gprint->glistp++, gprint->position_x, gprint->position_y,
                        gprint->position_x + 28, gprint->position_y + 28, tile,
                        x0 << 5, x1 << 5, 1024, 1024);
  }

  gprint->position_x += 32;
}

extern void gfxprint_putc(gfxprint_t* gprint, char c) {
  u8 param = c;

  if (param == ' ') {
    gprint->position_x += 32;
  } else if (param > ' ' && param <= 0x7E) {
    gfxprint_putc1(gprint, param);
  } else if (param >= 0xA0 && param <= 0xDF) {
    if (gfxprint_isHiragana(gprint)) {
      if (param <= 0xBF) {
        param -= 0x20;
      } else {
        param += 0x20;
      }
    }

    gfxprint_putc1(gprint, param);
  } else {
    switch (param) {
      case '\0':
        break;
      case '\n':
        gprint->position_y += 32;
#pragma fallthrough
      case '\r':
        gprint->position_x = gprint->offset_x;
        break;
      case '\t':
        do {
          gfxprint_putc1(gprint, ' ');
        } while ((gprint->position_x - gprint->offset_x) % 256);
        break;
      case '\x8D':
        gfxprint_setHiragana(gprint);
        break;
      case '\x8C':
        gfxprint_setKatakana(gprint);
        break;
      case '\x8B':
        gfxprint_setGradient(gprint);
        gfxprint_setChanged(gprint);
        break;
      case '\x8A':
        gfxprint_clrGradient(gprint);
        gfxprint_setChanged(gprint);
        break;
      case '\x8E':
        break;
      default:
        break;
    }
  }
}

extern void gfxprint_write(gfxprint_t* gprint, const void* buffer, size_t size,
                           size_t n) {
  char* buf = (char*)buffer;
  size_t i;

  for (i = size * n; i != 0; i--) {
    gfxprint_putc(gprint, *buf++);
  }
}

static void* gfxprint_prout(void* gprint, const char* buffer, int n) {
  gfxprint_write((gfxprint_t*)gprint, buffer, sizeof(char), (size_t)n);
  return gprint;
}

extern void gfxprint_init(gfxprint_t* gprint) {
  gfxprint_clrOpened(gprint);
  gprint->prout_func = &gfxprint_prout;
  gprint->glistp = NULL;
  gprint->position_x = 0;
  gprint->position_y = 0;
  gprint->offset_x = 0;
  gprint->offset_y = 0;
  gprint->color.rgba8888 = 0;
  gfxprint_setKatakana(gprint);
  gfxprint_clrGradient(gprint);
  gfxprint_setShadow(gprint);
  gfxprint_setChanged(gprint);

  if ((__gfxprint_default_flags & GFXPRINT_FLAG_HIGHRES) != 0) {
    gfxprint_setHighres(gprint);
  } else {
    gfxprint_clrHighres(gprint);
  }
}

extern void gfxprint_cleanup(gfxprint_t* gprint) {}

extern void gfxprint_open(gfxprint_t* gprint, Gfx* glistp) {
  if (!gfxprint_isOpened(gprint)) {
    gfxprint_setOpened(gprint);
    gprint->glistp = glistp;
    gfxprint_setup(gprint);
  } else {
    osSyncPrintf("gfxprint_open:２重オープンです\n");
  }
}

extern Gfx* gfxprint_close(gfxprint_t* gprint) {
  Gfx* list;

  gfxprint_clrOpened(gprint);
  gDPPipeSync(gprint->glistp++);
  list = gprint->glistp;
  gprint->glistp = NULL;
  return list;
}

extern int gfxprint_vprintf(gfxprint_t* gprint, const char* fmt, va_list ap) {
  return vaprintf((aprout_func_t*)&gprint->prout_func, fmt, ap);
}

extern int gfxprint_printf(gfxprint_t* gprint, const char* fmt, ...) {
  int res;

  va_list ap;
  va_start(ap, fmt);
  res = gfxprint_vprintf(gprint, fmt, ap);
  va_end(ap);

  return res;
}
