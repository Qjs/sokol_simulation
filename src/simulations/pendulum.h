#ifndef PENDULUM_H
#define PENDULUM_H

void sim_pendulum_init(void);
void sim_pendulum_destroy(void);
void sim_pendulum_update(float dt);
void sim_pendulum_params_ui(void);
void sim_pendulum_plot_ui(void);
void sim_pendulum_render(void);

#endif /* PENDULUM_H */