#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "./util/sokol_imgui.h"
#include "cimplot.h"

#include "simulations/simulations.h"

typedef struct {
    const simulation_desc_t* sim;
    sg_pass_action pass_action;
    ImPlotContext* plt_ctx;
} state_t;

static state_t state;

static simulation_id_t current_sim = SIM_NONE;

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });
    simgui_setup(&(simgui_desc_t){
        .logger.func = slog_func,
    });
    state = (state_t) {
        .sim = simulations_get(current_sim),
        .pass_action = {
            .colors[0] = {
                .load_action = SG_LOADACTION_CLEAR,
                .clear_value = {0.0f, 0.2f, 0.5f, 1.0f}
            }
        },
        .plt_ctx = ImPlot_CreateContext()
    };
    // ImPlot_CreateContext();

    simulations_init_registry();
    // Initialize the default simulation
    if (state.sim && state.sim->init) {
        state.sim->init();
    }
}

static void frame(void) {
    float dt = (float)sapp_frame_duration();

    if (state.sim && state.sim->update) state.sim->update(dt);

    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = dt,
        .dpi_scale = sapp_dpi_scale()
    });

    // UI: Simulation Selector
    igBegin("Simulation Selector", NULL, 0);

    const char* current_name = simulations_get(current_sim)->name;
    if (igBeginCombo("Simulation", current_name, 0)) {
        for (int i = 0; i < SIM_COUNT; i++) {
            bool selected = (i == (int)current_sim);

            if (igSelectable_Bool(simulations_get((simulation_id_t)i)->name, selected,0,(ImVec2){0.f,0.f})) {
                // Switch simulations
                // Destroy old simulation
                if (state.sim && state.sim->destroy) state.sim->destroy();
                current_sim = (simulation_id_t)i;
                state.sim = simulations_get(current_sim);
                if (state.sim && state.sim->init) state.sim->init();
            }
            if (selected) igSetItemDefaultFocus();
        }
        igEndCombo();
    }

    // Draw parameters (if simulation uses param arrays, set them in its init)
    // If the simulation just uses its params_ui for sliders, call that:
    if (state.sim && state.sim->params_ui) state.sim->params_ui();

    igEnd(); // End "Simulation Selector" window

    igBegin("Simulation Plot", NULL, 0);
    if (state.sim && state.sim->plot_ui) state.sim->plot_ui();
    igEnd();
    
    // Render current simulation
    igBegin("Simulation Render", NULL, 0);
    if (state.sim && state.sim->render) state.sim->render();
    igEnd();

    sg_begin_pass(&(sg_pass) {.action = state.pass_action, .swapchain = sglue_swapchain()});

    // Render IMGUI
    simgui_render();

    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    if (state.sim && state.sim->destroy) state.sim->destroy();

    simulations_shutdown_registry();
    ImPlot_DestroyContext(state.plt_ctx);
    simgui_shutdown();
    sg_shutdown();
}

static void event(const sapp_event* ev) {
    simgui_handle_event(ev);
}

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;(void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 800,
        .height = 600,
        .window_title = "Simulation X Macro Example",
        .logger.func = slog_func
    };
}