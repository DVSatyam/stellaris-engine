#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "engine.h"


static float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

static float mixf(float a, float b, float t) {
    return a + (b - a) * t;
}

int main() {
    int N = 1000;
    double G = 1.0;
    // Base integration step remains constant for engine accuracy
    double base_dt = 0.01; 

    Simulation* sim = init_simulation(N, G, base_dt);
    if (sim == NULL || sim->bodies == NULL) {
        std::cout << "Engine failed to initialize memory!" << std::endl;
        return -1;
    }

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 800, "Cinematic Galaxy Simulation", NULL, NULL);
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

        // Frame-rate independent interactive panning
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

        // PASS 1: Soft blue-violet galactic envelope
        float coreX = (float)sim->bodies[0].x;
        float coreY = (float)sim->bodies[0].y;

        glPointSize(190.0f * (1.0f / scale));
        glBegin(GL_POINTS);
            glColor4f(0.10f, 0.18f, 0.55f, 0.018f);
            glVertex2f(coreX, coreY);
        glEnd();

        glPointSize(118.0f * (1.0f / scale));
        glBegin(GL_POINTS);
            glColor4f(0.24f, 0.10f, 0.42f, 0.026f);
            glVertex2f(coreX, coreY);
        glEnd();

        // PASS 2: Faint stellar motion streaks
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        for (int i = 1; i < sim->num_bodies; i++) {
            body* b = &sim->bodies[i];

            double dist = sqrt(b->x * b->x + b->y * b->y);
            float distFade = clamp01((float)(1.0 - (dist / 170.0)));
            double speed = sqrt(b->vx * b->vx + b->vy * b->vy);
            float speedGlow = clamp01((float)(speed / 15.0));
            float armTint = 0.5f + 0.5f * (float)sin(atan2(b->y, b->x) * 2.0 + dist * 0.045);
            float trail = (0.55f + speedGlow * 1.2f) * (1.0f / scale);

            float r = mixf(0.25f, 1.0f, speedGlow);
            float g = mixf(0.46f, 0.74f, armTint);
            float blue = mixf(0.90f, 1.0f, 1.0f - speedGlow);

            glColor4f(r, g, blue, 0.060f * distFade);
            glVertex2f((float)b->x, (float)b->y);
            glColor4f(r, g, blue, 0.0f);
            glVertex2f((float)(b->x - b->vx * trail), (float)(b->y - b->vy * trail));
        }
        glEnd();

        // PASS 3: Interstellar dust and orbiting stars
        for (int i = 1; i < sim->num_bodies; i++) { 
            body* b = &sim->bodies[i];

            double dist = sqrt(b->x * b->x + b->y * b->y);
            float distFade = (dist > 128.0) ? (float)(128.0 / dist) : 1.0f;
            float edgeFade = clamp01((float)(1.0 - (dist / 180.0)));

            double speed = sqrt(b->vx * b->vx + b->vy * b->vy);
            float intensity = clamp01((float)(speed / 15.0)); 
            float armTint = 0.5f + 0.5f * (float)sin(atan2(b->y, b->x) * 2.0 + dist * 0.045);
            float twinkle = 0.72f + 0.28f * (float)sin(currentTime * 1.7 + i * 12.9898);

            float ptSize;
            float alpha;
            // Pseudo-random index assignments establish multi-layered visual depth
            if (i % 7 == 0) {
                ptSize = (3.0f + 1.4f * intensity) * (1.0f / sqrt(scale));  
                alpha = 0.78f * distFade * twinkle;
            } else if (i % 3 == 0) {
                ptSize = (2.0f + 0.8f * armTint) * (1.0f / sqrt(scale));  
                alpha = 0.46f * distFade;
            } else {
                ptSize = (1.0f + 0.5f * edgeFade) * (1.0f / sqrt(scale));  
                alpha = 0.24f * distFade;
            }

            float red = mixf(0.42f, 1.0f, intensity);
            float green = mixf(0.56f, 0.86f, armTint);
            float blue = mixf(0.92f, 1.0f, 1.0f - intensity * 0.35f);

            glPointSize(ptSize * 2.7f);
            glBegin(GL_POINTS);
                glColor4f(red, green, blue, alpha * 0.12f);
                glVertex2f((float)b->x, (float)b->y);
            glEnd();

            glPointSize(ptSize);
            glBegin(GL_POINTS);
                glColor4f(red, green, blue, alpha);
                glVertex2f((float)b->x, (float)b->y);
            glEnd();
        }

        // PASS 4: Multi-pass luminous core and warm dust bulge
        glPointSize(92.0f * (1.0f / scale)); 
        glBegin(GL_POINTS);
            glColor4f(1.0f, 0.32f, 0.08f, 0.050f); 
            glVertex2f(coreX, coreY);
        glEnd();

        glPointSize(52.0f * (1.0f / scale)); 
        glBegin(GL_POINTS);
            glColor4f(1.0f, 0.57f, 0.18f, 0.105f); 
            glVertex2f(coreX, coreY);
        glEnd();

        glPointSize(24.0f * (1.0f / scale));
        glBegin(GL_POINTS);
            glColor4f(1.0f, 0.82f, 0.42f, 0.28f);
            glVertex2f(coreX, coreY);
        glEnd();

        glPointSize(9.0f * (1.0f / scale));
        glBegin(GL_POINTS);
            glColor4f(1.0f, 0.97f, 0.86f, 0.98f);
            glVertex2f(coreX, coreY);
        glEnd();

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
