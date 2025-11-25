// Minimal implementation of cli_lib using SDL2
#include "cli_lib.h"
#include <SDL2/SDL.h>
#include <stdio.h>

static SDL_Window* g_win = NULL;
static SDL_Renderer* g_ren = NULL;

int cli_init(const char* title, int w, int h) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return -1;
    }
    g_win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    if (!g_win) {
        fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    g_ren = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED);
    if (!g_ren) {
        fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_win);
        SDL_Quit();
        return -1;
    }
    return 0;
}

void cli_quit(void) {
    if (g_ren) SDL_DestroyRenderer(g_ren);
    if (g_win) SDL_DestroyWindow(g_win);
    SDL_Quit();
}

void cli_clear(void) {
    SDL_SetRenderDrawColor(g_ren, 20, 20, 30, 255);
    SDL_RenderClear(g_ren);
}

void cli_present(void) {
    SDL_RenderPresent(g_ren);
}

void cli_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(g_ren, r, g, b, 255);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderDrawRect(g_ren, &rect);
}

void cli_draw_fillrect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
    SDL_SetRenderDrawColor(g_ren, r, g, b, 255);
    SDL_Rect rect = { x, y, w, h };
    SDL_RenderFillRect(g_ren, &rect);
}

void cli_delay(uint32_t ms) {
    SDL_Delay(ms);
}

void cli_poll_input(CLI_Input* in) {
    SDL_Event e;
    in->quit = 0;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) in->quit = 1;
    }
    in->key_state = SDL_GetKeyboardState(NULL);
    in->mouse_buttons = SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
}

uint32_t cli_ticks(void) {
    return SDL_GetTicks();
}
