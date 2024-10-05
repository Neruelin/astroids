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
    Kinematic asteroids[256];
    int activeAsteroids;
    Kinematic bullets[256];
    int keyPressed[128];
} GameData;

void GameData_init(GameData* gd) {
    Kinematic_init(&(gd->player));
    gd->player.active = 1;
    gd->bulletReadyTime = 0.0;
    for (int i = 0; i < 256; i++) {
        Kinematic_init(&(gd->asteroids[i]));
    }
    gd->activeAsteroids = 0;
    for (int i = 0; i < 256; i++) {
        Kinematic_init(&(gd->bullets[i]));
    }
    for (int i = 0; i < 128; i++) {
        gd->keyPressed[i] = 0;
    }
}

void GameData_handleUpdate(GameData* gd, float deltaT) {
    Kinematic_handleUpdate(&(gd->player), deltaT);
    for (int i = 0; i < 256; i++) {
        if (gd->asteroids[i].active) {
            Kinematic_handleUpdate(&(gd->asteroids[i]), deltaT);
        }
    }
    for (int i = 0; i < 256; i++) {
        if (gd->bullets[i].active) {
            Kinematic_handleUpdate(&(gd->bullets[i]), deltaT);
        }
    }
}