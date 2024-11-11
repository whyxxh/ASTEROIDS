#include <raylib.h>
#include <time.h>
#include <math.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>

// Type aliases and constants
typedef Vector2 v2;

static const int WIN_W = 800;
static const int WIN_H = 600;
static const int SCALE = 7;

static const float ROT_SPEED = 4.0f;
static const float PLAYER_ACCEL = 100.0f;

static const int INIT_ASTEROIDS_NUM = 5;
static const int MAX_ASTEROIDS_NUM = 50;
static const int MAX_BULLETS = 10;
int bulletCount = 0;
int asteroidNum = INIT_ASTEROIDS_NUM;


// Ship model coordinates
static const float SHIP[3][2][2] = {
    {{0, -2.5}, {-1.2, 2.5}},
    {{0, -2.5}, {1.2, 2.5}},
    {{-1.2, 2.5}, {1.2, 2.5}}
};

// Struct definitions
typedef struct {
    v2 pos;
    v2 vel;
    float dir;
    float rad;
} Asteroid;

typedef struct {
    v2 pos;
    v2 vel;
    float dir;
    int isAlive;
} Player;

typedef struct {
    v2 pos;
    v2 vel;
    int active;
} Bullet;

// Utility functions
float randFloat(int min, int max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

Asteroid createAsteroid(Player *p, int rad, v2 origin) {
    return (Asteroid){
        .pos = origin,
        .vel = (v2){randFloat(-50, 50), randFloat(-50, 50)},
        .rad = randFloat(rad - 5, rad + 5)
    };
}

void initGame(Player *p, Asteroid *a, Bullet *b) {
    InitWindow(WIN_W, WIN_H, "ASTEROIDS");
    SetTargetFPS(60);
    
    // Initialize player
    *p = (Player){
        .pos = (v2){WIN_W / 2.0f, WIN_H / 2.0f},
        .vel = (v2){0, 0},
        .dir = 0.0f,
        .isAlive = 1
    };

    // Initialize asteroids
    v2 spawnPos;
    do {
        spawnPos = (v2){randFloat(0, WIN_W), randFloat(0, WIN_H)};
    } while (fabs(p->pos.x - spawnPos.x) <= 100 && fabs(p->pos.y - spawnPos.y) <= 100);

    for (int i = 0; i < INIT_ASTEROIDS_NUM; i++) {
        a[i] = createAsteroid(p, 40, spawnPos);
    }

    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        b[i].active = 0;
    }


}

// Drawing functions
void drawShip(const Player p) {
    for (int i = 0; i < 3; i++) {
        v2 aPos = Vector2Scale((v2){SHIP[i][0][0], SHIP[i][0][1]}, SCALE);
        v2 bPos = Vector2Scale((v2){SHIP[i][1][0], SHIP[i][1][1]}, SCALE);

        aPos = Vector2Add(Vector2Rotate(aPos, p.dir), p.pos);
        bPos = Vector2Add(Vector2Rotate(bPos, p.dir), p.pos);

        DrawLineV(aPos, bPos, WHITE);
    }
}

void drawAsteroid(const Asteroid a) {
    DrawCircleLines(a.pos.x, a.pos.y, a.rad, WHITE);
}

void drawBullets(const Bullet *b) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            DrawCircleV(b[i].pos, 2, WHITE);
        }
    }
}

// update functions
void updatePlayer(Player *p, float dt) {
    if (IsKeyDown(KEY_A)) p->dir -= ROT_SPEED * dt;
    if (IsKeyDown(KEY_D)) p->dir += ROT_SPEED * dt;
    float moveDir = p->dir - PI / 2;

    // Apply acceleration
    if (IsKeyDown(KEY_W)) {
        p->vel.x += cos(moveDir) * PLAYER_ACCEL * dt;
        p->vel.y += sin(moveDir) * PLAYER_ACCEL * dt;
    }
    if (IsKeyDown(KEY_S)) {
        p->vel.x -= cos(moveDir) * PLAYER_ACCEL * dt;
        p->vel.y -= sin(moveDir) * PLAYER_ACCEL * dt;
    }

    // Update position and wrap around screen edges
    p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, dt));
    if (p->pos.x > WIN_W) p->pos.x = 0;
    if (p->pos.x < 0) p->pos.x = WIN_W;
    if (p->pos.y > WIN_H) p->pos.y = 0;
    if (p->pos.y < 0) p->pos.y = WIN_H;

    // Apply friction
    p->vel = Vector2Scale(p->vel, 0.99f);
}

void updateAsteroids(Asteroid *a, float dt, Player *p, int asteroidShot) {
    if (asteroidShot >= 0) {
        if (asteroidShot + 2 <= MAX_ASTEROIDS_NUM && a[asteroidShot].rad >= 10) {
            for (int i = 0; i < 2; i++) {
                a[asteroidNum + i] = createAsteroid(p, a[asteroidShot].rad / 2, a[asteroidShot].pos);
            }
            asteroidNum += 2;
        }
        for (int i = asteroidShot; i < asteroidNum - 1; i++) {
            a[i] = a[i + 1];
        }
        asteroidNum--; 
    }

    for (int i = 0; i < asteroidNum; i++) {
        a[i].pos = Vector2Add(a[i].pos, Vector2Scale(a[i].vel, dt));
        if (a[i].pos.x > WIN_W) a[i].pos.x = 0;
        if (a[i].pos.x < 0) a[i].pos.x = WIN_W;
        if (a[i].pos.y > WIN_H) a[i].pos.y = 0;
        if (a[i].pos.y < 0) a[i].pos.y = WIN_H;
    }
}



void updateBullets(Bullet *b, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].active) {
            b[i].pos = Vector2Add(b[i].pos, Vector2Scale(b[i].vel, dt));

            if (b[i].pos.x < 0 || b[i].pos.x > WIN_W || b[i].pos.y < 0 || b[i].pos.y > WIN_H) {
                b[i].active = 0;
                bulletCount--;
            }
        }
    }
}

// checks
void checkBullets(Player *p, Bullet *b) {
    if (IsKeyPressed(KEY_SPACE) && bulletCount < MAX_BULLETS) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!b[i].active) {
                b[i] = (Bullet){
                    .pos = p->pos,
                    .vel = (v2){
                        cos(p->dir - PI / 2) * 500,
                        sin(p->dir - PI / 2) * 500
                    },
                    .active = 1
                };
                bulletCount++;
                break;
            }
        }
    }
}

void checkPlayerCollision(Player *p, Asteroid *a) {
    for (int i = 0; i < asteroidNum; i++) {
        float dx = fabs(p->pos.x - a[i].pos.x);
        float dy = fabs(p->pos.y - a[i].pos.y);

        float dist = dx * dx + dy * dy;
        
        float sumRadii = a[i].rad + 5;

        if (dist <= sumRadii * sumRadii) {
            p->isAlive = 0;
        }
    }
}

int checkAsteroidShot(Bullet *b, Asteroid *a) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        for (int j = 0; j < asteroidNum; j++) {
            float dx = b[i].pos.x - a[j].pos.x;  
            float dy = b[i].pos.y - a[j].pos.y;  
            if (b[i].active == 1 && dx * dx + dy * dy <= a[j].rad * a[j].rad) {
                b[i].active = 0;
                bulletCount--;
                return j; 
            }
        }
    }
    return -1;
}

// Update and Render functions
void update(Player *p, Asteroid *a, Bullet *b, float dt) {
    int asteroidShot = 0;
    checkPlayerCollision(p, a);
    asteroidShot = checkAsteroidShot(b, a);
    updatePlayer(p, dt);
    updateAsteroids(a, dt, p, asteroidShot);  
    checkBullets(p, b);
    updateBullets(b, dt);
}

void render(Player p, Asteroid *a, Bullet *b) {
    BeginDrawing();
    ClearBackground(BLACK);

    if (p.isAlive) { drawShip(p); }
    for (int i = 0; i < asteroidNum; i++) {
        drawAsteroid(a[i]);
    }
    drawBullets(b);

    EndDrawing();
}

// Main function
int main() {
    srand(time(NULL));
    Player p;
    Asteroid a[MAX_ASTEROIDS_NUM];
    Bullet b[MAX_BULLETS];
    
    initGame(&p, a, b);
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(&p, a, b, dt);
        render(p, a, b);
    }

    CloseWindow();
    return 0;
}
