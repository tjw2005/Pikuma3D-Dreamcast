#include <SDL/SDL.h>
#include <kos.h>
#include <stdint.h>
#include <stdio.h>

bool running = false;
SDL_Surface *screen;
SDL_Event event;
SDL_Joystick *joystick;

uint32_t *color_buffer = NULL;
SDL_Surface *color_buffer_surface = NULL;

int window_width = 640;
int window_height = 480;

/* Tell KOS to initialize the hardware with default settings */
KOS_INIT_FLAGS(INIT_DEFAULT);

bool initialize_window(void) {
  // Initialize SDL Video and Joystick APIs
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
    printf("SDL_Init failed: %s\n", SDL_GetError());
    return false;
  }
  // Set the video mode (640x480, 16-bit, double buffered)
  screen = SDL_SetVideoMode(window_width, window_height, 16,
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

void setup(void) {
  color_buffer =
      (uint32_t *)malloc(window_width * window_height * sizeof(uint32_t));
  // Create an SDL 1.2 surface that wraps around your color_buffer array
  color_buffer_surface = SDL_CreateRGBSurfaceFrom(
      (void *)color_buffer, window_width, window_height,
      32,                              // bits per pixel
      window_width * sizeof(uint32_t), // pitch (bytes per row)
      0x00FF0000,                      // R mask
      0x0000FF00,                      // G mask
      0x000000FF,                      // B mask
      0xFF000000                       // A mask
  );
}

void clear_color_buffer(uint32_t color) {
  for (int y = 0; y < window_height; y++) {
    for (int x = 0; x < window_width; x++) {
      color_buffer[(y * window_width) + x] = color;
    }
  }
}

void update(void) {
  // TODO: Update game logic here
}

void process_input(void) {
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = 0;
    }
    if (event.type == SDL_JOYBUTTONDOWN) {
      // If Start button is pressed (button index 3), exit back to console
      if (event.jbutton.button == 3) {
        running = 0;
      }
    }
  }
}

void draw_grid(void) {
  for (int y = 0; y < window_height; y += 10) {
    for (int x = 0; x < window_width; x += 10) {
      color_buffer[(y * window_width) + x] = 0xFFFF0000;
    }
  }
}

void render_color_buffer(void) {
  SDL_BlitSurface(color_buffer_surface, NULL, screen, NULL);
}

void render(void) {

  draw_grid();
  render_color_buffer();

  // Clear the screen with black (e.g. 0xFF000000)
  // Format is AARRGGBB
  clear_color_buffer(0xFF000000);

  // Flip buffers to display the rendered frame
  SDL_Flip(screen);
}

void cleanup(void) {
  SDL_FreeSurface(color_buffer_surface);
  free(color_buffer);
  SDL_Quit();
}

int main(int argc, char **argv) {

  // 1. Initialize SDL
  running = initialize_window();

  // 2. Setup your color buffer & off-screen surface
  setup();

  while (running) {
    // Handle events
    process_input();
    update();
    render();
  }

  // Cleanup and exit gracefully
  cleanup();
  return 0;
}
