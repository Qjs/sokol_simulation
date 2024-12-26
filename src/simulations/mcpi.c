#include "mcpi.h"
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

#include <math.h>

// A simulation that estimates Pi by a random sampling of numbers in a box, checks if it's bounded by a radius <1

void sim_mcpi_init(void) {
    // No need for a Render block here that is a GL window
    // Draw two random numbers between 0 and 1
    // Add it to a rolling buff
    // sim_mcpi_params_ui should have a slider for number of random samples
    // Check if x^2 + y^2 <= 1 assign color
    // Plot them in a scatter on rand
    // Compute with the entire set the ratio for Pi/4.
    // Plot the ratio on a time series as each new point is added at each dt.
    //
    
}

void sim_mcpi_update(float dt) { dt }
void sim_mcpi_params_ui(void) {}
void sim_mcpi_plot_ui(void) {  }
void sim_mcpi_render(void) {}

void sim_mcpi_destroy(void) {}
