#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "engine.h"

#define MAX_VIEW_DISTANCE 220.0f
#define STAR_SPEED_SCALE 20.0f
#define BACKGROUND_GLOW_SIZE 150.0f
#define TRAIL_LENGTH_FACTOR 0.8f

static float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

static float mixf(float a, float b, float t) {
    return a + (b - a) * t;
}

static void renderBackgroundGlow(float x, float y, float scale) {
    float zoomFactor = 1.0f / scale;

    glPointSize(BACKGROUND_GLOW_SIZE * zoomFactor);
    glBegin(GL_POINTS);
        glColor4f(0.10f, 0.16f, 0.45f, 0.02f);
        glVertex2f(x, y);
    glEnd();

    glPointSize((BACKGROUND_GLOW_SIZE * 0.6f) * zoomFactor);
    glBegin(GL_POINTS);
        glColor4f(0.25f, 0.12f, 0.40f, 0.03f);
        glVertex2f(x, y);
    glEnd();
}

static void renderTrails(Simulation* sim, int massiveCount, double baryX, double baryY, float scale) {
    glLineWidth(1.0f);
    for (int i=massiveCount; i<sim->num_bodies; i++) {
        body *b=&sim->bodies[i];
        glBegin(GL_LINE_STRIP);
        for (int j=0; j<TRAIL_LENGTH;j++) {
            int idx=(b->trail_index+j)%TRAIL_LENGTH;
            float alpha=(float)j/TRAIL_LENGTH;
            glColor4f(0.7f,0.8f,1.0f,alpha*0.17f);
            glVertex2f((float)b->trail_x[idx],(float)b->trail_y[idx]);

        }
        glEnd();
    }


    glEnd();
}

static void renderStars(Simulation* sim, int massiveCount, double baryX, double baryY, float scale) {
    float zoomFactor = 1.0f / sqrt(scale);

    for (int i = massiveCount; i < sim->num_bodies; i++) {
        body* b = &sim->bodies[i];

        double dx = b->x - baryX;
        double dy = b->y - baryY;
        double dist = sqrt(dx * dx + dy * dy);

        float distFade = (dist > 150.0) ? (float)(150.0 / dist) : 1.0f;

        double speed = sqrt(b->vx * b->vx + b->vy * b->vy);
        float intensity = clamp01((float)(speed / STAR_SPEED_SCALE));

        // Particle color is derived from velocity.
        // Faster particles appear hotter and brighter.
        float red = mixf(0.45f, 1.0f, intensity);
        float green = mixf(0.55f, 0.85f, intensity);
        float blue = 1.0f;

        // Faster stars appear brighter and larger
        float pointSize;
        float alpha;

        if (speed > 15.0) {
            // Fast stars near massive bodies should stand out,
            // but not overpower the rest of the galaxy
            pointSize = 2.5f * zoomFactor;
            alpha = 0.62f * distFade;
        } else if (speed > 8.0) {
            pointSize = 1.9f * zoomFactor;
            alpha = 0.50f * distFade;
        } else {
            // Boost slower background stars slightly more
            // so the galaxy feels fuller and more luminous
            pointSize = 1.5f * zoomFactor;
            alpha = 0.42f * distFade;
        }

        // Soft outer glow
        // Glow strength scales more gently now so
        // high-speed stars do not dominate the frame
        float glowStrength = mixf(0.05f, 0.10f, intensity);

        glPointSize(pointSize * 1.9f);
        glBegin(GL_POINTS);
            glColor4f(red, green, blue, alpha * glowStrength);
            glVertex2f((float)b->x, (float)b->y);
        glEnd();

        // Bright star core
        glPointSize(pointSize);
        glBegin(GL_POINTS);
            glColor4f(red, green, blue, alpha);
            glVertex2f((float)b->x, (float)b->y);
        glEnd();
    }
}

static void drawCentralBody(const body* b, float scale, float massScale) { //bright center + glow
    float zoomFactor = 1.0f / scale;

    // Outer soft glow
    glPointSize(90.0f * massScale * zoomFactor);
    glBegin(GL_POINTS);
        glColor4f(0.25f, 0.35f, 0.85f, 0.05f);
        glVertex2f((float)b->x, (float)b->y);
    glEnd();

    // Inner brighter glow
    glPointSize(42.0f * massScale * zoomFactor);
    glBegin(GL_POINTS);
        glColor4f(0.85f, 0.75f, 1.0f, 0.12f);
        glVertex2f((float)b->x, (float)b->y);
    glEnd();

    // Bright center core
    glPointSize(10.0f * massScale * zoomFactor);
    glBegin(GL_POINTS);
        glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
        glVertex2f((float)b->x, (float)b->y);
    glEnd();
}

int main() {
    int N = 2000;
    double G = 1.0;
    // Base integration step remains constant for engine accuracy
    double base_dt = 0.008; 

    Simulation* sim = init_simulation(N, G, base_dt);
    if (sim == NULL || sim->bodies == NULL) {
        std::cout << "Engine failed to initialize memory!" << std::endl;
        return -1;
    }

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 800, "Binary Central Body Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Persistent transformation states
    float scale = 1.0f; 
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float moveSpeed = 150.0f; // Scaled up to accommodate delta time normalization

    // Initialize time tracking for frame-rate independence
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // Calculate dynamic Delta Time
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Pass frame-normalized parameter to the physics cycle
        sim->dt = base_dt * (deltaTime * 60.0); // Calibrated to baseline 60 FPS target
        step_simulation(sim);

        // Frame rate independent interactive panning
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) offsetY += moveSpeed * deltaTime * scale;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) offsetY -= moveSpeed * deltaTime * scale;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) offsetX -= moveSpeed * deltaTime * scale;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) offsetX += moveSpeed * deltaTime * scale;

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)   scale *= (1.0f - 0.5f * deltaTime); 
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) scale *= (1.0f + 0.5f * deltaTime);

        glClearColor(0.006f, 0.008f, 0.018f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Visual Quality Configuration
        glEnable(GL_POINT_SMOOTH);
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho((-200.0f * scale) + offsetX, (200.0f * scale) + offsetX, 
                (-200.0f * scale) + offsetY, (200.0f * scale) + offsetY, 
                -1, 1);

        int massiveCount = std::max(1, sim->massive_body_count);
        double massSum = 0.0;
        double baryX = 0.0;
        double baryY = 0.0;
        for (int i = 0; i < massiveCount && i < sim->num_bodies; i++) {
            baryX += sim->bodies[i].x * sim->bodies[i].mass;
            baryY += sim->bodies[i].y * sim->bodies[i].mass;
            massSum += sim->bodies[i].mass;
        }
        if (massSum > 0.0) {
            baryX /= massSum;
            baryY /= massSum;
        }
        float focusX = (float)baryX;
        float focusY = (float)baryY;

        // Rendering pipeline:
        // 1. Background glow for large-scale galactic atmosphere
        // 2. Motion trails to visualize velocity and orbital motion
        // 3. Stars and interstellar particles
        // 4. Massive central bodies with additive glow

        // PASS 1: Background galactic glow
        renderBackgroundGlow(focusX, focusY, scale);

        // PASS 2: Motion trails
        renderTrails(sim, massiveCount, baryX, baryY, scale);

        // PASS 3: Orbiting stars and dust
        renderStars(sim, massiveCount, baryX, baryY, scale);

        // PASS 4: Binary black hole accretion glow
        if (massiveCount >= 2) {
            body* a = &sim->bodies[0];
            body* b = &sim->bodies[1];
            glLineWidth(2.0f * (1.0f / sqrt(scale)));
            glBegin(GL_LINES);
                glColor4f(0.35f, 0.08f, 0.75f, 0.12f);
                glVertex2f((float)a->x, (float)a->y);
                glColor4f(1.0f, 0.46f, 0.08f, 0.02f);
                glVertex2f((float)b->x, (float)b->y);
            glEnd();
        }

        for (int i = 0; i < massiveCount && i < sim->num_bodies; i++) {
            body* hole = &sim->bodies[i];
            float massScale = (float)cbrt(std::max(1.0, hole->mass) / 2800.0);
            drawCentralBody(hole, scale, massScale);
        }

        glDisable(GL_BLEND);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POINT_SMOOTH);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free_simulation(sim);
    glfwTerminate();
    return 0;
}
