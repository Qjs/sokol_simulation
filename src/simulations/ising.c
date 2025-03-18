#include "ising.h"
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
// 2D Ising Model Simulation
// -----------------------------------------------------------------------------

// Global simulation parameters
static int ising_grid_size = 64;         // Current grid size
static int ising_grid_size_new = 64;     // Parameter for grid size (modifiable via UI)
static float ising_temperature = 2.5f;     // Temperature parameter

// Lattice: each cell is either +1 or -1.
static int *ising_grid = NULL;           

// Texture and sampler for rendering the lattice
static sg_image ising_image;
static sg_sampler ising_sampler;
static unsigned char *ising_pixels = NULL; // Pixel buffer for texture update

// Plot data for energy and magnetization versus time
#define ISING_BUFFER_LEN 600
static float ising_time_data[ISING_BUFFER_LEN];
static float ising_energy_data[ISING_BUFFER_LEN];
static float ising_mag_data[ISING_BUFFER_LEN];
static int ising_data_count = 0;
static float ising_sim_time = 0.0f;

// Simulation parameters: grid size and temperature
static sim_parameter_t ising_params[] = {
    { "Grid Size",   &ising_grid_size_new, SIM_PARAM_INT,   0, 0, 16, 128 },
    { "Temperature", &ising_temperature,   SIM_PARAM_FLOAT, 0.5f, 5.0f, 0, 0 }
};

// -----------------------------------------------------------------------------
// Helper: Compute periodic index
// -----------------------------------------------------------------------------
static inline int mod(int a, int b) {
    return (a % b + b) % b;
}

// -----------------------------------------------------------------------------
// Initialization: allocate the lattice, set initial spins, and create texture
// -----------------------------------------------------------------------------
void sim_ising_init(void) {
    // Use new grid size if changed
    ising_grid_size = ising_grid_size_new;
    
    // Allocate lattice array (size: grid_size x grid_size)
    ising_grid = (int*)malloc(ising_grid_size * ising_grid_size * sizeof(int));
    
    // Initialize lattice with random spins (+1 or -1)
    srand((unsigned int)time(NULL));
    for (int i = 0; i < ising_grid_size * ising_grid_size; i++) {
        ising_grid[i] = (rand() % 2) ? 1 : -1;
    }
    
    // Reset simulation time and plot data count, and clear time series arrays
    ising_sim_time = 0.0f;
    ising_data_count = 0;
    for (int i = 0; i < ISING_BUFFER_LEN; i++) {
        ising_time_data[i] = 0.0f;
        ising_energy_data[i] = 0.0f;
        ising_mag_data[i] = 0.0f;
    }
    
    // Create a dynamic texture whose dimensions match the lattice
    ising_image = sg_make_image(&(sg_image_desc){
        .width = ising_grid_size,
        .height = ising_grid_size,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .usage = SG_USAGE_DYNAMIC,
    });
    
    // Create a sampler with nearest filtering for a sharp pixel look
    ising_sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_NEAREST,
        .mag_filter = SG_FILTER_NEAREST,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    });
    
    // Allocate pixel buffer (4 bytes per cell: RGBA)
    ising_pixels = (unsigned char*)malloc(ising_grid_size * ising_grid_size * 4);
}

// -----------------------------------------------------------------------------
// Destroy: free allocated arrays and GPU resources
// -----------------------------------------------------------------------------
void sim_ising_destroy(void) {
    if (ising_grid) { free(ising_grid); ising_grid = NULL; }
    if (ising_pixels) { free(ising_pixels); ising_pixels = NULL; }
    sg_destroy_image(ising_image);
    sg_destroy_sampler(ising_sampler);
}

// -----------------------------------------------------------------------------
// Update: perform Monte Carlo updates, compute energy and magnetization, and update texture
// -----------------------------------------------------------------------------
void sim_ising_update(float dt) {
    // Perform one Monte Carlo sweep (N = grid_size^2 random spin updates)
    int N = ising_grid_size * ising_grid_size;
    for (int step = 0; step < N; step++) {
        int i = rand() % ising_grid_size;
        int j = rand() % ising_grid_size;
        int idx = j * ising_grid_size + i;
        int spin = ising_grid[idx];
        
        // Sum over four nearest neighbors (with periodic boundaries)
        int up    = ising_grid[ mod(j - 1, ising_grid_size) * ising_grid_size + i ];
        int down  = ising_grid[ mod(j + 1, ising_grid_size) * ising_grid_size + i ];
        int left  = ising_grid[ j * ising_grid_size + mod(i - 1, ising_grid_size) ];
        int right = ising_grid[ j * ising_grid_size + mod(i + 1, ising_grid_size) ];
        int sum_neighbors = up + down + left + right;
        
        // Energy change if the spin is flipped (with coupling constant J = 1)
        int deltaE = 2 * spin * sum_neighbors;
        
        // Metropolis criterion
        if (deltaE <= 0) {
            ising_grid[idx] = -spin;
        } else {
            float r = (float)rand() / (float)RAND_MAX;
            if (r < expf(-deltaE / ising_temperature)) {
                ising_grid[idx] = -spin;
            }
        }
    }
    
    // Compute total energy and magnetization
    float energy = 0.0f;
    long total_spin = 0;
    // To avoid double counting, only consider right and down neighbors per site
    for (int j = 0; j < ising_grid_size; j++) {
        for (int i = 0; i < ising_grid_size; i++) {
            int idx = j * ising_grid_size + i;
            int s = ising_grid[idx];
            int right = ising_grid[j * ising_grid_size + mod(i + 1, ising_grid_size)];
            int down  = ising_grid[ mod(j + 1, ising_grid_size) * ising_grid_size + i ];
            energy += -s * (right + down);
            total_spin += s;
        }
    }
    // Normalize energy per spin
    float energy_per_spin = energy / (ising_grid_size * ising_grid_size);
    // Average magnetization per spin
    float magnetization = (float)total_spin / (ising_grid_size * ising_grid_size);
    
    // Update simulation time and record data if within buffer limit
    ising_sim_time += dt;
    if (ising_data_count < ISING_BUFFER_LEN) {
        ising_time_data[ising_data_count]   = ising_sim_time;
        ising_energy_data[ising_data_count] = energy_per_spin;
        ising_mag_data[ising_data_count]    = magnetization;
        ising_data_count++;
    }
    
    // Update the pixel buffer for visualization:
    // Color each cell based on its spin: +1 → red (255,0,0,255); -1 → blue (0,0,255,255)
    for (int j = 0; j < ising_grid_size; j++) {
        for (int i = 0; i < ising_grid_size; i++) {
            int idx = j * ising_grid_size + i;
            int pixel_idx = idx * 4;
            if (ising_grid[idx] == 1) {
                ising_pixels[pixel_idx + 0] = 255;
                ising_pixels[pixel_idx + 1] = 0;
                ising_pixels[pixel_idx + 2] = 0;
                ising_pixels[pixel_idx + 3] = 255;
            } else {
                ising_pixels[pixel_idx + 0] = 0;
                ising_pixels[pixel_idx + 1] = 0;
                ising_pixels[pixel_idx + 2] = 255;
                ising_pixels[pixel_idx + 3] = 255;
            }
        }
    }
    // Update the dynamic texture with the new pixel data
    sg_update_image(ising_image, &(sg_image_data){
        .subimage[0][0] = {
            .ptr = ising_pixels,
            .size = (size_t)(ising_grid_size * ising_grid_size * 4)
        }
    });
}

// -----------------------------------------------------------------------------
// Parameters UI: slider for grid size and temperature plus a reset button
// -----------------------------------------------------------------------------
void sim_ising_params_ui(void) {
    if (igButton("Reset Simulation", (ImVec2){0,0})) {
        // If the grid size has been modified, update ising_grid_size accordingly
        if (ising_grid_size != ising_grid_size_new) {
            ising_grid_size = ising_grid_size_new;
        }
        sim_ising_destroy();
        sim_ising_init();
    }
    simulations_draw_params(ising_params, 2);
}

// -----------------------------------------------------------------------------
// Plot UI: display a time-series plot of energy and magnetization over time
// -----------------------------------------------------------------------------
void sim_ising_plot_ui(void) {
    if (ising_data_count < 2)
        return;
    float max_time = ising_time_data[ising_data_count - 1];
    float min_time = 0.0f;
    // Set y-axis limits to cover the expected ranges (energy near -2 to 2, magnetization between -1 and 1)
    ImPlot_SetNextAxesLimits(min_time, max_time, -2.5f, 2.5f, ImPlotCond_Always);
    if (ImPlot_BeginPlot("Energy and Magnetization", (ImVec2){0,0}, ImPlotFlags_None)) {
        ImPlot_PlotLine_FloatPtrFloatPtr("Energy", ising_time_data, ising_energy_data, ising_data_count, 0, 0, sizeof(float));
        ImPlot_PlotLine_FloatPtrFloatPtr("Magnetization", ising_time_data, ising_mag_data, ising_data_count, 0, 0, sizeof(float));
        ImPlot_EndPlot();
    }
}

// -----------------------------------------------------------------------------
// Render UI: display the lattice using the offscreen texture and show current stats
// -----------------------------------------------------------------------------
void sim_ising_render(void) {
    ImVec4 white = {1.0f, 1.0f, 1.0f, 1.0f};
    ImVec2 size = {256,256};
    ImVec2 uv0 = {0,0};
    ImVec2 uv1 = {1,1};
    ImTextureID tex_id = simgui_imtextureid_with_sampler(ising_image, ising_sampler);
    igImage(tex_id, size, uv0, uv1, white, (ImVec4){0,0,0,0});
    float current_energy = (ising_data_count > 0) ? ising_energy_data[ising_data_count - 1] : 0.0f;
    float current_mag = (ising_data_count > 0) ? ising_mag_data[ising_data_count - 1] : 0.0f;
    igText("Energy per spin: %.3f", current_energy);
    igText("Magnetization: %.3f", current_mag);
}