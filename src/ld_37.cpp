
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

class Entity {
public:
    Vec2 pos;
    Vec2 size;
};

class Mob : public Entity {
public: 
    Sub_Texture_Data sub_texture_data;
    i32 direction;
    i32 animation_frame;
    i32 animation_timer;
    f32 speed;
    bool moving;
    i32 health;
    i32 damage;
};

class Player : public Mob {

};


class Enemy : public Mob {
public:
    i32 ai_state;
    i32 chase_speed;
    Vec2 wander_destination;
};

typedef struct {
    Sub_Texture_Data sub_texture_data;
    Vec2 size;
    bool solid;
    bool activatable;
} Tile;

typedef struct {
    i32 width;
    i32 height;
    u8* tile_ids;
    Tile* tiles;
} Map;

#define UP                              0
#define DOWN                            1
#define LEFT                            2
#define RIGHT                           3

const f32 PIXELS_PER_METER              = 16.0;
const i32 TILE_SIZE                     = 16;
const Vec2 TILE_SIZE_IN_METERS          = {(i32)TILE_SIZE / PIXELS_PER_METER, (i32)TILE_SIZE / PIXELS_PER_METER};
#define TILE_BLANK                      0
#define TILE_BRICK                      1
#define TILE_DIRT                       2
#define TILE_BUTTON_UNACTIVATED         3
#define TILE_BUTTON_ACTIVATED           4
#define TILE_BARRICATE_VIRTICAL         5
#define TILE_BARRICATE_HORIZONTAL       6

#define PLAYER_ANIMATION_TIME           200
#define PLAYER_SPEED                    4.2
#define PLAYER_HEALTH                   20
#define PLAYER_DAMAGE                   1

#define AI_STATE_WANDER                 0
#define AI_STATE_CHASE                  1
#define ENEMY_ANIMATION_TIME            200
#define ENEMY_CHASE_SPEED               4.3
#define ENEMY_WANDER_SPEED              3.4
#define ENEMY_HEALTH                    10
#define ENEMY_DAMAGE                    1
#define NUM_ENEMIES_MIN                 8
#define NUM_ENEMIES_MAX                 15

#define MAP_FILE_EMPTY_COLOUR           0x00FF00FF
#define MAP_FILE_BRICK_COLOUR           0x00666666
#define MAP_FILE_DIRT_COLOUR            0x006A5A44
#define MAP_FILE_BARRICATE_COLOUR       0x003D3D3D
#define MAP_FILE_FLOOR_BUTTON_COLOUR    0x00C3C3C3
#define MAP_FILE_START_POS_COLOUR       0x000000FF

void CreatePlayer(Player* player, Vec2* start_pos, f32 pixels_per_meter) {
    player->pos = *start_pos;
    // NOTE (Mathew): Making player a bit smaller than the texture is so that 
    //                you don't collide when there are 2 tiles on either side of you.
    player->size = {15.0 / pixels_per_meter, 15.0 / pixels_per_meter};
    player->sub_texture_data = {0, 32, TILE_SIZE, TILE_SIZE};
    player->direction = 1;
    player->speed = 4.2;
    player->moving = false;
    player->animation_timer = 0;
    player->health = PLAYER_HEALTH;
    player->damage = PLAYER_DAMAGE;
}

void RenderPlayer(Display* display, Player* player, Texture* texture_sheet) {
    i32 pstx = player->sub_texture_data.x + (player->direction * player->sub_texture_data.width);
    i32 psty = player->sub_texture_data.y + (player->animation_frame * player->sub_texture_data.height);
    Vec2 player_pos_centered = {player->pos.x - display->camera_pos.x, 
                                player->pos.y - display->camera_pos.y};
    RenderSubTexture(display, &player_pos_centered, texture_sheet, pstx, psty, player->sub_texture_data.width, player->sub_texture_data.height);
}

bool EntityCollidingWithTileY(Entity* entity, Map* map, Display* display) {
    bool colliding = false;

    i32 tile_x = roundf((((entity->pos.x - (entity->size.w / 2)) * display->pixels_per_meter)) / TILE_SIZE);
    i32 tile_y = roundf((((entity->pos.y - (entity->size.h / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    Tile* tile_top_left_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    tile_x = roundf(((entity->pos.x + (entity->size.w / 2)) * display->pixels_per_meter) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y - (entity->size.h / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    Tile* tile_top_right_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    tile_x = roundf(((entity->pos.x - (entity->size.w / 2)) * display->pixels_per_meter) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    Tile* tile_bottom_left_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    tile_x = roundf(((entity->pos.x + (entity->size.w / 2)) * display->pixels_per_meter) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    Tile* tile_bottom_right_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    if(tile_bottom_right_corner->solid || tile_bottom_left_corner->solid ||
       tile_top_left_corner->solid || tile_top_right_corner->solid) {
        colliding = true;
    }

    return colliding;
}

bool EntityCollidingWithTileX(Entity* entity, Map* map, Display* display) {
    bool colliding = false;

    i32 tile_x = roundf((((entity->pos.x - (entity->size.w / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    i32 tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter)) / TILE_SIZE);
    Tile* tile_left_top_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    tile_x = roundf((((entity->pos.x + (entity->size.w / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y - (entity->size.h / 2)) * display->pixels_per_meter)) / TILE_SIZE);
    Tile* tile_right_top_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    tile_x = roundf((((entity->pos.x - (entity->size.w / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter)) / TILE_SIZE);
    Tile* tile_left_bottom_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    tile_x = roundf((((entity->pos.x + (entity->size.w / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter)) / TILE_SIZE);
    Tile* tile_right_bottom_corner = &map->tiles[map->tile_ids[tile_y * map->width + tile_x]];

    if(tile_right_bottom_corner->solid || tile_left_bottom_corner->solid ||
       tile_left_top_corner->solid || tile_right_top_corner->solid) {
        colliding = true;
    }

    return colliding;
}

bool EntityCollidingWithMob(Entity* entity1, Entity* entity2, Map* map, Display* display) {
    bool colliding = false;

    return colliding;
}

typedef struct {
    Enemy* enemies;
    i32 num_enemies;
} Enemies;

i32 main(i32 argc, char** argv) {
    srand(time(0));
    if(InitSDL() < 0) {
        return -1;
    }

    Display display = {0};
    display.width = 1024;
    display.height = 576;
    display.title = "Sneaky Humanses";
    display.pixels_per_meter = PIXELS_PER_METER;

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

    display.pixel_buffer.width = display.width / 4;
    display.pixel_buffer.height = display.height / 4;
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

    Texture map_file_texture;
    Vec2 player_start_pos = {0, 0};
    LoadTexture("../res/textures/map1.png", &map_file_texture, display.pixel_format);
    Map map = {map_file_texture.width, map_file_texture.height, 0};
    map.tile_ids = (u8*)malloc(map.width * map.height);
    for(i32 y = 0; y < map.height; y++) {
        for(i32 x = 0; x < map.width; x++) {
            u32 pixel = *((u32*)map_file_texture.pixels + (y * map_file_texture.width) + x);

            switch(pixel) {
                case MAP_FILE_BRICK_COLOUR: {
                    map.tile_ids[(y * map.width) + x] = TILE_BRICK;
                } break;

                case MAP_FILE_DIRT_COLOUR: {
                    map.tile_ids[(y * map.width) + x] = TILE_DIRT;
                } break;

                case MAP_FILE_START_POS_COLOUR: {
                    player_start_pos = {(f32)x * (f32)TILE_SIZE / (f32)display.pixels_per_meter, (f32)y * (f32)TILE_SIZE / (f32)display.pixels_per_meter};
                    map.tile_ids[(y * map.width) + x] = TILE_DIRT;
                } break;

                case MAP_FILE_BARRICATE_COLOUR: {
                    // We'll change the barricades to their proper orientation after
                    map.tile_ids[(y * map.width) + x] = TILE_BARRICATE_VIRTICAL;
                } break;

                case MAP_FILE_FLOOR_BUTTON_COLOUR: {
                    map.tile_ids[(y * map.width) + x] = TILE_BUTTON_UNACTIVATED;
                } break;

                case MAP_FILE_EMPTY_COLOUR:
                default: {
                    map.tile_ids[(y * map.width) + x] = TILE_BLANK;
                } break;
            }
        }
    }
    FreeTexture(&map_file_texture);

    Tile tiles[7] = {
        {{0, 0,  TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false, false}, // unknown
        {{16, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true, false}, // brick
        {{32, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false, false}, // dirt
        {{48, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false, true}, // floor button unactivated
        {{64, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false, true}, // floor button activated
        {{80, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true, false}, // barricade virtical
        {{96, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true, false}, // barricade horizontal
    };
    map.tiles = tiles;
    
    Player player;
    CreatePlayer(&player, &player_start_pos, display.pixels_per_meter);

    Enemies enemies = {0};
    enemies.num_enemies = rand() % (NUM_ENEMIES_MAX - NUM_ENEMIES_MIN) + NUM_ENEMIES_MIN;
    printf("%d enemies created\n", enemies.num_enemies);
    enemies.enemies = (Enemy*)malloc(sizeof(Enemy) * enemies.num_enemies);

    for(i32 i = 0; i < enemies.num_enemies; i++) {
        Enemy* enemy = &enemies.enemies[i];
        // Only spawn on dirt tiles
        while(true) {
            i32 tile_x = rand() % (i32)(map.width * TILE_SIZE_IN_METERS.w);
            i32 tile_y = rand() % (i32)(map.height * TILE_SIZE_IN_METERS.w);
            i32 tile_id = map.tile_ids[tile_y * map.width + tile_x];

            if(tile_id == TILE_DIRT) {
                enemy->pos.x = tile_x;
                enemy->pos.y = tile_y;
                break;
            }
        }

        enemy->size = {15.0 / PIXELS_PER_METER, 15.0 / PIXELS_PER_METER};
        enemy->sub_texture_data = {0, 80, 16, 16};
        enemy->direction = rand() % 4;
        enemy->speed = ENEMY_WANDER_SPEED;
        enemy->chase_speed = ENEMY_CHASE_SPEED;
        enemy->ai_state = AI_STATE_WANDER;
        enemy->moving = false;
        enemy->animation_frame = 0;
        enemy->animation_timer = 0;
        enemy->health = ENEMY_HEALTH;
        enemy->damage = ENEMY_DAMAGE;
    }

    display.camera_pos = {player.pos.x - ((f32)display.pixel_buffer.width / 2.0 / (f32)display.pixels_per_meter), 
                          player.pos.y - ((f32)display.pixel_buffer.height / 2.0 / (f32)display.pixels_per_meter)};
    display.camera_size = {(f32)display.pixel_buffer.width / (f32)display.pixels_per_meter, 
                           (f32)display.pixel_buffer.height / (f32)display.pixels_per_meter};

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

        for(i32 i = 0; i < enemies.num_enemies; i++) {
            Enemy* enemy = &enemies.enemies[i];
            bool moved = false;

            switch(enemy->ai_state) {
                case AI_STATE_WANDER: {
                    if(enemy->moving) {
                        bool reached_destination = false;
                        Entity temp_enemy = {enemy->pos, enemy->size};
                        switch(enemy->direction) {
                            case UP: {
                                temp_enemy.pos.y -= enemy->speed * last_delta;
                                if(EntityCollidingWithTileY(&temp_enemy, &map, &display)) {
                                    reached_destination = true;
                                    break;
                                }

                                enemy->pos.y = temp_enemy.pos.y;
                                if(enemy->pos.y <= enemy->wander_destination.y){
                                    reached_destination = true;
                                }
                            } break;

                            case DOWN: {
                                temp_enemy.pos.y += enemy->speed * last_delta;
                                if(EntityCollidingWithTileY(&temp_enemy, &map, &display)) {
                                    reached_destination = true;
                                    break;
                                }

                                enemy->pos.y = temp_enemy.pos.y;
                                if(enemy->pos.y >= enemy->wander_destination.y){
                                    reached_destination = true;
                                }
                            } break;

                            case LEFT: {
                                temp_enemy.pos.x -= enemy->speed * last_delta;
                                if(EntityCollidingWithTileX(&temp_enemy, &map, &display)) {
                                    reached_destination = true;
                                    break;
                                }

                                enemy->pos.x = temp_enemy.pos.x;
                                if(enemy->pos.x <= enemy->wander_destination.x){
                                    reached_destination = true;
                                }
                            } break;

                            case RIGHT: {
                                temp_enemy.pos.x += enemy->speed * last_delta;
                                if(EntityCollidingWithTileX(&temp_enemy, &map, &display)) {
                                    reached_destination = true;
                                    break;
                                }

                                enemy->pos.x = temp_enemy.pos.x;
                                if(enemy->pos.x >= enemy->wander_destination.x){
                                    reached_destination = true;
                                }
                            } break;
                        }

                        if(reached_destination) {
                            enemy->moving = false;
                            enemy->animation_timer = 0;
                        }
                    }
                    else {
                        if(rand() % 333 < 10) {
                            enemy->wander_destination = enemy->pos;
                            enemy->direction = rand() % 4;
                            enemy->moving = true;

                            switch(enemy->direction) {
                                case UP: {
                                    enemy->wander_destination.y -= rand() % 5 + 2;
                                } break;

                                case DOWN: {
                                    enemy->wander_destination.y += rand() % 5 + 2;
                                } break;

                                case LEFT: {
                                    enemy->wander_destination.x -= rand() % 5 + 2;
                                } break;

                                case RIGHT: {
                                    enemy->wander_destination.x += rand() % 5 + 2;
                                } break;
                            }
                        }
                    }
                } break;

                case AI_STATE_CHASE: {
                    
                } break;

                default: {
                    printf("Unknown AI state %d\n", enemy->ai_state);
                } break;
            }

            if(enemy->moving && enemy->animation_timer <= 0) {
                enemy->animation_frame = (enemy->animation_frame == 0) ? 1 : 0;
                enemy->animation_timer = ENEMY_ANIMATION_TIME;
            }
        }

        u8* keys_state = (u8*)SDL_GetKeyboardState(0);

        bool moved = false;
        Entity temp_player;
        temp_player.size = player.size;
        temp_player.pos = player.pos;
        if(keys_state[SDL_SCANCODE_UP] && !keys_state[SDL_SCANCODE_DOWN]) {
            player.direction = UP;
            moved = true;
            temp_player.pos.y -= player.speed * last_delta;

            if(!EntityCollidingWithTileY(&temp_player, &map, &display)) {
                player.pos.y = temp_player.pos.y;
            }
        }
        else if(keys_state[SDL_SCANCODE_DOWN] && !keys_state[SDL_SCANCODE_UP]) {
            player.direction = DOWN;
            moved = true;
            temp_player.pos.y += player.speed * last_delta;

            if(!EntityCollidingWithTileY(&temp_player, &map, &display)) {
                player.pos.y = temp_player.pos.y;
            }
        }

        if(keys_state[SDL_SCANCODE_LEFT] && !keys_state[SDL_SCANCODE_RIGHT]) {
            player.direction = LEFT;
            moved = true;
            temp_player.pos.x -= player.speed * last_delta;

            if(!EntityCollidingWithTileX(&temp_player, &map, &display)) {
                player.pos.x = temp_player.pos.x;
            }
        }
        else if(keys_state[SDL_SCANCODE_RIGHT] && !keys_state[SDL_SCANCODE_LEFT]) {
            player.direction = RIGHT;
            moved = true;
            temp_player.pos.x += player.speed * last_delta;

            if(!EntityCollidingWithTileY(&temp_player, &map, &display)) {
                player.pos.x = temp_player.pos.x;
            }
        }

        if(!moved && player.moving) {
            player.moving = false;
        }
        else if(moved && !player.moving) {
            // Start at 0 so the we advance the frames as soon as we start moving.
            player.moving = true;
            player.animation_timer = 0; 
        }

        display.camera_pos = {player.pos.x - (display.camera_size.w / 2), 
                              player.pos.y - (display.camera_size.h / 2)};

        SDL_RenderClear(display.renderer);
        for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
            *((u32*)display.pixel_buffer.pixels + i) = 0xFF000000;
        }

        for(i32 y = 0; y < map.height; y++) {
            for(i32 x = 0; x < map.width; x++) {
                i32 tile_x = x * TILE_SIZE - roundf(display.camera_pos.x * (f32)display.pixels_per_meter);
                i32 tile_y = y * TILE_SIZE - roundf(display.camera_pos.y * (f32)display.pixels_per_meter);
                if(tile_x + TILE_SIZE + TILE_SIZE >= display.camera_pos.x && tile_x - TILE_SIZE < display.camera_pos.x + display.pixel_buffer.width &&
                   tile_y + TILE_SIZE + TILE_SIZE >= display.camera_pos.y && tile_y - TILE_SIZE < display.camera_pos.y + display.pixel_buffer.height) {

                    Vec2 render_pos = {(f32)tile_x / (f32)display.pixels_per_meter, (f32)tile_y / (f32)display.pixels_per_meter};
                    Tile* tile = &tiles[map.tile_ids[y * map.width + x]];
                    
                    RenderSubTexture(&display, &render_pos, &texture_sheet, tile->sub_texture_data.x, tile->sub_texture_data.y,
                                     tile->sub_texture_data.width, tile->sub_texture_data.height);
                }
            }
        }

        if(player.animation_timer <= 0) {
            player.animation_frame = (player.animation_frame == 1) ? 0 : 1;
            player.animation_timer = PLAYER_ANIMATION_TIME;
        }

        RenderPlayer(&display, &player, &texture_sheet);

        for(i32 i = 0; i < enemies.num_enemies; i++) {
            Enemy* enemy = &enemies.enemies[i];
            i32 estx = enemy->sub_texture_data.x + (enemy->direction * enemy->sub_texture_data.width);
            i32 esty = enemy->sub_texture_data.y + (enemy->animation_frame * enemy->sub_texture_data.height);
            Vec2 enemy_world_pos = {enemy->pos.x - display.camera_pos.x, 
                                    enemy->pos.y - display.camera_pos.y};
            RenderSubTexture(&display, &enemy_world_pos, &texture_sheet, estx, esty, enemy->sub_texture_data.width, enemy->sub_texture_data.height);
        }

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
            player.animation_timer -= ms_this_frame;
        }
        for(i32 i = 0; i < enemies.num_enemies; i++) {
            Enemy* enemy = &enemies.enemies[i];
            if(enemy->moving) {
                enemy->animation_timer -= ms_this_frame;
            }
        }

        //printf("%d ms this frame\n", ms_this_frame);
        last_delta = ms_this_frame / 1000.0f;
    }

    // Freeing stuff is overrated

    return 0;
}