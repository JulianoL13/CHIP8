#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

bool init_sdl(sdl_t *sdl, const config_t *config)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }

    sdl->window = SDL_CreateWindow("CHIP-8 EMULATOR", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config->window_width * config->scale_factor,
                                   config->window_height * config->scale_factor,
                                   0);
    if (!sdl->window)
    {
        SDL_Log("Could not create SDL window %s\n", SDL_GetError());
        return false;
    }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl->renderer)
    {
        SDL_Log("Could not create SDL renderer %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool set_config_from_args(config_t *config, int argc, char **argv)
{
    *config = (config_t){
        .window_width = 64,
        .window_height = 32,
        .fg_color = 0xFFFFFFFF,
        .bg_color = 0x00000000,
        .scale_factor = 20};

    for (int i = 1; i < argc; ++i)
    {
        (void)argv[i];
    }

    return true;
}

bool init_chip8(chip8_t *chip8, const char rom_name[])
{
    const uint32_t entry_point = 0x200;
    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,
        0x20, 0x60, 0x20, 0x20, 0x70,
        0xF0, 0x10, 0xF0, 0x80, 0xF0,
        0xF0, 0x10, 0xF0, 0x10, 0xF0,
        0x90, 0x90, 0xF0, 0x10, 0x10,
        0xF0, 0x80, 0xF0, 0x10, 0xF0,
        0xF0, 0x80, 0xF0, 0x90, 0xF0,
        0xF0, 0x10, 0x20, 0x40, 0x40,
        0xF0, 0x90, 0xF0, 0x90, 0xF0,
        0xF0, 0x90, 0xF0, 0x10, 0xF0,
        0xF0, 0x90, 0xF0, 0x90, 0x90,
        0xE0, 0x90, 0xE0, 0x90, 0xE0,
        0xF0, 0x80, 0x80, 0x80, 0xF0,
        0xE0, 0x90, 0x90, 0x90, 0xE0,
        0xF0, 0x80, 0xF0, 0x80, 0xF0,
        0xF0, 0x80, 0xF0, 0x80, 0x80};

    memcpy(&chip8->ram[0], font, sizeof(font));

    FILE *rom = fopen(rom_name, "rb");
    if (!rom)
    {
        SDL_Log("Rom file %s is invalid or does not exist\n", rom_name);
        return false;
    };
    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom);
    const size_t max_size = sizeof chip8->ram - entry_point;
    rewind(rom);
    if (rom_size > max_size)
    {
        SDL_Log("Rom file %s is too big! ROm size %zu, Max size allowd: %zu \n", rom_name, rom_size, max_size);
        return false;
    };
    if (fread(&chip8->ram[entry_point], rom_size, 1, rom) != 1)
    {
        SDL_Log("Could not readm Rom file %s into CHIP8 memory\n", rom_name);
        return false;
    };
    fclose(rom);

    chip8->state = RUNNING;
    chip8->PC = entry_point;
    chip8->rom_name = rom_name;
    return true;
}

void final_cleanup(const sdl_t *sdl)
{
    SDL_DestroyRenderer(sdl->renderer);
    SDL_DestroyWindow(sdl->window);
    SDL_Quit();
}

void clear_screen(const sdl_t *sdl, const config_t *config)
{
    uint8_t r = (config->bg_color >> 24) & 0xFF;
    uint8_t g = (config->bg_color >> 16) & 0xFF;
    uint8_t b = (config->bg_color >> 8) & 0xFF;
    uint8_t a = (config->bg_color >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl->renderer, r, g, b, a);
    SDL_RenderClear(sdl->renderer);
}

void update_screen(const sdl_t *sdl)
{
    SDL_RenderPresent(sdl->renderer);
}

void handle_input(chip8_t *chip8)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            chip8->state = QUIT;
            return;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                chip8->state = QUIT;
                return;
            case SDLK_SPACE:
                if (chip8->state == RUNNING)
                    chip8->state = PAUSED;
                else
                {
                    chip8->state = RUNNING;
                    puts("===== RESUMED =====");
                }
                return;
            default:
                break;
            }
            break;
        case SDL_KEYUP:
            break;
        default:
            break;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <rom_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    config_t config = {0};
    if (!set_config_from_args(&config, argc, argv))
    {
        exit(EXIT_FAILURE);
    }

    sdl_t sdl = {0};
    if (!init_sdl(&sdl, &config))
    {
        exit(EXIT_FAILURE);
    }

    char *rom_name = argv[1];
    chip8_t chip8 = {0};
    if (!init_chip8(&chip8, rom_name))
    {
        final_cleanup(&sdl);
        exit(EXIT_FAILURE);
    }

    clear_screen(&sdl, &config);

    while (chip8.state != QUIT)
    {
        handle_input(&chip8);
        if (chip8.state != PAUSED)
            continue;
        SDL_Delay(16);
        update_screen(&sdl);
    }

    final_cleanup(&sdl);
    exit(EXIT_SUCCESS);
}
