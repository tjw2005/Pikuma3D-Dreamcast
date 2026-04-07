#include "display.h"
#include <stdio.h>

SDL_Surface *screen = NULL;
SDL_Surface *color_buffer_surface = NULL;
uint32_t *color_buffer = NULL;
SDL_Joystick *joystick = NULL;

bool initialize_window(void) {
  // Initialize SDL Video and Joystick APIs
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }
  // Set the video mode (640x480, 16-bit, double buffered)
  screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16,
                            SDL_HWSURFACE | SDL_DOUBLEBUF);
  if (screen == NULL) {
    printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
    return false;
  }
  if (!screen) {
    fprintf(stderr, "Error creating SDL window.\n");
    return false;
  }

  // Open the first controller
  joystick = SDL_JoystickOpen(0);
  if (!joystick) {
    printf("SDL_JoystickOpen failed: %s\n", SDL_GetError());
    return false;
  }

  // Enable Joystick Events in the SDL Event Queue (Required for PollEvent)
  SDL_JoystickEventState(SDL_ENABLE);

  return true;
}

void clear_color_buffer(uint32_t color) {
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      color_buffer[(y * SCREEN_WIDTH) + x] = color;
    }
  }
}

void draw_grid(void) {
  for (int y = 0; y < SCREEN_HEIGHT; y += 10) {
    for (int x = 0; x < SCREEN_WIDTH; x += 10) {
      color_buffer[(y * SCREEN_WIDTH) + x] = 0xFFFF0000;
    }
  }
}

void draw_pixel(int x, int y, uint32_t color) {
  if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
    color_buffer[(y * SCREEN_WIDTH) + x] = color;
  }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      int current_x = x + i;
      int current_y = y + j;
      draw_pixel(current_x, current_y, color);
    }
  }
}

void render_color_buffer(void) {
  SDL_BlitSurface(color_buffer_surface, NULL, screen, NULL);
}

void destroy_window(void) {
  SDL_FreeSurface(color_buffer_surface);
  free(color_buffer);
  SDL_Quit();
}
