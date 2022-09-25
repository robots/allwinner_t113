#include "platform.h"
#include "gr.h"

#include <string.h>

void gr_fill(void *fb, uint32_t c)
{
	for (uint32_t y = 0; y < GR_Y; y++) {
		for (uint32_t x = 0; x < GR_X; x++) {
			gr_draw_pixel(fb, x, y, c);
		}
	}
}

uint32_t gr_get_pixel(void *fb, int16_t x, int16_t y)
{
	if (x < 0 || y < 0 || x >= GR_X || y >= GR_Y) return 0;

	return *(volatile uint32_t *)((uint32_t)fb + 4 * (y * GR_X + x));
}

void gr_draw_pixel(void *fb, int16_t x, int16_t y, uint32_t c)
{
	if (x < 0 || y < 0 || x >= GR_X || y >= GR_Y) return;

	//uint32_t *p = (uint32_t)fb + 4 * ((uint32_t)y * GR_X + (uint32_t)x);
	//uart_printf("pixel at x,y = %d,%d %08x - %08x -> %08x\n\r",x ,y , p, *p, c);
	//for (volatile unsigned int i = 0; i < 100000; i ++);
	//memcpy(p, &c, 4);
	*(volatile uint32_t *)((uint32_t)fb + 4 * (y * GR_X + x)) = c;
}

void gr_draw_circle(void *fb, int16_t xx, int16_t yy, int16_t r, uint32_t color)
{ 
	int x, y, error;

	if (r == 0) {
		gr_draw_pixel(fb, xx, yy, color);
		return;
	}

	/* Clip negative r */
	r = r < 0 ? -r : r;

	for (x = 0, error = -r, y = r; y >= 0; y--) {
		/* Iterate X until we can pass to the next line. */
		while (error < 0) {
			error += 2*x + 1;
			x++;
			gr_draw_pixel(fb, xx-x+1, yy-y, color);
			gr_draw_pixel(fb, xx+x-1, yy-y, color);
			gr_draw_pixel(fb, xx-x+1, yy+y, color);
			gr_draw_pixel(fb, xx+x-1, yy+y, color);
		}

		/* Enough changes accumulated, go to next line. */
		error += -2*y + 1;
		gr_draw_pixel(fb, xx-x+1, yy-y, color);
		gr_draw_pixel(fb, xx+x-1, yy-y, color);
		gr_draw_pixel(fb, xx-x+1, yy+y, color);
		gr_draw_pixel(fb, xx+x-1, yy+y, color);
	}
}

void gr_draw_vline_xyh(void *fb, int16_t x, int16_t y, int16_t h, uint32_t color)
{
	if ((x >= GR_WIDTH) || (y >= GR_HEIGHT)) return;
	if ((y+h-1) >= GR_HEIGHT) h = GR_HEIGHT - y;

	for (int i = 0; i < h; i++) {
		gr_draw_pixel(fb, x, y+i, color);
	}
}

void gr_draw_vline_xyy(void *fb, int16_t x, int16_t y0, int16_t y1, uint32_t color)
{
	int16_t h;

	if ((x >= GR_WIDTH) || (y0 >= GR_HEIGHT)) return;
	if ((y1) >= GR_HEIGHT) y1 = GR_HEIGHT-1;

	h = MAX(y0, y1) - MIN(y0, y1);

	gr_draw_vline_xyh(fb, x, MIN(y0, y1), h, color);
}

void gr_draw_hline_xyw(void *fb, int16_t x, int16_t y, int16_t w, uint32_t color)
{
	if ((x >= GR_WIDTH) || (y >= GR_HEIGHT)) return;
	if ((x + w - 1) >= GR_WIDTH)  w = GR_WIDTH - x;

	for (int i = 0; i < w; i++) {
		gr_draw_pixel(fb, x+i, y, color);
	}
}

void gr_draw_hline_xxy(void *fb, int16_t x0, int16_t x1, int16_t y, uint32_t color)
{
	int16_t w;

	// Rudimentary clipping
	if ((x0 >= GR_WIDTH) || (y >= GR_HEIGHT)) return;
	if (x1 >= GR_WIDTH)  x1 = GR_WIDTH-1;

	w = MAX(x0, x1) - MIN(x0, x1) + 1;

	gr_draw_hline_xyw(fb, MIN(x0, x1), y, w, color);
}


void gr_draw_line(void *fb, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color)
{
	if (x0 == x1) {
		if (y0 == y1) {
			gr_draw_pixel(fb, x0, y0, color);
			return;
		}
		gr_draw_vline_xyy(fb, x0, y0, y1, color);
		return;
	}
	if (y0 == y1) {
		gr_draw_hline_xxy(fb, x0, x1, y0, color);
		return;
	}

	int steep = ABS(y1 - y0) / ABS(x1 - x0);
	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}
	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	int deltax = x1 - x0;
	int deltay = ABS(y1 - y0);

	int error = deltax / 2;

	int y = y0, x;
	int ystep = (y0 < y1) ? 1 : -1;
	for (x = x0; x <= x1; x++) {

		if (steep) {
			gr_draw_pixel(fb, y, x, color);
		} else {
			gr_draw_pixel(fb, x, y, color);
		}

		error -= deltay;
		if (error < 0) {
			y += ystep;
			error += deltax;
		}
	}
}
