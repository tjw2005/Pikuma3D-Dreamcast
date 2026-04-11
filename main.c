#include <dc/matrix.h> // Include KOS hardware matrix mathematics
#include <kos.h>

/* Tell KOS to initialize the hardware with default settings */
KOS_INIT_FLAGS(INIT_DEFAULT);

// A simple structure to hold our 3D point
typedef struct {
  float x;
  float y;
  float z;
} vertex3_t;

// Helper function to process a 3D point through the SH4 hardware math unit,
// and output a formatted pvr_vertex_t ready for drawing.
pvr_vertex_t submit_vertex3d(vertex3_t p, float u, float v, uint32_t color, bool is_end_of_list) {
  pvr_vertex_t vert;

  // Copy the input point into temporary variables
  float tx = p.x;
  float ty = p.y;
  float tz = p.z;

  // 1. HARDWARE MATH: This single KOS macro applies the currently active matrix
  //    (which holds our camera and rotations) to our temporary coordinates.
  //    It does NOT apply perspective!
  mat_trans_single3_nodiv(tx, ty, tz);

  // 2. MANUAL PERSPECTIVE DIVIDE & SCREEN SCALE:
  // We simulate a camera lens by dividing X and Y by Z (things further away get
  // squished smaller). The "256.0f" acts as our Field of View (zoom
  // multiplier).
  vert.x = (tx * 256.0f) / tz + 320.0f;
  vert.y = -(ty * 256.0f) / tz + 240.0f;

  // Z-depth tells the GPU how to order objects (things closer to 0 are hidden
  // behind others). KOS requires '1 / Z' depth buffering for hardware
  // efficiency.
  vert.z = 1.0f / tz;

  vert.u = u;
  vert.v = v;
  vert.argb = color;
  vert.oargb = 0;
  vert.flags = is_end_of_list ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;

  // Submit right to the PVR hardware
  pvr_prim(&vert, sizeof(vert));
  return vert;
}

int main(int argc, char **argv) {
  // Initialize the PowerVR GPU with default settings
  pvr_init_defaults();

  // Load our raw texture into VRAM
  pvr_ptr_t texture_ptr = pvr_mem_malloc(256 * 256 * 2);
  file_t f = fs_open("/rd/cube_texture.raw", O_RDONLY);
  if (f != FILEHND_INVALID) {
      fs_read(f, texture_ptr, 256 * 256 * 2);
      fs_close(f);
  }

  // Main loop
  float r_angle = 0.0f;
  float pos_x = 0.0f;
  float pos_y = 0.0f;
  float pos_z = 10.0f;

  while (1) {
    maple_device_t *cont;
    cont_state_t *state;

    // Check for controller input
    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (cont) {
      state = (cont_state_t *)maple_dev_status(cont);
      if (state) {
        // Press Start to exit back to the console
        if (state->buttons & CONT_START) {
          break;
        }

        // Analog stick movement (with a deadzone of +/- 10)
        // KallistiOS joyx/joyy range from -128 to 127
        if (state->joyx > 10 || state->joyx < -10) {
            pos_x += (float)state->joyx * 0.001f;
        }
        if (state->joyy > 10 || state->joyy < -10) {
            pos_y -= (float)state->joyy * 0.001f; // joyy is negative when pushed UP
        }

        // Triggers for Z-depth
        // rtrig/ltrig range from 0 to 255
        if (state->rtrig > 10) {
            pos_z -= (float)state->rtrig * 0.001f; // Right trigger brings it closer (smaller Z)
        }
        if (state->ltrig > 10) {
            pos_z += (float)state->ltrig * 0.001f; // Left trigger pushes it further (larger Z)
        }

        // Prevent pushing the camera *through* the object (or breaking geometry behind it)
        if (pos_z < 2.0f) {
            pos_z = 2.0f;
        }
      }
    }

    // --- GPU Rendering Loop ---
    // 1. Wait until the PVR is ready to accept a new frame's data
    pvr_wait_ready();

    // 2. Begin the overall scene
    pvr_scene_begin();

    // 3. Inform the PVR that we are going to send Opaque (solid) polygons
    pvr_list_begin(PVR_LIST_OP_POLY);

    // (We will submit our shapes here in the next steps)

    // --- STEP 3 & 4: Headers and Vertices ---
    // Before passing raw points to the GPU, we have to provide a "Header".
    // This header dictates the rules for the upcoming shape (e.g. is it
    // textured? what list does it belong to?)
    pvr_poly_cxt_t cxt;
    pvr_poly_hdr_t hdr;

    // Create a textured, opaque polygon context
    pvr_poly_cxt_txr(&cxt, PVR_LIST_OP_POLY, PVR_TXRFMT_RGB565, 256, 256, texture_ptr, PVR_FILTER_NONE);

    // Compile that context into a header the GPU hardware understands
    pvr_poly_compile(&hdr, &cxt);

    // Submit the header
    pvr_prim(&hdr, sizeof(hdr));

    // --- STEP 3: Submitting our 3D Local Vertices for a Cube ---
    // The PVR uses "Face Culling" which hides triangles facing away from the camera.
    // We must submit the 4 corners of each face in Clockwise Winding Order:
    // Top-Left -> Top-Right -> Bottom-Left -> Bottom-Right
    
    // Define the 8 corners of our cube
    vertex3_t f_tl = {-1.0f, 1.0f, -1.0f};  // Front Top-Left (Z=-1 is closer)
    vertex3_t f_tr = {1.0f, 1.0f, -1.0f};   // Front Top-Right
    vertex3_t f_bl = {-1.0f, -1.0f, -1.0f}; // Front Bottom-Left
    vertex3_t f_br = {1.0f, -1.0f, -1.0f};  // Front Bottom-Right
    
    vertex3_t b_tl = {-1.0f, 1.0f, 1.0f};  // Back Top-Left (Z=1 is further)
    vertex3_t b_tr = {1.0f, 1.0f, 1.0f};   // Back Top-Right
    vertex3_t b_bl = {-1.0f, -1.0f, 1.0f}; // Back Bottom-Left
    vertex3_t b_br = {1.0f, -1.0f, 1.0f};  // Back Bottom-Right

    // We prepare our math unit for this specific shape:
    mat_identity(); // 1. Start fresh
    mat_translate(
        pos_x, pos_y,
        pos_z); // 2. Move the shape to our dynamic position variables (+Z is away)

    // 3. Rotate the shape
    mat_rotate_x(r_angle);
    mat_rotate_y(r_angle * 0.75f);
    mat_rotate_z(r_angle * 0.5f);

    r_angle += 0.02f; // Animate rotation

    // --- STEP 4: Submit the 6 faces ---
    
    // Helper color
    uint32_t c_white = PVR_PACK_COLOR(1.0f, 1.0f, 1.0f, 1.0f); // White allows the texture to be drawn at full brightness

    // Front Face
    submit_vertex3d(f_tl, 0.0f, 0.0f, c_white, false); 
    submit_vertex3d(f_tr, 1.0f, 0.0f, c_white, false); 
    submit_vertex3d(f_bl, 0.0f, 1.0f, c_white, false);
    submit_vertex3d(f_br, 1.0f, 1.0f, c_white, true);

    // Back Face - Remember, looking from the back, left & right are swapped!
    submit_vertex3d(b_tr, 0.0f, 0.0f, c_white, false); 
    submit_vertex3d(b_tl, 1.0f, 0.0f, c_white, false); 
    submit_vertex3d(b_br, 0.0f, 1.0f, c_white, false);
    submit_vertex3d(b_bl, 1.0f, 1.0f, c_white, true);

    // Left Face
    submit_vertex3d(b_tl, 0.0f, 0.0f, c_white, false);
    submit_vertex3d(f_tl, 1.0f, 0.0f, c_white, false);
    submit_vertex3d(b_bl, 0.0f, 1.0f, c_white, false);
    submit_vertex3d(f_bl, 1.0f, 1.0f, c_white, true);

    // Right Face
    submit_vertex3d(f_tr, 0.0f, 0.0f, c_white, false);
    submit_vertex3d(b_tr, 1.0f, 0.0f, c_white, false);
    submit_vertex3d(f_br, 0.0f, 1.0f, c_white, false);
    submit_vertex3d(b_br, 1.0f, 1.0f, c_white, true);

    // Top Face
    submit_vertex3d(b_tl, 0.0f, 0.0f, c_white, false);
    submit_vertex3d(b_tr, 1.0f, 0.0f, c_white, false);
    submit_vertex3d(f_tl, 0.0f, 1.0f, c_white, false);
    submit_vertex3d(f_tr, 1.0f, 1.0f, c_white, true);

    // Bottom Face
    submit_vertex3d(f_bl, 0.0f, 0.0f, c_white, false);
    submit_vertex3d(f_br, 1.0f, 0.0f, c_white, false);
    submit_vertex3d(b_bl, 0.0f, 1.0f, c_white, false);
    submit_vertex3d(b_br, 1.0f, 1.0f, c_white, true);

    // 4. Tell the PVR we are finished sending Opaque polygons
    pvr_list_finish();

    // 5. Tell the PVR we have finished the entire scene
    // This causes the GPU to execute the rendering process and flip it to the
    // screen
    pvr_scene_finish();
  }

  return 0;
}
