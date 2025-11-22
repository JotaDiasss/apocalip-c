// apocalip C - simple bullet-hell zombie survival in C using SDL2
// Controls: WASD to move, mouse to aim, left mouse button or automatic fire to shoot

#include <SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_W 1024
#define WINDOW_H 768

#define MAX_BULLETS 512 
#define MAX_ENEMIES 256

typedef struct {
    float x, y;
    float vx, vy;
    bool alive;
    float life; // optional
} Bullet;

typedef struct {
    float x, y;
    float speed;
    bool alive;
    int hp;
} Enemy;

typedef struct {
    float x, y;
    float speed;
    int hp;
} Player;

static Bullet bullets[MAX_BULLETS];
static Enemy enemies[MAX_ENEMIES];
static Player player;

static Uint32 last_shot_time = 0;
static Uint32 last_enemy_spawn = 0;
static int score = 0;
static bool running = true;

float randf(float a, float b) { return a + (b - a) * ((float)rand() / (float)RAND_MAX); }

void spawn_bullet(float px, float py, float tx, float ty) {
    // find free bullet
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!bullets[i].alive) {
            float dx = tx - px;
            float dy = ty - py;
            float len = sqrtf(dx*dx + dy*dy);
            if (len < 0.001f) len = 1.0f;
            dx /= len; dy /= len;
            bullets[i].x = px;
            bullets[i].y = py;
            bullets[i].vx = dx * 600.0f; // speed pixels/s
            bullets[i].vy = dy * 600.0f;
            bullets[i].alive = true;
            bullets[i].life = 2.0f;
            break;
        }
    }
}

void spawn_enemy() {
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!enemies[i].alive) {
            float side = randf(0.0f, 4.0f);
            float x, y;
            if (side < 1.0f) { x = randf(-50, -10); y = randf(-50, WINDOW_H + 50); }
            else if (side < 2.0f) { x = randf(WINDOW_W + 10, WINDOW_W + 50); y = randf(-50, WINDOW_H + 50); }
            else if (side < 3.0f) { x = randf(-50, WINDOW_W + 50); y = randf(-50, -10); }
            else { x = randf(-50, WINDOW_W + 50); y = randf(WINDOW_H + 10, WINDOW_H + 50); }
            enemies[i].x = x;
            enemies[i].y = y;
            enemies[i].speed = randf(40.0f, 120.0f);
            enemies[i].alive = true;
            enemies[i].hp = 1 + (rand() % 3);
            break;
        }
    }
}

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    srand((unsigned)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("apocalip C - Zombie Bullet Hell",
                                       SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if (!win) { fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError()); SDL_Quit(); return 1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) { fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError()); SDL_DestroyWindow(win); SDL_Quit(); return 1; }

    // init game state
    player.x = WINDOW_W * 0.5f;
    player.y = WINDOW_H * 0.5f;
    player.speed = 280.0f;
    player.hp = 5;

    for (int i = 0; i < MAX_BULLETS; ++i) bullets[i].alive = false;
    for (int i = 0; i < MAX_ENEMIES; ++i) enemies[i].alive = false;

    Uint32 last_time = SDL_GetTicks();
    const Uint32 shoot_interval_ms = 120; // auto-fire interval
    const Uint32 enemy_spawn_interval = 900;

    while (running) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_time) / 1000.0f;
        if (dt > 0.05f) dt = 0.05f; // clamp
        last_time = now;

        // events
        SDL_Event e;
        const Uint8* keystate = SDL_GetKeyboardState(NULL);
        int mx, my; Uint32 mouse_flags = SDL_GetMouseState(&mx, &my);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // movement
        float dx = 0, dy = 0;
        if (keystate[SDL_SCANCODE_W]) dy -= 1.0f;
        if (keystate[SDL_SCANCODE_S]) dy += 1.0f;
        if (keystate[SDL_SCANCODE_A]) dx -= 1.0f;
        if (keystate[SDL_SCANCODE_D]) dx += 1.0f;
        if (dx != 0 || dy != 0) {
            float len = sqrtf(dx*dx + dy*dy);
            dx /= len; dy /= len;
            player.x += dx * player.speed * dt;
            player.y += dy * player.speed * dt;
            if (player.x < 0) player.x = 0; if (player.x > WINDOW_W) player.x = WINDOW_W;
            if (player.y < 0) player.y = 0; if (player.y > WINDOW_H) player.y = WINDOW_H;
        }

        // shooting (auto-fire while left button down or automatic)
        bool mouse_down = (mouse_flags & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        if (mouse_down || true) { // set to `true` to always auto-fire
            if (now - last_shot_time >= shoot_interval_ms) {
                spawn_bullet(player.x, player.y, (float)mx, (float)my);
                last_shot_time = now;
            }
        }

        // spawn enemies
        if (now - last_enemy_spawn >= enemy_spawn_interval) {
            spawn_enemy();
            last_enemy_spawn = now;
        }

        // update bullets
        for (int i = 0; i < MAX_BULLETS; ++i) {
            if (!bullets[i].alive) continue;
            bullets[i].x += bullets[i].vx * dt;
            bullets[i].y += bullets[i].vy * dt;
            bullets[i].life -= dt;
            if (bullets[i].life <= 0.0f) bullets[i].alive = false;
            // out of bounds
            if (bullets[i].x < -20 || bullets[i].x > WINDOW_W + 20 || bullets[i].y < -20 || bullets[i].y > WINDOW_H + 20) bullets[i].alive = false;
        }

        // update enemies and check collision with bullets and player
        for (int i = 0; i < MAX_ENEMIES; ++i) {
            if (!enemies[i].alive) continue;
            float ex = enemies[i].x, ey = enemies[i].y;
            float pdx = player.x - ex;
            float pdy = player.y - ey;
            float plen = sqrtf(pdx*pdx + pdy*pdy);
            if (plen < 0.001f) plen = 1.0f;
            pdx /= plen; pdy /= plen;
            enemies[i].x += pdx * enemies[i].speed * dt;
            enemies[i].y += pdy * enemies[i].speed * dt;

            // collision with bullets
            for (int b = 0; b < MAX_BULLETS; ++b) {
                if (!bullets[b].alive) continue;
                float bx = bullets[b].x, by = bullets[b].y;
                float dxbe = bx - enemies[i].x;
                float dybe = by - enemies[i].y;
                float dist2 = dxbe*dxbe + dybe*dybe;
                if (dist2 < (14.0f * 14.0f)) {
                    bullets[b].alive = false;
                    enemies[i].hp -= 1;
                    if (enemies[i].hp <= 0) {
                        enemies[i].alive = false;
                        score += 10;
                    }
                    break;
                }
            }

            // collision with player
            float dxp = player.x - enemies[i].x;
            float dyp = player.y - enemies[i].y;
            if (dxp*dxp + dyp*dyp < (22.0f * 22.0f)) {
                enemies[i].alive = false;
                player.hp -= 1;
                if (player.hp <= 0) {
                    // game over
                    running = false;
                }
            }
        }

        // rendering
        SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
        SDL_RenderClear(ren);

        // draw player
        SDL_Rect pr = { (int)(player.x - 10), (int)(player.y - 10), 20, 20 };
        SDL_SetRenderDrawColor(ren, 180, 200, 80, 255);
        SDL_RenderFillRect(ren, &pr);

        // draw bullets
        SDL_SetRenderDrawColor(ren, 255, 220, 80, 255);
        for (int i = 0; i < MAX_BULLETS; ++i) {
            if (!bullets[i].alive) continue;
            SDL_Rect brect = { (int)(bullets[i].x - 3), (int)(bullets[i].y - 3), 6, 6 };
            SDL_RenderFillRect(ren, &brect);
        }

        // draw enemies
        SDL_SetRenderDrawColor(ren, 220, 60, 60, 255);
        for (int i = 0; i < MAX_ENEMIES; ++i) {
            if (!enemies[i].alive) continue;
            SDL_Rect er = { (int)(enemies[i].x - 12), (int)(enemies[i].y - 12), 24, 24 };
            SDL_RenderFillRect(ren, &er);
        }

        // HUD (simple)
        // draw HP as rectangles
        for (int i = 0; i < player.hp; ++i) {
            SDL_Rect h = { 8 + i*18, 8, 14, 14 };
            SDL_SetRenderDrawColor(ren, 200, 50, 50, 255);
            SDL_RenderFillRect(ren, &h);
        }

        // score - we won't render text (no font) but a simple rectangle bar indicates score roughly
        int barW = (score > 500) ? 500 : score;
        SDL_Rect sb = { 8, 30, barW / 2, 10 };
        SDL_SetRenderDrawColor(ren, 80, 160, 240, 255);
        SDL_RenderFillRect(ren, &sb);

        SDL_RenderPresent(ren);
    }

    // simple game over message to console
    printf("Game ended. Score: %d\n", score);

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
