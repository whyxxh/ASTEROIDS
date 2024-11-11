#include <raylib.h>
#include <string.h>
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

static const float SHIP[3][2][2] = {
    {{0, -3.0}, {-1.5, 1.5}},   // Left part of the ship
    {{0, -3.0}, {1.5, 1.5}},    // Right part of the ship
    {{-1.5, 1.5}, {1.5, 1.5}}   // Bottom part (spokes)
};


// Struct definitions
typedef struct {
    v2 pos;
    v2 vel;
    float dir;
    float size;
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
        .size = randFloat(rad - 2, rad + 2)
    };
}

void initGame(Player *p, Asteroid *a, Bullet *b) { 
    // Initialize player
    *p = (Player){
        .pos = (v2){WIN_W / 2.0f, WIN_H / 2.0f},
        .vel = (v2){0, 0},
        .dir = 0.0f,
        .isAlive = 1
    };

    // Initialize asteroidso
    memset(a, 0, sizeof(Asteroid) * MAX_ASTEROIDS_NUM);
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
    DrawCircleLines(a.pos.x, a.pos.y, a.size, WHITE);
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

void updateAsteroids(Asteroid *a, float dt, Player *p, int asteroidShot, Sound bangL, Sound bangM, Sound bangS) {
    if (asteroidShot >= 0) {
        a[asteroidShot].size > 30 ? PlaySound(bangL) : PlaySound(bangS);
        if (asteroidShot + 2 <= MAX_ASTEROIDS_NUM && a[asteroidShot].size >= 10) {
            for (int i = 0; i < 2; i++) {
                a[asteroidNum + i] = createAsteroid(p, a[asteroidShot].size / 2, a[asteroidShot].pos);
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
void checkBullets(Player *p, Bullet *b, Sound fire) {
    if (IsKeyPressed(KEY_SPACE) && bulletCount < MAX_BULLETS) {
        PlaySound(fire);
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
        
        float sumRadii = a[i].size + 5;

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
            if (b[i].active == 1 && dx * dx + dy * dy <= a[j].size * a[j].size) {
                b[i].active = 0;
                bulletCount--;
                return j; 
            }
        }
    }
    return -1;
}

// Update and Render functions
void update(Player *p, Asteroid *a, Bullet *b, float dt, Sound fire, Sound bangL, Sound bangM, Sound bangS) {
    int asteroidShot = 0;
    checkPlayerCollision(p, a);
    asteroidShot = checkAsteroidShot(b, a);
    updatePlayer(p, dt);
    updateAsteroids(a, dt, p, asteroidShot, bangL, bangM, bangS);  
    checkBullets(p, b, fire);
    updateBullets(b, dt);
}

void render(Player p, Asteroid *a, Bullet *b) {
    BeginDrawing();
    ClearBackground(BLACK);

    drawShip(p);
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

    InitWindow(WIN_W, WIN_H, "ASTEROIDS");
    InitAudioDevice();

    initGame(&p, a, b);
    
    Sound fire = LoadSound("assets/fire.wav");
    Sound bangLarge = LoadSound("assets/bangLarge.wav");
    Sound bangMed = LoadSound("assets/bangMedium.wav");
    Sound bangSmall = LoadSound("assets/bangSmall.wav");
    SetSoundVolume(fire, 1.0f);
    SetSoundVolume(bangLarge, 1.0f);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(&p, a, b, dt, fire, bangLarge, bangSmall, bangMed);
        if (!p.isAlive) {
               initGame(&p, a, b);
        }

        render(p, a, b);
    }

    UnloadSound(fire);
    UnloadSound(bangLarge);
    UnloadSound(bangSmall);
    UnloadSound(bangMed);

    CloseAudioDevice();

    CloseWindow();
    return 0;
}
