#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "GL/freeglut.h"
#define PI 3.141592654
#define SCREENWIDTH 1200
#define SCREENHEIGHT 800
#define SHIPHEIGHT 10
#define SHIPWIDTH 10
#define TARGETFPS 120.0
#define BULLETCDSECS 0.1
#include "lib.c"

// pixel to gl coords utils

float xScreen(float pixelX) {
    return ((pixelX / SCREENWIDTH) * 2) - 1.0;
}
float yScreen(float pixelY) {
    return  ((-pixelY / SCREENHEIGHT) * 2) + 1.0;
}

// 2D vertex transformation util struct and funcs

typedef struct {
    float a;
    float b;
} FPair;

FPair applyScale(FPair pair, float xScale, float yScale) {
    pair.a *= xScale;
    pair.b *= yScale;
    return pair;
}

FPair applyRot(FPair pair, float rot) {
    rot *= PI * 2;
    FPair result;
    float thetaRads = atan2f(pair.b, pair.a);
    float r = sqrtf(powf(pair.a, 2.0) + powf(pair.b, 2.0));
    result.a = r * cosf(thetaRads + rot);
    result.b = r * sinf(thetaRads + rot);
    return result;
}

// drawing utils

void triDraw(int pixelX, int pixelY, float rot, int pixelW, int pixelH) {
    glBegin(GL_TRIANGLES);

    float vertices[] = {
        0.5, 0.0,
        -0.5, -0.25,
        -0.5, 0.25
    };

    for (int i = 0; i < 6; i += 2) {
        FPair rotated = applyRot(applyScale((FPair) {.a = vertices[i], .b = vertices[i+1]}, 25, 25), rot);
        glVertex3f(xScreen(pixelX + rotated.a), yScreen(pixelY + rotated.b), 0);
    }

    glEnd();
}

void quadDraw(int pixelX, int pixelY, float rot, int pixelW, int pixelH) {
    glBegin(GL_QUADS);

    float vertices[] = {
        0.5, 0.5,
        0.5, -0.5,
        -0.5, -0.5,
        -0.5, 0.5
    };

    for (int i = 0; i < 8; i += 2) {
        FPair rotated = applyRot(applyScale((FPair) {.a = vertices[i], .b = vertices[i+1]}, pixelW, pixelH), rot);
        glVertex3f(xScreen(pixelX + rotated.a), yScreen(pixelY + rotated.b), 0);
    }
    glEnd();
}

void polyDraw(int pixelX, int pixelY, float vertices[], int vCount, float rot, int pixelW, int pixelH) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < vCount; i += 2) {
        FPair rotated = applyRot(applyScale((FPair) {.a = vertices[i], .b = vertices[i+1]}, pixelW, pixelH), rot);
        glVertex3f(xScreen(pixelX + rotated.a), yScreen(pixelY + rotated.b), 0);
    }
    glEnd();
}

void shipDraw(int pixelX, int pixelY, float rot, float r, float g, float b) {
    glColor3f(r,g,b);
    triDraw(pixelX, pixelY, rot, 2 * PLAYERRADIUS, 2 * PLAYERRADIUS);
}

void asteroidDraw(int pixelX, int pixelY, float rot, float scale, float r, float g, float b) {
    glColor3f(r,g,b);
    float vertices[] = {
        0.4, 0.1,
        0.3, -0.2,
        0.1, -0.3,
        -0.3, -0.2,
        -0.4, 0.2,
        -0.3, 0.4,
        0.0, 0.5,
        0.2, 0.4
    };
    // asteroid shape is irregular, slightly scaling up to cover collision area
    polyDraw(pixelX, pixelY, vertices, 16, rot, 2.5 * ASTEROIDRADIUS * scale, 2.5 * ASTEROIDRADIUS * scale);
}

void bulletDraw(int pixelX, int pixelY, float rot, float r, float g, float b) {
    glColor3f(r,g,b);
    quadDraw(pixelX, pixelY, rot, 30, 2);
}

// event callbacks

void handleDisplay(void* data) {
    GameData* gd = (GameData*) data;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT); 
    const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    // shipDraw(300 + (200 * cos(0.5 * t)), 300 + (100 * sin(t)), t, 1.0, 1.0, 1.0);
    // Draw player
    shipDraw(gd->player.x, gd->player.y, gd->player.rot, 1.0, 1.0, 1.0);
    // Draw asteroids
    for (int i = 0; i < ASTEROID_CAP; i++) {
        if (gd->asteroids[i].active) {
            asteroidDraw(gd->asteroids[i].x, gd->asteroids[i].y, gd->asteroids[i].rot, gd->asteroidTier[i], 1.0, (0.2 * gd->asteroidTier[i]), 0.2);
        }
    }
    // Draw bullets
    for (int i = 0; i < BULLET_CAP; i++) {
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

// glut init

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

// render loop

void loop(GameData* gd) {
    int running = 1;
    double nextFrameTime = 0.0;
    while (running) {
        const double t = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

        if (t < nextFrameTime) {
            continue;
        }   
        
        running = GameData_handleUpdate(gd, t, t - (nextFrameTime - (1.0 / TARGETFPS)));

        glutPostRedisplay();
        glutMainLoopEvent();

        nextFrameTime = t + (1.0 / TARGETFPS);
    }
    int _;
    printf("You have died\nPress enter to exit...\n");
    scanf("%d", &_);
}

int main() {

    GameData gd;
    GameData_init(&gd);

    gd.player.rot = 0.25;
    gd.player.x = SCREENWIDTH / 2;
    gd.player.y = SCREENHEIGHT / 2;

    initGLUT(&gd);
    loop(&gd);

    return 0;
}