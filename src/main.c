#include <raylib.h>

typedef Vector2 v2;

static const int WIN_W = 800;
static const int WIN_H = 600;
static const int SCALE = 5;

const float SHIP_SHAPE[3][2][2] = {
    {{0, -10}, {5, 10}},   
    {{5, 10}, {-5, 10}},   
    {{-5, 10}, {0, -10}}   
};

const float ASTEROID_SHAPE[5][2][2] = {
    {{-10, -8}, {10, -10}},
    {{10, -10}, {12, 5}},
    {{12, 5}, {0, 12}},
    {{0, 12}, {-12, 5}},
    {{-12, 5}, {-10, -8}}
};

typedef struct {
    v2 pos;
    v2 vel;
    float dir;
} Player;

void initGame(Player *p) {
    InitWindow(WIN_W, WIN_H, "ASTEROIDS");
    SetTargetFPS(60);
    // sets player init state
    p->pos.x = 0; p->pos.y = 0;
    p->vel.x = 0; p->vel.y = 0;
}

void drwShip() {
    int x = 0; int y = 1;
    int a = 0; int b = 1;
    for (int i = 0; i < 3; i++) {
        DrawLine(SHIP_SHAPE[i][a][x] , SHIP_SHAPE[i][a][y], SHIP_SHAPE[i][b][x], SHIP_SHAPE[i][b][y], WHITE);
    }
}

void update() {

}

void render() {
    BeginDrawing();
    drwShip();
    EndDrawing();
}

int main() {
    Player p;
    initGame(&p);
    while (!WindowShouldClose()) {
        update();
        render();
    }
    CloseWindow();
    return 0;
}
