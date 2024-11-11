#include <raylib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <raymath.h>
#include <stdlib.h>
#include <stdio.h>

// Constants and Type Aliases
typedef Vector2 v2;

const int WIN_W = 800;
const int WIN_H = 600;
const int SCALE = 7;

const float ROT_SPEED = 4.0f;
const float PLAYER_ACCEL = 100.0f;

const int INIT_ASTEROIDS_NUM = 5;
const int MAX_ASTEROIDS_NUM = 50;
const int MAX_BULLETS = 10;

// Ship Shape Definition
const float SHIP[3][2][2] = {
    {{0, -3.0f}, {-1.5f, 1.5f}},   // Left part of the ship
    {{0, -3.0f}, {1.5f, 1.5f}},    // Right part of the ship
    {{-1.5f, 1.5f}, {1.5f, 1.5f}}   // Bottom part (spokes)
};

// Struct Definitions 
typedef struct {
    int isPaused;
    int isStartScreen;
    int bulletCount;
    int asteroidNum;
} GameState;

GameState gameState;

typedef struct {
    v2 pos;
    v2 vel;
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
    int isActive;
} Bullet;

// Utility Functions
float randFloat(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

Asteroid createAsteroid(Player *player, float radius, v2 origin) {
    return (Asteroid){
        .pos = origin,
        .vel = (v2){randFloat(-50, 50), randFloat(-50, 50)},
        .size = randFloat(radius - 2, radius + 2)
    };
}

void initGame(Player *player, Asteroid *asteroidArr, Bullet *bulletArr) { 
    // initialize gamestate
    gameState = (GameState){
        .isPaused = 1,
        .isStartScreen = 0,
        .asteroidNum = INIT_ASTEROIDS_NUM,
        .bulletCount = 0
    };

    // Initialize Player
    *player = (Player){
        .pos = (v2){WIN_W / 2.0f, WIN_H / 2.0f},
        .vel = (v2){0, 0},
        .dir = 0.0f,
        .isAlive = 1
    };

    // Initialize Asteroids
    memset(asteroidArr, 0, sizeof(Asteroid) * MAX_ASTEROIDS_NUM);
    v2 spawnPos;
    for (int i = 0; i < INIT_ASTEROIDS_NUM; i++) {
        do {
            spawnPos = (v2){randFloat(0, WIN_W), randFloat(0, WIN_H)};
        } while (fabs(player->pos.x - spawnPos.x) <= 150 && fabs(player->pos.y - spawnPos.y) <= 150);

        asteroidArr[i] = createAsteroid(player, 40, spawnPos);     
    }
    
    // Initialize Bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        bulletArr[i].isActive = 0;
    }
}

// Drawing Functions
void drawShip(const Player player) {
    for (int i = 0; i < 3; i++) {
        v2 aPos = Vector2Scale((v2){SHIP[i][0][0], SHIP[i][0][1]}, SCALE);
        v2 bPos = Vector2Scale((v2){SHIP[i][1][0], SHIP[i][1][1]}, SCALE);

        aPos = Vector2Add(Vector2Rotate(aPos, player.dir), player.pos);
        bPos = Vector2Add(Vector2Rotate(bPos, player.dir), player.pos);

        DrawLineV(aPos, bPos, WHITE);
    }
}

void drawAsteroid(const Asteroid asteroid) {
    DrawCircleLines(asteroid.pos.x, asteroid.pos.y, asteroid.size, WHITE);
}

void drawBullets(const Bullet *bulletArr) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bulletArr[i].isActive) {
            DrawCircleV(bulletArr[i].pos, 2, WHITE);
        }
    }
}

// Update Functions
void updatePlayer(Player *player, float dt) {
    if (IsKeyDown(KEY_A)) player->dir -= ROT_SPEED * dt;
    if (IsKeyDown(KEY_D)) player->dir += ROT_SPEED * dt;
    float moveDir = player->dir - PI / 2;

    // Apply Acceleration
    if (IsKeyDown(KEY_W)) {
        player->vel.x += cos(moveDir) * PLAYER_ACCEL * dt;
        player->vel.y += sin(moveDir) * PLAYER_ACCEL * dt;
    }
    if (IsKeyDown(KEY_S)) {
        player->vel.x -= cos(moveDir) * PLAYER_ACCEL * dt;
        player->vel.y -= sin(moveDir) * PLAYER_ACCEL * dt;
    }

    // Update Position and Wrap Around Screen Edges
    player->pos = Vector2Add(player->pos, Vector2Scale(player->vel, dt));
    if (player->pos.x > WIN_W) player->pos.x = 0;
    if (player->pos.x < 0) player->pos.x = WIN_W;
    if (player->pos.y > WIN_H) player->pos.y = 0;
    if (player->pos.y < 0) player->pos.y = WIN_H;

    // Apply Friction
    player->vel = Vector2Scale(player->vel, 0.99f);
}

void updateAsteroids(Asteroid *asteroidArr, float dt, Player *player, int asteroidShot, Sound bangL, Sound bangS) {
    if (asteroidShot >= 0) {
        asteroidArr[asteroidShot].size > 30 ? PlaySound(bangL) : PlaySound(bangS);
        if (asteroidArr[asteroidShot].size >= 10) {
            for (int i = 0; i < 2; i++) {
                asteroidArr[gameState.asteroidNum + i] = createAsteroid(player, asteroidArr[asteroidShot].size / 2, asteroidArr[asteroidShot].pos);
            }
            gameState.asteroidNum += 2;
        }
        for (int i = asteroidShot; i < gameState.asteroidNum - 1; i++) {
            asteroidArr[i] = asteroidArr[i + 1];
        }
        gameState.asteroidNum--; 
    }

    for (int i = 0; i < gameState.asteroidNum; i++) {
        asteroidArr[i].pos = Vector2Add(asteroidArr[i].pos, Vector2Scale(asteroidArr[i].vel, dt));
        if (asteroidArr[i].pos.x > WIN_W) asteroidArr[i].pos.x = 0;
        if (asteroidArr[i].pos.x < 0) asteroidArr[i].pos.x = WIN_W;
        if (asteroidArr[i].pos.y > WIN_H) asteroidArr[i].pos.y = 0;
        if (asteroidArr[i].pos.y < 0) asteroidArr[i].pos.y = WIN_H;
    }
}

void updateBullets(Bullet *bulletArr, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bulletArr[i].isActive) {
            bulletArr[i].pos = Vector2Add(bulletArr[i].pos, Vector2Scale(bulletArr[i].vel, dt));

            if (bulletArr[i].pos.x < 0 || bulletArr[i].pos.x > WIN_W || bulletArr[i].pos.y < 0 || bulletArr[i].pos.y > WIN_H) {
                bulletArr[i].isActive = 0;
                gameState.bulletCount--;
            }
        }
    }
}

// Checks
void checkBullets(Player *player, Bullet *bulletArr, Sound fire) {
    if (IsKeyPressed(KEY_SPACE) && gameState.bulletCount < MAX_BULLETS) {
        PlaySound(fire);
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!bulletArr[i].isActive) {
                bulletArr[i] = (Bullet){
                    .pos = player->pos,
                    .vel = (v2){
                        cos(player->dir - PI / 2) * 500,
                        sin(player->dir - PI / 2) * 500
                    },
                    .isActive = 1
                };
                gameState.bulletCount++;
                break;
            }
        }
    }
}

void checkPlayerCollision(Player *player, Asteroid *asteroidArr) {
    for (int i = 0; i < gameState.asteroidNum; i++) {
        float dx = fabs(player->pos.x - asteroidArr[i].pos.x);
        float dy = fabs(player->pos.y - asteroidArr[i].pos.y);

        float dist = dx * dx + dy * dy;
        
        float sumRadii = asteroidArr[i].size + 5;

        if (dist <= sumRadii * sumRadii) {
            player->isAlive = 0;
        }
    }
}

int checkAsteroidShot(Bullet *bulletArr, Asteroid *asteroidArr) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        for (int j = 0; j < gameState.asteroidNum; j++) {
            float dx = bulletArr[i].pos.x - asteroidArr[j].pos.x;  
            float dy = bulletArr[i].pos.y - asteroidArr[j].pos.y;  
            if (bulletArr[i].isActive == 1 && dx * dx + dy * dy <= asteroidArr[j].size * asteroidArr[j].size) {
                bulletArr[i].isActive = 0;
                gameState.bulletCount--;
                return j; 
            }
        }
    }
    return -1;
}

// Update and Render Functions
void update(Player *player, Asteroid *asteroidArr, Bullet *bulletArr, float dt, Sound fire, Sound bangL, Sound bangS) {
    int asteroidShot = 0;
    checkPlayerCollision(player, asteroidArr);
    asteroidShot = checkAsteroidShot(bulletArr, asteroidArr);
    updatePlayer(player, dt);
    updateAsteroids(asteroidArr, dt, player, asteroidShot, bangL, bangS);  
    checkBullets(player, bulletArr, fire);
    updateBullets(bulletArr, dt);
}

void render(Player player, Asteroid *asteroidArr, Bullet *bulletArr) {
    BeginDrawing();
    ClearBackground(BLACK);

    drawShip(player);
    for (int i = 0; i < gameState.asteroidNum; i++) {
        drawAsteroid(asteroidArr[i]);
    }
    drawBullets(bulletArr);

    EndDrawing();
}

// Main Function
int main() {
    srand(time(NULL));

    Player player;
    Asteroid asteroidArr[MAX_ASTEROIDS_NUM];
    Bullet bulletArr[MAX_BULLETS];

    InitWindow(WIN_W, WIN_H, "ASTEROIDS");
    InitAudioDevice();

    initGame(&player, asteroidArr, bulletArr);
    
    Sound fire = LoadSound("assets/fire.wav");
    Sound bangLarge = LoadSound("assets/bangLarge.wav");
    Sound bangSmall = LoadSound("assets/bangSmall.wav");
    SetSoundVolume(fire, 1.0f);
    SetSoundVolume(bangLarge, 1.0f);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        update(&player, asteroidArr, bulletArr, dt, fire, bangLarge, bangSmall);
        if (!player.isAlive) {
               initGame(&player, asteroidArr, bulletArr);
        }

        render(player, asteroidArr, bulletArr);
    }

    UnloadSound(fire);
    UnloadSound(bangLarge);
    UnloadSound(bangSmall);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
