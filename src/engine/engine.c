#include <stdio.h>
#include<stdlib.h>
#include "engine.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double random01(void) {
    return (double)rand() / RAND_MAX;
}

Simulation* init_simulation(int N,double G,double dt) {
    if (N < 3) return NULL;

    Simulation*sim=(Simulation*)malloc(sizeof(Simulation));
    if (sim == NULL) return NULL;

    sim->num_bodies=N;
    sim->massive_body_count=2;
    sim->G=G;
    sim->dt=dt;

    sim->bodies=(body*)malloc(N*sizeof(body));
    if (sim->bodies == NULL) {
        free(sim);
        return NULL; //failure in memory allocation
    }

    // Equal mass binary black holes orbiting their shared barycenter
    double black_hole_mass=2800.0;
    double separation=64.0;
    double half_separation=separation*0.5;
    double binary_speed=sqrt((G*black_hole_mass)/(2.0*separation));

    sim->bodies[0].x=-half_separation;
    sim->bodies[0].y=0.0; 
    sim->bodies[0].vx=0.0;
    sim->bodies[0].vy=binary_speed;
    sim->bodies[0].mass=black_hole_mass;

    //To show trails previous position must be stored
    for (int j=0; j< TRAIL_LENGTH; j++) {
        sim->bodies[0].trail_x[j]=sim->bodies[0].x;
        sim->bodies[0].trail_y[j]=sim->bodies[0].y;
        sim->bodies[0].trail_index=0;
    }
    sim->bodies[1].x=half_separation;
    sim->bodies[1].y=0.0;
    sim->bodies[1].vx=0.0;
    sim->bodies[1].vy=-binary_speed;
    sim->bodies[1].mass=black_hole_mass;

    for (int j=0; j<TRAIL_LENGTH; j++) {
        sim->bodies[1].trail_x[j]=sim->bodies[1].x;
        sim->bodies[1].trail_y[j]=sim->bodies[1].y;
        sim->bodies[1].trail_index=0;
    }
    for(int i=sim->massive_body_count; i<N; i++) {
        double r= 72+random01()*118; //circumbinary disk outside the black hole pair
        double angle=random01()*2*M_PI; //random angles spawn to declutter
        double turbulence=(random01()-0.5)*0.16;

        sim->bodies[i].x=r*cos(angle);
        sim->bodies[i].y=r*sin(angle);
        sim->bodies[i].mass=1.2+random01()*0.8;

        // Approximate circular velocity around the binary's combined mass.
        double v=sqrt((G*(black_hole_mass*2.0))/r);
        sim->bodies[i].vx=-v*sin(angle)+v*turbulence*cos(angle);
        sim->bodies[i].vy=v*cos(angle)+v*turbulence*sin(angle);

        for (int j=0; j<TRAIL_LENGTH; j++) {
            sim->bodies[i].trail_x[j]=sim->bodies[i].x;
            sim->bodies[i].trail_y[j]=sim->bodies[i].y;
            sim->bodies[i].trail_index=0;
        }

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
    b[i].trail_x[b[i].trail_index]=b[i].x;
    b[i].trail_y[b[i].trail_index]=b[i].y;
    b[i].trail_index=(b[i].trail_index+1)%TRAIL_LENGTH;
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
    sim->kinetic_energy=0.0;
    for (int i=0; i<N; i++) { //KE Calculation
        body *b=&sim->bodies[i];
        double v2=(b->vx*b->vx)+(b->vy*b->vy);
        sim->kinetic_energy+=0.5*b->mass*v2;
    }
    sim->potential_energy=0.0;
    for (int i=0; i<N; i++) {
        for (int j=i+1; j<N; j++) {
            double dx=b[j].x-b[i].x;
            double dy=b[j].y-b[i].y;
            double eps=1.0;
            double r2=dx*dx+dy*dy+eps; //Gravitational Softening- so the force doesnt tend to infinity when d=0
            double r=sqrt(r2);
            sim->potential_energy-=(G*b[i].mass*b[j].mass)/r;
        }
    }
    sim->total_energy=sim->kinetic_energy+sim->potential_energy;
    //Velocity update- Verlet Integration- second order error prop to t^2 rather than euler being t
    for (int i=0;i<N;i++) {
        b[i].vx+=ax[i]*dt;
        b[i].vy+=ay[i]*dt;
    }
    free(ax);
    free(ay);
}
