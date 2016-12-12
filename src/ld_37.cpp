
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
    Enemy* enemies;
    i32 enemies_size;
    i32 num_enemies;
    i32 num_alive_enemies;
} Enemy_List;

typedef struct {
    char item_use_char; // The character corrospending to a key that uses this item
    Sub_Texture_Data icon_tex_data;
    Vec2 pos;
    Vec2 size;
} UI_Item_Box;

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
    Vec2 player_start_pos;
} Map;


typedef struct {
    bool live;
    i32 time_left;
    i32 anim_frame;
    i32 anim_timer;
    Sub_Texture_Data tex_data;
    Vec2 pos;
    Vec2 size;
} Bomb;

typedef struct {
    i32 update_and_render_state;
    u8 current_map;
    Map map;
    Player player;
    Enemy_List enemy_list;
    Font font;
    Texture texture_sheet;
    Vec2 hud_pos;
    Vec2 hud_size;
    u32 hud_colour;
    Vec2 health_display_pos;
    UI_Item_Box* item_boxes;
    Bomb* bombs;
    i32 bombs_size;
    i32 num_bombs;
} Game_State;

const f32 PIXELS_PER_METER                  = 16.0;
const f32 DIAGONAL_MOVEMENT_CONSTANT        = 0.70711;
const f32 MAP_ROT_ANIM_SPEED_MODIFIER       = 1;
const f32 MAX_MAP_ROT_SPEED                 = 53;
#define ITEM_BOX_RECT_COLOUR                0xFF696969
#define HUD_TEXT_COLOUR                     0xFFEEEEEE
#define POTION_HEAL_VALUE                   5
#define DISPLAY_SCALE                       3.5
#define WINDOW_FLAGS                        SDL_WINDOW_SHOWN /* | SDL_WINDOW_MAXIMIZED  */
#define WINDOW_WIDTH                        1280
#define WINDOW_HEIGHT                       (WINDOW_WIDTH * 9 / 16)
#define TARGET_MS_PER_FRAME                 16
#define CAP_FRAMERATE                       1
#define PLAYER_COLLISION_ON                 1
#define UPDATE_ENEMIES                      0

#define UP                                  0
#define DOWN                                1
#define LEFT                                2
#define RIGHT                               3

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

#define CHEST_TILE_CONTENTS_POTION          0
#define CHEST_TILE_CONTENTS_BOMB            1

#define ACTION_STATE_IDLE                   0
#define ACTION_STATE_WALK                   1
#define ACTION_STATE_ATTACK                 2

#define PLAYER_HITBOX_WIDTH                 (13.0 / PIXELS_PER_METER)
#define PLAYER_HITBOX_HEIGHT                (13.0 / PIXELS_PER_METER)
#define PLAYER_TEXTURE_SIZE                 16
#define PLAYER_WALK_ANIMATION_TIME          300
#define PLAYER_ATTACK_ANIMATION_TIME        300
#define PLAYER_SPEED                        4.2
#define PLAYER_HEALTH                       20
#define PLAYER_DAMAGE                       2
#define PLAYER_PUNCH_KNOCKBACK_SPEED        0.30

#define ENEMIES_LIST_SIZE                   100
#define DEMON_HITBOX_WIDTH                  (13.0 / PIXELS_PER_METER)
#define DEMON_HITBOX_HEIGHT                 (13.0 / PIXELS_PER_METER)
#define DEMON_TEXTURE_SIZE                  16
#define DEMON_AI_STATE_WANDER               0
#define DEMON_AI_STATE_CHASE                1
#define DEMON_AI_STATE_ATTACK               2
#define DEMON_WALK_ANIMATION_TIME           200
#define DEMON_ATTACK_ANIMATION_TIME_FRAME_0 100
#define DEMON_ATTACK_ANIMATION_TIME_FRAME_1 400
#define DEMON_CHASE_SPEED                   3.3
#define DEMON_WANDER_SPEED                  3.1
#define DEMON_HEALTH                        10
#define DEMON_DAMAGE                        2
#define DEMON_ATTACK_KNOCKBACK_SPEED        0.4

#define WRAITH_HITBOX_WIDTH                  (13.0 / PIXELS_PER_METER)
#define WRAITH_HITBOX_HEIGHT                 (13.0 / PIXELS_PER_METER)
#define WRAITH_TEXTURE_SIZE                  16
#define WRAITH_AI_STATE_WANDER               0
#define WRAITH_AI_STATE_CHASE                1
#define WRAITH_AI_STATE_ATTACK               2
#define WRAITH_WALK_ANIMATION_TIME           200
#define WRAITH_ATTACK_ANIMATION_TIME_FRAME_0 100
#define WRAITH_ATTACK_ANIMATION_TIME_FRAME_1 400
#define WRAITH_CHASE_SPEED                   3.3
#define WRAITH_WANDER_SPEED                  3.1
#define WRAITH_HEALTH                        20
#define WRAITH_DAMAGE                        3
#define WRAITH_ATTACK_KNOCKBACK_SPEED        0.4

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

#define PLAY_STATE                          0
#define START_STATE                         1
#define DEATH_STATE                         2
#define WIN_STATE                           3

#define ATTACK_KEY                          SDL_SCANCODE_Z
#define USE_POTION_KEY                      SDL_SCANCODE_Q
#define USE_BOMB_KEY                        SDL_SCANCODE_D
#define USE_GRAPPLE_KEY                     SDL_SCANCODE_S
#define USE_SHEILD_KEY                      SDL_SCANCODE_X

#define ITEM_BOX_WEAPON                     0
#define ITEM_BOX_SHIELD                     1
#define ITEM_BOX_GRAPPLE                    2
#define ITEM_BOX_POTION                     3
#define ITEM_BOX_BOMB                       4

#define MAP_LEVEL_1                         0
#define MAP_LEVEL_2                         1
#define MAP_LEVEL_3                         2

#define MAX_NUM_BOMBS                       3
#define BOMB_TIME                           3000
#define BOMB_ANIM_TIME                      200
#define BOMB_FRAME_1_X                      128
#define BOMB_FRAME_1_Y                      80
#define BOMB_TEXTURE_SIZE                   16
#define BOMB_PLACEMENT_OFFSET               0.5
#define BOMB_HITBOX_WIDTH                   (8.0 / PIXELS_PER_METER)
#define BOMB_HITBOX_HEIGHT                  (11.0 / PIXELS_PER_METER)

Tile tile_set[13] = {
    {{0, 0,  TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false},    // blank
    {{16, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},     // brick
    {{32, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},     // breakable brick
    {{48, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false},    // dirt
    {{64, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false},    // floor button unactivated
    {{80, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, false},    // floor button activated
    {{96, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},     // barricade virtical
    {{112, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},    // barricade horizontal
    {{128, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},    // gem
    {{144, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},    // chest unopened
    {{160, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},    // chest opended
    {{176, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true},    // void
    {{192, 0, TILE_SIZE, TILE_SIZE}, TILE_SIZE_IN_METERS, true}     // rock
};

void CreateDemon(Enemy_List* enemy_list, i32 x, i32 y) {
    if(!enemy_list->enemies) {
        enemy_list->enemies_size = ENEMIES_LIST_SIZE * sizeof(Enemy);
        enemy_list->enemies = (Enemy*)malloc(enemy_list->enemies_size);
        enemy_list->num_enemies = 0;
        enemy_list->num_alive_enemies = 0;
    }

    if(enemy_list->num_enemies == ENEMIES_LIST_SIZE) {
        printf("NEED TO RESIZE ENEMIES LIST\n");
        return;
    }

    Enemy* demon = &enemy_list->enemies[enemy_list->num_enemies];
    enemy_list->num_enemies++;
    enemy_list->num_alive_enemies++;

    demon->pos                          = {(f32)x / PIXELS_PER_METER, (f32)y / PIXELS_PER_METER};
    demon->size                         = {DEMON_HITBOX_WIDTH, DEMON_HITBOX_HEIGHT};
    demon->walk_anim_tex_data           = {0, 96, DEMON_TEXTURE_SIZE, DEMON_TEXTURE_SIZE};
    demon->walk_attack_anim_tex_data    = {64, 96, DEMON_TEXTURE_SIZE, DEMON_TEXTURE_SIZE};
    demon->direction                    = rand() % 4;
    demon->speed                        = DEMON_WANDER_SPEED;
    demon->chase_speed                  = DEMON_CHASE_SPEED;
    demon->ai_state                     = DEMON_AI_STATE_WANDER;
    demon->health                       = DEMON_HEALTH;
    demon->damage                       = DEMON_DAMAGE;
    demon->anim_frame                   = 0;
    demon->anim_timer                   = 0;
    demon->knockback_frames             = 0;
    demon->knockback_direction          = 0;
    demon->knockback_speed              = 0;
    demon->alive                        = true;
}

void CreateWraith(Enemy_List* enemy_list, i32 x, i32 y) {
    if(!enemy_list->enemies) {
        enemy_list->enemies_size = ENEMIES_LIST_SIZE * sizeof(Enemy);
        enemy_list->enemies = (Enemy*)malloc(enemy_list->enemies_size);
        enemy_list->num_enemies = 0;
        enemy_list->num_alive_enemies = 0;
    }

    if(enemy_list->num_enemies == ENEMIES_LIST_SIZE) {
        printf("NEED TO RESIZE ENEMIES LIST\n");
        return;
    }

    Enemy* wraith = &enemy_list->enemies[enemy_list->num_enemies];
    enemy_list->num_enemies++;
    enemy_list->num_alive_enemies++;

    wraith->pos                          = {(f32)x / PIXELS_PER_METER, (f32)y / PIXELS_PER_METER};
    wraith->size                         = {WRAITH_HITBOX_WIDTH, WRAITH_HITBOX_HEIGHT};
    wraith->walk_anim_tex_data           = {0, 128, WRAITH_TEXTURE_SIZE, WRAITH_TEXTURE_SIZE};
    wraith->walk_attack_anim_tex_data    = {64, 128, WRAITH_TEXTURE_SIZE, WRAITH_TEXTURE_SIZE};
    wraith->direction                    = rand() % 4;
    wraith->speed                        = WRAITH_WANDER_SPEED;
    wraith->chase_speed                  = WRAITH_CHASE_SPEED;
    wraith->ai_state                     = WRAITH_AI_STATE_WANDER;
    wraith->health                       = WRAITH_HEALTH;
    wraith->damage                       = WRAITH_DAMAGE;
    wraith->anim_frame                   = 0;
    wraith->anim_timer                   = 0;
    wraith->knockback_frames             = 0;
    wraith->knockback_direction          = 0;
    wraith->knockback_speed              = 0;
    wraith->alive                        = true;
}

void LoadTileDataFromTexture(Game_State* game_state, Texture* texture) {
    game_state->map.width = texture->width; 
    game_state->map.height = texture->height;
    game_state->map.tile_data = (u32*)malloc(game_state->map.width * game_state->map.height * sizeof(u32));

    if(!game_state->map.tile_data) {
        printf("Error - could not create map tile data.\n");
        return;
    }

    i16 object_id = 1;
    for(i32 y = 0; y < game_state->map.height; y++) {
        for(i32 x = 0; x < game_state->map.width; x++) {
            u32 pixel = *((u32*)texture->pixels + (y * texture->width) + x);

            switch(pixel) {
                case MAP_FILE_BRICK_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_BRICK << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_DIRT_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_START_POS_COLOUR: {
                    game_state->map.player_start_pos = {(f32)x * (f32)TILE_SIZE / PIXELS_PER_METER, (f32)y * (f32)TILE_SIZE / PIXELS_PER_METER};
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_BARRICATE_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_BARRICATE_VIRTICAL << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_FLOOR_BUTTON_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_BUTTON_UNACTIVATED << 16) | object_id;
                    object_id++;
                } break;

                case MAP_FILE_BREAKABLE_BRICK_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_BREAKABLE_BRICK << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_CHEST_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_CHEST_UNOPENED << 16) | object_id;
                    object_id++;
                } break;

                case MAP_FILE_GEM_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_GEM << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_VOID_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_VOID << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_ROCK_COLOUR: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_ROCK << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_SPAWN_DEMON_COLOUR: {
                    CreateDemon(&game_state->enemy_list, x * TILE_SIZE, y * TILE_SIZE);
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_SPAWN_WRAITH_COLOUR: {
                    CreateWraith(&game_state->enemy_list, x * TILE_SIZE, y * TILE_SIZE);
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_DIRT << 16) | TILE_NO_EXTRA_DATA;
                } break;

                case MAP_FILE_EMPTY_COLOUR:
                default: {
                    game_state->map.tile_data[(y * game_state->map.width) + x] = (TILE_BLANK << 16) | TILE_NO_EXTRA_DATA;
                } break;
            }
        }
    }
}

void LoadMap(Game_State* game_state, SDL_PixelFormat* pixel_format, u8 map_id) {
    if(game_state->map.tile_data) {
        free(game_state->map.tile_data);
    }

    game_state->map = {0};
    game_state->map.tiles = tile_set;

    game_state->current_map = map_id;
    switch(game_state->current_map) {
        case MAP_LEVEL_1: {
            Texture map_file_texture;
            LoadTexture("../res/textures/map1.png", &map_file_texture, pixel_format);
            LoadTileDataFromTexture(game_state, &map_file_texture);
            
            for(i32 y = 0; y < game_state->map.height; y++) {
                for(i32 x = 0; x < game_state->map.width; x++) {
                    u32 tile_data = game_state->map.tile_data[y * game_state->map.width + x];
                    u16 tile_id = (u16)((tile_data >> 16) & 0x0000FFFF);

                    switch(tile_id) {
                        case TILE_BARRICATE_VIRTICAL: {
                            // switch the door to a horizontal door if there are brick 
                            // tiles or other door tiles horizontally adjacent to them
                            u16 tile_adj_left_id = u16((game_state->map.tile_data[(y * game_state->map.width) + (x - 1)] >> 16) & 0x0000FFFF);
                            u16 tile_adj_right_id = u16((game_state->map.tile_data[(y * game_state->map.width) + (x + 1)] >> 16) & 0x0000FFFF);

                            if(tile_adj_left_id == TILE_BRICK || tile_adj_left_id == TILE_BARRICATE_VIRTICAL || 
                               tile_adj_left_id == TILE_BARRICATE_HORIZONTAL || tile_adj_right_id == TILE_BRICK || 
                               tile_adj_right_id == TILE_BARRICATE_VIRTICAL ||  tile_adj_right_id == TILE_BARRICATE_HORIZONTAL) {
                                game_state->map.tile_data[y * game_state->map.width + x] = (TILE_BARRICATE_HORIZONTAL << 16) | TILE_NO_EXTRA_DATA;
                            }
                        } break;

                        // set the tile extra data to the index of the first barricade tile the button opens.
                        case TILE_BUTTON_UNACTIVATED: {
                            u16 object_id = (u16)(tile_data & 0x0000FFFF);
                            // Opens the barricade at (20, 9).
                            if(object_id == 1) {
                                game_state->map.tile_data[y * game_state->map.width + x] = (tile_id << 16) | (u16)263;
                            }
                            // Opens the barricade at (6, 9).
                            else if(object_id == 3) {
                                game_state->map.tile_data[y * game_state->map.width + x] = (tile_id << 16) | (u16)249;
                            }
                            // Opens the barricade at (22, 4).
                            else if(object_id == 5) {
                                game_state->map.tile_data[y * game_state->map.width + x] = (tile_id << 16) | (u16)130;
                            }
                        } break;

                        // set the tile extra data to what the chest contains
                        case TILE_CHEST_UNOPENED: {
                            u16 object_id = (u16)(tile_data & 0x0000FFFF);
                            // This chest contains 3 bombs.
                            if(object_id == 2) {
                                game_state->map.tile_data[y * game_state->map.width + x] = (tile_id << 16) | (u16)((CHEST_TILE_CONTENTS_BOMB << 8) | (u8)3);
                            }
                            // This chest contains 3 potions.
                            if(object_id == 4) {
                                game_state->map.tile_data[y * game_state->map.width + x] = (tile_id << 16) | (u16)((CHEST_TILE_CONTENTS_POTION << 8) | (u8)3);
                            }
                        } break;
                    }
                }
            }

            FreeTexture(&map_file_texture);
        } break;

        case MAP_LEVEL_2: {
            Texture map_file_texture;
            LoadTexture("../res/textures/map2.png", &map_file_texture, pixel_format);
            LoadTileDataFromTexture(game_state, &map_file_texture);

            FreeTexture(&map_file_texture);
        } break;

        case MAP_LEVEL_3: {
            Texture map_file_texture;
            LoadTexture("../res/textures/map3.png", &map_file_texture, pixel_format);
            LoadTileDataFromTexture(game_state, &map_file_texture);

            FreeTexture(&map_file_texture);
        } break;

        default: {
            printf("Unkwon Map Id: %d\n", map_id);
        } break;
    }
}

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

i32 PlayerCollidingWithChestTile(Player* player, Map* map) {
    // TOP
    i16 tile_x = (i16)roundf((((player->pos.x - (player->size.w / 2)) * PIXELS_PER_METER) - 2) / TILE_SIZE);
    i16 tile_y = (i16)roundf((((player->pos.y - (player->size.h / 2)) * PIXELS_PER_METER) - 2) / TILE_SIZE);
    u8 tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_CHEST_UNOPENED) {
        return (tile_x << 16) | tile_y;
    } 

    // BOTTON
    tile_x = roundf((((player->pos.x + (player->size.w / 2)) * PIXELS_PER_METER) - 2) / TILE_SIZE);
    tile_y = roundf((((player->pos.y - (player->size.h / 2)) * PIXELS_PER_METER) - 2) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_CHEST_UNOPENED) {
        return (tile_x << 16) | tile_y;
    } 

    // LEFT
    tile_x = roundf((((player->pos.x - (player->size.w / 2)) * PIXELS_PER_METER) + 2) / TILE_SIZE);
    tile_y = roundf((((player->pos.y + (player->size.h / 2)) * PIXELS_PER_METER) + 2) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_CHEST_UNOPENED) {
        return (tile_x << 16) | tile_y;
    } 

    // RIGHT
    tile_x = roundf((((player->pos.x + (player->size.w / 2)) * PIXELS_PER_METER) + 2) / TILE_SIZE);
    tile_y = roundf((((player->pos.y + (player->size.h / 2)) * PIXELS_PER_METER) + 2) / TILE_SIZE);
    tile_id = (u8)((map->tile_data[tile_y * map->width + tile_x] >> 16) & 0x0000FFFFF);
    if(tile_id == TILE_CHEST_UNOPENED) {
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

void ReplaceAllSameAdjacentTilesRecursivly(Map* map, i32 x, i32 y, u16 tile_to_replace, u16 replacement_tile) {
    u16 tile_left_adj_id =   (u16)((map->tile_data[y * map->width + x - 1] >> 16) & 0x0000FFFF);
    u16 tile_right_adj_id =  (u16)((map->tile_data[y * map->width + x + 1] >> 16) & 0x0000FFFF);
    u16 tile_top_adj_id =    (u16)((map->tile_data[(y - 1) * map->width + x] >> 16) & 0x0000FFFF);
    u16 tile_bottom_adj_id = (u16)((map->tile_data[(y + 1) * map->width + x] >> 16) & 0x0000FFFF);

    // If we find a tile to the left replace it then branch off that tile
    if(tile_left_adj_id == tile_to_replace) {
        map->tile_data[y * map->width + x - 1] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTilesRecursivly(map, x - 1, y, tile_to_replace, replacement_tile);
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_right_adj_id == tile_to_replace) {
        map->tile_data[y * map->width + x + 1] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTilesRecursivly(map, x + 1, y, tile_to_replace, replacement_tile);
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_top_adj_id == tile_to_replace) {
        map->tile_data[(y - 1) * map->width + x] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTilesRecursivly(map, x, y - 1, tile_to_replace, replacement_tile);
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_bottom_adj_id == tile_to_replace) {
        map->tile_data[(y + 1) * map->width + x] = (replacement_tile << 16) | 0x0000FFFF;
        ReplaceAllSameAdjacentTilesRecursivly(map, x, y + 1, tile_to_replace, replacement_tile);
    }
}

void ReplaceAllSameAdjacentTiles(Map* map, i32 x, i32 y, u16 tile_to_replace, u16 replacement_tile) {
    u16 tile_left_adj_id =   (u16)((map->tile_data[y * map->width + x - 1] >> 16) & 0x0000FFFF);
    u16 tile_right_adj_id =  (u16)((map->tile_data[y * map->width + x + 1] >> 16) & 0x0000FFFF);
    u16 tile_top_adj_id =    (u16)((map->tile_data[(y - 1) * map->width + x] >> 16) & 0x0000FFFF);
    u16 tile_bottom_adj_id = (u16)((map->tile_data[(y + 1) * map->width + x] >> 16) & 0x0000FFFF);

    // If we find a tile to the left replace it then branch off that tile
    if(tile_left_adj_id == tile_to_replace) {
        map->tile_data[y * map->width + x - 1] = (replacement_tile << 16) | TILE_NO_EXTRA_DATA;
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_right_adj_id == tile_to_replace) {
        map->tile_data[y * map->width + x + 1] = (replacement_tile << 16) | TILE_NO_EXTRA_DATA;
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_top_adj_id == tile_to_replace) {
        map->tile_data[(y - 1) * map->width + x] = (replacement_tile << 16) | TILE_NO_EXTRA_DATA;
    }

    // If we find a tile to the left replace it then branch off that tile
    if(tile_bottom_adj_id == tile_to_replace) {
        map->tile_data[(y + 1) * map->width + x] = (replacement_tile << 16) | TILE_NO_EXTRA_DATA;
    }
}

void ClearEnemyList(Enemy_List* enemy_list) {
    free(enemy_list->enemies);
    enemy_list->enemies = 0;
    enemy_list->enemies_size = 0;
    enemy_list->num_enemies = 0;
}

void InitPlayState(Game_State* game_state, Display* display) {
    // Player
    game_state->player.pos                          = game_state->map.player_start_pos;
    game_state->player.size                         = {PLAYER_HITBOX_WIDTH, PLAYER_HITBOX_HEIGHT};
    game_state->player.walk_anim_tex_data           = {0, 32, PLAYER_TEXTURE_SIZE, PLAYER_TEXTURE_SIZE};
    game_state->player.walk_attack_anim_tex_data    = {64, 32, PLAYER_TEXTURE_SIZE, PLAYER_TEXTURE_SIZE};
    game_state->player.direction                    = 1;
    game_state->player.speed                        = 4.2;
    game_state->player.anim_state                   = ACTION_STATE_IDLE;
    game_state->player.anim_timer                   = 0;
    game_state->player.anim_frame                   = 0;
    game_state->player.knockback_frames             = 0;
    game_state->player.knockback_direction          = 0;
    game_state->player.health                       = PLAYER_HEALTH;
    game_state->player.damage                       = PLAYER_DAMAGE;
    game_state->player.knockback_speed              = 0;
    game_state->player.has_sword                    = false;
    game_state->player.has_shield                   = false;
    game_state->player.has_grapple                  = false;
    game_state->player.potions                      = 3;
    game_state->player.bombs                        = 0;

    // UI
    game_state->hud_size = {(f32)display->pixel_buffer.width / PIXELS_PER_METER, display->pixel_buffer.height / 3.5 / PIXELS_PER_METER};
    game_state->hud_pos = {display->pixel_buffer.width / 2.0 / PIXELS_PER_METER, 
                           display->pixel_buffer.height / PIXELS_PER_METER - (game_state->hud_size.h / 2)};
    game_state->hud_colour = 0xFF444444;
    game_state->health_display_pos = {0.3, game_state->hud_pos.y - 0.4};
}

i32 main(i32 argc, char** argv) {
    srand(time(0));
    if(InitSDL() < 0) {
        return -1;
    }

    Display display = {0};
    Game_State game_state = {0};

    // Initalize display
    display.width = WINDOW_WIDTH;
    display.height = WINDOW_HEIGHT;
    display.title = "LD 37";
    display.pixels_per_meter = PIXELS_PER_METER;

    display.window = SDL_CreateWindow(display.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, display.width, 
                                      display.height, WINDOW_FLAGS);
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

    display.pixel_buffer.width = display.width / DISPLAY_SCALE;
    display.pixel_buffer.height = display.height / DISPLAY_SCALE;
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

    // Load some game state stuff
    LoadTexture("../res/textures/texture_sheet.png", &game_state.texture_sheet, display.pixel_format);
    LoadFont(&game_state.font, display.pixel_format);
    game_state.update_and_render_state = START_STATE;

    display.camera_size = {(f32)display.pixel_buffer.width / PIXELS_PER_METER, 
                           (f32)display.pixel_buffer.height / PIXELS_PER_METER - game_state.hud_size.h};
    display.camera_pos = {game_state.player.pos.x - (display.camera_size.w / 2), game_state.player.pos.y - (display.camera_size.h / 2)};

    InitPlayState(&game_state, &display);

    const i32 NUM_ITEM_BOXES = 5;
    Vec2 inital_item_box_pos = {game_state.hud_pos.x + 0.2, game_state.hud_pos.y - 0.65};
    UI_Item_Box item_boxes[NUM_ITEM_BOXES] = {
        {'z', {240, 48, 16, 16}, inital_item_box_pos, {1.1, 1.1}},                                          // weapon item box
        {'x', {224, 64, 16, 16}, {inital_item_box_pos.x + 2.3, inital_item_box_pos.y}, {1.1, 1.1}},         // Sheild item box
        {'s', {224, 48, 16, 16}, {inital_item_box_pos.x + 4.6, inital_item_box_pos.y}, {1.1, 1.1}},         // grapple item box
        {'q', {224, 32, 16, 16}, {inital_item_box_pos.x, inital_item_box_pos.y + 1.3}, {1.1, 1.1}},         // potion item box
        {'d', {240, 32, 16, 16}, {inital_item_box_pos.x + 4.6, inital_item_box_pos.y + 1.3}, {1.1, 1.1}}    // bomb item box
    };
    game_state.item_boxes = item_boxes;

    //Load the SOUNDS
    Mix_Chunk* hit_sound = LoadWAV("../res/sounds/hit.wav");
    Mix_Chunk* death_sound = LoadWAV("../res/sounds/death.wav");
    Mix_Chunk* button_activation_sound = LoadWAV("../res/sounds/button_activation.wav");
    Mix_Chunk* use_potion_sound = LoadWAV("../res/sounds/use_potion.wav");
    Mix_Chunk* bomb_explosion_sound = LoadWAV("../res/sounds/bomb_explosion.wav");
    Mix_Chunk* place_bomb_sound = LoadWAV("../res/sounds/place_bomb.wav");
    Mix_Chunk* chest_open_sound = LoadWAV("../res/sounds/open_chest.wav");

    // Game Loop
    bool running = true;
    SDL_Event event = {0};
    f32 last_delta = 0.0f;
    bool animating_map = false;
    i32 map_animation_step = 0; // 0 for speed up 1 for speed down 2 for flip and change map
    f32 map_rot = 0;
    f32 map_rot_speed = 0;
    SDL_RendererFlip map_flip = SDL_FLIP_NONE;
    while(running) {
        i32 ms_before = SDL_GetTicks();

        switch(game_state.update_and_render_state) {
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
                            else if(event.key.keysym.scancode == USE_POTION_KEY && game_state.player.potions > 0 
                                    && game_state.player.health < PLAYER_HEALTH) {
                                game_state.player.health += POTION_HEAL_VALUE;
                                if(game_state.player.health > PLAYER_HEALTH) {
                                    game_state.player.health = PLAYER_HEALTH;
                                }
                                game_state.player.potions--;

                                Mix_PlayChannel(-1, use_potion_sound, 0);
                            }
                            else if(event.key.keysym.scancode == USE_BOMB_KEY && game_state.player.bombs > 0) {
                                if(game_state.num_bombs < MAX_NUM_BOMBS) {
                                    game_state.player.bombs--;
                                    Mix_PlayChannel(-1, place_bomb_sound, 0);

                                    if(!game_state.bombs) {
                                        game_state.bombs_size = sizeof(Bomb) * MAX_NUM_BOMBS;
                                        game_state.bombs = (Bomb*)malloc(game_state.bombs_size);
                                        game_state.num_bombs = 0;
                                        
                                        for(i32 i = 0; i < MAX_NUM_BOMBS; i++) {
                                            Bomb* bomb = &game_state.bombs[i];
                                            *bomb = {0};
                                        }
                                    }

                                    // Find the first dead bomb and replace it.
                                    Bomb* bomb = 0;

                                    for(i32 i = 0; i < MAX_NUM_BOMBS; ++i) {
                                        if(!game_state.bombs[i].live) {
                                            bomb = &game_state.bombs[i];
                                        }
                                    }

                                    if(!bomb) {
                                        // shouldn't reach this point
                                        printf("Cannot create bomb.\n");
                                    }
                                    bomb->live = true;
                                    bomb->time_left = BOMB_TIME;
                                    bomb->anim_frame = 0;
                                    bomb->anim_timer = BOMB_ANIM_TIME;
                                    bomb->tex_data = {BOMB_FRAME_1_X, BOMB_FRAME_1_Y, BOMB_TEXTURE_SIZE, BOMB_TEXTURE_SIZE};
                                    
                                    switch(game_state.player.direction) {
                                        case UP: {
                                            bomb->pos = {game_state.player.pos.x, game_state.player.pos.y - 0.5};
                                        } break;

                                        case DOWN: {
                                            bomb->pos = {game_state.player.pos.x, game_state.player.pos.y + 0.5};
                                        } break;

                                        case LEFT: {
                                            bomb->pos = {game_state.player.pos.x - 0.5, game_state.player.pos.y};
                                        } break;

                                        case RIGHT: {
                                            bomb->pos = {game_state.player.pos.x + 0.5, game_state.player.pos.y};
                                        } break;
                                    }
                                    bomb->size = {BOMB_HITBOX_WIDTH, BOMB_HITBOX_HEIGHT};
                                    game_state.num_bombs++;
                                }
                            }
                            else if(event.key.keysym.scancode == USE_SHEILD_KEY) {
                                //DoSheildStuff();
                            }
                            else if(event.key.keysym.scancode == USE_GRAPPLE_KEY) {
                                //DoGrappleStuff();
                            }
                        } break;
                    }
                }

                // Update Enemies
                if(!animating_map) {
                    for(i32 i = 0; i < game_state.enemy_list.num_enemies; i++) {
                        Enemy* enemy = &game_state.enemy_list.enemies[i];
                        if(!enemy->alive) {
                            continue;
                        }

                        bool moved = false;

                        // Handle AI 
                        switch(enemy->ai_state) {
                            case DEMON_AI_STATE_WANDER: {
                                if(game_state.player.pos.x >= enemy->pos.x - 3.5 && game_state.player.pos.x <= enemy->pos.x + 3.5 &&
                                    game_state.player.pos.y >= enemy->pos.y - 3.5 && game_state.player.pos.y <= enemy->pos.y + 3.5) {
                                    
                                    enemy->ai_state = DEMON_AI_STATE_CHASE;
                                    enemy->anim_state = ACTION_STATE_WALK;
                                    enemy->anim_frame = 0;
                                    enemy->anim_timer = DEMON_WALK_ANIMATION_TIME;
                                    break;
                                }

                                if((game_state.player.pos.x >= enemy->pos.x - 0.5 && game_state.player.pos.x <= enemy->pos.x + 0.5) &&
                                    (game_state.player.pos.y >= enemy->pos.y - 0.5 && game_state.player.pos.y <= enemy->pos.y + 0.5)) {
                                    
                                    enemy->ai_state = DEMON_AI_STATE_ATTACK;
                                    enemy->anim_state = ACTION_STATE_ATTACK;
                                    enemy->anim_timer = DEMON_ATTACK_ANIMATION_TIME_FRAME_0;
                                    enemy->anim_frame = 0;
                                    break;
                                }

                                if(enemy->anim_state == ACTION_STATE_WALK) {
                                    bool reached_destination = false;
                                    Entity temp_enemy = {enemy->pos, enemy->size};
                                    switch(enemy->direction) {
                                        case UP: {
                                            temp_enemy.pos.y -= enemy->speed * last_delta;
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
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
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
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
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
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
                                            if(EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
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
                                        enemy->anim_state = ACTION_STATE_IDLE;
                                        enemy->anim_timer = 0;
                                    }
                                }
                                else {
                                    if(rand() % 333 < 10) {
                                        enemy->wander_destination = enemy->pos;
                                        enemy->direction = rand() % 4;
                                        enemy->anim_state = ACTION_STATE_WALK;
                                        enemy->anim_timer = DEMON_WALK_ANIMATION_TIME;

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

                            case DEMON_AI_STATE_CHASE: {
                                if((game_state.player.pos.x >= enemy->pos.x - 0.5 && game_state.player.pos.x <= enemy->pos.x + 0.5) &&
                                    (game_state.player.pos.y >= enemy->pos.y - 0.5 && game_state.player.pos.y <= enemy->pos.y + 0.5)) {
                                    
                                    enemy->ai_state = DEMON_AI_STATE_ATTACK;
                                    enemy->anim_state = ACTION_STATE_ATTACK;
                                    enemy->anim_timer = DEMON_ATTACK_ANIMATION_TIME_FRAME_0;
                                    enemy->anim_frame = 0;
                                    break;
                                }

                                Entity temp_enemy = {enemy->pos, enemy->size};
                                if(game_state.player.pos.y < enemy->pos.y) {
                                    enemy->direction = UP;
                                    temp_enemy.pos.y -= enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
                                        enemy->pos.y = temp_enemy.pos.y;
                                    }
                                }
                                else if(game_state.player.pos.y > enemy->pos.y) {
                                    enemy->direction = DOWN;
                                    temp_enemy.pos.y += enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
                                        enemy->pos.y = temp_enemy.pos.y;
                                    }
                                }

                                if(game_state.player.pos.x < enemy->pos.x) {
                                    enemy->direction = LEFT;
                                    temp_enemy.pos.x -= enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
                                        enemy->pos.x = temp_enemy.pos.x;
                                    }
                                }
                                else if(game_state.player.pos.x > enemy->pos.x) {
                                    enemy->direction = RIGHT;
                                    temp_enemy.pos.x += enemy->chase_speed * last_delta;
                                    if(!EntityCollidingWithSolidTile(&temp_enemy, &game_state.map, &display)) {
                                        enemy->pos.x = temp_enemy.pos.x;
                                    }
                                }
                            } break;

                            case DEMON_AI_STATE_ATTACK: {
                                if(enemy->anim_timer <= 0 && enemy->anim_frame == 0) {
                                    
                                    enemy->anim_timer = DEMON_ATTACK_ANIMATION_TIME_FRAME_1;
                                    enemy->anim_frame = 1;

                                    Entity enemy_entity = {enemy->pos, enemy->size};
                                    Entity player_entity = {game_state.player.pos, game_state.player.size};

                                    if(EntityCollidingWithEntity(&enemy_entity, &player_entity)) {
                                        Mix_PlayChannel(-1, hit_sound, 0);

                                        game_state.player.health -= enemy->damage;

                                        game_state.player.knockback_frames = 6;
                                        game_state.player.knockback_direction = enemy->direction;
                                        game_state.player.knockback_speed = DEMON_ATTACK_KNOCKBACK_SPEED;

                                        if(game_state.player.health <= 0) {
                                            Mix_PlayChannel(-1, death_sound, 0);
                                            game_state.update_and_render_state = DEATH_STATE;
                                        }
                                    }
                                }
                                else if(enemy->anim_timer <= 0 && enemy->anim_frame == 1) {
                                    enemy->ai_state = DEMON_AI_STATE_WANDER;
                                    enemy->anim_frame = 0;
                                    enemy->anim_timer = DEMON_WALK_ANIMATION_TIME;
                                }
                            } break;

                            default: {
                                printf("Unknown AI state %d\n", enemy->ai_state);
                            } break;
                        }

                        // Update walk animation
                        if(enemy->anim_state == ACTION_STATE_WALK && enemy->anim_timer <= 0) {
                            enemy->anim_frame = (enemy->anim_frame == 0) ? 1 : 0;
                            enemy->anim_timer = DEMON_WALK_ANIMATION_TIME;
                        }

                        // Knock enemy back
                        if(enemy->knockback_frames > 0) {
                            enemy->knockback_frames--;
                            Entity enemy_entity = {enemy->pos, enemy->size};

                            switch(enemy->knockback_direction) {
                                case UP: {
                                    enemy_entity.pos.y -= enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &game_state.map, &display)) {
                                        enemy->pos.y = enemy_entity.pos.y;
                                    }
                                } break;

                                case DOWN: {
                                    enemy_entity.pos.y += enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &game_state.map, &display)) {
                                        enemy->pos.y = enemy_entity.pos.y;
                                    }
                                } break;

                                case LEFT: {
                                    enemy_entity.pos.x -= enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &game_state.map, &display)) {
                                        enemy->pos.x = enemy_entity.pos.x;
                                    }
                                } break;

                                case RIGHT: {
                                    enemy_entity.pos.x += enemy->knockback_speed;
                                    
                                    if(!EntityCollidingWithSolidTile(&enemy_entity, &game_state.map, &display)) {
                                        enemy->pos.x = enemy_entity.pos.x;
                                    }
                                } break;
                            }
                        }
                    }

                    if(game_state.enemy_list.num_alive_enemies == 0) {
                        game_state.update_and_render_state = WIN_STATE;
                    }

                    // Update Player
                    u8* keys_state = (u8*)SDL_GetKeyboardState(0);

                    bool moved = false;
                    Entity temp_player;
                    temp_player.size = game_state.player.size;
                    temp_player.pos = game_state.player.pos;

                    if(keys_state[SDL_SCANCODE_UP] && !keys_state[SDL_SCANCODE_DOWN]) {
                        if(game_state.player.knockback_frames == 0) {
                            game_state.player.direction = UP;
                            moved = true;
                            temp_player.pos.y -= game_state.player.speed * last_delta;

                            if(!EntityCollidingWithSolidTile(&temp_player, &game_state.map, &display)) {
                                game_state.player.pos.y = temp_player.pos.y;
                            }
                        }
                    }
                    else if(keys_state[SDL_SCANCODE_DOWN] && !keys_state[SDL_SCANCODE_UP]) {
                        if(game_state.player.knockback_frames == 0) {
                            game_state.player.direction = DOWN;
                            moved = true;
                            temp_player.pos.y += game_state.player.speed * last_delta;

                            if(!EntityCollidingWithSolidTile(&temp_player, &game_state.map, &display)) {
                                game_state.player.pos.y = temp_player.pos.y;
                            }
                        }
                    }

                    if(keys_state[SDL_SCANCODE_LEFT] && !keys_state[SDL_SCANCODE_RIGHT]) {
                        if(game_state.player.knockback_frames == 0) {
                            game_state.player.direction = LEFT;
                            moved = true;
                            temp_player.pos.x -= game_state.player.speed * last_delta;

                            if(!EntityCollidingWithSolidTile(&temp_player, &game_state.map, &display)) {
                                game_state.player.pos.x = temp_player.pos.x;
                            }
                        }
                    }
                    else if(keys_state[SDL_SCANCODE_RIGHT] && !keys_state[SDL_SCANCODE_LEFT]) {
                        if(game_state.player.knockback_frames == 0) {
                            game_state.player.direction = RIGHT;
                            moved = true;
                            temp_player.pos.x += game_state.player.speed * last_delta;

                            if(!EntityCollidingWithSolidTile(&temp_player, &game_state.map, &display)) {
                                game_state.player.pos.x = temp_player.pos.x;
                            }
                        }
                    }

                    // Knock player back
                    if(game_state.player.knockback_frames > 0) {
                        game_state.player.knockback_frames--;
                        Entity player_entity = {game_state.player.pos, game_state.player.size};

                        switch(game_state.player.knockback_direction) {
                            case UP: {
                                player_entity.pos.y -= game_state.player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &game_state.map, &display)) {
                                    game_state.player.pos.y = player_entity.pos.y;
                                }
                            } break;

                            case DOWN: {
                                player_entity.pos.y += game_state.player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &game_state.map, &display)) {
                                    game_state.player.pos.y = player_entity.pos.y;
                                }
                            } break;

                            case LEFT: {
                                player_entity.pos.x -= game_state.player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &game_state.map, &display)) {
                                    game_state.player.pos.x = player_entity.pos.x;
                                }
                            } break;

                            case RIGHT: {
                                player_entity.pos.x += game_state.player.knockback_speed;
                                
                                if(!EntityCollidingWithSolidTile(&player_entity, &game_state.map, &display)) {
                                    game_state.player.pos.x = player_entity.pos.x;
                                }
                            } break;
                        }
                    }

                    // Player Activation
                    i32 tile_pos_data;
                    if((tile_pos_data = PlayerOnFloorButton(&game_state.player, &game_state.map)) != -1) {
                        i32 x = (tile_pos_data >> 16) & 0x0000FFFF;
                        i32 y = tile_pos_data & 0x0000FFFF;
                        u32 tile_data = game_state.map.tile_data[y * game_state.map.width + x];
                        u16 tile_id = (u16)((tile_data >> 16) & 0x0000FFFF);
                        u16 tile_extra_data = (u16)(tile_data & 0x0000FFFF);

                        switch(tile_id) {
                            case TILE_BUTTON_UNACTIVATED: {
                                Mix_PlayChannel(-1, button_activation_sound, 0);
                                i32 baricade_tile_x = tile_extra_data % game_state.map.width;
                                i32 baricade_tile_y = tile_extra_data / game_state.map.width;
                                ReplaceAllSameAdjacentTilesRecursivly(&game_state.map, baricade_tile_x, baricade_tile_y, TILE_BARRICATE_VIRTICAL, TILE_DIRT);
                                ReplaceAllSameAdjacentTilesRecursivly(&game_state.map, baricade_tile_x, baricade_tile_y, TILE_BARRICATE_HORIZONTAL, TILE_DIRT);
                                game_state.map.tile_data[y * game_state.map.width + x] = (TILE_BUTTON_ACTIVATED << 16) | TILE_NO_EXTRA_DATA;
                            } break;
                        }
                    }

                    if((tile_pos_data = PlayerCollidingWithChestTile(&game_state.player, &game_state.map)) != -1) {
                        i32 x = (tile_pos_data >> 16) & 0x0000FFFF;
                        i32 y = tile_pos_data & 0x0000FFFF;
                        u32 tile_data = game_state.map.tile_data[y * game_state.map.width + x];
                        u16 tile_id = (u16)((tile_data >> 16) & 0x0000FFFF);
                        u16 tile_extra_data = (u16)(tile_data & 0x0000FFFF);

                        if(tile_id == TILE_CHEST_UNOPENED) {
                            Mix_PlayChannel(-1, chest_open_sound, 0);
                            u8 item_id = (u8)((tile_extra_data >> 8) & 0x00FF);
                            u8 item_quantity = (u8)(tile_extra_data & 0x00FF);
                            switch(item_id) {
                                case CHEST_TILE_CONTENTS_POTION: {
                                    game_state.player.potions += (i32)item_quantity;
                                } break;

                                case CHEST_TILE_CONTENTS_BOMB: {
                                    game_state.player.bombs += (i32)item_quantity;
                                }
                            }

                            game_state.map.tile_data[y * game_state.map.width + x] = (TILE_CHEST_OPENED << 16) | TILE_NO_EXTRA_DATA;
                        }
                        else {
                            printf("Collided with chest but the returned id was not the chest.\n");
                        }
                    }

                    // Player Attack
                    if(attacked) {
                        attacked = false;
                        if(game_state.player.anim_state == ACTION_STATE_ATTACK || game_state.player.anim_state == ACTION_STATE_ATTACK) {
                            game_state.player.anim_frame = (game_state.player.anim_frame == 0) ? 1 : 0;
                        }
                        else if(game_state.player.anim_state == ACTION_STATE_IDLE) {
                            game_state.player.anim_state = ACTION_STATE_ATTACK;
                        }
                        else if(game_state.player.anim_state == ACTION_STATE_WALK) {
                            game_state.player.anim_state = ACTION_STATE_ATTACK;
                        }

                        game_state.player.anim_timer = PLAYER_ATTACK_ANIMATION_TIME;

                        for(i32 i = 0; i < game_state.enemy_list.num_enemies; i++) {
                            Enemy* enemy = &game_state.enemy_list.enemies[i];
                            if(!enemy->alive) {
                                continue;
                            }

                            Entity enemy_entity = {enemy->pos, enemy->size};
                            Entity player_entity = {game_state.player.pos, game_state.player.size};

                            if(EntityCollidingWithEntity(&player_entity, &enemy_entity)) {
                                Mix_PlayChannel(-1, hit_sound, 0);

                                enemy->health -= game_state.player.damage;
                                enemy->knockback_frames = 6;
                                enemy->knockback_direction = game_state.player.direction;
                                enemy->knockback_speed = PLAYER_PUNCH_KNOCKBACK_SPEED;

                                if(enemy->health <= 0) {
                                    Mix_PlayChannel(-1, death_sound, 0);
                                    enemy->alive = false;
                                    game_state.enemy_list.num_alive_enemies--;
                                }
                            }
                        }
                    }

                    if(!moved && game_state.player.anim_state == ACTION_STATE_WALK) {
                        game_state.player.anim_state = ACTION_STATE_IDLE;
                    }
                    else if(!moved && game_state.player.anim_state == ACTION_STATE_ATTACK) {
                        game_state.player.anim_state = ACTION_STATE_ATTACK;
                    }
                    else if(moved && game_state.player.anim_state == ACTION_STATE_ATTACK) {
                        game_state.player.anim_state = ACTION_STATE_ATTACK;
                    }
                    else if(moved && game_state.player.anim_state == ACTION_STATE_IDLE) {
                        game_state.player.anim_state = ACTION_STATE_WALK;
                        // Start at 0 so the we advance the frames as soon as we start moving.
                        game_state.player.anim_timer = 0;
                    }

                    if(game_state.player.anim_state != ACTION_STATE_IDLE && game_state.player.anim_timer <= 0) {
                        game_state.player.anim_frame = (game_state.player.anim_frame == 0) ? 1 : 0;
                        switch(game_state.player.anim_state) {
                            case ACTION_STATE_WALK: {
                                game_state.player.anim_timer = PLAYER_WALK_ANIMATION_TIME;
                            } break;

                            case ACTION_STATE_ATTACK: {
                                game_state.player.anim_timer = 0;
                                game_state.player.anim_state = ACTION_STATE_WALK;
                            }
                        }
                    }

                    display.camera_pos = {game_state.player.pos.x - (display.camera_size.w / 2), game_state.player.pos.y - (display.camera_size.h / 2)};
                }

                // Clear renderer
                SDL_RenderClear(display.renderer);
                for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
                    *((u32*)display.pixel_buffer.pixels + i) = 0xFF00FFFF;
                }

                // Render map
                for(i32 y = 0; y < game_state.map.height; y++) {
                    for(i32 x = 0; x < game_state.map.width; x++) {
                        i32 tile_x = x * TILE_SIZE - roundf(display.camera_pos.x * PIXELS_PER_METER);
                        i32 tile_y = y * TILE_SIZE - roundf(display.camera_pos.y * PIXELS_PER_METER);
                        i32 cam_min_x = roundf((display.camera_pos.x - (display.camera_size.w  / 2)) * PIXELS_PER_METER);
                        i32 cam_min_y = roundf((display.camera_pos.y - (display.camera_size.h  / 2)) * PIXELS_PER_METER);
                        i32 cam_max_x = roundf((display.camera_pos.x + (display.camera_size.w  / 2)) * PIXELS_PER_METER);
                        i32 cam_max_y = roundf((display.camera_pos.y + (display.camera_size.h  / 2)) * PIXELS_PER_METER);

                        if(tile_x + TILE_SIZE + TILE_SIZE >= cam_min_x || tile_x - TILE_SIZE < cam_max_x ||
                           tile_y + TILE_SIZE + TILE_SIZE >= cam_min_y || tile_y - TILE_SIZE < cam_max_y) {

                            Vec2 render_pos = {(f32)tile_x / (f32)display.pixels_per_meter, (f32)tile_y / (f32)display.pixels_per_meter};
                            u16 tile_id = (u16)((game_state.map.tile_data[y * game_state.map.width + x] >> 16) & 0x0000FFFF);
                            Tile* tile = &game_state.map.tiles[tile_id];
                            
                            RenderSubTexture(&display, &render_pos, &game_state.texture_sheet, tile->sub_texture_data.x, tile->sub_texture_data.y,
                                             tile->sub_texture_data.width, tile->sub_texture_data.height);
                        }
                    }
                }

                // Render Player
                i32 pstw = game_state.player.walk_anim_tex_data.width;
                i32 psth = game_state.player.walk_anim_tex_data.height;
                i32 pstx = game_state.player.walk_anim_tex_data.x + (game_state.player.direction * pstw);
                i32 psty = game_state.player.walk_anim_tex_data.y + (game_state.player.anim_frame * psth);
                if(game_state.player.anim_state == ACTION_STATE_ATTACK) {
                    pstw = game_state.player.walk_attack_anim_tex_data.width;
                    psth = game_state.player.walk_attack_anim_tex_data.height;
                    pstx = game_state.player.walk_attack_anim_tex_data.x + (game_state.player.direction * pstw);
                    psty = game_state.player.walk_attack_anim_tex_data.y + (game_state.player.anim_frame * psth);
                }

                Vec2 camera_relevent_pos = {game_state.player.pos.x - display.camera_pos.x, game_state.player.pos.y - display.camera_pos.y};
                RenderSubTexture(&display, &camera_relevent_pos, &game_state.texture_sheet, pstx, psty, pstw, psth);

                // Render Enemies
                for(i32 i = 0; i < game_state.enemy_list.num_enemies; i++) {
                    Enemy* enemy = &game_state.enemy_list.enemies[i];
                    if(!enemy->alive) {
                        continue;
                    }

                    i32 estw = enemy->walk_anim_tex_data.width;
                    i32 esth = enemy->walk_anim_tex_data.height;
                    i32 estx = enemy->walk_anim_tex_data.x + (enemy->direction * pstw);
                    i32 esty = enemy->walk_anim_tex_data.y + (enemy->anim_frame * psth);

                    if(enemy->anim_state == ACTION_STATE_ATTACK) {
                        estw = enemy->walk_attack_anim_tex_data.width;
                        esth = enemy->walk_attack_anim_tex_data.height;
                        estx = enemy->walk_attack_anim_tex_data.x + (enemy->direction * pstw);
                        esty = enemy->walk_attack_anim_tex_data.y + (enemy->anim_frame * psth);
                    }

                    Vec2 enemy_world_pos = {enemy->pos.x - display.camera_pos.x, 
                                            enemy->pos.y - display.camera_pos.y};
                    RenderSubTexture(&display, &enemy_world_pos, &game_state.texture_sheet, estx, esty, estw, esth);
                }

                // Render Bombs
                if(game_state.bombs) {
                    for(i32 i = 0; i < MAX_NUM_BOMBS; i++) {
                        Bomb* bomb = &game_state.bombs[i];

                        if(!bomb->live) {
                            continue;
                        }

                        i32 bstw = bomb->tex_data.width;
                        i32 bsth = bomb->tex_data.height;
                        i32 bstx = bomb->tex_data.x + (bomb->anim_frame * bstw);
                        i32 bsty = bomb->tex_data.y;

                        Vec2 bomb_world_pos = {bomb->pos.x - display.camera_pos.x, 
                                               bomb->pos.y - display.camera_pos.y};
                        RenderSubTexture(&display, &bomb_world_pos, &game_state.texture_sheet, bstx, bsty, bstw, bsth);
                    }
                }

                // Render UI
                RenderFilledRect(&display, &game_state.hud_pos, &game_state.hud_size, game_state.hud_colour);
                
                std::stringstream health_display_output;
                health_display_output << "HP: " << game_state.player.health << "/" << PLAYER_HEALTH;
                RenderString(&display, &game_state.font, &game_state.health_display_pos, (char*)health_display_output.str().c_str(), HUD_TEXT_COLOUR);

                for(i32 i = 0; i < NUM_ITEM_BOXES; i++) {
                    UI_Item_Box* item_box = &game_state.item_boxes[i];

                    RenderFilledRect(&display, &item_box->pos, &item_box->size, ITEM_BOX_RECT_COLOUR);

                    Vec2 letter_pos = {item_box->pos.x - (item_box->size.w / 2) - 0.7, item_box->pos.y - 0.4};
                    char temp_str[2] = {item_box->item_use_char, '\0'};
                    RenderString(&display, &game_state.font, &letter_pos, temp_str, HUD_TEXT_COLOUR);
                    
                    switch(i) {
                        case ITEM_BOX_WEAPON: {
                            i32 icon_tex_y = item_box->icon_tex_data.y;
                            if(game_state.player.has_sword) {
                                icon_tex_y = item_box->icon_tex_data.y + 16;
                            }

                            RenderSubTexture(&display, &item_box->pos, &game_state.texture_sheet, item_box->icon_tex_data.x, 
                                            item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                            item_box->icon_tex_data.height);
                        } break;

                        case ITEM_BOX_SHIELD: {
                            if(game_state.player.has_shield) {
                                RenderSubTexture(&display, &item_box->pos, &game_state.texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);
                            }

                        } break;

                        case ITEM_BOX_GRAPPLE: {
                            if(game_state.player.has_grapple) {
                                RenderSubTexture(&display, &item_box->pos, &game_state.texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);
                            }
                        } break;

                        case ITEM_BOX_POTION: {
                            if(game_state.player.potions > 0) {
                                RenderSubTexture(&display, &item_box->pos, &game_state.texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);

                                Vec2 value_pos = {item_box->pos.x + (item_box->size.w / 2) + 0.2, item_box->pos.y - 0.3};
                                char temp_str[4];
                                itoa(game_state.player.potions, temp_str, 10);
                                RenderString(&display, &game_state.font, &value_pos, temp_str, HUD_TEXT_COLOUR);
                            }
                        } break;

                        case ITEM_BOX_BOMB: {
                            if(game_state.player.bombs > 0) {
                                RenderSubTexture(&display, &item_box->pos, &game_state.texture_sheet, item_box->icon_tex_data.x, 
                                                item_box->icon_tex_data.y, item_box->icon_tex_data.width, 
                                                item_box->icon_tex_data.height);

                                Vec2 value_pos = {item_box->pos.x + (item_box->size.w / 2) + 0.2, item_box->pos.y - 0.3};
                                char temp_str[4];
                                itoa(game_state.player.bombs, temp_str, 10);
                                RenderString(&display, &game_state.font, &value_pos, temp_str, HUD_TEXT_COLOUR);
                            }
                        } break;

                    }
                }

                // animate map
                #if 0
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
                #endif
            } break;

            case START_STATE: {
                while(SDL_PollEvent(&event)) {
                    switch(event.type) {
                        case SDL_QUIT: {
                            running = false;
                        } break;

                        case SDL_KEYUP: {
                            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                                game_state.update_and_render_state = PLAY_STATE;
                                ClearEnemyList(&game_state.enemy_list);
                                LoadMap(&game_state, display.pixel_format, MAP_LEVEL_1);
                                InitPlayState(&game_state, &display);
                            }
                        } break;
                    }
                }

                SDL_RenderClear(display.renderer);
                for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
                    *((u32*)display.pixel_buffer.pixels + i) = 0xFF500050;
                }
                // Max 11 characters per line
                char* render_string1 = "Use the arrow keys to move";
                char* render_string2 = "Press SPACE to start game";
                Vec2 render_string1_pos = {0.2, 2};
                Vec2 render_string2_pos = {0.2, 3};
                RenderString(&display, &game_state.font, &render_string1_pos, render_string1, 0xFFEEEEEE);
                RenderString(&display, &game_state.font, &render_string2_pos, render_string2, 0xFFEEEEEE);
            } break;

            case DEATH_STATE: {
                while(SDL_PollEvent(&event)) {
                    switch(event.type) {
                        case SDL_QUIT: {
                            running = false;
                        } break;

                        case SDL_KEYUP: {
                            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                                game_state.update_and_render_state = PLAY_STATE;
                                ClearEnemyList(&game_state.enemy_list);
                                LoadMap(&game_state, display.pixel_format, MAP_LEVEL_1);
                                InitPlayState(&game_state, &display);
                            }
                        } break;
                    }
                }

                SDL_RenderClear(display.renderer);
                for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
                    *((u32*)display.pixel_buffer.pixels + i) = 0xFF500050;
                }

                char* render_string1 = "You Died! Haha, you suck!";
                char* render_string2 = "Press SPACE to restart game";
                Vec2 render_string1_pos = {0.2, 2};
                Vec2 render_string2_pos = {0.2, 3};
                RenderString(&display, &game_state.font, &render_string1_pos, render_string1, 0xFFEEEEEE);
                RenderString(&display, &game_state.font, &render_string2_pos, render_string2, 0xFFEEEEEE);
            } break;

            case WIN_STATE: {
                while(SDL_PollEvent(&event)) {
                    switch(event.type) {
                        case SDL_QUIT: {
                            running = false;
                        } break;

                        case SDL_KEYUP: {
                            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                                game_state.update_and_render_state = PLAY_STATE;
                                ClearEnemyList(&game_state.enemy_list);
                                LoadMap(&game_state, display.pixel_format, MAP_LEVEL_1);
                                InitPlayState(&game_state, &display);
                            }
                        } break;
                    }
                }

                SDL_RenderClear(display.renderer);
                for(i32 i = 0; i < display.pixel_buffer.height * display.pixel_buffer.width; i++) {
                    *((u32*)display.pixel_buffer.pixels + i) = 0xFF500050;
                }
                // Max 11 characters per line
                char* render_string1 = "Yay! You Win, You are Hero!";
                char* render_string2 = "Press SPACE to restart game";
                Vec2 render_string1_pos = {0.2, 2};
                Vec2 render_string2_pos = {0.2, 3};
                RenderString(&display, &game_state.font, &render_string1_pos, render_string1, 0xFFEEEEEE);
                RenderString(&display, &game_state.font, &render_string2_pos, render_string2, 0xFFEEEEEE);
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
        if(game_state.update_and_render_state == PLAY_STATE) {
                // Update Bombs
                if(game_state.bombs) {
                    for(i32 i = 0; i < MAX_NUM_BOMBS; i++) {
                        Bomb* bomb = &game_state.bombs[i];

                        if(!bomb->live) {
                            continue;
                        }

                        bomb->time_left -= ms_this_frame;
                        bomb->anim_timer -= ms_this_frame;
                        if(bomb->time_left <= 0) {
                            bomb->live = false;
                            game_state.num_bombs--;
                            Mix_PlayChannel(-1, bomb_explosion_sound, 0);
                            i32 tile_x = bomb->pos.x * PIXELS_PER_METER / TILE_SIZE;
                            i32 tile_y = bomb->pos.y * PIXELS_PER_METER / TILE_SIZE;
                            ReplaceAllSameAdjacentTilesRecursivly(&game_state.map, tile_x, tile_y, TILE_BREAKABLE_BRICK, TILE_DIRT);
                        } 
                        if(bomb->anim_timer <= 0) {
                            bomb->anim_frame = (bomb->anim_frame == 0) ? 1 : 0;
                            bomb->anim_timer = BOMB_ANIM_TIME;
                        }
                    }
                }

            if(game_state.player.anim_state != ACTION_STATE_IDLE) {
                game_state.player.anim_timer -= ms_this_frame;
            }
            for(i32 i = 0; i < game_state.enemy_list.num_enemies; i++) {
                Enemy* enemy = &game_state.enemy_list.enemies[i];
                if(!enemy->alive) {
                    continue;
                }

                if(enemy->anim_state != ACTION_STATE_IDLE) {
                    enemy->anim_timer -= ms_this_frame;
                }
            }
        }

        //printf("%d ms this frame\n", ms_this_frame);
        last_delta = ms_this_frame / 1000.0f;
    }

    // Freeing stuff is overrated

    return 0;
}