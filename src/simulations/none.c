#include "none.h"
#ifndef CIMGUI_DEFINE_ENUMS_AND_STRUCTS
    #define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#endif
#include "cimgui.h"



// A simulation that does nothing
void sim_none_init(void) {}
void sim_none_destroy(void) {}
void sim_none_update(float dt) { (void)dt; }
void sim_none_params_ui(void) { igText("No simulation selected."); }
void sim_none_plot_ui(void) { igText("No plot."); }
void sim_none_render(void) {igText("No simulation Render");}

