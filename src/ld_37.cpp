
/*
    Theme 1 room:
    =============

    a 1 room dungeon that changes shape when you figure out the puzzle
*/


#include "ld_37_lib.h"

#define TARGET_MS_PER_FRAME 16
#define CAP_FRAMERATE       1

typedef struct {
    i32 x;
    i32 y;
    i32 width;
    i32 height;
} Sub_Texture_Data;

typedef struct {
    Vec2 pos;
    Vec2 size;
    Sub_Texture_Data sub_texture_data;
    i32 direction;
    i32 animation_frame;
    f32 speed;
    bool moving;
} Player;

typedef struct {
    Sub_Texture_Data sub_texture_data;
    Vec2 size;
    bool solid;
} Tile;

typedef struct {
    i32 width;
    i32 height;
    u8* tile_ids;
} Map;

#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3

#define TILE_SIZE   16
#define TILE_BRICK  0
#define TILE_DIRT   1

#define PLAYER_ANIMATION_TIME 200

i32 main(i32 argc, char** argv) {
    srand(time(0));
    if(InitSDL() < 0) {
        return -1;
    }

    Display display = {0};
    display.width = 1024;
    display.height = 576;
    display.title = "LD 37";
    display.pixels_per_meter = 16;

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

    display.pixel_buffer.width = display.width / 2;
    display.pixel_buffer.height = display.height / 2;
    display.pixel_buffer.pitch = display.pixel_buffer.width * display.pixel_format->BytesPerPixel;
    display.pixel_buffer.pixels_size = display.pixel_buffer.pitch * display.pixel_buffer.height;
    display.pixel_buffer.pixels = malloc(display.pixel_buffer.pixels_size);
    if(!display.pixel_buffer.pixels) {
        printf("Error - Could not create pixel buffer. %s.\n", SDL_GetError());
        return -1;
    }
    
    display.texture = SDL_CreateTexture(display.renderer, display.pixel_format->format, SDL_TEXTUREACCESS_STREAMING,
                                        display.pixel_buffer.width, display.pixel_buffer.height);
    if(!display.texture) {
        printf("Error - Could not create display texture. %s.\n", SDL_GetError());
        return -1;
    }
    
    Texture texture_sheet;
    LoadTexture("../res/textures/texture_sheet.png", &texture_sheet, display.pixel_format);

    Tile tiles[2] = {
        {{0, 0, TILE_SIZE, TILE_SIZE}, {(f32)TILE_SIZE / (f32)display.pixels_per_meter, (f32)TILE_SIZE / (f32)display.pixels_per_meter}, true}, // brick
        {{16, 0, TILE_SIZE, TILE_SIZE}, {(f32)TILE_SIZE / (f32)display.pixels_per_meter, (f32)TILE_SIZE / (f32)display.pixels_per_meter}, false} // dirt
    };

    Map map = {20, 16, 0};
    Vec2 map_pos = {((display.pixel_buffer.width / 2) - (map.width * TILE_SIZE / 2)) / display.pixels_per_meter,
                    ((display.pixel_buffer.height / 2) - (map.height * TILE_SIZE / 2)) / display.pixels_per_meter};
    map.tile_ids = (u8*)malloc(map.width * map.height);
    for(i32 y = 0; y < map.height; y++) {
        for(i32 x = 0; x < map.width; x++) {
            if(y == 0 || y == map.height - 1 || x == 0 || x == map.width - 1) {
                map.tile_ids[y * map.width + x] = TILE_BRICK;
            } 
            else {
                if(rand() % 40 < 5) {
                    map.tile_ids[y * map.width + x] = TILE_BRICK;
                }
                else {
                    map.tile_ids[y * map.width + x] = TILE_DIRT;
                }
            }
        }
    }

    Player player = {0};
    player.pos = {map_pos.x + 2, map_pos.y + 2};
    player.size = {15.0 / (f32)display.pixels_per_meter, 15.0 / (f32)display.pixels_per_meter};
    player.sub_texture_data = {0, 32, TILE_SIZE, TILE_SIZE};
    player.direction = 1;
    player.speed = 2.5;
    player.moving = false;

    bool running = true;
    SDL_Event event = {0};
    f32 last_delta = 0.0f;
    i32 ms_per_animation = PLAYER_ANIMATION_TIME;
    while(running) {
        i32 ms_before = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT: {
                    running = false;
                } break;
            }
        }

        u8* keys_state = (u8*)SDL_GetKeyboardState(0);

        bool moved = false;
        f32 speed = player.speed;
        if(keys_state[SDL_SCANCODE_LSHIFT]) {
            speed *= 2.5;
        }
        if(keys_state[SDL_SCANCODE_UP] && !keys_state[SDL_SCANCODE_DOWN]) {
            player.direction = UP;
            moved = true;

            i32 tile_x = roundf((((player.pos.x - (player.size.w / 2) - map_pos.x) * display.pixels_per_meter)) / TILE_SIZE);
            i32 tile_y = roundf((((player.pos.y - (player.size.h / 2) - map_pos.y) * display.pixels_per_meter) - 1) / TILE_SIZE);
            Tile* tile_left_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            tile_x = roundf((((player.pos.x + (player.size.w / 2) - map_pos.x) * display.pixels_per_meter)) / TILE_SIZE);
            tile_y = roundf((((player.pos.y - (player.size.h / 2) - map_pos.y) * display.pixels_per_meter) - 1) / TILE_SIZE);
            Tile* tile_right_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            if(!tile_left_corner->solid && !tile_right_corner->solid) {
                player.pos.y -= speed * last_delta;
            }
        }
        else if(keys_state[SDL_SCANCODE_DOWN] && !keys_state[SDL_SCANCODE_UP]) {
            player.direction = DOWN;
            moved = true;

            i32 tile_x = roundf((((player.pos.x - (player.size.w / 2) - map_pos.x) * display.pixels_per_meter)) / TILE_SIZE);
            i32 tile_y = roundf((((player.pos.y + (player.size.h / 2) - map_pos.y) * display.pixels_per_meter) + 1) / TILE_SIZE);
            Tile* tile_left_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            tile_x = roundf((((player.pos.x + (player.size.w / 2) - map_pos.x) * display.pixels_per_meter)) / TILE_SIZE);
            tile_y = roundf((((player.pos.y + (player.size.h / 2) - map_pos.y) * display.pixels_per_meter) + 1) / TILE_SIZE);
            Tile* tile_right_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            if(!tile_left_corner->solid && !tile_right_corner->solid) {
                player.pos.y += speed * last_delta;
            }
        }

        if(keys_state[SDL_SCANCODE_LEFT] && !keys_state[SDL_SCANCODE_RIGHT]) {
            player.direction = LEFT;
            moved = true;

            i32 tile_x = roundf((((player.pos.x - (player.size.w / 2) - map_pos.x) * display.pixels_per_meter) - 1) / TILE_SIZE);
            i32 tile_y = roundf((((player.pos.y + (player.size.h / 2) - map_pos.y) * display.pixels_per_meter)) / TILE_SIZE);
            Tile* tile_bottom_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            tile_x = roundf((((player.pos.x - (player.size.w / 2) - map_pos.x) * display.pixels_per_meter) - 1) / TILE_SIZE);
            tile_y = roundf((((player.pos.y - (player.size.h / 2) - map_pos.y) * display.pixels_per_meter)) / TILE_SIZE);
            Tile* tile_top_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            if(!tile_bottom_corner->solid && !tile_top_corner->solid) {
                player.pos.x -= speed * last_delta;
            }
        }
        else if(keys_state[SDL_SCANCODE_RIGHT] && !keys_state[SDL_SCANCODE_LEFT]) {
            player.direction = RIGHT;
            moved = true;

            i32 tile_x = roundf((((player.pos.x + (player.size.w / 2) - map_pos.x) * display.pixels_per_meter) + 1) / TILE_SIZE);
            i32 tile_y = roundf((((player.pos.y + (player.size.h / 2) - map_pos.y) * display.pixels_per_meter)) / TILE_SIZE);
            Tile* tile_bottom_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            tile_x = roundf((((player.pos.x + (player.size.w / 2) - map_pos.x) * display.pixels_per_meter) + 1) / TILE_SIZE);
            tile_y = roundf((((player.pos.y - (player.size.h / 2) - map_pos.y) * display.pixels_per_meter)) / TILE_SIZE);
            Tile* tile_top_corner = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

            if(!tile_bottom_corner->solid && !tile_top_corner->solid) {
                player.pos.x += speed * last_delta;
            }
        }

        if(!moved && player.moving) {
            player.moving = false;
        }
        else if(moved && !player.moving) {
            // Start at 0 so the we advance the frames as soon as we start moving.
            player.moving = true;
            ms_per_animation = 0; 
        }

        // i32 tile_x = (player.pos.x - map_pos.x) * display.pixels_per_meter / TILE_SIZE;
        // i32 tile_y = (player.pos.y - (player.size.h / 2) - map_pos.y) * display.pixels_per_meter / TILE_SIZE;
        // Tile* tile_above1 = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

        // tile_x = (player.pos.x - map_pos.x) * display.pixels_per_meter / TILE_SIZE;
        // tile_y = (player.pos.y - map_pos.y) * display.pixels_per_meter / TILE_SIZE;
        // Tile* tile_below = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

        // tile_x = (player.pos.x - map_pos.x) * display.pixels_per_meter / TILE_SIZE;
        // tile_y = (player.pos.y - map_pos.y) * display.pixels_per_meter / TILE_SIZE;
        // Tile* tile_left = &tiles[map.tile_ids[tile_y * map.width + tile_x]];
        
        // tile_x = (player.pos.x + player.size.w - map_pos.x) * display.pixels_per_meter / TILE_SIZE;
        // tile_y = (player.pos.y - player.size.h - map_pos.y) * display.pixels_per_meter / TILE_SIZE;
        // Tile* tile_right = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

        // if(!tile_above->solid || !tile_below->solid) {
        //     player.pos.y = player.pos.y;
        // }
        // if(!tile_left->solid || !tile_right->solid) {
        //     player.pos.x = player.pos.x;
        // }

        // i32 tile_x = (player.pos.x - map_pos.x) * display.pixels_per_meter / TILE_SIZE;
        // i32 tile_y = (player.pos.y - map_pos.y) * display.pixels_per_meter / TILE_SIZE;
        // Tile* tile = &tiles[map.tile_ids[tile_y * map.width + tile_x]];

        // if(!tile->solid) {
        //     player.pos = player.pos;
        // }


        SDL_RenderClear(display.renderer);
        for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
            *((u32*)display.pixel_buffer.pixels + i) = 0xFF00FFFF;
        }

        for(i32 y = 0; y < map.height; y++) {
            for(i32 x = 0; x < map.width; x++) {
                Vec2 render_pos = {map_pos.x + (x * TILE_SIZE / display.pixels_per_meter), map_pos.y + (y * TILE_SIZE / display.pixels_per_meter)};
                Tile* tile = &tiles[map.tile_ids[y * map.width + x]];
                
                RenderSubTexture(&display, &render_pos, &texture_sheet, tile->sub_texture_data.x, tile->sub_texture_data.y,
                                 tile->sub_texture_data.width, tile->sub_texture_data.height);
            }
        }

        if(ms_per_animation <= 0) {
            player.animation_frame = (player.animation_frame == 1) ? 0 : 1;
            ms_per_animation = PLAYER_ANIMATION_TIME;
        }

        i32 pstx = player.sub_texture_data.x + (player.direction * player.sub_texture_data.width);
        i32 psty = player.sub_texture_data.y + (player.animation_frame * player.sub_texture_data.height);
        RenderSubTexture(&display, &player.pos, &texture_sheet, pstx, psty, player.sub_texture_data.width, player.sub_texture_data.height);

        SDL_UpdateTexture(display.texture, 0, display.pixel_buffer.pixels, display.pixel_buffer.pitch);
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
        if(player.moving) {
            ms_per_animation -= ms_this_frame;
        }
        //printf("%d ms this frame\n", ms_this_frame);
        last_delta = ms_this_frame / 1000.0f;
    }

    // Freeing stuff is overrated

    return 0;
}