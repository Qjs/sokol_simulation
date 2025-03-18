#ifndef ISING_H
#define ISING_H



void sim_ising_init(void);
void sim_ising_destroy(void);
void sim_ising_update(float dt);
void sim_ising_params_ui(void);
void sim_ising_plot_ui(void);
void sim_ising_render(void);


#endif /* ISING_H */