#include <math.h>
#define PI 3.141592654
#define ASTEROID_CAP 256
#define ASTEROID_SPAWN_CAP ASTEROID_CAP * 0.5
#define ASTEROID_DEFLECTION_ANGLE 0.3
#define BULLET_CAP 256
#define KEYS_CAP 127
#ifndef SCREENWIDTH
#define SCREENWIDTH 1200
#endif
#ifndef SCREENHEIGHT
#define SCREENHEIGHT 800
#endif
#ifndef PLAYERSPEED
#define PLAYERSPEED 200.0
#endif
#ifndef PLAYERTURNSPEED
#define PLAYERTURNSPEED 0.75
#endif
#ifndef BULLETCDSECS
#define BULLETCDSECS 0.1
#endif
#ifndef BULLETSPEED
#define BULLETSPEED 500
#endif
#ifndef ASTEROIDRADIUS
#define ASTEROIDRADIUS 10
#endif
#ifndef PLAYERRADIUS
#define PLAYERRADIUS 10
#endif
#ifndef BULLETRADIUS
#define BULLETRADIUS 10
#endif

typedef struct {
    int active;
    float x;
    float y;
    float rot;
    float deltaX;
    float deltaY;
    float deltaRot;
} Kinematic;

void Kinematic_init(Kinematic* k) {
    k->active = 0;
    k->x = 0.0;
    k->y = 0.0;
    k->rot = 0.0;
    k->deltaX = 0.0;
    k->deltaY = 0.0;
    k->deltaRot = 0.0;
}

void Kinematic_handleUpdate(Kinematic* k, float deltaT) {
    k->x += k->deltaX * deltaT;
    k->y += k->deltaY * deltaT;
    k->rot += k->deltaRot * deltaT;
}

int Kinematic_checkCircularCollision(Kinematic* k1, Kinematic* k2, float rad1, float rad2) {
    float distanceSqrd = powf(k1->x - k2->x, 2.0) + powf(k1->y - k2->y, 2);
    float radSqrd = powf(rad1 + rad2, 2.0);
    return distanceSqrd <= radSqrd ? 1 : 0;
}

typedef struct {
    Kinematic player;
    float bulletReadyTime;
    Kinematic asteroids[ASTEROID_CAP];
    int asteroidTier[ASTEROID_CAP];
    int activeAsteroids;
    Kinematic bullets[BULLET_CAP];
    int keyPressed[KEYS_CAP];
} GameData;

void GameData_init(GameData* gd) {
    Kinematic_init(&(gd->player));
    gd->player.active = 1;
    gd->bulletReadyTime = 0.0;
    for (int i = 0; i < ASTEROID_CAP; i++) {
        Kinematic_init(&(gd->asteroids[i]));
        gd->asteroidTier[i] = 0;
    }
    gd->activeAsteroids = 0;
    for (int i = 0; i < BULLET_CAP; i++) {
        Kinematic_init(&(gd->bullets[i]));
    }
    for (int i = 0; i < KEYS_CAP; i++) {
        gd->keyPressed[i] = 0;
    }
}

void GameData_handleKinematicUpdate(GameData* gd, float deltaT) {
    Kinematic_handleUpdate(&(gd->player), deltaT);
    for (int i = 0; i < ASTEROID_CAP; i++) {
        if (gd->asteroids[i].active) {
            Kinematic_handleUpdate(&(gd->asteroids[i]), deltaT);
        }
    }
    for (int i = 0; i < BULLET_CAP; i++) {
        if (gd->bullets[i].active) {
            Kinematic_handleUpdate(&(gd->bullets[i]), deltaT);
        }
    }
}

int GameData_spawnAsteroid(GameData* gd, float x, float y, float rot, float dx, float dy, float drot, int tier) {
    int asteroidIndex = -1;
    if (gd->activeAsteroids == ASTEROID_CAP) return asteroidIndex;

    for (int i = 0; i < ASTEROID_CAP; i++) {
        if (gd->asteroids[i].active) continue;
        gd->asteroids[i].active = 1;
        gd->asteroids[i].x = x;
        gd->asteroids[i].y = y;
        gd->asteroids[i].rot = rot;
        gd->asteroids[i].deltaX = dx;
        gd->asteroids[i].deltaY = dy;
        gd->asteroids[i].deltaRot = drot;
        gd->asteroidTier[i] = tier;
        gd->activeAsteroids++;
        asteroidIndex = i;
        break;
    }

    return asteroidIndex;
}

void GameData_deleteAsteroid(GameData* gd, int idx) {
    Kinematic_init(&(gd->asteroids[idx]));
    gd->asteroidTier[idx] = 0;
    gd->activeAsteroids--;
}

int GameData_spawnBullet(GameData* gd, float x, float y, float rot, float speed) {
    int bulletIndex = -1;

    for (int i = 0; i < BULLET_CAP; i++) {
        if (gd->bullets[i].active) continue;
        gd->bullets[i].active = 1;
        gd->bullets[i].x = x;
        gd->bullets[i].y = y;
        gd->bullets[i].rot = rot;
        gd->bullets[i].deltaX = speed * cosf(rot * 2 * PI);
        gd->bullets[i].deltaY = speed * sinf(rot * 2 * PI);
        gd->bullets[i].deltaRot = 0;
        bulletIndex = i;
        break;
    }

    return bulletIndex;
}

void GameData_deleteBullet(GameData* gd, int idx) {
    Kinematic_init(&(gd->bullets[idx]));
}

int GameData_handleAsteroidHit(GameData* gd, int bulletIdx, int asteroidIdx) {

    GameData_deleteBullet(gd, bulletIdx);
    if (gd->asteroids[asteroidIdx].active && gd->asteroidTier[asteroidIdx] == 1) {
        GameData_deleteAsteroid(gd, asteroidIdx);
        return 1;
    }

    Kinematic ast = gd->asteroids[asteroidIdx];
    int tier = gd->asteroidTier[asteroidIdx];

    float dirAngle = atan2f(ast.deltaY, ast.deltaX);
    float speed = sqrtf(powf(ast.deltaX,2) + powf(ast.deltaY, 2)) * 1.5;

    float leftAngle = dirAngle - ASTEROID_DEFLECTION_ANGLE;
    float rightAngle = dirAngle + ASTEROID_DEFLECTION_ANGLE;

    GameData_spawnAsteroid(gd, ast.x, ast.y, ast.rot, speed * cosf(leftAngle), speed * sinf(leftAngle), -ast.deltaRot, tier - 1);
    GameData_spawnAsteroid(gd, ast.x, ast.y, ast.rot, speed * cosf(rightAngle), speed * sinf(rightAngle), ast.deltaRot, tier - 1);
    GameData_deleteAsteroid(gd, asteroidIdx);

    return tier;
}

int GameData_handleUpdate(GameData* gd, double t, double delta) {

    // Handle WASD input to update player rotation and speeds
    float speed = (gd->keyPressed['w'] - gd->keyPressed['s']) * PLAYERSPEED;
    float rads = gd->player.rot * 2 * PI;
    gd->player.deltaRot = (gd->keyPressed['d'] - gd->keyPressed['a']) * PLAYERTURNSPEED;
    gd->player.deltaX = speed * cosf(rads);
    gd->player.deltaY = speed * sinf(rads);
    
    // Handle Spacebar input to spawn bullet
    if (gd->keyPressed[' '] && t >= gd->bulletReadyTime) {
        if (-1 != GameData_spawnBullet(gd, gd->player.x, gd->player.y, gd->player.rot, BULLETSPEED)) {
            gd->bulletReadyTime = t + BULLETCDSECS;
        }
    }

    // Delete offscreen asteroids
    for (int i = 0; i < ASTEROID_CAP; i++) {
        if (gd->asteroids[i].active) {
            float x = gd->asteroids[i].x;
            float y = gd->asteroids[i].y;
            if (x < 0 || x > SCREENWIDTH || y < 0 || y > SCREENHEIGHT) {
                GameData_deleteAsteroid(gd, i);
            }
        }
    }
    // Delete offscreen bullets
    for (int i = 0; i < BULLET_CAP; i++) {
        if (gd->bullets[i].active) {
            float x = gd->bullets[i].x;
            float y = gd->bullets[i].y;
            if (x < 0 || x > SCREENWIDTH || y < 0 || y > SCREENHEIGHT) {
                GameData_deleteBullet(gd, i);
            }
        }
    }

    // Create up to 5 new asteroids if possible
    for (int i = 0; i < 5 && gd->activeAsteroids < ASTEROID_SPAWN_CAP; i++) {
        GameData_spawnAsteroid(
            gd, rand() % SCREENWIDTH, 1, 1, -30 + (rand() % 60), 
            20 + (rand() % 30), 1, 1 + (rand() % 3));
        int side = rand() % 4;
        if (side == 0) { // top
            GameData_spawnAsteroid(gd, 
                rand() % SCREENWIDTH, 2, 1, 
                -30 + (rand() % 60), 15 + (rand() % 15), 1, 
                1 + (rand() % 3));
        } else if (side == 1) { // right
            GameData_spawnAsteroid(gd, 
                SCREENWIDTH - 2, 2 + rand() % (SCREENHEIGHT - 2), 1, 
                -15 - (rand() % 15), -30 + (rand() % 60), 1,
                 1 + (rand() % 3));
        } else if (side == 2) { // bottom
            GameData_spawnAsteroid(gd, 
                rand() % SCREENWIDTH, SCREENHEIGHT - 2, 1, 
                -30 + (rand() % 60), -15 - (rand() % 15), 1,
                1 + (rand() % 3));
        } else { // left
            GameData_spawnAsteroid(gd, 
                2, 2 + rand() % (SCREENHEIGHT - 2), 1, 
                15 + (rand() % 15), -30 + (rand() % 60), 1, 
                1 + (rand() % 3));
        }
    }

    // Check collisions between bullets and asteroids and delete both
    for (int i = 0; i < BULLET_CAP; i++) {
        if (!gd->bullets[i].active) continue;
        for (int j = 0; j < ASTEROID_CAP; j++) {
            if (!gd->asteroids[j].active) continue;
            // TODO asteroid radius scaling
            if (Kinematic_checkCircularCollision(&(gd->bullets[i]), &(gd->asteroids[j]), BULLETRADIUS, gd->asteroidTier[j] * ASTEROIDRADIUS)) {
                GameData_handleAsteroidHit(gd, i, j);
                break;
            }
        }
    }

    // Check collisions between player and asteroids and end game
    for (int i = 0; i < ASTEROID_CAP; i++) {
        if (!gd->asteroids[i].active) continue;
        if (Kinematic_checkCircularCollision(&(gd->player), &(gd->asteroids[i]), PLAYERRADIUS, gd->asteroidTier[i] * ASTEROIDRADIUS)) {
            return 0;
        }
    }

    // Handle all kinematic object movements
    GameData_handleKinematicUpdate(gd, delta);

    return 1;
}