#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "cli_lib.h"

#define W 800
#define H 600

typedef struct Player {
    float x, y;
    int lives;
} Player;

typedef struct Bullet {
    float x, y, vx, vy;
    struct Bullet* next;
} Bullet;

typedef struct Enemy {
    float x, y;
    float speed;
    int hp;
    int size;
    struct Enemy* next;
} Enemy;

typedef struct Grid {
    int rows, cols;
    int* cells;
} Grid;

Player player;
Bullet* bullets = NULL;
Enemy* enemies = NULL;
Grid spawn_grid;

int score = 0;
float game_time = 0.0f;
int running = 1;

// auto-fire / burst control (ms)
uint32_t last_shot_ms = 0;
uint32_t burst_last_fire_ms = 0;
int burst_shots = 0;
int last_printed_second = -1;

static float clampf(float v, float a, float b) { if (v < a) return a; if (v > b) return b; return v; }

void add_bullet(float x, float y, float vx, float vy) {
    Bullet* b = malloc(sizeof(Bullet));
    if (!b) return;
    b->x = x; b->y = y; b->vx = vx; b->vy = vy; b->next = bullets; bullets = b;
}

void free_bullets(void) {
    Bullet* cur = bullets;
    while (cur) { Bullet* nxt = cur->next; free(cur); cur = nxt; }
    bullets = NULL;
}

void add_enemy(float x, float y, int size, float speed) {
    Enemy* e = malloc(sizeof(Enemy));
    if (!e) return;
    e->x = x; e->y = y; e->size = size; e->speed = speed; e->hp = 3; e->next = enemies; enemies = e;
}

void free_enemies(void) {
    Enemy* cur = enemies;
    while (cur) { Enemy* nxt = cur->next; free(cur); cur = nxt; }
    enemies = NULL;
}

void grid_init(Grid* g, int rows, int cols) {
    g->rows = rows; g->cols = cols; g->cells = malloc(rows * cols * sizeof(int));
    if (!g->cells) { g->rows = g->cols = 0; return; }
    for (int i = 0; i < rows*cols; ++i) g->cells[i] = 1;
}

void grid_free(Grid* g) { if (g->cells) free(g->cells); g->cells = NULL; }

void spawn_wave(int wave_strength) {
    for (int i = 0; i < wave_strength; ++i) {
        int side = rand() % 4;
        float x,y;
        if (side == 0) { x = -20; y = rand() % H; }
        else if (side == 1) { x = W + 20; y = rand() % H; }
        else if (side == 2) { x = rand() % W; y = -20; }
        else { x = rand() % W; y = H + 20; }
        int size = 1 + (int)(game_time / 20.0f);
        if (size > 6) size = 6;
        float speed = 40.0f + (rand()%60) + game_time*0.1f;
        add_enemy(x,y,size,speed);
    }
}

void save_score(const char* name, int final_score, float time_run) {
    FILE* f = fopen("scores.txt","a");
    if (!f) return;
    fprintf(f, "%s %d %.2f\n", name, final_score, time_run);
    fclose(f);
}

void print_podium(void) {
    FILE* f = fopen("scores.txt","r");
    if (!f) { printf("No scores yet.\n"); return; }
    typedef struct S { char name[64]; int s; float t; } S;
    S arr[256]; int n=0;
    while (n < 256 && fscanf(f, "%63s %d %f", arr[n].name, &arr[n].s, &arr[n].t) == 3) n++;
    fclose(f);
    for (int i = 0; i < n; ++i) for (int j = i+1; j < n; ++j) if (arr[j].s > arr[i].s) { S tmp = arr[i]; arr[i]=arr[j]; arr[j]=tmp; }
    printf("--- PODIUM ---\n");
    for (int i = 0; i < 3 && i < n; ++i) printf("%d. %s  score=%d  time=%.1f\n", i+1, arr[i].name, arr[i].s, arr[i].t);
}

int circle_collide(float x1,float y1,float r1,float x2,float y2,float r2) {
    float dx = x1-x2, dy = y1-y2;
    return dx*dx + dy*dy <= (r1+r2)*(r1+r2);
}

int main(int argc, char* argv[]) {
    srand((unsigned)time(NULL));
    printf("Welcome to apocalip-c (simple)!\n");

    char name[64];
    printf("Enter your name: "); fflush(stdout);
    if (!fgets(name, sizeof(name), stdin)) strncpy(name, "Player", sizeof(name));
    name[strcspn(name, "\n")] = '\0';

    if (cli_init("apocalip-c", W, H) != 0) return 1;

    player.x = W/2; player.y = H/2; player.lives = 3;
    score = 0; game_time = 0.0f; running = 1;
    grid_init(&spawn_grid, 4, 4);

    const float tick_s = 1.0f/60.0f;
    float spawn_timer = 0.0f;
    int wave = 0;

    while (running) {
        uint32_t t0 = cli_ticks();
        CLI_Input in; cli_poll_input(&in);
        if (in.quit) break;

        float mvx=0, mvy=0;
        if (in.key_state[SDL_SCANCODE_W]) mvy -= 1.0f;
        if (in.key_state[SDL_SCANCODE_S]) mvy += 1.0f;
        if (in.key_state[SDL_SCANCODE_A]) mvx -= 1.0f;
        if (in.key_state[SDL_SCANCODE_D]) mvx += 1.0f;

        float mvlen = sqrtf(mvx*mvx + mvy*mvy);
        if (mvlen > 0.0f) { mvx/=mvlen; mvy/=mvlen; }

        player.x += mvx * 200.0f * tick_s;
        player.y += mvy * 200.0f * tick_s;
        player.x = clampf(player.x, 0, W);
        player.y = clampf(player.y, 0, H);

        // Auto-fire burst: every 1000ms start a burst of 4 shots spaced by ~80ms
        const uint32_t BURST_INTERVAL_MS = 1000;
        const uint32_t BURST_GAP_MS = 80;
        const int BURST_SIZE = 4;
        // start a new burst if enough time passed since last_shot_ms
        if (burst_shots == 0) {
            if ((t0 - last_shot_ms) >= BURST_INTERVAL_MS) {
                // prepare to fire immediately
                burst_shots = 0; /* will increment when firing */
                burst_last_fire_ms = t0 - BURST_GAP_MS;
                last_shot_ms = t0; // mark burst start
            }
        }
        // fire shots in burst
        if (burst_shots < BURST_SIZE) {
            if ((t0 - burst_last_fire_ms) >= BURST_GAP_MS) {
                float dx = in.mouse_x - player.x;
                float dy = in.mouse_y - player.y;
                float L = sqrtf(dx*dx + dy*dy); if (L < 1) L = 1;
                add_bullet(player.x, player.y, dx/L*400.0f, dy/L*400.0f);
                burst_shots += 1;
                burst_last_fire_ms = t0;
                if (burst_shots >= BURST_SIZE) {
                    // finished burst; reset so next burst waits BURST_INTERVAL_MS
                    burst_shots = 0;
                    // last_shot_ms already set to burst start above
                }
            }
        }

        for (Bullet** pb = &bullets; *pb;) {
            Bullet* b = *pb;
            b->x += b->vx * tick_s; b->y += b->vy * tick_s;
            if (b->x < -50 || b->x > W+50 || b->y < -50 || b->y > H+50) { *pb = b->next; free(b); }
            else pb = &b->next;
        }

        for (Enemy** pe = &enemies; *pe;) {
            Enemy* e = *pe;
            float dx = player.x - e->x;
            float dy = player.y - e->y;
            float L = sqrtf(dx*dx + dy*dy); if (L<1) L=1;
            e->x += (dx/L) * e->speed * tick_s;
            e->y += (dy/L) * e->speed * tick_s;

            int killed = 0;
            for (Bullet** pb = &bullets; *pb;) {
                Bullet* b = *pb;
                if (circle_collide(b->x,b->y,3, e->x,e->y, e->size*6)) {
                    e->hp -= 1;
                    *pb = b->next;
                    free(b);
                    if (e->hp <= 0) { killed = 1; break; }
                } else pb = &b->next;
            }
            if (killed) { score += 10 * e->size; Enemy* tod = e; *pe = e->next; free(tod); continue; }

            if (circle_collide(player.x,player.y,10, e->x,e->y, e->size*6)) {
                player.lives -= 1;
                Enemy* tod = e; *pe = e->next; free(tod);
                if (player.lives <= 0) running = 0;
                continue;
            }

            pe = &e->next;
        }

        spawn_timer += tick_s;
        if (spawn_timer >= 6.0f) {
            wave += 1; spawn_timer = 0.0f;
            int count = 3 + wave;
            spawn_wave(count);
        }

        cli_clear();
        cli_draw_fillrect((int)player.x-8, (int)player.y-8, 16,16, 180,200,80);

        for (Bullet* b = bullets; b; b = b->next)
            cli_draw_fillrect((int)b->x-2,(int)b->y-2,4,4,255,220,80);

        for (Enemy* e = enemies; e; e = e->next)
            cli_draw_fillrect((int)e->x - e->size*6, (int)e->y - e->size*6, e->size*12, e->size*12, 220,60,60);

        for (int i=0;i<player.lives;i++)
            cli_draw_fillrect(8 + i*20, 8, 16, 16, 200,50,50);

        int fullW = 200;
        float total_run = 120.0f;
        int tbw = (int)((1.0f - fminf(game_time/total_run,1.0f)) * fullW);
        cli_draw_fillrect(8, 32, fullW, 10, 60,60,80);
        cli_draw_fillrect(8, 32, tbw, 10, 80,160,240);

        game_time += tick_s;
        // print game time to console once per second (shows in absence of text rendering)
        int cur_sec = (int)game_time;
        if (cur_sec != last_printed_second) {
            printf("[TIME] %.0f seconds\n", game_time);
            last_printed_second = cur_sec;
        }

        cli_present();

        uint32_t t1 = cli_ticks();
        int took = (int)(t1 - t0);
        if (took < 16) cli_delay(16 - took);
    }

    // end run - save score and print podium
    printf("Game Over. Score=%d time=%.1f lives=%d\n", score, game_time, player.lives);
    save_score(name, score, game_time);
    print_podium();

    free_bullets();
    free_enemies();
    grid_free(&spawn_grid);
    cli_quit();
    return 0;
}