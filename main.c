#include <SDL/SDL.h>
#include <kos.h>
#include <stdint.h>
#include <stdio.h>
#include "display.h"
#include "vector.h"

bool is_running = false;
SDL_Event event;

/* Tell KOS to initialize the hardware with default settings */
KOS_INIT_FLAGS(INIT_DEFAULT);

void setup(void) {
  color_buffer =
      (uint32_t *)malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(uint32_t));
  // Create an SDL 1.2 surface that wraps around your color_buffer array
  color_buffer_surface = SDL_CreateRGBSurfaceFrom(
      (void *)color_buffer, SCREEN_WIDTH, SCREEN_HEIGHT,
      32,                              // bits per pixel
      SCREEN_WIDTH * sizeof(uint32_t), // pitch (bytes per row)
      0x00FF0000,                      // R mask
      0x0000FF00,                      // G mask
      0x000000FF,                      // B mask
      0xFF000000                       // A mask
  );
}

void process_input(void) {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      is_running = false;
    }
    if (event.type == SDL_JOYBUTTONDOWN) {
      // If Start button is pressed (button index 3), exit back to console
      if (event.jbutton.button == 3) {
        is_running = false;
      }
    }
  }
}
void update(void) {
  // TODO: Update game logic here
}

void render(void) {

  draw_grid();

  draw_pixel(20, 20, 0xFF00FF00);

  draw_rect(300, 200, 300, 150, 0xFFFF00FF);
  render_color_buffer();

  // Clear the screen with black (e.g. 0xFF000000)
  // Format is AARRGGBB
  clear_color_buffer(0xFF000000);

  // Flip buffers to display the rendered frame
  SDL_Flip(screen);
}

int main(int argc, char **argv) {

  // 1. Initialize SDL
  is_running = initialize_window();

  // 2. Setup your color buffer & off-screen surface
  setup();

  while (is_running) {
    // Handle events
    process_input();
    // Update game logic
    update();
    // Render the frame
    render();
  }

  // Cleanup and exit gracefully
  destroy_window();
  return 0;
}
