#ifndef CLI_LIB_H
#define CLI_LIB_H

#include <stdint.h>

int cli_init(const char* title, int w, int h);
void cli_quit(void);
void cli_clear(void);
void cli_present(void);
void cli_draw_rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void cli_draw_fillrect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void cli_delay(uint32_t ms);

int cli_init_ttf(const char* fontpath, int ptsize);
void cli_shutdown_ttf(void);
void cli_render_text(const char* text, int x, int y, uint8_t r, uint8_t g, uint8_t b);

typedef struct {
    int quit;
    const uint8_t* key_state;
    int mouse_x, mouse_y;
    uint32_t mouse_buttons;
    int enter;
    
    
    char text[64];
    int text_len;
} CLI_Input;

void cli_poll_input(CLI_Input* in);

void cli_start_text_input(void);
void cli_stop_text_input(void);
void cli_clear_text_input(void);

uint32_t cli_ticks(void);

#endif
