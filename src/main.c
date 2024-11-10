#include <raylib.h>
#include <time.h>
#include <math.h>
#include <raymath.h>
#include <stdlib.h>

// Type aliases and constants
typedef Vector2 v2;

static const int WIN_W = 800;
static const int WIN_H = 600;
static const int SCALE = 7;

static const float ROT_SPEED = 4;
static const float PLAYER_ACCEL = 100;

static const int INIT_ASTEROIDS_NUM = 5;

static const int MAX_BULLETS = 10;
int bulletCount = 0;

// Ship model coordinates
static const float SHIP[3][2][2] = {
    {{0, -2.5}, {-1.2, 2.5}},
    {{0, -2.5}, {1.2, 2.5}},
    {{-1.2, 2.5}, {1.2, 2.5}},
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
} Player;

typedef struct {
    v2 pos;
    v2 vel;
    int state;
} Bullet;

// Utility functions
float randFloat(int min, int max) {
    float range = (float)(max - min);
    return min + (float)rand() / (float)(RAND_MAX) * range;
}

Asteroid createAsteroid() {
    Asteroid a;
    a.pos = (v2){randFloat(0, WIN_W), randFloat(0, WIN_H)};
    a.vel = (v2){randFloat(-50, 50), randFloat(-50, 50)};
    a.size = randFloat(40, 60);
    return a;
}

void initGame(Player *p, Asteroid *a, Bullet *b) {
    InitWindow(WIN_W, WIN_H, "ASTEROIDS");
    SetTargetFPS(60);
    
    // Initialize player
    p->pos = (v2){ WIN_W / 2.0f, WIN_H / 2.0f };
    p->vel = (v2){ 0, 0 };
    p->dir = 0.0f; 

    // Initialize asteroids
    for (int i = 0; i < INIT_ASTEROIDS_NUM; i++) {
        a[i] = createAsteroid();
    }

    // initialize bullet array
    for (int i = 0; i < MAX_BULLETS; i++) {
        b[i].pos = p->pos;
        b[i].state = 0;
    } 

}

// Drawing functions
void drawShip(Player p) {
    int x = 0, y = 1, a = 0, b = 1;
    
    for (int i = 0; i < 3; i++) {
        // Scale and position the relative coordinates
        float xA = SHIP[i][a][x] * SCALE;
        float yA = SHIP[i][a][y] * SCALE;
        float xB = SHIP[i][b][x] * SCALE;
        float yB = SHIP[i][b][y] * SCALE;

        v2 aPos = {xA, yA};
        v2 bPos = {xB, yB};

        // Rotate around the origin, then translate to player position
        v2 rotAPos = Vector2Rotate(aPos, p.dir);
        v2 rotBPos = Vector2Rotate(bPos, p.dir);

        rotAPos = Vector2Add(rotAPos, p.pos);
        rotBPos = Vector2Add(rotBPos, p.pos);    

        DrawLineV(rotAPos, rotBPos, WHITE);
    }
}

void drawAsteroid(Asteroid a) {
    DrawCircleLines(a.pos.x, a.pos.y, a.size, WHITE);
}

void drawBullets(Bullet *b) {
    for (int i = 0; i <= MAX_BULLETS; i++) {
        if (b[i].state) 
            DrawCircleV(b[i].pos, 2, WHITE);
    }
}

// Movement functions
void movePlayer(Player *p, float dt) {
    // Rotation controls
    if (IsKeyDown(KEY_A)) { 
        p->dir -= ROT_SPEED * dt;
    }
    if (IsKeyDown(KEY_D)) { 
        p->dir += ROT_SPEED * dt; 
    }

    float moveDir = p->dir - PI / 2;

    // Acceleration controls
    if (IsKeyDown(KEY_W)) {
        p->vel.x += cos(moveDir) * PLAYER_ACCEL * dt; 
        p->vel.y += sin(moveDir) * PLAYER_ACCEL * dt; 
    }
    if (IsKeyDown(KEY_S)) {
        p->vel.x -= cos(moveDir) * PLAYER_ACCEL * dt; 
        p->vel.y -= sin(moveDir) * PLAYER_ACCEL * dt; 
    }

    // Update position based on velocity
    p->pos.x += p->vel.x * dt;
    p->pos.y += p->vel.y * dt;

    // Screen wrapping for player
    if (p->pos.x > WIN_W) p->pos.x = 0;
    if (p->pos.x < 0) p->pos.x = WIN_W;
    if (p->pos.y > WIN_H) p->pos.y = 0;
    if (p->pos.y < 0) p->pos.y = WIN_H;

    // Friction effect
    p->vel.x *= 0.99f;
    p->vel.y *= 0.99f;
}

void moveAsteroids(Asteroid *a, float dt) {
    for (int i = 0; i < INIT_ASTEROIDS_NUM; i++) {
        a[i].pos.x += a[i].vel.x * dt;
        a[i].pos.y += a[i].vel.y * dt;

        // Screen wrapping for asteroids
        if (a[i].pos.x > WIN_W) a[i].pos.x = 0;
        if (a[i].pos.x < 0) a[i].pos.x = WIN_W;
        if (a[i].pos.y > WIN_H) a[i].pos.y = 0;
        if (a[i].pos.y < 0) a[i].pos.y = WIN_H;
    }
}

void checkBullets(Player *p, Bullet *b) {
    if (IsKeyPressed(KEY_SPACE)) {
        if (bulletCount <= MAX_BULLETS) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!b[i].state) {
                    b[i].pos = p->pos;
                    b[i].vel.x = cos(p->dir) * 50;
                    b[i].vel.y = sin(p->dir) * 50;
                    b[i].state = 1;
                    bulletCount++;
                }
            }
        }
    }
}

void updateBullets(Bullet *b, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (b[i].state) {
            // Move active bullets
            b[i].pos.x += b[i].vel.x * dt;
            b[i].pos.y += b[i].vel.y * dt;

            // Deactivate bullets if they go off-screen
            if (b[i].pos.x < 0 || b[i].pos.x > WIN_W || b[i].pos.y < 0 || b[i].pos.y > WIN_H) {
                b[i].state = 0;
                bulletCount--;
            }
        }
    }
}


// Update and Render functions
void update(Player *p, Asteroid *a, Bullet *b, float dt) {
    movePlayer(p, dt);
    moveAsteroids(a, dt);  
    checkBullets(p, b);
    updateBullets(b, dt);
}


void render(Player p, Asteroid *a, Bullet *b) {
    BeginDrawing();
    ClearBackground(BLACK);

    drawShip(p);
    for (int i = 0; i < INIT_ASTEROIDS_NUM; i++) {
        drawAsteroid(a[i]);
    }
    drawBullets(b);
    EndDrawing();
}

// Main function
int main() {
    srand(time(NULL));
    Player p;
    Asteroid a[INIT_ASTEROIDS_NUM];
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
