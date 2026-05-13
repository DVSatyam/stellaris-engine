#include <stdio.h>
#include<stdlib.h>
#include "engine.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Simulation* init_simulation(int N,double G,double dt) {
    Simulation*sim=(Simulation*)malloc(sizeof(Simulation));
    if (sim == NULL) return NULL;
    sim->num_bodies=N;
    sim->G=G;
    sim->dt=dt;

    sim->bodies=(body*)malloc(N*sizeof(body));
    if (sim->bodies == NULL) {
        free(sim);
        return NULL;
    }

    //Central Point (Black hole/Star)
    sim->bodies[0].x=0.0;
    sim->bodies[0].y=0.0;
    sim->bodies[0].vx=0.0;
    sim->bodies[0].vy=0.0;
    sim->bodies[0].mass=4000;

    for(int i=1; i<N; i++) {
        double r= 20+(((double) rand())/RAND_MAX)*100; //random radius b/w 20 to 120 for proper effect
        double angle=0+(((double) rand()/RAND_MAX))*2*M_PI; //random angles spawn to declutter
        //setting coordinates (polar)-> x and y component
        sim->bodies[i].x=r*cos(angle);
        sim->bodies[i].y=r*sin(angle);
        sim->bodies[i].mass=2.0;

        //orbital mech
        double v= sqrt((sim->G*sim->bodies[0].mass)/r);
        //tangential velocity
        sim->bodies[i].vx=-v*sin(angle);
        sim->bodies[i].vy=v*cos(angle);


    }
    return sim;

}
//fn to free all the allocated space by me
void free_simulation(Simulation*sim) {
    if (sim == NULL) return;
    free(sim->bodies);
    free(sim);
}

void step_simulation(Simulation*sim) {
    if (sim == NULL || sim->bodies == NULL) return;
    int N=sim->num_bodies;
    double G=sim->G;
    double dt=sim->dt;
    body *b=sim->bodies;
    //position update
    for (int i=0; i<N; i++) {
    sim->bodies[i].x+=b[i].vx*dt;
    sim->bodies[i].y+=b[i].vy*dt;
    }
    //temp arrays for acceleration values
    double *ax=(double*)malloc(N*sizeof(double));
    double *ay=(double*)malloc(N*sizeof(double));
    if (ax == NULL || ay == NULL) {
        free(ax);
        free(ay);
        return;
    }

    for(int i=0; i<N; i++) {
        ax[i]=0.0;
        ay[i]=0.0;
        for (int j=0; j<N;j++) {
            if (i==j) continue;
            double dx=b[j].x-b[i].x;
            double dy=b[j].y-b[i].y;
            double eps=1.0;
            double r2=dx*dx+dy*dy+eps; //Gravitational Softening- so the force doesnt tend to infinity when d=0
            double r=sqrt(r2); //radius vector
            //NLG
            double force=(G*b[j].mass)/r2;
            ax[i]+=force*(dx/r); //force decomposition into components
            ay[i]+=force*(dy/r); 

        }
    }
    //Velocity update- Verlet Integration- second order error prop to t^2 rather than euler being t
    for (int i=0;i<N;i++) {
        b[i].vx+=ax[i]*dt;
        b[i].vy+=ay[i]*dt;
    }
    free(ax);
    free(ay);
}
