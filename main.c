#include "display.h"
#include "vector.h"
#include <SDL/SDL.h>
#include <kos.h>
#include <stdint.h>
#include <stdio.h>

// Declare an array of 3D vectors to store the points of the cube
#define N_POINTS (9 * 9 * 9)
// Original 3D points
vec3_t cube_points[N_POINTS];
// Projected 2D points
vec2_t projected_points[N_POINTS];

vec3_t camera_position = {.x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};

// Field of View
float fov_factor = 640;

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

  int point_count = 0;

  // Initialize the cube points
  // From -1 to 1 (in this 9x9x9 cube)
  for (float x = -1; x <= 1; x += 0.25) {
    for (float y = -1; y <= 1; y += 0.25) {
      for (float z = -1; z <= 1; z += 0.25) {
        vec3_t new_point = {.x = x, .y = y, .z = z};
        cube_points[point_count++] = new_point;
      }
    }
  }
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

// Function recieves a 3D point and returns a projected 2D point
vec2_t project(vec3_t point) {
  vec2_t projected_point;
  projected_point.x = (fov_factor * point.x) / point.z;
  projected_point.y = (fov_factor * point.y) / point.z;
  return projected_point;
}

void update(void) {
  // Rotate the cube
  cube_rotation.x += 0.01;
  cube_rotation.y += 0.01;
  cube_rotation.z += 0.01;

  for (int i = 0; i < N_POINTS; i++) {
    vec3_t point = cube_points[i];

    vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
    transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
    transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

    // translate the points away from the camera
    transformed_point.z -= camera_position.z;

    // Project the curent 3D point to a 2D point
    vec2_t projected_point = project(transformed_point);

    // Store the projected 2D point in the projected_points array
    projected_points[i] = projected_point;
  }
}

void render(void) {
  draw_grid();

  // Loop all projected points and render them
  for (int i = 0; i < N_POINTS; i++) {
    vec2_t projected_point = projected_points[i];
    draw_rect(projected_point.x + SCREEN_WIDTH / 2,
              projected_point.y + SCREEN_HEIGHT / 2, 4, 4, 0xFFFFFF00);
  }

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
