#include "lvgl.h"

/* Three podium steps with 2, 1, and 3 cut out of the 24 x 18 bitmap. */
static const uint8_t podium_bitmap[] = {
    0x00, 0x00, 0x00,
    0x00, 0xff, 0x00,
    0x00, 0xff, 0x00,
    0x00, 0xff, 0x00,
    0x00, 0xff, 0x00,
    0x00, 0xef, 0x00,
    0xfe, 0xcf, 0x00,
    0xfe, 0xef, 0x00,
    0xfe, 0xef, 0x00,
    0xc6, 0xc7, 0x7f,
    0xf6, 0xff, 0x7f,
    0xc6, 0xff, 0x63,
    0xde, 0xff, 0x7b,
    0xc6, 0xff, 0x63,
    0xfe, 0xff, 0x7b,
    0xfe, 0xff, 0x63,
    0xfe, 0xff, 0x7f,
    0xff, 0xff, 0xff,
};

static const lv_font_fmt_txt_glyph_dsc_t podium_glyphs[] = {
    {0},
    {.adv_w = 24 * 16, .box_w = 24, .box_h = 18},
};

static const lv_font_fmt_txt_cmap_t podium_cmap[] = {
    {
        .range_start = 0xE000,
        .range_length = 1,
        .glyph_id_start = 1,
        .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    },
};

static const lv_font_fmt_txt_dsc_t podium_font_data = {
    .glyph_bitmap = podium_bitmap,
    .glyph_dsc = podium_glyphs,
    .cmaps = podium_cmap,
    .cmap_num = 1,
    .bpp = 1,
    .bitmap_format = LV_FONT_FMT_TXT_PLAIN,
};

const lv_font_t scoreboard_podium_24 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,
    .line_height = 18,
    .dsc = &podium_font_data,
};
