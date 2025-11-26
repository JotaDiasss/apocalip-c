#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "cli_lib.h"

#define W 1280
#define H 720
#define PLAYER_SIZE 16

static const float player_speed = 150.0f;
static const float ENEMY_SPEED = 160.0f;
static const float DASH_MULT = 3.75f;
static const int DASH_DURATION_MS = 300;
static const int DASH_COOLDOWN_MS = 3000;
static const int INVUL_MS = 1200;

typedef struct Player { float x, y; int lives; } Player;
typedef struct Bullet { float x, y, vx, vy; struct Bullet* next; } Bullet;
typedef struct Enemy { float x, y; int size_px; float speed; int hp; struct Enemy* next; } Enemy;
typedef struct ScoreRec { char name[64]; int s; int w; } ScoreRec;

static Player player;
static Bullet* bullets = NULL;
static Enemy* enemies = NULL;
static int score = 0;
static int wave_num = 0;

static int grid_rows = 10;
static int grid_cols = 16;
static int* spawn_grid = NULL;

static void alloc_grid(void) {
    if (spawn_grid) return;
    spawn_grid = (int*)calloc((size_t)grid_rows * (size_t)grid_cols, sizeof(int));
}

static void free_grid(void) {
    if (spawn_grid) { free(spawn_grid); spawn_grid = NULL; }
}

static void inc_grid_at_xy(float x, float y) {
    if (!spawn_grid) return;
    int cx = (int)(x / ((float)W / (float)grid_cols));
    int cy = (int)(y / ((float)H / (float)grid_rows));
    if (cx < 0) cx = 0;
    if (cx >= grid_cols) cx = grid_cols - 1;
    if (cy < 0) cy = 0;
    if (cy >= grid_rows) cy = grid_rows - 1;
    spawn_grid[cy * grid_cols + cx]++;
}

static int last_mouse_down = 0;
static int last_space_down = 0;
static int dash_active = 0;
static uint32_t dash_start_ms = 0;
static uint32_t last_dash_ms = 0;
static float dash_dir_x = 0.0f, dash_dir_y = -1.0f;
static int invulnerable = 0;
static uint32_t invul_start_ms = 0;

static float clampf(float v, float a, float b) { if (v < a) return a; if (v > b) return b; return v; }
static int circle_collide(float x1,float y1,float r1,float x2,float y2,float r2) { float dx = x1-x2, dy = y1-y2; float d2 = dx*dx + dy*dy; float r = r1 + r2; return d2 <= r*r; }

static void add_bullet(float x, float y, float vx, float vy) { Bullet* b = malloc(sizeof(Bullet)); if (!b) return; b->x=x;b->y=y;b->vx=vx;b->vy=vy;b->next=bullets;bullets=b; }
static void free_bullets(void) { while (bullets) { Bullet* n = bullets->next; free(bullets); bullets = n; } }

static void add_enemy(float x, float y, int size_px, float speed, int hp) { Enemy* e = malloc(sizeof(Enemy)); if (!e) return; e->x=x;e->y=y;e->size_px=size_px;e->speed=speed;e->hp=hp;e->next=enemies;enemies=e; }
static void free_enemies(void) { while (enemies) { Enemy* n = enemies->next; free(enemies); enemies = n; } }

static void spawn_wave(int wave) {
    const int ENEMIES_PER_WAVE = 10;
        for (int i=0;i<ENEMIES_PER_WAVE;i++) {
        int side = rand()%4; float x,y;
        if (side==0) { x=-30; y = rand()%H; }
        else if (side==1) { x=W+30; y = rand()%H; }
        else if (side==2) { x = rand()%W; y = -30; }
        else { x = rand()%W; y = H+30; }
        int size_px = (int)(PLAYER_SIZE * powf(1.2f, (float)(wave - 1)) + 0.5f);
        float speed = ENEMY_SPEED + (float)((rand()%41)-20);
        add_enemy(x,y,size_px,speed,wave);
        inc_grid_at_xy(x < 0 ? 0 : (x>W?W:x), y < 0 ? 0 : (y>H?H:y));
    }
}

static void save_score(const char* name, int s, int w) { FILE* f = fopen("scores.txt","a"); if (!f) return; fprintf(f, "%s %d %d\n", name, s, w); fclose(f); }
static int load_scores(ScoreRec* out, int max) {
    int cnt=0; FILE* f = fopen("scores.txt","r"); if (!f) return 0;
    char line[256]; while (cnt<max && fgets(line,sizeof(line),f)) { char n[64]; int s,w; if (sscanf(line, "%63s %d %d", n, &s, &w)==3) { strncpy(out[cnt].name,n,sizeof(out[cnt].name)); out[cnt].s=s; out[cnt].w=w; cnt++; } }
    fclose(f);
    for (int i=0;i<cnt;i++) for (int j=i+1;j<cnt;j++) if (out[j].s > out[i].s) { ScoreRec t = out[i]; out[i]=out[j]; out[j]=t; }
    return cnt;
}

int main(int argc, char* argv[]) {
    (void)argc;(void)argv; srand((unsigned)time(NULL));
    if (cli_init("apocalip-c", W, H) != 0) return 1;
    int have_ttf = 0; const char* fonts[] = {"./DejaVuSans.ttf", "C:/Windows/Fonts/arial.ttf", NULL};
    for (int i=0; fonts[i]; ++i) if (cli_init_ttf(fonts[i],18)==0) { have_ttf = 1; break; }

    int quit_program = 0;
    while (!quit_program) {
        char name[64] = "Player";
        if (have_ttf) {
            cli_start_text_input(); cli_clear_text_input(); int entered = 0;
            while (!entered && !quit_program) {
                uint32_t t0 = cli_ticks(); CLI_Input input; cli_poll_input(&input);
                if (input.quit) { quit_program = 1; break; }
                cli_clear(); cli_render_text("Digite seu nome:", W/2-120, H/2-48, 230,230,230);
                cli_render_text(input.text_len?input.text:"(digite seu nome)", W/2-120, H/2-16, 200,255,200);
                int bx = W/2-64, by = H/2+24, bw=128, bh=40; cli_draw_fillrect(bx,by,bw,bh,80,200,120); cli_render_text("Jogar", bx+36, by+10, 10,10,10);
                if ((input.mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))!=0) { int mx=input.mouse_x,my=input.mouse_y; if (mx>=bx && mx<=bx+bw && my>=by && my<=by+bh) { if (input.text_len>0) strncpy(name,input.text,sizeof(name)); name[sizeof(name)-1]='\0'; entered = 1; break; } }
                if (input.enter) { if (input.text_len>0) strncpy(name,input.text,sizeof(name)); name[sizeof(name)-1]='\0'; entered = 1; break; }
                cli_present(); uint32_t t1 = cli_ticks(); int took = (int)(t1-t0); if (took<16) cli_delay(16-took);
            }
            cli_stop_text_input(); if (quit_program) break;
        } else {
            printf("Digite seu nome: "); fflush(stdout); if (fgets(name,sizeof(name),stdin)) name[strcspn(name, "\n")]='\0';
        }

        free_bullets(); free_enemies(); free_grid(); alloc_grid(); player.x=W/2.0f; player.y=H/2.0f; player.lives=3; score=0; wave_num=1; spawn_wave(wave_num);
        int running = 1; uint32_t last_enemy_dead_ms = 0; int wave_in_progress = 1;

        while (running) {
            uint32_t t0 = cli_ticks(); CLI_Input input; cli_poll_input(&input); if (input.quit) { running=0; quit_program=1; break; }
            if (dash_active && (t0 - dash_start_ms >= DASH_DURATION_MS)) dash_active = 0;
            float mvx=0,mvy=0; if (input.key_state[SDL_SCANCODE_W]) mvy -= 1; if (input.key_state[SDL_SCANCODE_S]) mvy += 1; if (input.key_state[SDL_SCANCODE_A]) mvx -= 1; if (input.key_state[SDL_SCANCODE_D]) mvx += 1;
            float input_mvx = mvx, input_mvy = mvy; float mvlen = sqrtf(mvx*mvx + mvy*mvy); if (mvlen>0) { mvx/=mvlen; mvy/=mvlen; } else { mvx=0; mvy=0; }
            float move_speed = player_speed; if (dash_active) { move_speed = player_speed * DASH_MULT; mvx = dash_dir_x; mvy = dash_dir_y; }
            player.x += mvx * move_speed * (1.0f/60.0f); player.y += mvy * move_speed * (1.0f/60.0f);
            player.x = clampf(player.x,0,(float)W); player.y = clampf(player.y,0,(float)H);
            int space_down = (input.key_state[SDL_SCANCODE_SPACE] != 0);
            if (space_down && !last_space_down && !dash_active && (t0 - last_dash_ms >= DASH_COOLDOWN_MS)) { dash_active = 1; dash_start_ms = t0; last_dash_ms = t0; if (input_mvx==0 && input_mvy==0) { dash_dir_x=0; dash_dir_y=-1; } else { float il=sqrtf(input_mvx*input_mvx+input_mvy*input_mvy); dash_dir_x=input_mvx/(il>0?il:1); dash_dir_y=input_mvy/(il>0?il:1); } }
            last_space_down = space_down;
            int mouse_down = (input.mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))!=0; if (mouse_down && !last_mouse_down) { float dx = (float)input.mouse_x - player.x; float dy = (float)input.mouse_y - player.y; float L = sqrtf(dx*dx+dy*dy); if (L<1) L=1; add_bullet(player.x,player.y,dx/L*400.0f,dy/L*400.0f); } last_mouse_down = mouse_down;

            for (Bullet** pb=&bullets; *pb;) { Bullet* b = *pb; b->x += b->vx*(1.0f/60.0f); b->y += b->vy*(1.0f/60.0f); if (b->x < -50 || b->x > W+50 || b->y < -50 || b->y > H+50) { *pb = b->next; free(b); } else pb = &b->next; }

            for (Enemy** pe=&enemies; *pe;) { Enemy* e = *pe; float dx = player.x - e->x; float dy = player.y - e->y; float L = sqrtf(dx*dx+dy*dy); if (L<1) L=1; e->x += (dx/L)*e->speed*(1.0f/60.0f); e->y += (dy/L)*e->speed*(1.0f/60.0f); int killed=0; for (Bullet** pb=&bullets; *pb;) { Bullet* b=*pb; if (circle_collide(b->x,b->y,3.0f,e->x,e->y,(float)e->size_px*0.5f)) { e->hp -=1; *pb = b->next; free(b); if (e->hp<=0) { killed=1; break; } } else pb = &b->next; } if (killed) { score += wave_num; Enemy* tod = e; *pe = e->next; free(tod); continue; } if (circle_collide(player.x,player.y,(float)PLAYER_SIZE*0.5f,e->x,e->y,(float)e->size_px*0.5f)) { if (!invulnerable) { player.lives -= 1; invulnerable = 1; invul_start_ms = t0; } Enemy* tod = e; *pe = e->next; free(tod); if (player.lives <= 0) running = 0; continue; } pe = &e->next; }

            if (enemies == NULL) { if (wave_in_progress) { last_enemy_dead_ms = cli_ticks(); wave_in_progress = 0; } else { if (last_enemy_dead_ms != 0 && (cli_ticks() - last_enemy_dead_ms >= 3000)) { wave_num++; spawn_wave(wave_num); wave_in_progress = 1; last_enemy_dead_ms = 0; } } }

            int draw_player = 1; if (invulnerable) { uint32_t inv_elapsed = t0 - invul_start_ms; if (inv_elapsed >= (uint32_t)INVUL_MS) invulnerable = 0; else { uint32_t bi = INVUL_MS/6; if (((inv_elapsed/bi)&1)==1) draw_player = 0; } }
            cli_clear(); if (draw_player) cli_draw_fillrect((int)player.x-PLAYER_SIZE/2,(int)player.y-PLAYER_SIZE/2,PLAYER_SIZE,PLAYER_SIZE,180,200,80);
            for (Bullet* b=bullets;b;b=b->next) cli_draw_fillrect((int)b->x-2,(int)b->y-2,4,4,255,220,80);
            for (Enemy* e=enemies;e;e=e->next) cli_draw_fillrect((int)e->x-e->size_px/2,(int)e->y-e->size_px/2,e->size_px,e->size_px,220,60,60);
            for (int i=0;i<player.lives;i++) cli_draw_fillrect(8+i*20,8,16,16,200,50,50);
            if (have_ttf) { char buf[128]; snprintf(buf,sizeof(buf),"Pontos: %d",score); cli_render_text(buf,W-220,8,230,230,230); char wb[64]; snprintf(wb,sizeof(wb),"Onda: %d",wave_num); cli_render_text(wb,W-220,32,200,200,255); }
            if (spawn_grid) {
                int gx = 8;
                int gy = 40;
                for (int ry=0; ry<grid_rows; ++ry) {
                    for (int cx=0; cx<grid_cols; ++cx) {
                        int v = spawn_grid[ry*grid_cols + cx];
                        if (v <= 0) continue;
                        int intensity = v > 5 ? 255 : 50 + v*40;
                        int sx = gx + cx*6;
                        int sy = gy + ry*6;
                        cli_draw_fillrect(sx, sy, 5, 5, intensity, 40, 40);
                    }
                }
            }
            cli_present(); uint32_t t1 = cli_ticks(); int took = (int)(t1-t0); if (took<16) cli_delay(16-took);
        }

        save_score(name, score, wave_num);
        ScoreRec scores[256]; int scnt = load_scores(scores,256);
        int placement = -1; for (int i=0;i<scnt;i++) if (strcmp(scores[i].name,name)==0 && scores[i].s==score) { placement = i+1; break; }

        int choice_restart = 0, choice_exit = 0;
        while (!choice_restart && !choice_exit) {
            CLI_Input in2; cli_poll_input(&in2); if (in2.quit) { choice_exit = 1; break; }
            cli_clear(); int podium_x = W/2-200, podium_y = H/2-140; cli_draw_fillrect(podium_x-8,podium_y-8,416,220,40,40,50);
            if (have_ttf) {
                cli_render_text("--- PÓDIO ---", podium_x+110, podium_y-4, 255,220,180);
                for (int i=0;i<3;i++) {
                    if (i<scnt) { char buf[128]; snprintf(buf,sizeof(buf),"%d. %.48s  %d pts (onda %d)", i+1, scores[i].name, scores[i].s, scores[i].w); cli_render_text(buf,podium_x+16,podium_y+24+i*36,220,220,220); }
                    else { char buf[128]; snprintf(buf,sizeof(buf),"%d. ---", i+1); cli_render_text(buf,podium_x+16,podium_y+24+i*36,120,120,120); }
                }
                char cur[160]; if (placement>0) snprintf(cur,sizeof(cur),"Você: %.48s  %d pts  posição: %d",name,score,placement); else snprintf(cur,sizeof(cur),"Você: %.48s  %d pts  posição: -",name,score);
                cli_render_text(cur,podium_x+16,podium_y+24+3*36+8,200,255,200);
                int bx = W/2-180, by = podium_y+24+3*36+56, bw = 160, bh = 36; int bx2 = W/2+20, by2 = by, bw2=120, bh2=36;
                cli_draw_fillrect(bx,by,bw,bh,80,200,120); cli_render_text("Jogar Novamente", bx+8, by+8, 10,10,10);
                cli_draw_fillrect(bx2,by2,bw2,bh2,200,80,80); cli_render_text("Sair", bx2+28, by2+8, 10,10,10);
                if ((in2.mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT))!=0) { int mx=in2.mouse_x,my=in2.mouse_y; if (mx>=bx && mx<=bx+bw && my>=by && my<=by+bh) { choice_restart=1; break; } if (mx>=bx2 && mx<=bx2+bw2 && my>=by2 && my<=by2+bh2) { choice_exit=1; break; } }
                if (in2.enter) { choice_restart = 1; break; }
            } else {
                cli_render_text("Fim de Jogo",20,H/2-20,255,200,200);
                printf("Fim de Jogo - %s\n",name); printf("Pontos: %d   Onda: %d\n",score,wave_num);
                printf("Pódio:\n"); for (int i=0;i<3 && i<scnt;i++) printf("%d. %s %d\n", i+1, scores[i].name, scores[i].s);
                printf("Sua colocação: %d\n", placement); printf("Pressione Enter para reiniciar ou feche para sair\n"); if (in2.enter) { choice_restart = 1; break; }
            }
            cli_present(); cli_delay(100);
        }
        free_bullets(); free_enemies(); free_grid(); if (choice_exit) { quit_program = 1; break; }
    }
    cli_quit(); return 0;
}
