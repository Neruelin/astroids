#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#define PI 3.141592654
#include "GL/freeglut.h"
#include "lib.c"
#define SCREENWIDTH 1200
#define SCREENHEIGHT 800
#define SHIPHEIGHT 10
#define SHIPWIDTH 10
#define TARGETFPS 60.0
#define PLAYERSPEED 100.0
#define PLAYERTURNSPEED 0.5
#define ACTIVEASTEROIDS 10
#define BULLETSPEED 500
#define BULLETCDSECS 0.1

float xScreen(float pixelX) {
    return ((pixelX / SCREENWIDTH) * 2) - 1.0;
}
float yScreen(float pixelY) {
    return  ((-pixelY / SCREENHEIGHT) * 2) + 1.0;
}

void triDraw(int pixelX, int pixelY, float rot, int pixelW, int pixelH) {
    glBegin(GL_TRIANGLES);
    //                   .  <- tip
    //                  .@.  @ = position 
    // left corner ->  ..... <- right corner
    double rads = rot * 2 * PI;
    double tipRads = rads;
    double lCornerRads = rads + (2.0 * PI / 3.0);
    double rCornerRads = rads + (2.0 * 2.0 * PI / 3.0);
    glVertex3f(
        xScreen(pixelX + (pixelH * cosf(rads))), 
        yScreen(pixelY + (pixelH * sinf(rads))),
        0);
    glVertex3f(
        xScreen(pixelX + (0.5 * pixelH * cosf(rCornerRads))), 
        yScreen(pixelY + (0.5 * pixelH * sinf(rCornerRads))),
        0);
    glVertex3f(
        xScreen(pixelX + (0.5 * pixelH * cosf(lCornerRads))), 
        yScreen(pixelY + (0.5 * pixelH * sinf(lCornerRads))),
        0);
    glEnd();
}

void quadDraw(int pixelX, int pixelY, float rot, int pixelW, int pixelH) {
    glBegin(GL_QUADS);
    for (int i = 0; i < 4; i++) {
        double rads = (rot + (i * 0.25)) * 2 * PI;
        glVertex3f(
            xScreen(pixelX + (pixelW * cos(rads))),
            yScreen(pixelY + (pixelW * sin(rads))),0);    
    }
    glEnd();
}

void shipDraw(int pixelX, int pixelY, float rot, float r, float g, float b) {
    glColor3f(r,g,b);
    triDraw(pixelX, pixelY, rot, SHIPWIDTH, SHIPHEIGHT);
}

void asteroidDraw(int pixelX, int pixelY, float rot, float r, float g, float b) {
    glColor3f(r,g,b);
    quadDraw(pixelX, pixelY, rot, 10, 10);
}

void bulletDraw(int pixelX, int pixelY, float rot, float r, float g, float b) {
    glColor3f(r,g,b);
    quadDraw(pixelX, pixelY, rot, 3, 1);
}

void handleDisplay(void* data) {
    GameData* gd = (GameData*) data;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT); 
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    shipDraw(300 + (200 * cos(0.5 * t)), 300 + (100 * sin(t)), t, 1.0, 1.0, 1.0);
    // Draw player
    shipDraw(gd->player.x, gd->player.y, gd->player.rot, 1.0, 1.0, 1.0);
    // Draw asteroids
    for (int i = 0; i < 256; i++) {
        if (gd->asteroids[i].active) {
            asteroidDraw(gd->asteroids[i].x, gd->asteroids[i].y, gd->asteroids[i].rot, 1.0, 0.2, 0.2);
        }
    }
    // Draw bullets
    for (int i = 0; i < 256; i++) {
        if (gd->bullets[i].active) {
            bulletDraw(gd->bullets[i].x, gd->bullets[i].y, gd->bullets[i].rot, 0.2, 1.0, 0.2);
        }
    }
    glutSwapBuffers();
}

void handleKeyboard(unsigned char key, int x, int y, void* data) {
    GameData* gd = (GameData*) data;

    if (key < 128) {
        gd->keyPressed[key] = 1;
    }
}

void handleKeyboardUp(unsigned char key, int x, int y, void* data) {
    GameData* gd = (GameData*) data;

    if (key < 128) {
        gd->keyPressed[key] = 0;
    }
}

void initGLUT(GameData* gd) {
    glutInitWindowSize(SCREENWIDTH, SCREENHEIGHT);
    glutInitWindowPosition(40, 40);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    int pargc = 0;
    glutInit(&pargc, NULL);
    int mainWindow = glutCreateWindow("asteroids");
    glutSetKeyRepeat(0);
    glutDisplayFuncUcall(handleDisplay, gd);
    glutKeyboardFuncUcall(handleKeyboard, gd);
    glutKeyboardUpFuncUcall(handleKeyboardUp, gd);
}

int handleGame(GameData* gd, double t, double delta) {

    // Handle WASD input to update player rotation and speeds
    float speed = (gd->keyPressed['w'] - gd->keyPressed['s']) * PLAYERSPEED;
    float rads = gd->player.rot * 2 * PI;
    gd->player.deltaRot = (gd->keyPressed['d'] - gd->keyPressed['a']) * PLAYERTURNSPEED;
    gd->player.deltaX = speed * cosf(rads);
    gd->player.deltaY = speed * sinf(rads);
    
    // Handle Spacebar input to spawn bullet
    if (gd->keyPressed[' '] && t >= gd->bulletReadyTime) {
        int bulletIdx = 0;
        while (bulletIdx < 256 && gd->bullets[bulletIdx].active) bulletIdx++;
        if (bulletIdx < 256) {
            gd->bullets[bulletIdx].active = 1;
            gd->bullets[bulletIdx].x = gd->player.x;
            gd->bullets[bulletIdx].y = gd->player.y;
            gd->bullets[bulletIdx].rot = gd->player.rot;
            gd->bullets[bulletIdx].deltaX = BULLETSPEED * cosf(gd->bullets[bulletIdx].rot * 2 * PI);
            gd->bullets[bulletIdx].deltaY = BULLETSPEED * sinf(gd->bullets[bulletIdx].rot * 2 * PI);
            gd->bulletReadyTime = t + BULLETCDSECS;
        }
    }

    // Delete offscreen asteroids
    for (int i = 0; i < 256; i++) {
        if (gd->asteroids[i].active) {
            float x = gd->asteroids[i].x;
            float y = gd->asteroids[i].y;
            if (x < 0 || x > SCREENWIDTH || y < 0 || y > SCREENHEIGHT) {
                Kinematic_init(&(gd->asteroids[i]));
                gd->activeAsteroids--;
            }
        }
    }

    // Create new asteroids up to target
    int asteroidIdx = 0;
    for (int i = ACTIVEASTEROIDS - gd->activeAsteroids; i >= 0; i--) {
        while (asteroidIdx < 256 && gd->asteroids[asteroidIdx].active) asteroidIdx++;
        if (asteroidIdx >= 256) {
            break;
        }
        gd->asteroids[asteroidIdx].active = 1;
        gd->asteroids[asteroidIdx].x = rand() % SCREENWIDTH;
        gd->asteroids[asteroidIdx].y = 1;
        gd->asteroids[asteroidIdx].rot = 1;
        gd->asteroids[asteroidIdx].deltaX = -30 + (rand() % 60);
        gd->asteroids[asteroidIdx].deltaY = 20 + (rand() % 30);
        gd->asteroids[asteroidIdx].deltaRot = 1.0;
        asteroidIdx++;
        gd->activeAsteroids++;
    }

    // Check collisions between bullets and asteroids and delete both
    for (int i = 0; i < 256; i++) {
        if (!gd->bullets[i].active) continue;
        for (int j = 0; j < 256; j++) {
            if (!gd->asteroids[j].active) continue;
            if (Kinematic_checkCircularCollision(&(gd->bullets[i]), &(gd->asteroids[j]), 5.0, 5.0)) {
                Kinematic_init(&(gd->bullets[i]));
                Kinematic_init(&(gd->asteroids[j]));
                gd->activeAsteroids--;
                break;
            }
        }
    }

    // Check collisions between player and asteroids and end game
    for (int i = 0; i < 256; i++) {
        if (!gd->asteroids[i].active) continue;
        if (Kinematic_checkCircularCollision(&(gd->player), &(gd->asteroids[i]), 5.0, 5.0)) {
            return 0;
        }
    }

    // Handle all kinematic object movements
    GameData_handleUpdate(gd, delta);

    return 1;
}

void loop(GameData* gd) {
    int running = 1;
    double nextFrameTime = 0.0;
    while (running) {
        const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

        if (t < nextFrameTime) {
            continue;
        }   
        
        running = handleGame(gd, t, t - (nextFrameTime - (1.0 / TARGETFPS)));

        glutPostRedisplay();
        glutMainLoopEvent();

        nextFrameTime = t + (1.0 / TARGETFPS);
    }
    int _;
    scanf("%d", &_);
}

int main() {

    GameData gd;
    GameData_init(&gd);

    gd.player.deltaRot = 5.0;
    gd.player.x = 50;
    gd.player.y = 50;

    initGLUT(&gd);
    loop(&gd);

    // test();
    return 0;
}