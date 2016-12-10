
#include "ld_37_lib.h"

#define TARGET_MS_PER_FRAME 16
#define CAP_FRAMERATE       1

i32 main(i32 argc, char** argv) {
    if(InitSDL() < 0) {
        return -1;
    }

    Display display = {0};
    display.width = 1024;
    display.height = 576;
    display.title = "LD 37";
    display.pixels_per_meter = 64;

    display.window = SDL_CreateWindow(display.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, display.width, 
                                      display.height, SDL_WINDOW_SHOWN);
    if(!display.window) {
        printf("Error - Could not create window. SDL_Error: %s.\n", SDL_GetError());
        return -1;
    }

    display.renderer = SDL_CreateRenderer(display.window, -1, SDL_RENDERER_ACCELERATED);
    if(!display.renderer) {
        printf("Error - Could not create renderer. SDL_Error: %s.\n", SDL_GetError());
        return -1;
    }

    display.pixel_format = SDL_GetWindowSurface(display.window)->format;
    display.texture = SDL_CreateTexture(display.renderer, display.pixel_format->format, SDL_TEXTUREACCESS_STREAMING, display.width, display.height);
    if(!display.texture) {
        printf("Error - Could not create display texture. %s.\n", SDL_GetError());
        return -1;
    }

    display.pixel_buffer_pitch = display.width * display.pixel_format->BytesPerPixel;
    display.pixel_buffer_size = display.pixel_buffer_pitch * display.height;
    display.pixel_buffer = malloc(display.pixel_buffer_size);
    if(!display.pixel_buffer) {
        printf("Error - Could not create pixel buffer. %s.\n", SDL_GetError());
        return -1;
    }

    bool running = true;
    SDL_Event event = {0};
    f32 last_delta = 0.0f;
    while(running) {
        i32 ms_before = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: {
                    running = false;
                } break;
            }
        }

        SDL_RenderClear(display.renderer);
        for(i32 i = 0; i < display.height * display.width; i++) {
            *((u32*)display.pixel_buffer + i) = 0xFF00FFFF;
        }

        SDL_UpdateTexture(display.texture, 0, display.pixel_buffer, display.pixel_buffer_pitch);
        SDL_RenderCopy(display.renderer, display.texture, NULL, NULL);
        SDL_RenderPresent(display.renderer);

        i32 ms_after = SDL_GetTicks();
        i32 ms_this_frame = ms_after - ms_before;
#if CAP_FRAMERATE
        if(ms_this_frame < TARGET_MS_PER_FRAME) {
            i32 ms_to_wait = TARGET_MS_PER_FRAME - ms_this_frame;
            SDL_Delay(ms_to_wait);
            ms_this_frame += ms_to_wait;
        }
#endif 
        //printf("%d ms this frame\n", ms_this_frame);
        last_delta = ms_this_frame / 1000.0f;
    }

    // Freeing stuff is overrated
    //IMG_Quit();
    //Mix_CloseAudio();
    //SDL_Quit();

    return 0;
}