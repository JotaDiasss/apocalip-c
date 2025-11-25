// Simple CLI-Lib wrapper (uses SDL2 underneath)
// Provides minimal functions for drawing, input and timing.

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

typedef struct {
    int quit;
    const uint8_t* key_state; // points to SDL internal array
    int mouse_x, mouse_y;
    uint32_t mouse_buttons;
} CLI_Input;

// Poll events and fill input struct
void cli_poll_input(CLI_Input* in);

// Return current ticks in ms
uint32_t cli_ticks(void);

#endif // CLI_LIB_H
