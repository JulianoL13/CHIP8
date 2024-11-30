#ifndef CHIP8_H
#define CHIP8_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef enum
{
    QUIT,
    RUNNING,
    PAUSED,
} emulator_state_t;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

typedef struct
{
    uint32_t window_width;
    uint32_t window_height;
    uint32_t fg_color;
    uint32_t bg_color;
    uint32_t scale_factor;
} config_t;

typedef struct
{
    emulator_state_t state;
    uint8_t ram[4096];
    bool display[64 * 32];
    uint16_t stack[12];
    uint8_t V[16];
    uint8_t I;
    uint16_t PC;
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool keypad[16];
    const char *rom_name;
} chip8_t;

bool init_sdl(sdl_t *sdl, const config_t *config);

bool set_config_from_args(config_t *config, int argc, char **argv);

bool init_chip8(chip8_t *chip8, const char rom_name[]);

void final_cleanup(const sdl_t *sdl);

void clear_screen(const sdl_t *sdl, const config_t *config);
void update_screen(const sdl_t *sdl);

void handle_input(chip8_t *chip8);

#endif
