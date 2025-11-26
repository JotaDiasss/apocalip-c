#include "cli_lib.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

static SDL_Window* g_win = NULL;
static SDL_Renderer* g_ren = NULL;
static TTF_Font* g_font = NULL;
static char g_text[64] = {0};
static int g_text_len = 0;
static int g_text_active = 0;

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
    if (g_font) { TTF_CloseFont(g_font); g_font = NULL; }
    TTF_Quit();
    SDL_Quit();
}

int cli_init_ttf(const char* fontpath, int ptsize) {
    if (TTF_WasInit() == 0) {
        if (TTF_Init() != 0) {
            fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
            return -1;
        }
    }
    if (!fontpath) return -1;
    g_font = TTF_OpenFont(fontpath, ptsize);
    if (!g_font) {
        fprintf(stderr, "TTF_OpenFont('%s'): %s\n", fontpath, TTF_GetError());
        return -1;
    }
    return 0;
}

void cli_shutdown_ttf(void) {
    if (g_font) { TTF_CloseFont(g_font); g_font = NULL; }
    TTF_Quit();
}

void cli_render_text(const char* text, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (!g_font || !g_ren || !text) return;
    SDL_Color col = { r, g, b, 255 };
    SDL_Surface* surf = TTF_RenderUTF8_Blended(g_font, text, col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(g_ren, surf);
    SDL_Rect dst = { x, y, surf->w, surf->h };
    SDL_FreeSurface(surf);
    if (!tex) return;
    SDL_RenderCopy(g_ren, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
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
    in->enter = 0;
    in->text_len = 0;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) in->quit = 1;
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) in->enter = 1;
        if (e.type == SDL_TEXTINPUT) {
            if (g_text_active && g_text_len + (int)strlen(e.text.text) < (int)sizeof(g_text)) {
                strcat(g_text, e.text.text);
                g_text_len = (int)strlen(g_text);
            }
        }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE) {
            if (g_text_active && g_text_len > 0) {
                g_text[--g_text_len] = '\0';
            }
        }
    }
    in->key_state = SDL_GetKeyboardState(NULL);
    in->mouse_buttons = SDL_GetMouseState(&in->mouse_x, &in->mouse_y);
    if (g_text_len > 0) {
        int cpy = g_text_len < (int)sizeof(in->text)-1 ? g_text_len : (int)sizeof(in->text)-1;
        memcpy(in->text, g_text, cpy);
        in->text[cpy] = '\0';
        in->text_len = cpy;
    } else {
        in->text[0] = '\0'; in->text_len = 0;
    }
}

uint32_t cli_ticks(void) {
    return SDL_GetTicks();
}

void cli_start_text_input(void) {
    SDL_StartTextInput();
    g_text_active = 1;
    g_text_len = 0;
    g_text[0] = '\0';
}

void cli_stop_text_input(void) {
    SDL_StopTextInput();
    g_text_active = 0;
}

void cli_clear_text_input(void) {
    g_text_len = 0;
    g_text[0] = '\0';
}
