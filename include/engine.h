#ifndef ENGINE_H
#define ENGINE_H
#define TRAIL_LENGTH 64
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double x,y;
    double vx,vy;
    double mass;   
    double trail_x[TRAIL_LENGTH];
    double trail_y[TRAIL_LENGTH];
    int trail_index;
} body;

typedef struct {
    int num_bodies;
    int massive_body_count;
    double G;
    double dt;
    
    body *bodies; //dynamically allocated array pointing to bodies
} Simulation;
//function prototypes
Simulation* init_simulation(int N,double G,double dt);
void step_simulation(Simulation*sim);
void free_simulation(Simulation*sim);

#ifdef __cplusplus
}
#endif

#endif
