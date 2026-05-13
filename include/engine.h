#ifndef ENGINE_H
#define ENGINE_H

typedef struct {
    double x,y;
    double vx,vy;
    double mass;   
} body;

typedef struct {
    int num_bodies;
    double G;
    double dt;
    body *bodies; //dynamically allocated array pointing to bodies
} Simulation;
//function prototypes
Simulation* init_simulation(int N,double G,double dt);
void step_simulation(Simulation*sim);
void free_simulation(Simulation*sim);

#endif