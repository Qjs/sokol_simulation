#ifndef SIMULATIONS_H
#define SIMULATIONS_H

#include <stdint.h>
/* 
Format of each line:
X(ID, DisplayName, Init, Destroy, Update, ParamUI, PlotUI, Render)

- ID: Enum ID for this simulation
- DisplayName: A string shown in the combo box
- Init: Initialization
- Destroy: Cleanup code
- Update: void func(float dt) updates simulation state
- ParamUI: void func() creates sliders/params in IMGUI for this sim
- PlotUI: void func() plots simulation-specific data using ImPlot
- Render: what to display
*/
#define X_SIMULATIONS \
    X(SIM_NONE,      "None",      sim_none_init,      sim_none_destroy,      sim_none_update,      sim_none_params_ui,      sim_none_plot_ui,      sim_none_render) \
    X(SIM_PENDULUM,  "Pendulum",  sim_pendulum_init,  sim_pendulum_destroy,  sim_pendulum_update,  sim_pendulum_params_ui,  sim_pendulum_plot_ui,  sim_pendulum_render) \
    X(SIM_MCPI,  "Monte Carlo Pi",  sim_mcpi_init,  sim_mcpi_destroy,  sim_mcpi_update,  sim_mcpi_params_ui,  sim_mcpi_plot_ui,  sim_mcpi_render) 

/* Generate enum */
typedef enum {
    #define X(ID,NAME,INIT,DEST,UPDATE,PARAMS_UI,PLOT_UI,RENDER) ID,
    X_SIMULATIONS
    #undef X
    SIM_COUNT
} simulation_id_t;

typedef enum { SIM_PARAM_FLOAT, SIM_PARAM_INT } sim_param_type_t;

typedef struct sim_parameter_t {
    const char* name;
    void*       value_ptr;
    sim_param_type_t type;
    float       f_min;
    float       f_max;
    int         i_min;
    int         i_max;
} sim_parameter_t;

typedef struct simulation_desc_t {
    const char* name;
    void (*init)(void);
    void (*destroy)(void);
    void (*update)(float dt);
    sim_parameter_t* params;
    int16_t param_count;
    void (*params_ui)(void);
    void (*plot_ui)(void);
    void (*render)(void);
} simulation_desc_t;

void simulations_init_registry(void);
void simulations_shutdown_registry(void);
const simulation_desc_t* simulations_get(simulation_id_t id);

void simulations_draw_params(sim_parameter_t* params, int16_t count);

#endif

