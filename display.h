#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL/SDL.h>
#include <stdint.h>
#include <stdbool.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

extern SDL_Surface *screen;
extern SDL_Surface *color_buffer_surface;
extern uint32_t *color_buffer;

bool initialize_window(void);
void draw_grid(void);
void draw_pixel(int x, int y, uint32_t color);  
void draw_rect(int x, int y, int width, int height, uint32_t color);
void render_color_buffer(void);
void clear_color_buffer(uint32_t color);
void destroy_window(void);

#endif