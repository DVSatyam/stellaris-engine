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

static void drawAccretionGlow(const body* b, float scale, float massScale, float pulse) {
    float invScale = 1.0f / scale;

    glPointSize(118.0f * massScale * invScale);
    glBegin(GL_POINTS);
        glColor4f(0.05f, 0.12f, 0.42f, 0.055f * pulse);
        glVertex2f((float)b->x, (float)b->y);
    glEnd();

    glPointSize(68.0f * massScale * invScale);
    glBegin(GL_POINTS);
        glColor4f(0.42f, 0.11f, 0.72f, 0.075f * pulse);
        glVertex2f((float)b->x, (float)b->y);
    glEnd();

    glPointSize(28.0f * massScale * invScale);
    glBegin(GL_POINTS);
        glColor4f(1.0f, 0.42f, 0.10f, 0.18f * pulse);
        glVertex2f((float)b->x, (float)b->y);
    glEnd();

    glPointSize(9.0f * massScale * invScale);
    glBegin(GL_POINTS);
        glColor4f(1.0f, 0.96f, 0.82f, 0.96f);
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
    GLFWwindow* window = glfwCreateWindow(800, 800, "Binary Black Hole Simulation", NULL, NULL);
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

        // PASS 1: Soft blue-violet circumbinary envelope

        glPointSize(190.0f * (1.0f / scale));
        glBegin(GL_POINTS);
            glColor4f(0.10f, 0.18f, 0.55f, 0.018f);
            glVertex2f(focusX, focusY);
        glEnd();

        glPointSize(118.0f * (1.0f / scale));
        glBegin(GL_POINTS);
            glColor4f(0.24f, 0.10f, 0.42f, 0.026f);
            glVertex2f(focusX, focusY);
        glEnd();

        // PASS 2: Faint stellar motion streaks
        glLineWidth(1.0f);
        glBegin(GL_LINES);
        for (int i = massiveCount; i < sim->num_bodies; i++) {
            body* b = &sim->bodies[i];

            double dx = b->x - baryX;
            double dy = b->y - baryY;
            double dist = sqrt(dx * dx + dy * dy);
            float distFade = clamp01((float)(1.0 - (dist / 210.0)));
            double speed = sqrt(b->vx * b->vx + b->vy * b->vy);
            float speedGlow = clamp01((float)(speed / 19.0));
            float armTint = 0.5f + 0.5f * (float)sin(atan2(dy, dx) * 2.0 + dist * 0.045);
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
        for (int i = massiveCount; i < sim->num_bodies; i++) {
            body* b = &sim->bodies[i];

            double dx = b->x - baryX;
            double dy = b->y - baryY;
            double dist = sqrt(dx * dx + dy * dy);
            float distFade = (dist > 150.0) ? (float)(150.0 / dist) : 1.0f;
            float edgeFade = clamp01((float)(1.0 - (dist / 220.0)));

            double speed = sqrt(b->vx * b->vx + b->vy * b->vy);
            float intensity = clamp01((float)(speed / 19.0));
            float armTint = 0.5f + 0.5f * (float)sin(atan2(dy, dx) * 2.0 + dist * 0.045);
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
            float pulse = 0.88f + 0.12f * (float)sin(currentTime * 3.0 + i * 1.9);
            drawAccretionGlow(hole, scale, massScale, pulse);
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
