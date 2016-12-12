
/*
    Theme 1 room:
    =============

    a 1 room dungeon that changes shape when you figure out the puzzle
*/


#include "ld_37_lib.h"
#include <string>
#include <sstream>

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
    Sub_Texture_Data walk_anim_tex_data;
    Sub_Texture_Data walk_attack_anim_tex_data;
    Sub_Texture_Data idle_attack_anim_tex_data;
    i32 anim_state;
    i32 anim_frame;
    i32 anim_timer;
    i32 direction;
    f32 speed;
    i32 health;
    i32 damage;
    i32 knockback_frames;
    i32 knockback_direction;
    f32 knockback_speed;
};

class Player : public Mob {
public:
    i32 potions;
    i32 bombs;
    bool has_sword;
    bool has_shield;
    bool has_grapple;
};


class Enemy : public Mob {
public:
    i32 ai_state;
    i32 chase_speed;
    Vec2 wander_destination;
    i32 dead_timer;
    bool alive;
};

typedef struct {
    Sub_Texture_Data sub_texture_data;
    Vec2 size;
    bool solid;
} Tile;

typedef struct {
    i32 width;
    i32 height;
    u32* tile_data;
    Tile* tiles;
} Map;

typedef struct {
    Enemy* enemies;
    i32 num_enemies;
    i32 num_alive_enemies;
} Enemy_List;

#define TARGET_MS_PER_FRAME                 16
#define CAP_FRAMERATE                       1
#define PLAYER_COLLISION_ON                 0
#define UPDATE_ENEMIES                      0

#define UP                                  0
#define DOWN                                1
#define LEFT                                2
#define RIGHT                               3

const f32 PIXELS_PER_METER                  = 16.0;
#define TILE_NO_EXTRA_DATA                  0x0000
const i32 TILE_SIZE                         = 16;
const Vec2 TILE_SIZE_IN_METERS              = {(i32)TILE_SIZE / PIXELS_PER_METER, (i32)TILE_SIZE / PIXELS_PER_METER};
#define TILE_BLANK                          0
#define TILE_BRICK                          1
#define TILE_BREAKABLE_BRICK                2
#define TILE_DIRT                           3
#define TILE_BUTTON_UNACTIVATED             4
#define TILE_BUTTON_ACTIVATED               5
#define TILE_BARRICATE_VIRTICAL             6
#define TILE_BARRICATE_HORIZONTAL           7
#define TILE_GEM                            8
#define TILE_CHEST_UNOPENED                 9
#define TILE_CHEST_OPENED                   10
#define TILE_VOID                           11
#define TILE_ROCK                           12

#define ANIM_STATE_IDLE                     0
#define ANIM_STATE_WALK                     1
#define ANIM_STATE_WALK_ATTACK              2
#define ANIM_STATE_IDLE_ATTACK              3

#define PLAYER_WALK_ANIMATION_TIME          300
#define PLAYER_ATTACK_ANIMATION_TIME        300
#define PLAYER_SPEED                        4.2
#define PLAYER_HEALTH                       20
#define PLAYER_DAMAGE                       2
const f32 PLAYER_PUNCH_KNOCKBACK_SPEED      = 0.30;

#define AI_STATE_WANDER                     0
#define AI_STATE_CHASE                      1
#define AI_STATE_ATTACK                     2
#define ENEMY_WALK_ANIMATION_TIME           200
#define ENEMY_ATTACK_ANIMATION_TIME_FRAME_0 100
#define ENEMY_ATTACK_ANIMATION_TIME_FRAME_1 400
#define ENEMY_CHASE_SPEED                   3.3
#define ENEMY_WANDER_SPEED                  3.1
#define ENEMY_HEALTH                        10
#define ENEMY_DAMAGE                        1
#define NUM_ENEMIES_MIN                     6
#define NUM_ENEMIES_MAX                     10
const f32 ENEMY_PICHFORK_KNOCKBACK_SPEED    = 0.4;

#define MAP_FILE_EMPTY_COLOUR               0x00FF00FF
#define MAP_FILE_BRICK_COLOUR               0x00666666
#define MAP_FILE_BREAKABLE_BRICK_COLOUR     0x00858585
#define MAP_FILE_DIRT_COLOUR                0x006A5A44
#define MAP_FILE_BARRICATE_COLOUR           0x003D3D3D
#define MAP_FILE_FLOOR_BUTTON_COLOUR        0x00C3C3C3
#define MAP_FILE_CHEST_COLOUR               0x00FFFF64
#define MAP_FILE_GEM_COLOUR                 0x0000FFFF
#define MAP_FILE_VOID_COLOUR                0x00000000
#define MAP_FILE_ROCK_COLOUR                0x00EEEEEE
#define MAP_FILE_START_POS_COLOUR           0x000000FF
#define MAP_FILE_SPAWN_DEMON_COLOUR         0x00FF0000
#define MAP_FILE_SPAWN_WRAITH_COLOUR        0x00008888

const f32 DIAGONAL_MOVEMENT_CONSTANT        = 0.70711;
const f32 MAP_ROT_ANIM_SPEED_MODIFIER       = 1;
const f32 MAX_MAP_ROT_SPEED                 = 53;
#define ITEM_BOX_RECT_COLOUR                0xFF696969
#define HUD_TEXT_COLOUR                     0xFFEEEEEE
#define POTION_HEAL_VALUE                   5

#define PLAY_STATE                          0
#define START_STATE                         1
#define DEATH_STATE                         2
#define WIN_STATE                           3

#define ATTACK_KEY                          SDL_SCANCODE_Z
#define USE_POTION_KEY                      SDL_SCANCODE_A
#define USE_BOMB_KEY                        SDL_SCANCODE_D
#define USE_GRAPPLE_KEY                     SDL_SCANCODE_S
#define USE_SHEILD_KEY                      SDL_SCANCODE_X

#define ITEM_BOX_WEAPON                     0
#define ITEM_BOX_SHIELD                     1
#define ITEM_BOX_GRAPPLE                    2
#define ITEM_BOX_POTION                     3
#define ITEM_BOX_BOMB                       4

typedef struct {
    char item_use_char; // The character corrospending to a key that uses this item
    Sub_Texture_Data icon_tex_data;
    Vec2 pos;
    Vec2 size;
} UI_Item_Box;

bool EntityCollidingWithSolidTile(Entity* entity, Map* map, Display* display) {
    bool colliding = false;

    i32 tile_x = roundf((((entity->pos.x - (entity->size.w / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    i32 tile_y = roundf((((entity->pos.y - (entity->size.h / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    u8 tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFF);
    Tile* tile_top_left_corner = &map->tiles[tile_id];

    tile_x = roundf((((entity->pos.x + (entity->size.w / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y - (entity->size.h / 2)) * display->pixels_per_meter) - 1) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFF);
    Tile* tile_top_right_corner = &map->tiles[tile_id];

    tile_x = roundf((((entity->pos.x - (entity->size.w / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFF);
    Tile* tile_bottom_left_corner = &map->tiles[tile_id];

    tile_x = roundf((((entity->pos.x + (entity->size.w / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    tile_y = roundf((((entity->pos.y + (entity->size.h / 2)) * display->pixels_per_meter) + 1) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFF);
    Tile* tile_bottom_right_corner = &map->tiles[tile_id];


    if(tile_bottom_right_corner->solid || tile_bottom_left_corner->solid ||
       tile_top_left_corner->solid || tile_top_right_corner->solid) {
        colliding = true;
    }

    return colliding;
}

// If entity 1 is within entity 2
bool EntityCollidingWithEntity(Entity* entity1, Entity* entity2) {
    bool colliding = false;

    if(entity1->pos.x + entity1->size.w >= entity2->pos.x && entity1->pos.x <= entity2->pos.x + entity2->size.w &&
        entity1->pos.y + entity1->size.h >= entity2->pos.y && entity1->pos.h <= entity2->pos.y + entity2->size.h) {
        colliding = true;
    }

    return colliding;
}

i32 PlayerOnFloorButton(Player* player, Map* map) {
    // Top left corner tile
    i16 tile_x = (i16)roundf((((player->pos.x - (player->size.w / 2)) * PIXELS_PER_METER) - 1) / TILE_SIZE);
    i16 tile_y = (i16)roundf((((player->pos.y - (player->size.h / 2)) * PIXELS_PER_METER) - 1) / TILE_SIZE);
    u8 tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_BUTTON_UNACTIVATED) {
        return (tile_x << 16) | tile_y;
    } 

    tile_x = roundf((((player->pos.x + (player->size.w / 2)) * PIXELS_PER_METER) - 1) / TILE_SIZE);
    tile_y = roundf((((player->pos.y - (player->size.h / 2)) * PIXELS_PER_METER) - 1) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_BUTTON_UNACTIVATED) {
        return (tile_x << 16) | tile_y;
    } 

    tile_x = roundf((((player->pos.x - (player->size.w / 2)) * PIXELS_PER_METER) + 1) / TILE_SIZE);
    tile_y = roundf((((player->pos.y + (player->size.h / 2)) * PIXELS_PER_METER) + 1) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_BUTTON_UNACTIVATED) {
        return (tile_x << 16) | tile_y;
    } 

    tile_x = roundf((((player->pos.x + (player->size.w / 2)) * PIXELS_PER_METER) + 1) / TILE_SIZE);
    tile_y = roundf((((player->pos.y + (player->size.h / 2)) * PIXELS_PER_METER) + 1) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_BUTTON_UNACTIVATED) {
        return (tile_x << 16) | tile_y;
    } 

    return -1;
}

Mix_Chunk* LoadWAV(char* path) {
    Mix_Chunk* sound = Mix_LoadWAV(path);
    if(sound) {
        Mix_VolumeChunk(sound, MIX_MAX_VOLUME / 10);
    }
    else {
        printf("Could not load wav file: `%s`. Mix_Error: %s\n", "../res/sounds/hit.wav", Mix_GetError());
    }

    return sound;
}

void ReplaceAllSameAdjacentTiles(Map* map, i32 x, i32 y, u16 tile_to_replace, u16 replacement_tile) {
    u16 tile_left_adj_id =   (u16)((map->tile_data[y * map->width + x - 1] >> 16) & 0x0000FFFF);
    u16 tile_right_adj_id =  (u16)((map->tile_data[y * map->width + x + 1] >> 16) & 0x0000FFFF);
    u16 tile_top_adj_id =    (u16)((map->tile_data[(y - 1) * map->width + x] >> 16) & 0x0000FFFF);
    u16 tile_bottom_adj_id = (u16)((map->tile_data[(y + 1) * map->width + x] >> 16) & 0x0000FFFF);

    // If we find a tile to the left replace it then branch off that tile
    if(tile_left_adj_id == tile_to_replace) {
        map->tile_data[y * map->width + x - 1] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTiles(map, x - 1, y, tile_to_replace, replacement_tile);
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_right_adj_id == tile_to_replace) {
        map->tile_data[y * map->width + x + 1] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTiles(map, x + 1, y, tile_to_replace, replacement_tile);
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_top_adj_id == tile_to_replace) {
        map->tile_data[(y - 1) * map->width + x] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTiles(map, x, y - 1, tile_to_replace, replacement_tile);
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_bottom_adj_id == tile_to_replace) {
        map->tile_data[(y + 1) * map->width + x] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTiles(map, x, y + 1, tile_to_replace, replacement_tile);
    }
}

void LoadEnemiesList(Enemy_List* list, Map* map) {
    if(list->enemies) {
        free(list->enemies);
    }

    list->num_enemies = rand() % (NUM_ENEMIES_MAX - NUM_ENEMIES_MIN) + NUM_ENEMIES_MIN;
    list->num_alive_enemies = list->num_enemies;
    printf("%d enemies created\n", list->num_enemies);
    list->enemies = (Enemy*)malloc(sizeof(Enemy) * list->num_enemies);

    for(i32 i = 0; i < list->num_enemies; i++) {
        Enemy* enemy = &list->enemies[i];
        // Only spawn on dirt tiles
        while(true) {
            i32 tile_x = rand() % (i32)(map->width * TILE_SIZE_IN_METERS.w);
            i32 tile_y = rand() % (i32)(map->height * TILE_SIZE_IN_METERS.w);
            u16 tile_id = (u16)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFF);

            if(tile_id == TILE_DIRT) {
                enemy->pos.x = tile_x;
                enemy->pos.y = tile_y;
                break;
            }
        }

        enemy->size = {15.0 / PIXELS_PER_METER, 15.0 / PIXELS_PER_METER};
        enemy->walk_anim_tex_data = {0, 80, 16, 16};
        enemy->walk_attack_anim_tex_data = {80, 80, 16, 16};
        enemy->idle_attack_anim_tex_data = {160, 80, 16, 16};
        enemy->direction = rand() % 4;
        enemy->speed = ENEMY_WANDER_SPEED;
        enemy->chase_speed = ENEMY_CHASE_SPEED;
        enemy->ai_state = AI_STATE_WANDER;
        enemy->anim_frame = 0;
        enemy->anim_timer = 0;
        enemy->knockback_frames = 0;
        enemy->knockback_direction = 0;
        enemy->health = ENEMY_HEALTH;
        enemy->damage = ENEMY_DAMAGE;
        enemy->knockback_speed = 0;
        enemy->alive = true;
    }
}

i32 main(i32 argc, char** argv) {
    srand(time(0));
    if(InitSDL() < 0) {
        return -1;
    }

    // Initalize display
    Display display = {0};
    display.width = 1024;
    display.height = 576;
    display.title = "LD 37";
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

    // Load in the map
    Tile tiles[13] = {
        {{0, 0,  TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false}, // blank
        {{16, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // brick
        {{32, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // breakable brick
        {{48, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false}, // dirt
        {{64, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false}, // floor button unactivated
        {{80, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false}, // floor button activated
        {{96, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // barricade virtical
        {{112, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // barricade horizontal
        {{128, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // gem
        {{144, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // chest unopened
        {{160, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // chest opended
        {{176, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}, // void
        {{192, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true} // rock
    };

    Texture map_file_texture;
    Vec2 player_start_pos = {0, 0};
    LoadTexture("../res/textures/map1.png", &map_file_texture, display.pixel_format);
    Map map = {map_file_texture.width, map_file_texture.height, 0};
    map.tiles = tiles;
    map.tile_data = (u32*)malloc(map.width * map.height * sizeof(u32));
    i16 current_id = 1;
    for(i32 y = 0; y < map.height; y++) {
        for(i32 x = 0; x < map.width; x++) {
            u32 pixel = *((u32*)map_file_texture.pixels + (y * map_file_texture.width) + x);

            switch(pixel) {
                case MAP_FILE_BRICK_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_BRICK << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_DIRT_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_START_POS_COLOUR: {
                    player_start_pos = {(f32)x * (f32)TILE_SIZE / (f32)display.pixels_per_meter, (f32)y * (f32)TILE_SIZE / (f32)display.pixels_per_meter};
                    map.tile_data[(y * map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_BARRICATE_COLOUR: {
                    // We'll change the barricades to their proper orientation after
                    map.tile_data[(y * map.width) + x] = (TILE_BARRICATE_VIRTICAL << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_FLOOR_BUTTON_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_BUTTON_UNACTIVATED << 16) | current_id;
                    current_id++;
                } break;

                case MAP_FILE_BREAKABLE_BRICK_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_BREAKABLE_BRICK << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_CHEST_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_CHEST_UNOPENED << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_GEM_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_GEM << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_VOID_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_VOID << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_ROCK_COLOUR: {
                    map.tile_data[(y * map.width) + x] = (TILE_ROCK << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_SPAWN_DEMON_COLOUR: {
                    // create demon
                    map.tile_data[(y * map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_SPAWN_WRAITH_COLOUR: {
                    // create wraith
                    map.tile_data[(y * map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_EMPTY_COLOUR:
                default: {
                    map.tile_data[(y * map.width) + x] = (TILE_BLANK << 16) | TILE_NO_EXTRA_DATA;
                } break;
            }
        }
    }
    FreeTexture(&map_file_texture);

    //Reprocess the map adding in button activations and flipping doors
    for(i32 y = 0; y < map.height; y++) {
        for(i32 x = 0; x < map.width; x++) {
            u32 tile_data = map.tile_data[y * map.width + x];
            u16 tile_id = (u16)((tile_data >> 16) & 0x0000FFFF);

            switch(tile_id) {
                case TILE_BARRICATE_VIRTICAL: {
                    // switch the door to a horizontal door if there are brick 
                    // tiles or other door tiles horizontally adjacent to them
                    u16 tile_adj_left_id = u16((map.tile_data[(y * map.width) + (x - 1)] >> 16) & 0x0000FFFF);
                    u16 tile_adj_right_id = u16((map.tile_data[(y * map.width) + (x + 1)] >> 16) & 0x0000FFFF);

                    if(tile_adj_left_id == TILE_BRICK || tile_adj_left_id == TILE_BARRICATE_VIRTICAL || 
                       tile_adj_left_id == TILE_BARRICATE_HORIZONTAL || tile_adj_right_id == TILE_BRICK || 
                       tile_adj_right_id == TILE_BARRICATE_VIRTICAL ||  tile_adj_right_id == TILE_BARRICATE_HORIZONTAL) {
                        map.tile_data[y * map.width + x] = (TILE_BARRICATE_HORIZONTAL << 16) | TILE_NO_EXTRA_DATA;
                    }
                } break;

                case TILE_BUTTON_UNACTIVATED: {
                    u16 activation_id = (u16)(tile_data & 0x0000FFFF);
                    // set the extra data to the first tile in the series of door tiles the buttons open
                    // This will be the button that opens the door that opens the right side of the room
                    if(activation_id == 1) {
                        // set to the first tile in the series of door tiles
                        // door tile coords = (20, 9)
                        map.tile_data[y * map.width + x] = (tile_id << 16) | (u16)263;
                    }
                    // This will be the button that opens the door that opens the top portion of the room
                    else if(activation_id == 2) {
                        // set to the first tile in the series of door tiles
                        // door tile coords = (6, 9)
                        map.tile_data[y * map.width + x] = (tile_id << 16) | (u16)249;
                        i32 j = 0;
                    }
                    // This will be the button tht opens the door that opens the top portion of the room
                    else if(activation_id == 3) {
                        // door tile coords = (12, 5)
                        map.tile_data[y * map.width + x] = (tile_id << 16) | (u16)147;
                    }
                } break;
            }
        }
    }

    // Create Player
    Player player;
    player.pos = player_start_pos;
    // NOTE (Mathew): Making player a bit smaller than the texture is so that 
    //                you don't collide when there are 2 tiles on either side of you.
    player.size = {15.0 / PIXELS_PER_METER, 15.0 / PIXELS_PER_METER};
    player.walk_anim_tex_data = {0, 32, TILE_SIZE, TILE_SIZE};
    player.walk_attack_anim_tex_data = {80, 32, TILE_SIZE, TILE_SIZE};
    player.idle_attack_anim_tex_data = {160, 32, TILE_SIZE, TILE_SIZE};
    player.direction = 1;
    player.speed = 4.2;
    player.anim_state = ANIM_STATE_IDLE;
    player.anim_timer = 0;
    player.anim_frame = 0;
    player.knockback_frames = 0;
    player.knockback_direction = 0;
    player.health = PLAYER_HEALTH;
    player.damage = PLAYER_DAMAGE;
    player.knockback_speed = 0;
    player.has_sword = true;
    player.has_shield = true;
    player.has_grapple = false;
    player.potions = 10;
    player.bombs = 69;

    //Load the SOUNDS
    Mix_Chunk* hit_sound = LoadWAV("../res/sounds/hit.wav");
    Mix_Chunk* death_sound = LoadWAV("../res/sounds/death.wav");
    Mix_Chunk* button_activation_sound = LoadWAV("../res/sounds/button_activation.wav");

    // Create Enemeies
    Enemy_List enemy_list = {0};
    LoadEnemiesList(&enemy_list, &map);

    // Setup camera and hub
    Font font = {};
    LoadFont(&font, display.pixel_format);

    Vec2 hud_size = {(f32)display.pixel_buffer.width / PIXELS_PER_METER, display.pixel_buffer.height / 3.5 / PIXELS_PER_METER};
    Vec2 hud_pos = {display.pixel_buffer.width / 2.0 / PIXELS_PER_METER, display.pixel_buffer.height / PIXELS_PER_METER - (hud_size.h / 2)};
    u32  hud_colour = 0xFF444444;
    Vec2 health_display_pos = {0.3, hud_pos.y - 0.4};

    display.camera_size = {(f32)display.pixel_buffer.width / PIXELS_PER_METER, 
                           (f32)display.pixel_buffer.height / PIXELS_PER_METER - hud_size.h};
    display.camera_pos = {player.pos.x - (display.camera_size.w / 2), player.pos.y - (display.camera_size.h / 2)};

    // Initalize UI
    const i32 NUM_ITEM_BOXES = 5;
    Vec2 inital_item_box_pos = {hud_pos.x + 0.2, hud_pos.y - 0.65};
    UI_Item_Box item_boxes[NUM_ITEM_BOXES] = {
        {'z', {240, 48, 16, 16}, inital_item_box_pos, {1.1, 1.1}},                                          // weapon item box
        {'x', {224, 64, 16, 16}, {inital_item_box_pos.x + 2.3, inital_item_box_pos.y}, {1.1, 1.1}},         // Sheild item box
        {'s', {224, 48, 16, 16}, {inital_item_box_pos.x + 4.6, inital_item_box_pos.y}, {1.1, 1.1}},         // grapple item box
        {'a', {224, 32, 16, 16}, {inital_item_box_pos.x, inital_item_box_pos.y + 1.3}, {1.1, 1.1}},         // potion item box
        {'d', {240, 32, 16, 16}, {inital_item_box_pos.x + 4.6, inital_item_box_pos.y + 1.3}, {1.1, 1.1}}    // bomb item box

    };
        char item_use_char; // The character corrospending to a key that uses this item
    Sub_Texture_Data icon_tex_data;
    Vec2 pos;
    Vec2 size;

    // Game Loop
    bool running = true;
    SDL_Event event = {0};
    f32 last_delta = 0.0f;
    bool animating_map = false;
    i32 map_animation_step = 0; // 0 for speed up 1 for speed down 2 for flip and change map
    f32 map_rot = 0;
    f32 map_rot_speed = 0;
    SDL_RendererFlip map_flip = SDL_FLIP_NONE;
    i32 game_state = START_STATE;
    while(running) {
        i32 ms_before = SDL_GetTicks();

        switch(game_state) {
            case PLAY_STATE: {
                bool attacked = false;
                while(SDL_PollEvent(&event)) {
                    switch(event.type) {
                        case SDL_QUIT: {
                            running = false;
                        } break;

                        case SDL_KEYUP: {
                            if(event.key.keysym.scancode == SDL_SCANCODE_Z) {
                                attacked = true;
                            }
                            else if(event.key.keysym.scancode == USE_POTION_KEY && player.potions > 0 && player.health < PLAYER_HEALTH) {
                                player.health += POTION_HEAL_VALUE;
                                if(player.health > PLAYER_HEALTH) {
                                    player.health = PLAYER_HEALTH;
                                }

                                player.potions--;
                            }
                            else if(event.key.keysym.scancode == USE_BOMB_KEY && player.bombs > 0) {
                                player.bombs--;
                                //DoBombStuff();
                            }
                        } break;
                    }
                }

                // Update Enemy
                if(!animating_map) {
                    for(i32 i = 0; i < enemy_list.num_enemies; i++) {
                        Enemy* enemy = &enemy_list.enemies[i];
                        if(!enemy->alive) {
                            continue;
                        }

                        bool moved = false;

                        // Handle AI 
                        switch(enemy->ai_state) {
                            case AI_STATE_WANDER: {
                                if(player.pos.x >= enemy->pos.x - 3.5 && player.pos.x <= enemy->pos.x + 3.5 &&
                                    player.pos.y >= enemy->pos.y - 3.5 && player.pos.y <= enemy->pos.y + 3.5) {
                                    
                                    enemy->ai_state = AI_STATE_CHASE;
                                    enemy->anim_state = ANIM_STATE_WALK;
                                    enemy->anim_frame = 0;
                                    enemy->anim_timer = ENEMY_WALK_ANIMATION_TIME;
                                    break;
                                }

                                if((player.pos.x >= enemy->pos.x - 0.5 && player.pos.x <= enemy->pos.x + 0.5) &&
                                    (player.pos.y >= enemy->pos.y - 0.5 && player.pos.y <= enemy->pos.y + 0.5)) {
                                    
                                    enemy->ai_state = AI_STATE_ATTACK;
                                    enemy->anim_state = ANIM_STATE_WALK_ATTACK;
                                    enemy->anim_timer = ENEMY_ATTACK_ANIMATION_TIME_FRAME_0;
                                    enemy->anim_frame = 0;
                                    break;
                                }

                                if(enemy->anim_state == ANIM_STATE_WALK) {
                                    bool reached_destination = false;
                                    Entity temp_enemy = {enemy->pos, enemy->size};
                                    switch(enemy->direction) {
                                        case UP: {
                                            temp_enemy.pos.y -= enemy->speed * last_delta;
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
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
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
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
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
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
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
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
                                        enemy->anim_state = ANIM_STATE_IDLE;
                                        enemy->anim_timer = 0;
                                    }
                                }
                                else {
                                    if(rand() % 333 < 10) {
                                        enemy->wander_destination = enemy->pos;
                                        enemy->direction = rand() % 4;
                                        enemy->anim_state = ANIM_STATE_WALK;
                                        enemy->anim_timer = ENEMY_WALK_ANIMATION_TIME;

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
                                if((player.pos.x >= enemy->pos.x - 0.5 && player.pos.x <= enemy->pos.x + 0.5) &&
                                    (player.pos.y >= enemy->pos.y - 0.5 && player.pos.y <= enemy->pos.y + 0.5)) {
                                    
                                    enemy->ai_state = AI_STATE_ATTACK;
                                    enemy->anim_state = ANIM_STATE_WALK_ATTACK;
                                    enemy->anim_timer = ENEMY_ATTACK_ANIMATION_TIME_FRAME_0;
                                    enemy->anim_frame = 0;
                                    break;
                                }

                                Entity temp_enemy = {enemy->pos, enemy->size};
                                if(player.pos.y < enemy->pos.y) {
                                    enemy->direction = UP;
                                    temp_enemy.pos.y -= enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
                                        enemy->pos.y = temp_enemy.pos.y;
                                    }
                                }
                                else if(player.pos.y > enemy->pos.y) {
                                    enemy->direction = DOWN;
                                    temp_enemy.pos.y += enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
                                        enemy->pos.y = temp_enemy.pos.y;
                                    }
                                }

                                if(player.pos.x < enemy->pos.x) {
                                    enemy->direction = LEFT;
                                    temp_enemy.pos.x -= enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
                                        enemy->pos.x = temp_enemy.pos.x;
                                    }
                                }
                                else if(player.pos.x > enemy->pos.x) {
                                    enemy->direction = RIGHT;
                                    temp_enemy.pos.x += enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &map, &display)) {
                                        enemy->pos.x = temp_enemy.pos.x;
                                    }
                                }
                            } break;

                            case AI_STATE_ATTACK: {
                                if(enemy->anim_timer <= 0 && enemy->anim_frame == 0) {
                                    
                                    enemy->anim_timer = ENEMY_ATTACK_ANIMATION_TIME_FRAME_1;
                                    enemy->anim_frame = 1;

                                    Entity enemy_entity = {enemy->pos, enemy->size};
                                    Entity player_entity = {player.pos, player.size};

                                    if(EntityCollidingWithEntity(&enemy_entity, &player_entity)) {
                                        Mix_PlayChannel(-1, hit_sound, 0);

                                        player.health -= enemy->damage;
                                        printf("Your Health: %d\n", player.health);

                                        player.knockback_frames = 6;
                                        player.knockback_direction = enemy->direction;
                                        player.knockback_speed = ENEMY_PICHFORK_KNOCKBACK_SPEED;

                                        if(player.health <= 0) {
                                            Mix_PlayChannel(-1, death_sound, 0);
                                            printf("You Died You Suck NOOB!\n");
                                            system("pause");
                                            return 0;
                                        }
                                    }
                                }
                                else if(enemy->anim_timer <= 0 && enemy->anim_frame == 1) {
                                    enemy->ai_state = AI_STATE_WANDER;
                                    enemy->anim_frame = 0;
                                    enemy->anim_timer = ENEMY_WALK_ANIMATION_TIME;
                                }
                            } break;

                            default: {
                                printf("Unknown AI state %d\n", enemy->ai_state);
                            } break;
                        }

                        // Update walk animation
                        if(enemy->anim_state == ANIM_STATE_WALK && enemy->anim_timer <= 0) {
                            enemy->anim_frame = (enemy->anim_frame == 0) ? 1 : 0;
                            enemy->anim_timer = ENEMY_WALK_ANIMATION_TIME;
                        }

                        // Knock enemy back
                        if(enemy->knockback_frames > 0) {
                            enemy->knockback_frames--;
                            Entity enemy_entity = {enemy->pos, enemy->size};

                            switch(enemy->knockback_direction) {
                                case UP: {
                                    enemy_entity.pos.y -= enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &map, &display)) {
                                        enemy->pos.y = enemy_entity.pos.y;
                                    }
                                } break;

                                case DOWN: {
                                    enemy_entity.pos.y += enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &map, &display)) {
                                        enemy->pos.y = enemy_entity.pos.y;
                                    }
                                } break;

                                case LEFT: {
                                    enemy_entity.pos.x -= enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &map, &display)) {
                                        enemy->pos.x = enemy_entity.pos.x;
                                    }
                                } break;

                                case RIGHT: {
                                    enemy_entity.pos.x += enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &map, &display)) {
                                        enemy->pos.x = enemy_entity.pos.x;
                                    }
                                } break;
                            }
                        }
                    }

                    if(enemy_list.num_alive_enemies == 0) {
                        animating_map = true;
                    }

                    // Update Player
                    u8* keys_state = (u8*)SDL_GetKeyboardState(0);

                    bool moved = false;
                    Entity temp_player;
                    temp_player.size = player.size;
                    temp_player.pos = player.pos;

                    if(keys_state[SDL_SCANCODE_UP] && !keys_state[SDL_SCANCODE_DOWN]) {
                        if(player.knockback_frames == 0) {
                            player.direction = UP;
                            moved = true;
                            temp_player.pos.y -= player.speed * last_delta;

                            #if PLAYER_COLLISION_ON
                            if(!EntityCollidingWithSolidTile(&temp_player, &map, &display)) {
                                player.pos.y = temp_player.pos.y;
                            }
                            #else
                            player.pos.y = temp_player.pos.y;
                            #endif
                        }
                    }
                    else if(keys_state[SDL_SCANCODE_DOWN] && !keys_state[SDL_SCANCODE_UP]) {
                        if(player.knockback_frames == 0) {
                            player.direction = DOWN;
                            moved = true;
                            temp_player.pos.y += player.speed * last_delta;

                            #if PLAYER_COLLISION_ON
                            if(!EntityCollidingWithSolidTile(&temp_player, &map, &display)) {
                                player.pos.y = temp_player.pos.y;
                            }
                            #else
                            player.pos.y = temp_player.pos.y;
                            #endif
                        }
                    }

                    if(keys_state[SDL_SCANCODE_LEFT] && !keys_state[SDL_SCANCODE_RIGHT]) {
                        if(player.knockback_frames == 0) {
                            player.direction = LEFT;
                            moved = true;
                            temp_player.pos.x -= player.speed * last_delta;

                            #if PLAYER_COLLISION_ON
                            if(!EntityCollidingWithSolidTile(&temp_player, &map, &display)) {
                                player.pos.x = temp_player.pos.x;
                            }
                            #else
                            player.pos.x = temp_player.pos.x;
                            #endif
                        }
                    }
                    else if(keys_state[SDL_SCANCODE_RIGHT] && !keys_state[SDL_SCANCODE_LEFT]) {
                        if(player.knockback_frames == 0) {
                            player.direction = RIGHT;
                            moved = true;
                            temp_player.pos.x += player.speed * last_delta;

                            #if PLAYER_COLLISION_ON
                            if(!EntityCollidingWithSolidTile(&temp_player, &map, &display)) {
                                player.pos.x = temp_player.pos.x;
                            }            
                            #else
                            player.pos.x = temp_player.pos.x;
                            #endif
                        }
                    }

                    // Knock player back
                    if(player.knockback_frames > 0) {
                        player.knockback_frames--;
                        Entity player_entity = {player.pos, player.size};

                        switch(player.knockback_direction) {
                            case UP: {
                                player_entity.pos.y -= player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &map, &display)) {
                                    player.pos.y = player_entity.pos.y;
                                }
                            } break;

                            case DOWN: {
                                player_entity.pos.y += player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &map, &display)) {
                                    player.pos.y = player_entity.pos.y;
                                }
                            } break;

                            case LEFT: {
                                player_entity.pos.x -= player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &map, &display)) {
                                    player.pos.x = player_entity.pos.x;
                                }
                            } break;

                            case RIGHT: {
                                player_entity.pos.x += player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &map, &display)) {
                                    player.pos.x = player_entity.pos.x;
                                }
                            } break;
                        }
                    }

                    // Player Activation
                    i32 tile_pos_data;
                    if((tile_pos_data = PlayerOnFloorButton(&player, &map)) != -1) {
                        i32 x = (tile_pos_data >> 16) & 0x0000FFFF;
                        i32 y = tile_pos_data & 0x0000FFFF;
                        u32 tile_data = map.tile_data[y * map.width + x];
                        u16 tile_id = (u16)((tile_data >> 16) & 0x0000FFFF);
                        u16 tile_extra_data = (u16)(tile_data & 0x0000FFFF);

                        switch(tile_id) {
                            case TILE_BUTTON_UNACTIVATED: {
                                Mix_PlayChannel(-1, button_activation_sound, 0);
                                i32 baricade_tile_x = tile_extra_data % map.width;
                                i32 baricade_tile_y = tile_extra_data / map.width;
                                ReplaceAllSameAdjacentTiles(&map, baricade_tile_x, baricade_tile_y, TILE_BARRICATE_VIRTICAL, TILE_DIRT);
                                ReplaceAllSameAdjacentTiles(&map, baricade_tile_x, baricade_tile_y, TILE_BARRICATE_HORIZONTAL, TILE_DIRT);
                                map.tile_data[y * map.width + x] = (TILE_BUTTON_ACTIVATED << 16) | TILE_NO_EXTRA_DATA;
                            } break;
                        }

                    }

                    // Player Attack
                    if(attacked) {
                        attacked = false;
                        if(player.anim_state == ANIM_STATE_WALK_ATTACK || player.anim_state == ANIM_STATE_IDLE_ATTACK) {
                            player.anim_frame = (player.anim_frame == 0) ? 1 : 0;
                        }
                        else if(player.anim_state == ANIM_STATE_IDLE) {
                            player.anim_state = ANIM_STATE_IDLE_ATTACK;
                        }
                        else if(player.anim_state == ANIM_STATE_WALK) {
                            player.anim_state = ANIM_STATE_WALK_ATTACK;
                        }

                        player.anim_timer = PLAYER_ATTACK_ANIMATION_TIME;

                        for(i32 i = 0; i < enemy_list.num_enemies; i++) {
                            Enemy* enemy = &enemy_list.enemies[i];
                            if(!enemy->alive) {
                                continue;
                            }

                            Entity enemy_entity = {enemy->pos, enemy->size};
                            Entity player_entity = {player.pos, player.size};

                            if(EntityCollidingWithEntity(&player_entity, &enemy_entity)) {
                                Mix_PlayChannel(-1, hit_sound, 0);

                                enemy->health -= player.damage;
                                enemy->knockback_frames = 6;
                                enemy->knockback_direction = player.direction;
                                enemy->knockback_speed = PLAYER_PUNCH_KNOCKBACK_SPEED;

                                if(enemy->health <= 0) {
                                    Mix_PlayChannel(-1, death_sound, 0);
                                    enemy->alive = false;
                                    enemy_list.num_alive_enemies--;
                                }
                            }
                        }
                    }

                    if(!moved && player.anim_state == ANIM_STATE_WALK) {
                        player.anim_state = ANIM_STATE_IDLE;
                    }
                    else if(!moved && player.anim_state == ANIM_STATE_WALK_ATTACK) {
                        player.anim_state = ANIM_STATE_IDLE_ATTACK;
                    }
                    else if(moved && player.anim_state == ANIM_STATE_IDLE_ATTACK) {
                        player.anim_state = ANIM_STATE_WALK_ATTACK;
                    }
                    else if(moved && player.anim_state == ANIM_STATE_IDLE) {
                        player.anim_state = ANIM_STATE_WALK;
                        // Start at 0 so the we advance the frames as soon as we start moving.
                        player.anim_timer = 0;
                    }

                    if(player.anim_state != ANIM_STATE_IDLE && player.anim_timer <= 0) {
                        player.anim_frame = (player.anim_frame == 0) ? 1 : 0;
                        switch(player.anim_state) {
                            case ANIM_STATE_WALK: {
                                player.anim_timer = PLAYER_WALK_ANIMATION_TIME;
                            } break;

                            case ANIM_STATE_IDLE_ATTACK: {
                                player.anim_timer = 0;
                                player.anim_state = ANIM_STATE_IDLE;
                            }
                            case ANIM_STATE_WALK_ATTACK: {
                                player.anim_timer = 0;
                                player.anim_state = ANIM_STATE_WALK;
                            } break;
                        }
                    }

                    display.camera_pos = {player.pos.x - (display.camera_size.w / 2), player.pos.y - (display.camera_size.h / 2)};
                }

                // Clear renderer
                SDL_RenderClear(display.renderer);
                for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
                    *((u32*)display.pixel_buffer.pixels + i) = 0xFF00FFFF;
                }

                // Render map
                for(i32 y = 0; y < map.height; y++) {
                    for(i32 x = 0; x < map.width; x++) {
                        i32 tile_x = x * TILE_SIZE - roundf(display.camera_pos.x * (f32)display.pixels_per_meter);
                        i32 tile_y = y * TILE_SIZE - roundf(display.camera_pos.y * (f32)display.pixels_per_meter);
                        i32 cam_min_x = roundf((display.camera_pos.x - (display.camera_size.w  / 2)) * PIXELS_PER_METER);
                        i32 cam_min_y = roundf((display.camera_pos.y - (display.camera_size.h  / 2)) * PIXELS_PER_METER);
                        i32 cam_max_x = roundf((display.camera_pos.x + (display.camera_size.w  / 2)) * PIXELS_PER_METER);
                        i32 cam_max_y = roundf((display.camera_pos.y + (display.camera_size.h  / 2)) * PIXELS_PER_METER);

                        if(tile_x + TILE_SIZE + TILE_SIZE >= cam_min_x || tile_x - TILE_SIZE < cam_max_x ||
                           tile_y + TILE_SIZE + TILE_SIZE >= cam_min_y || tile_y - TILE_SIZE < cam_max_y) {

                            Vec2 render_pos = {(f32)tile_x / (f32)display.pixels_per_meter, (f32)tile_y / (f32)display.pixels_per_meter};
                            u16 tile_id = (u16)((map.tile_data[y * map.width + x] >> 16) & 0x0000FFFF);
                            Tile* tile = &tiles[tile_id];
                            
                            RenderSubTexture(&display, &render_pos, &texture_sheet, tile->sub_texture_data.x, tile->sub_texture_data.y,
                                             tile->sub_texture_data.width, tile->sub_texture_data.height);
                        }
                    }
                }

                // Render Player
                i32 pstw = player.walk_anim_tex_data.width;
                i32 psth = player.walk_anim_tex_data.height;
                i32 pstx = player.walk_anim_tex_data.x + (player.direction * pstw);
                i32 psty = player.walk_anim_tex_data.y + (player.anim_frame * psth);
                if(player.anim_state == ANIM_STATE_WALK_ATTACK) {
                    pstw = player.walk_attack_anim_tex_data.width;
                    psth = player.walk_attack_anim_tex_data.height;
                    pstx = player.walk_attack_anim_tex_data.x + (player.direction * pstw);
                    psty = player.walk_attack_anim_tex_data.y + (player.anim_frame * psth);
                }
                else if(player.anim_state == ANIM_STATE_IDLE_ATTACK) {
                    pstw = player.idle_attack_anim_tex_data.width;
                    psth = player.idle_attack_anim_tex_data.height;
                    pstx = player.idle_attack_anim_tex_data.x + (player.direction * pstw);
                    psty = player.idle_attack_anim_tex_data.y + (player.anim_frame * psth);
                }

                Vec2 camera_relevent_pos = {player.pos.x - display.camera_pos.x, player.pos.y - display.camera_pos.y};
                RenderSubTexture(&display, &camera_relevent_pos, &texture_sheet, pstx, psty, pstw, psth);

                // Render Enemies
                for(i32 i = 0; i < enemy_list.num_enemies; i++) {
                    Enemy* enemy = &enemy_list.enemies[i];
                    if(!enemy->alive) {
                        continue;
                    }

                    i32 estw = enemy->walk_anim_tex_data.width;
                    i32 esth = enemy->walk_anim_tex_data.height;
                    i32 estx = enemy->walk_anim_tex_data.x + (enemy->direction * pstw);
                    i32 esty = enemy->walk_anim_tex_data.y + (enemy->anim_frame * psth);

                    if(enemy->anim_state == ANIM_STATE_WALK_ATTACK) {
                        estw = enemy->walk_attack_anim_tex_data.width;
                        esth = enemy->walk_attack_anim_tex_data.height;
                        estx = enemy->walk_attack_anim_tex_data.x + (enemy->direction * pstw);
                        esty = enemy->walk_attack_anim_tex_data.y + (enemy->anim_frame * psth);
                    }
                    else if(enemy->anim_state == ANIM_STATE_IDLE_ATTACK) {
                        estw = enemy->idle_attack_anim_tex_data.width;
                        esth = enemy->idle_attack_anim_tex_data.height;
                        estx = enemy->idle_attack_anim_tex_data.x + (enemy->direction * pstw);
                        esty = enemy->idle_attack_anim_tex_data.y + (enemy->anim_frame * psth);
                    }

                    Vec2 enemy_world_pos = {enemy->pos.x - display.camera_pos.x, 
                                            enemy->pos.y - display.camera_pos.y};
                    RenderSubTexture(&display, &enemy_world_pos, &texture_sheet, estx, esty, estw, esth);
                }

                // Render UI
                RenderFilledRect(&display, &hud_pos, &hud_size, hud_colour);
                
                std::stringstream health_display_output;
                health_display_output << "HP: " << player.health << "/" << PLAYER_HEALTH;
                RenderString(&display, &font, &health_display_pos, (char*)health_display_output.str().c_str(), HUD_TEXT_COLOUR);

                for(i32 i = 0; i < NUM_ITEM_BOXES; i++) {
                    UI_Item_Box* item_box = &item_boxes[i];

                    RenderFilledRect(&display, &item_box->pos, &item_box->size, ITEM_BOX_RECT_COLOUR);

                    Vec2 letter_pos = {item_box->pos.x - (item_box->size.w / 2) - 0.7, item_box->pos.y - 0.4};
                    char temp_str[2] = {item_box->item_use_char, '\0'};
                    RenderString(&display, &font, &letter_pos, temp_str, HUD_TEXT_COLOUR);
                    
                    switch(i) {
                        case ITEM_BOX_WEAPON: {
                            i32 icon_tex_y = item_box->icon_tex_data.y;
                            if(player.has_sword) {
                                icon_tex_y = item_box->icon_tex_data.y + 16;
                            }

                            RenderSubTexture(&display, &item_box->pos, &texture_sheet, item_box->icon_tex_data.x, 
                                            item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                            item_box->icon_tex_data.height);
                        } break;

                        case ITEM_BOX_SHIELD: {
                            if(player.has_shield) {
                                RenderSubTexture(&display, &item_box->pos, &texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);
                            }

                        } break;

                        case ITEM_BOX_GRAPPLE: {
                            if(player.has_grapple) {
                                RenderSubTexture(&display, &item_box->pos, &texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);
                            }
                        } break;

                        case ITEM_BOX_POTION: {
                            if(player.potions > 0) {
                                RenderSubTexture(&display, &item_box->pos, &texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);

                                Vec2 value_pos = {item_box->pos.x + (item_box->size.w / 2) + 0.2, item_box->pos.y - 0.3};
                                char temp_str[4];
                                itoa(player.potions, temp_str, 10);
                                RenderString(&display, &font, &value_pos, temp_str, HUD_TEXT_COLOUR);
                            }
                        } break;

                        case ITEM_BOX_BOMB: {
                            if(player.bombs > 0) {
                                RenderSubTexture(&display, &item_box->pos, &texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);

                                Vec2 value_pos = {item_box->pos.x + (item_box->size.w / 2) + 0.2, item_box->pos.y - 0.3};
                                char temp_str[4];
                                itoa(player.bombs, temp_str, 10);
                                RenderString(&display, &font, &value_pos, temp_str, HUD_TEXT_COLOUR);
                            }
                        } break;

                    }
                }

                // animate map
                if(animating_map) {
                    // speeding up
                    if(map_animation_step == 0) {
                        map_rot_speed += MAP_ROT_ANIM_SPEED_MODIFIER;
                        if(map_rot_speed >= MAX_MAP_ROT_SPEED) {
                           map_animation_step = 1; 
                        }

                        map_rot += map_rot_speed;
                    }
                    // slowing down
                    else if(map_animation_step == 1) {
                        map_rot_speed -= MAP_ROT_ANIM_SPEED_MODIFIER;
                        if(map_rot_speed <= 0) {
                            map_rot_speed = 0;
                            if(map_rot < 180) {
                                map_rot += MAP_ROT_ANIM_SPEED_MODIFIER;
                            }
                            else if(map_rot > 180) {
                                map_rot -= MAP_ROT_ANIM_SPEED_MODIFIER;
                            }
                            else if(map_rot == 180) {
                                map_flip = (SDL_RendererFlip)(SDL_FLIP_VERTICAL | SDL_FLIP_HORIZONTAL);
                                map_animation_step = 2;
                            }
                        }
                        else {
                            map_rot += map_rot_speed;
                        }
                    }
                    // flip and reload map
                    else if(map_animation_step == 2) {
                        SDL_Delay(500);
                        LoadEnemiesList(&enemy_list, &map);
                        player.pos = player_start_pos;
                        map_flip = SDL_FLIP_NONE;
                        map_rot = 0;
                        map_animation_step = 0;
                        animating_map = false;
                    }
                    
                    if(map_rot >= 360) {
                        map_rot = 0;
                    }
                }
            } break;

            case START_STATE: {
                while(SDL_PollEvent(&event)) {
                    switch(event.type) {
                        case SDL_QUIT: {
                            running = false;
                        } break;

                        case SDL_KEYUP: {
                            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                                game_state = PLAY_STATE;
                            }
                        } break;
                    }
                }

                SDL_RenderClear(display.renderer);
                for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
                    *((u32*)display.pixel_buffer.pixels + i) = 0xFF500050;
                }
                // Max 11 characters per line
                char* render_string1 = "Press SPACE to start game";
                char* render_string2 = "";
                Vec2 render_string1_pos = {0.2, 2};
                Vec2 render_string2_pos = {0.2, 3};
                RenderString(&display, &font, &render_string1_pos, render_string1, 0xFFEEEEEE);
                RenderString(&display, &font, &render_string2_pos, render_string2, 0xFFEEEEEE);
            } break;

            case DEATH_STATE: {

            } break;

            case WIN_STATE: {

            } break;
        }

        // Update Display
        SDL_UpdateTexture(display.texture, 0, display.pixel_buffer.pixels, display.pixel_buffer.pitch);
        SDL_RenderCopyEx(display.renderer, display.texture, 0, 0, map_rot, 0, map_flip);
        SDL_RenderPresent(display.renderer);

        // Handle Framerate crap
        i32 ms_after = SDL_GetTicks();
        i32 ms_this_frame = ms_after - ms_before;
#if CAP_FRAMERATE
        if(ms_this_frame < TARGET_MS_PER_FRAME) {
            i32 ms_to_wait = TARGET_MS_PER_FRAME - ms_this_frame;
            SDL_Delay(ms_to_wait);
            ms_this_frame += ms_to_wait;
        }
#endif 
        // Update the animation timers
        if(player.anim_state != ANIM_STATE_IDLE) {
            player.anim_timer -= ms_this_frame;
        }
        for(i32 i = 0; i < enemy_list.num_enemies; i++) {
            Enemy* enemy = &enemy_list.enemies[i];
            if(!enemy->alive) {
                continue;
            }

            if(enemy->anim_state != ANIM_STATE_IDLE) {
                enemy->anim_timer -= ms_this_frame;
            }
        }

        //printf("%d ms this frame\n", ms_this_frame);
        last_delta = ms_this_frame / 1000.0f;
    }

    // Freeing stuff is overrated

    return 0;
}