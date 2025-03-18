#include "gol.h"
#include "simulations.h"
#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
    #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "cimgui.h"
#include "cimplot.h"
#include "sokol_gfx.h"
#include "sokol_app.h"
#include "./util/sokol_imgui.h"
#include "sokol_glue.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -----------------------------------------------------------------------------
// Game of Life Simulation
// -----------------------------------------------------------------------------

// Global parameters
static int gol_grid_size = 64; // Default grid size
static int gol_grid_size_new = 64; // Default grid size

static int *gol_grid = NULL;         // Current grid state: 0 dead, 1 alive
static int *gol_grid_buffer = NULL;  // Buffer for next generation

// Texture and sampler for rendering the grid
static sg_image gol_image;
static sg_sampler gol_sampler;
static unsigned char *gol_pixels = NULL; // Pixel buffer for texture updates

// Plot data for live ratio over time
#define GOL_BUFFER_LEN 600
static float gol_time_data[GOL_BUFFER_LEN];
static float gol_live_data[GOL_BUFFER_LEN];
static int gol_data_count = 0;
static float gol_sim_time = 0.0f;

// Simulation parameter: grid size slider (min: 16, max: 128)
static sim_parameter_t gol_params[] = {
    { "Grid Size", &gol_grid_size_new, SIM_PARAM_INT, 0, 0, 16, 128 }
};


void sim_gol_init(void) {
    // Allocate grid arrays
    gol_grid = (int*)malloc(gol_grid_size * gol_grid_size * sizeof(int));
    gol_grid_buffer = (int*)malloc(gol_grid_size * gol_grid_size * sizeof(int));

    // Initialize grid with a random state (0 or 1)
    srand((unsigned int)time(NULL));
    for (int i = 0; i < gol_grid_size * gol_grid_size; i++) {
        gol_grid[i] = rand() % 2;
    }

    // Reset simulation time and plot data count
    gol_sim_time = 0.0f;
    gol_data_count = 0;

    // Create a dynamic texture that matches the grid dimensions.
    // Each pixel represents one cell.
    gol_image = sg_make_image(&(sg_image_desc){
        .width = gol_grid_size,
        .height = gol_grid_size,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage = SG_USAGE_DYNAMIC,
    });

    // Create a sampler with nearest filtering for a crisp grid look
    gol_sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    });

    // Allocate a pixel buffer (4 bytes per cell for RGBA)
    gol_pixels = (unsigned char*)malloc(gol_grid_size * gol_grid_size * 4);
}


void sim_gol_destroy(void) {
    if (gol_grid) { free(gol_grid); gol_grid = NULL; }
    if (gol_grid_buffer) { free(gol_grid_buffer); gol_grid_buffer = NULL; }
    if (gol_pixels) { free(gol_pixels); gol_pixels = NULL; }
    sg_destroy_image(gol_image);
    sg_destroy_sampler(gol_sampler);
}


void sim_gol_update(float dt) {
    int live_count = 0;
    for (int y = 0; y < gol_grid_size; y++) {
        for (int x = 0; x < gol_grid_size; x++) {
            int idx = y * gol_grid_size + x;
            int neighbors = 0;
            // Check all 8 neighbors
            for (int j = -1; j <= 1; j++) {
                for (int i = -1; i <= 1; i++) {
                    if (i == 0 && j == 0) continue;
                    int nx = (x + i + gol_grid_size) % gol_grid_size;
                    int ny = (y + j + gol_grid_size) % gol_grid_size;
                    neighbors += gol_grid[ny * gol_grid_size + nx];
                }
            }
            // Apply the Game of Life rules
            if (gol_grid[idx] == 1) {
                gol_grid_buffer[idx] = (neighbors == 2 || neighbors == 3) ? 1 : 0;
            } else {
                gol_grid_buffer[idx] = (neighbors == 3) ? 1 : 0;
            }
            if (gol_grid_buffer[idx] == 1) {
                live_count++;
            }
        }
    }
    // Swap the grids (the new generation becomes the current state)
    int *temp = gol_grid;
    gol_grid = gol_grid_buffer;
    gol_grid_buffer = temp;

    // Update simulation time and record the live-cell ratio for plotting
    gol_sim_time += dt;
    if (gol_data_count < GOL_BUFFER_LEN) {
        gol_time_data[gol_data_count] = gol_sim_time;
        gol_live_data[gol_data_count] = (float)live_count / (gol_grid_size * gol_grid_size);
        gol_data_count++;
    }

    // Update the pixel buffer: each cell is rendered as a single pixel.
    // Alive cells are white; dead cells are black.
    for (int y = 0; y < gol_grid_size; y++) {
        for (int x = 0; x < gol_grid_size; x++) {
            int idx = y * gol_grid_size + x;
            int pixel_idx = idx * 4;
            if (gol_grid[idx] == 1) {
                gol_pixels[pixel_idx + 0] = 255;
                gol_pixels[pixel_idx + 1] = 255;
                gol_pixels[pixel_idx + 2] = 255;
                gol_pixels[pixel_idx + 3] = 255;
            } else {
                gol_pixels[pixel_idx + 0] = 0;
                gol_pixels[pixel_idx + 1] = 0;
                gol_pixels[pixel_idx + 2] = 0;
                gol_pixels[pixel_idx + 3] = 255;
            }
        }
    }
    // Update the dynamic texture with the new pixel data
    sg_update_image(gol_image, &(sg_image_data){
        .subimage[0][0] = {
            .ptr = gol_pixels,
            .size = (size_t)(gol_grid_size * gol_grid_size * 4)
        }
    });
}

// -----------------------------------------------------------------------------
// Parameters UI: slider for grid size and reset button (reinitializes the simulation)
// -----------------------------------------------------------------------------
void sim_gol_params_ui(void) {
    if (igButton("Reset Simulation", (ImVec2){0,0})) {
        if (gol_grid_size != gol_grid_size_new) {
            gol_grid_size = gol_grid_size_new;
        } 
        // On reset, destroy the current simulation and reinitialize it
        sim_gol_destroy();
        sim_gol_init();
    }
    simulations_draw_params(gol_params, 1);
}

// -----------------------------------------------------------------------------
// Plot UI: display a time-series plot of the live-cell ratio over time
// -----------------------------------------------------------------------------
void sim_gol_plot_ui(void) {
    if (gol_data_count < 2)
        return;
    float max_time = gol_time_data[gol_data_count - 1];
    float min_time = 0.0f;
    ImPlot_SetNextAxesLimits(min_time, max_time, 0.0f, 1.0f, ImPlotCond_Always);
    if (ImPlot_BeginPlot("Live Ratio Over Time", (ImVec2){0,0}, ImPlotFlags_None)) {
        ImPlot_PlotLine_FloatPtrFloatPtr("Live Ratio", gol_time_data, gol_live_data, gol_data_count, 0, 0, sizeof(float));
        ImPlot_EndPlot();
    }
}

// -----------------------------------------------------------------------------
// Render UI: display the offscreen texture using an OpenGL sampler and show stats
// -----------------------------------------------------------------------------
void sim_gol_render(void) {
    ImVec4 white = {1.0f, 1.0f, 1.0f, 1.0f};
    ImVec2 size = {256,256};
    ImVec2 uv0 = {0,0};
    ImVec2 uv1 = {1,1};
    ImTextureID tex_id = simgui_imtextureid_with_sampler(gol_image, gol_sampler);
    igImage(tex_id, size, uv0, uv1, white, (ImVec4){0,0,0,0});
    float current_ratio = (gol_data_count > 0) ? gol_live_data[gol_data_count - 1] : 0.0f;
    igText("Live Ratio: %.2f", current_ratio);
}