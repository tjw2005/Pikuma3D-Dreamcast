# Dreamcast Engine: 3D Hardware Rendering Notebook

This document explains exactly how our `main.c` code talks to the Sega Dreamcast's SH4 CPU and PowerVR2 GPU to render a 3D hardware-accelerated triangle.

## 1. Initializing the GPU
```c
pvr_init_defaults();
```
Rather than dealing with low-level tile binning and VRAM allocation manually, KallistiOS sets up a perfectly balanced configuration for us. This starts the PVR, configures the video mode (usually 640x480 at 60Hz), and creates memory queues for solid (opaque) and see-through (translucent) shapes.

## 2. 3D Math and the SH4 Coprocessor
The SH4 processor inside the Dreamcast has specialized float-math capabilities. KallistiOS maintains an "Active Matrix" stack inside the hardware's registers.

```c
mat_identity(); // Resets the hardware matrix to "zero" (identity)
mat_translate(0.0f, 0.0f, 10.0f); // Updates the matrix: push objects 10 units deep into the screen
```
We place these commands inside our loop. They construct our "Camera" and world positional data before we start drawing. +10.0 on the Z axis means "away from the player". 

## 3. The Custom Vertex Pipeline 
Before we hand points to the GPU to be colored, we pass them through our `submit_vertex3d` function. This function has three distinct phases:

### Phase A: Hardware Matrix Transformation
```c
mat_trans_single3_nodiv(tx, ty, tz);
```
This is a KallistiOS macro that injects raw inline SH4 assembly (`ftrv`). It instantly forces our custom `x`, `y`, `z` coordinate through the matrix we built earlier. The `_nodiv` version ensures that the math unit doesn't corrupt our true `Z` depth by preemptively applying perspective division routines intended for other matrix types.

### Phase B: Simulating a Camera Lens (Perspective Divide)
```c
vert.x = (tx * 256.0f) / tz + 320.0f;
vert.y = -(ty * 256.0f) / tz + 240.0f;
vert.z = 1.0f / tz;
```
Now that the points are properly positioned in 3D-Space by the SH4, we flatten them onto the 2D TV screen:
- We divide `X` and `Y` by `Z` because things look smaller the further away they are. 
- We multiply by `256.0f` to act as our Field of View (Zoom level).
- We add half the screen width/height (`320` and `240`) to ensure 0,0 is perfectly in the center of the TV.
- We set the final PVR Z-buffer depth to `1.0f / Z` (The PVR specifically requires depth values to be inverted integers to perform its sorting algorithms).

### Phase C: Assembling the PVR Vertex
The PVR needs vertices formatted in the specific `pvr_vertex_t` structure. We apply the coordinates from Phase B, give it an ARGB color, and most importantly, flag it correctly:
`vert.flags = is_end_of_list ? PVR_CMD_VERTEX_EOL : PVR_CMD_VERTEX;`
The very last point of any shape MUST be flagged as `_EOL` (End of List) to trigger the hardware rasterizer to actually draw it! We send the completed struct into the GPU queues using `pvr_prim()`.

## 4. The Render Loop
Everything is wrapped inside the main while-loop ticking 60 times a second:
1. `pvr_wait_ready()`: Halts the CPU until the TV has finished drawing the last frame.
2. `pvr_scene_begin()` & `pvr_list_begin(PVR_LIST_OP_POLY)`: Tells the GPU to clear out old data and prepare to receive solid polygons.
3. **The Header**: Before sending raw points, we submit a `pvr_poly_hdr_t` via `pvr_prim()`. This acts as a ruleset (e.g. "The next bunch of points are untextured and opaque").
4. **Submit Points**: We run our 3 custom triangle points through our `submit_vertex3d` function.
5. `pvr_scene_finish()`: Submits the lists and commands the GPU to rasterize and draw the frame!
