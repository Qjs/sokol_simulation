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
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

// A simulation that estimates Pi by a random sampling of numbers in a box, checks if it's bounded by a radius <1

#define MCPI_MAX_POINTS 10000

/* Monte Carlo Pi simulation globals */
static int mcpi_max_points = 1000;  // parameter: maximum points to use (slider)
static int mcpi_points_count = 0;
static int mcpi_points_inside = 0;

static float mcpi_x_data[MCPI_MAX_POINTS];
static float mcpi_y_data[MCPI_MAX_POINTS];
static int   mcpi_in_circle[MCPI_MAX_POINTS]; // 1 if inside the circle, 0 otherwise

static float mcpi_time_data[MCPI_MAX_POINTS];
static float mcpi_pi_estimate_data[MCPI_MAX_POINTS];

static sim_parameter_t mcpi_params[] = {
    { "Number of Points", &mcpi_max_points, SIM_PARAM_INT, 0, 0, 100, MCPI_MAX_POINTS }
};

/* Initialization: reset counters and seed random generator */
void sim_mcpi_init(void) {
    mcpi_points_count = 0;
    mcpi_points_inside = 0;
    srand((unsigned int)time(NULL));
    // Optionally clear data arrays
    for (int i = 0; i < MCPI_MAX_POINTS; i++) {
        mcpi_x_data[i] = 0.0f;
        mcpi_y_data[i] = 0.0f;
        mcpi_in_circle[i] = 0;
        mcpi_time_data[i] = 0.0f;
        mcpi_pi_estimate_data[i] = 0.0f;
    }
}

/* Update: add one new random point and update the π estimate */
void sim_mcpi_update(float dt) {
    if (mcpi_points_count < mcpi_max_points) {
        float x = (float)rand() / (float)RAND_MAX;
        float y = (float)rand() / (float)RAND_MAX;
        mcpi_x_data[mcpi_points_count] = x;
        mcpi_y_data[mcpi_points_count] = y;
        int inside = (x * x + y * y <= 1.0f) ? 1 : 0;
        mcpi_in_circle[mcpi_points_count] = inside;
        if (inside) {
            mcpi_points_inside++;
        }
        mcpi_points_count++;
        float pi_estimate = 4.0f * ((float)mcpi_points_inside / (float)mcpi_points_count);
        mcpi_pi_estimate_data[mcpi_points_count - 1] = pi_estimate;
        mcpi_time_data[mcpi_points_count - 1] = mcpi_points_count * dt;
    }
}

/* UI: Parameters slider and reset button */
void sim_mcpi_params_ui(void) {
    if (igButton("Reset Simulation", (ImVec2){0,0})) {
        mcpi_points_count = 0;
        mcpi_points_inside = 0;
        for (int i = 0; i < MCPI_MAX_POINTS; i++) {
            mcpi_x_data[i] = 0.0f;
            mcpi_y_data[i] = 0.0f;
            mcpi_in_circle[i] = 0;
            mcpi_time_data[i] = 0.0f;
            mcpi_pi_estimate_data[i] = 0.0f;
        }
    }
    simulations_draw_params(mcpi_params, 1);
}

/* Plot UI: display a time-series of the current π estimate versus actual π */
void sim_mcpi_plot_ui(void) {
    if (mcpi_points_count < 2)
        return;

    float max_time = mcpi_time_data[mcpi_points_count - 1];
    float min_time = 0.0f;
    // Set y-axis limits to cover the expected π range (around 3.14)
    ImPlot_SetNextAxesLimits(min_time, max_time, 2.5f, 4.0f, ImPlotCond_Always);
    if (ImPlot_BeginPlot("Pi Estimate Over Time", (ImVec2){0,0}, ImPlotFlags_None)) {
        ImPlot_PlotLine_FloatPtrFloatPtr("Estimated Pi", mcpi_time_data, mcpi_pi_estimate_data, mcpi_points_count, 0, 0, sizeof(float));
        // Plot a constant line for actual Pi
        float constant_x[2] = {min_time, max_time};
        float constant_y[2] = {M_PI, M_PI};
        ImPlot_PlotLine_FloatPtrFloatPtr("Actual Pi", constant_x, constant_y, 2, 0, 0, sizeof(float));
        ImPlot_EndPlot();
    }
}

/* Render UI: display a 2D scatter plot of the points colored by in-circle status */
void sim_mcpi_render(void) {
    if (mcpi_points_count < 1)
        return;

    if (ImPlot_BeginPlot("Monte Carlo Scatter", (ImVec2){256,256}, ImPlotFlags_None)) {
        // Separate points into "inside" and "outside" arrays
        static float inside_x[MCPI_MAX_POINTS], inside_y[MCPI_MAX_POINTS];
        static float outside_x[MCPI_MAX_POINTS], outside_y[MCPI_MAX_POINTS];
        int inside_count = 0, outside_count = 0;
        for (int i = 0; i < mcpi_points_count; i++) {
            if (mcpi_in_circle[i]) {
                inside_x[inside_count] = mcpi_x_data[i];
                inside_y[inside_count] = mcpi_y_data[i];
                inside_count++;
            } else {
                outside_x[outside_count] = mcpi_x_data[i];
                outside_y[outside_count] = mcpi_y_data[i];
                outside_count++;
            }
        }
        // Plot the points: points inside the circle and those outside
        ImPlot_PlotScatter_FloatPtrFloatPtr("Inside", inside_x, inside_y, inside_count, 0, 0, sizeof(float));
        ImPlot_PlotScatter_FloatPtrFloatPtr("Outside", outside_x, outside_y, outside_count, 0, 0, sizeof(float));
        ImPlot_EndPlot();
    }
}

/* Cleanup: no GPU resources to free */
void sim_mcpi_destroy(void) {
    // Nothing to destroy
}
