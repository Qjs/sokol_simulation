#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
    #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "cimgui.h"
#include "./util/sokol_imgui.h"
#include "cimplot.h"

#include "simulations.h"

#include "none.h"
#include "pendulum.h"
#include "mcpi.h"

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#undef X
#define X(ID,NAME,INIT,DEST,UPDATE,PARAMS_UI,PLOT_UI,RENDER) \
    [ID] = {NAME, INIT, DEST, UPDATE, NULL, 0, PARAMS_UI, PLOT_UI, RENDER},
static simulation_desc_t g_simulations[SIM_COUNT] = {
    X_SIMULATIONS
}; 
#undef X

static int g_sim_count = SIM_COUNT;

void simulations_init_registry(void) {
    // If additional initialization is needed when the registry starts up, do it here.
}

void simulations_shutdown_registry(void) {
    // If needed, clean up globally allocated resources here.
}

const simulation_desc_t* simulations_get(simulation_id_t id) {
    extern simulation_desc_t g_simulations[SIM_COUNT]; // forward declaration
    if (id >= 0 && id < SIM_COUNT) {
        return &g_simulations[id];
    }
    return NULL;
}

void simulations_draw_params(sim_parameter_t* params, int16_t count) {
    for (int16_t i = 0; i < count; i++) {
        sim_parameter_t* p = &params[i];
        switch (p->type) {
            case SIM_PARAM_FLOAT:
                igSliderFloat(p->name, (float*)p->value_ptr, p->f_min, p->f_max, "%.3f", ImGuiSliderFlags_None);
                break;
            case SIM_PARAM_INT:
                igSliderInt(p->name, (int*)p->value_ptr, p->i_min, p->i_max, "%d", ImGuiSliderFlags_None);
                break;
        }
    }
}


