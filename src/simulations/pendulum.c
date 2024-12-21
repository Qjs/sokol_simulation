#include "pendulum.h"
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
#include <stdlib.h>
#include <stdint.h>

/* Pendulum Parameters */
static float pendulum_gravity = 9.81f;
static float pendulum_length = 1.0f;
static float pendulum_angle = 0.5f;
static float pendulum_angular_vel = 0.0f;

/* Plot Data */
static float pendulum_time_data[200];
static float pendulum_angle_data[200];
static int   pendulum_data_count = 0;

#define PENDULUM_OFFSCREEN_WIDTH (256)
#define PENDULUM_OFFSCREEN_HEIGHT (256)
#define PENDULUM_COLOR_FORMAT (SG_PIXELFORMAT_RGBA8)
#define PENDULUM_DEPTH_FORMAT (SG_PIXELFORMAT_DEPTH)
#define PENDULUM_SAMPLE_COUNT (1)

// Offscreen rendering state
static sg_image pendulum_color_img;
static sg_image pendulum_depth_img;
static sg_attachments pendulum_attachments;
static sg_pass_action pendulum_pass_action;
static sg_pipeline pendulum_pip;
static sg_bindings pendulum_bind;
static sg_shader pendulum_shader;
static sg_buffer pendulum_vbuf;
static sg_sampler pendulum_sampler;

// Vertex data for the pendulum line: just two points: pivot at (0,0), bob at (0,-1).
// We'll rotate and scale by pendulum_length in the vertex shader.
typedef struct {
    float x, y;
} vertex_t;

// Uniform block for angle and length
typedef struct {
    float angle;
    float length;
} pendulum_uniforms_t;
pendulum_uniforms_t uniforms;

/* Parameter definitions */
static sim_parameter_t pendulum_params[] = {
    { "Gravity", &pendulum_gravity, SIM_PARAM_FLOAT, 1.0f, 20.0f, 0,0 },
    { "Length",  &pendulum_length,  SIM_PARAM_FLOAT, 0.5f, 5.0f,  0,0 },
};

/* Initialize GPU Resources */
void sim_pendulum_init(void) {
    
    // Create offscreen target images for rendering
    pendulum_color_img = sg_make_image(&(sg_image_desc){
        .render_target = true,
        .width = PENDULUM_OFFSCREEN_WIDTH,
        .height = PENDULUM_OFFSCREEN_HEIGHT,
        .pixel_format = PENDULUM_COLOR_FORMAT,
        .sample_count = PENDULUM_SAMPLE_COUNT,
    });

    pendulum_depth_img = sg_make_image(&(sg_image_desc){
        .render_target = true,
        .width = PENDULUM_OFFSCREEN_WIDTH,
        .height = PENDULUM_OFFSCREEN_HEIGHT,
        .pixel_format = PENDULUM_DEPTH_FORMAT,
        .sample_count = PENDULUM_SAMPLE_COUNT,
    });

    pendulum_attachments = sg_make_attachments(&(sg_attachments_desc){
        .colors[0].image = pendulum_color_img,
        .depth_stencil.image = pendulum_depth_img,
    });

    pendulum_pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = {0.0f,0.0f,0.0f,1.0f}
        }
    };

    // Create vertex buffer
    vertex_t vertices[2] = {
        {0.0f, 0.0f},     // pivot
        {0.0f, 1.0f}     // bob before scaling and rotation
    };
    pendulum_vbuf = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices)
    });

    // Minimal vertex and fragment shaders:
    // We'll do a 2D rotation and scaling in the vertex shader:
    const char* vs_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location=0) in vec2 pos;\n"
        "uniform vec2 u_params;\n" // u_params.x = angle, u_params.y = length
        "void main() {\n"
        "  float angle = u_params.x;\n"
        "  float length = u_params.y;\n"
        "  float c = cos(angle);\n"
        "  float s = sin(angle);\n"
        "  mat2 rot = mat2(c, -s, s, c);\n"
        "  vec2 p = pos;\n"
        "  p.y *= length;\n"
        "  // Simple orthographic projection to [-1,1]\n"
        "  // rotate the line about the pivot\n"
        "  p = rot * p;\n"
        "  gl_Position = vec4(p, 0.0, 0.9);\n"
        "}\n";

    const char* fs_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 frag_color;\n"
        "void main(){\n"
        "  frag_color=vec4(1.0,1.0,1.0,1.0);\n"
        "}\n";

    // Create shader
    pendulum_shader = sg_make_shader(&(sg_shader_desc){
        .vertex_func.source = vs_src,
        .fragment_func.source = fs_src,
        .uniform_blocks[0].size = sizeof(pendulum_uniforms_t),
        .uniform_blocks[0].glsl_uniforms[0] = {.type = SG_UNIFORMTYPE_FLOAT2, .glsl_name = "u_params"}
    });

    // Create pipeline
    pendulum_pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = pendulum_shader,
        .primitive_type = SG_PRIMITIVETYPE_LINES,
        .layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
        .colors[0].pixel_format = PENDULUM_COLOR_FORMAT,
        .depth.pixel_format = PENDULUM_DEPTH_FORMAT
    });

    // Setup bindings
    pendulum_bind.vertex_buffers[0] = pendulum_vbuf;

    // Optional sampler
    pendulum_sampler = sg_make_sampler(&(sg_sampler_desc){
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_LINEAR,
        .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
        .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    });

    // Reset pendulum initial conditions
    pendulum_angle = 0.5f;
    pendulum_angular_vel = 0.0f;
}

/* Destroy GPU resources */
void sim_pendulum_destroy(void) {
    sg_destroy_image(pendulum_color_img);
    sg_destroy_image(pendulum_depth_img);
    sg_destroy_buffer(pendulum_vbuf);
    sg_destroy_pipeline(pendulum_pip);
    sg_destroy_shader(pendulum_shader);
    sg_destroy_sampler(pendulum_sampler);

    pendulum_color_img = (sg_image){0};
    pendulum_depth_img = (sg_image){0};
    pendulum_vbuf = (sg_buffer){0};
    pendulum_pip = (sg_pipeline){0};
    pendulum_shader = (sg_shader){0};
    pendulum_sampler = (sg_sampler){0};
}


/* Update logic */
void sim_pendulum_update(float dt) {
    float angle_acc = -(pendulum_gravity/pendulum_length)*sinf(pendulum_angle);
    pendulum_angular_vel += angle_acc*dt;
    pendulum_angle += pendulum_angular_vel*dt;

    if (pendulum_data_count<200) {
        pendulum_time_data[pendulum_data_count] = pendulum_data_count*dt;
        pendulum_angle_data[pendulum_data_count] = pendulum_angle;
        pendulum_data_count++;
    } else {
        for (int i=1; i<200; i++){
            pendulum_time_data[i-1]=pendulum_time_data[i];
            pendulum_angle_data[i-1]=pendulum_angle_data[i];
        }
        pendulum_time_data[199] = pendulum_time_data[198]+dt;
        pendulum_angle_data[199]= pendulum_angle;
    }

    // Prepare uniforms
    uniforms.angle = pendulum_angle;
    uniforms.length = pendulum_length;

    // Offscreen pass
    sg_begin_pass(&(sg_pass){
        .action = pendulum_pass_action,
        .attachments = pendulum_attachments
    });
    sg_apply_pipeline(pendulum_pip);
    sg_apply_bindings(&pendulum_bind);
    sg_apply_uniforms(0, &SG_RANGE(uniforms));
    sg_draw(0, 2, 1);
    sg_end_pass();
}

/* Extra UI: a reset button */
void sim_pendulum_params_ui(void) {
    igText("Angle: %.3f rad", pendulum_angle);
    if (igButton("Reset Simulation",(ImVec2){0,0})) {
        pendulum_angle=0.5f;
        pendulum_angular_vel=0.0f;
        pendulum_data_count=0;
        for (int i=0; i<200; i++){
            pendulum_time_data[i]=0.0f;
            pendulum_angle_data[i]=0.0f;
        }
    }

    simulations_draw_params(pendulum_params,2);
}

/* Plot UI */
void sim_pendulum_plot_ui(void) {
    if (ImPlot_BeginPlot("Pendulum Angle",(ImVec2){0,0},ImPlotFlags_None)) {
        ImPlot_SetNextMarkerStyle(ImPlotMarker_Circle,1.0f,(ImVec4){1.0f,1.0f,1.0f,1.0f},1.0f,(ImVec4){1.0f,1.0f,1.0f,1.0f});
        ImPlot_PlotLine_FloatPtrFloatPtr("Angle",pendulum_time_data,pendulum_angle_data,pendulum_data_count,ImPlotLineFlags_None,0,sizeof(float));
        ImPlot_EndPlot();
    }
}
 
/* Render: apply pipeline and uniforms */
void sim_pendulum_render(void) {

    //sg_commit();

    ImVec4 white = {1.0,1.0,1.0,1};
    ImVec2 size = {256,256};
    ImVec2 uv0 = {0,0};
    ImVec2 uv1 = {1,1};

    ImTextureID tex_id = simgui_imtextureid_with_sampler(pendulum_color_img,pendulum_sampler);
    //igImage(simgui_imtextureid())
    igImage(tex_id, size, uv0, uv1, white,(ImVec4){0,0,0,0});
    igText("Pendulum angle: %.2f rad", pendulum_angle);
}
