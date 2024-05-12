#ifndef GR_h_
#define GR_h_

#define GR_X 320
#define GR_Y 200
#define GR_HEIGHT GR_Y
#define GR_WIDTH GR_X

#define C8888(r,g,b,a) ((b) | ((uint32_t)(g) << 8U) | ((uint32_t)(r) << 16U) | ((uint32_t)(a) << 24U))

void gr_fill(void *fb, uint32_t c);
void gr_fill_async(void *fb, uint32_t c);
void gr_fill_wait(void);
void gr_blit_A8_to_ARGB8888(void *fb, uint32_t x, uint32_t y, void *img, uint32_t img_w, uint32_t img_h, uint32_t c, uint32_t bg);
void gr_blit_RGB888_to_ARGB8888(void *fb, uint32_t x, uint32_t y, void *img, uint32_t img_w, uint32_t img_h);
uint32_t gr_get_pixel(void *fb, int16_t x, int16_t y);
void gr_draw_pixel(void *fb, int16_t x, int16_t y, uint32_t c);
void gr_draw_circle(void *fb, int16_t xx, int16_t yy, int16_t r, uint32_t color);
void gr_draw_vline_xyh(void *fb, int16_t x, int16_t y, int16_t h, uint32_t color);
void gr_draw_vline_xyy(void *fb, int16_t x, int16_t y0, int16_t y1, uint32_t color);
void gr_draw_hline_xyw(void *fb, int16_t x, int16_t y, int16_t w, uint32_t color);
void gr_draw_hline_xxy(void *fb, int16_t x0, int16_t x1, int16_t y, uint32_t color);
void gr_draw_line(void *fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);

#endif
