
/*
    Copyright (c) Mathew Bergen 2016

    A library used for Ludum Dare 37. Written in SDL and 
    C++. Anyone can use this code however they wish.
 */

#ifndef LD_37_LIB_H
#define LD_37_LIB_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

typedef union {
    struct {
        f32 x;
        f32 y;
    };

    struct {
        f32 w;
        f32 h;
    };
} Vec2;

typedef union {
    struct {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct {
        f32 a;
        f32 r;
        f32 g;
        f32 b;
    };
} Vec4;

typedef struct {
    void* pixels;
    i32 pixels_size;
    i32 width;
    i32 height;
    i32 pitch;
} Pixel_Buffer;

typedef struct {
    i32 width;
    i32 height;
    i32 pixels_per_meter;
    SDL_PixelFormat* pixel_format;
    Pixel_Buffer pixel_buffer;
    char* title;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    Vec2 camera_pos;
    Vec2 camera_size;
} Display;

typedef struct {
    i32 width;
    i32 height;
    void* pixels;
} Texture;

typedef struct {
    char* char_set;
    i32 char_set_size;
    Texture texture_sheet;
    i32 char_width;
    i32 char_height;
    i32 tailed_char_offset;
    i32 char_spacing;
} Font;

typedef struct {
    u64 size;
    void* data;
} File_Read_Result;

#define TRANSPARENT_COLOUR      0x00FF00FF
#define REPLACEMENT_COLOUR_1    0x00000000
#define FREQUENCY               44100
#define SAMPLE_SIZE             2048 // May need to adjust

i32 InitSDL() {
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS | SDL_INIT_AUDIO) < 0) {
        printf("Error - Could not init SDL. SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    int flags = IMG_INIT_PNG;
    if(IMG_Init(flags) & flags != flags) {
        printf("Error - Could not init SDL_Image. IMG_Error: %s\n", IMG_GetError());
        return -1;
    }

    if(Mix_OpenAudio(FREQUENCY, MIX_DEFAULT_FORMAT, 2, SAMPLE_SIZE) < 0) {
        printf("Error - Could not initalize SDL_Mixer. Mix_Error: %s\n", Mix_GetError());
        return -1;
    }

    return 0;
}

u32 ColourVec4ToU32(Vec4* vec4_colour) {
    u8 a = vec4_colour->a * 255;
    u8 r = vec4_colour->r * 255;
    u8 g = vec4_colour->g * 255;
    u8 b = vec4_colour->b * 255;

    u32 u32_colour = (a << 24) | (r << 16) | (g << 8) | b;
    return u32_colour;
}

void RenderFilledRect(Display* display, Vec2* pos, Vec2* size, u32 colour) {
    // render from the center
    i32 x_min = roundf((pos->x * display->pixels_per_meter) - (size->w / 2 * display->pixels_per_meter));
    i32 x_max = roundf((pos->x * display->pixels_per_meter) + (size->w / 2 * display->pixels_per_meter));
    i32 y_min = roundf((pos->y * display->pixels_per_meter) - (size->h / 2 * display->pixels_per_meter));
    i32 y_max = roundf((pos->y * display->pixels_per_meter) + (size->h / 2 * display->pixels_per_meter));

    // Don't bother rendering if the rect is completly off the screen.
    if(x_max < 0 || x_min >= display->pixel_buffer.width || y_max < 0 || y_min >= display->pixel_buffer.height) {
        return;
    }

    for(i32 y = y_min; y < y_max; y++) {
        for(i32 x = x_min; x < x_max; x++) {
            if(x >= 0 && x < display->pixel_buffer.width && y >= 0 && y < display->pixel_buffer.height) {
                u32* display_pixel = (u32*)display->pixel_buffer.pixels + (y * display->pixel_buffer.width) + x;
                *display_pixel = colour;
            }
        }
    }
}

void RenderTexture(Display* display, Vec2* pos, Texture* texture) {
    // render from the center
    i32 x_min = roundf((pos->x * display->pixels_per_meter) - (texture->width / 2));
    i32 x_max = roundf((pos->x * display->pixels_per_meter) + (texture->width / 2));
    i32 y_min = roundf((pos->y * display->pixels_per_meter) - (texture->height / 2));
    i32 y_max = roundf((pos->y * display->pixels_per_meter) + (texture->height / 2));

    // Don't bother rendering if the texture is completly off the screen.
    if(x_max < 0 || x_min >= display->pixel_buffer.width || y_max < 0 || y_min >= display->pixel_buffer.height) {
        return;
    }

    for(i32 y = y_min, ty = 0; y < y_max; y++, ty++) {
        for(i32 x = x_min, tx = 0; x < x_max; x++, tx++) {
            if(x >= 0 && x < display->pixel_buffer.width && y >= 0 && y < display->pixel_buffer.height) {
                u32* display_pixel = (u32*)display->pixel_buffer.pixels + (y * display->pixel_buffer.width) + x;
                u32* texture_pixel = (u32*)texture->pixels + (ty * texture->width) + tx;
                if(*texture_pixel != TRANSPARENT_COLOUR) {
                    *display_pixel = *texture_pixel;
                }
            }
        }
    }
}

void RenderSubTexture(Display* display, Vec2* pos, Texture* texture, i32 sub_texture_x, i32 
                      sub_texture_y, i32 sub_texture_w, i32 sub_texture_h) {
    // render from the center
    i32 x_min = roundf((pos->x * display->pixels_per_meter) - (sub_texture_w / 2));
    i32 x_max = roundf((pos->x * display->pixels_per_meter) + (sub_texture_w / 2));
    i32 y_min = roundf((pos->y * display->pixels_per_meter) - (sub_texture_h / 2));
    i32 y_max = roundf((pos->y * display->pixels_per_meter) + (sub_texture_h / 2));

    // Don't bother rendering if the texture is completly off the screen.
    if(x_max < 0 || x_min >= display->pixel_buffer.width || x_max < 0 || x_min >= display->pixel_buffer.width) {
        return;
    }

    for(i32 y = y_min, ty = sub_texture_y; y < y_max; y++, ty++) {
        for(i32 x = x_min, tx = sub_texture_x; x < x_max; x++, tx++) {
            if(x >= 0 && x < display->pixel_buffer.width && y >= 0 && y < display->pixel_buffer.height) {
                u32* display_pixel = (u32*)display->pixel_buffer.pixels + (y * display->pixel_buffer.width) + x;
                u32* texture_pixel = (u32*)texture->pixels + (ty * texture->width) + tx;
                if(*texture_pixel != TRANSPARENT_COLOUR) {
                    *display_pixel = *texture_pixel;
                }
            }
        }
    }
}

// NOTE (Mathew): replaces black with the replacement colour
void RenderSubTextureEx(Display* display, Vec2* pos, Texture* texture, i32 sub_texture_x, i32 sub_texture_y, 
                        i32 sub_texture_w, i32 sub_texture_h, u32 replacement_colour1) {
    i32 x_min = pos->x * display->pixels_per_meter;
    i32 x_max = x_min + sub_texture_w;
    i32 y_min = pos->y * display->pixels_per_meter;
    i32 y_max = y_min + sub_texture_h;

    // Don't bother rendering if the texture is completly off the screen.
    if(x_max < 0 || x_min >= display->pixel_buffer.width || x_max < 0 || x_min >= display->pixel_buffer.width) {
        return;
    }

    for(i32 y = y_min, ty = sub_texture_y; y < y_max; y++, ty++) {
        for(i32 x = x_min, tx = sub_texture_x; x < x_max; x++, tx++) {
            if(x >= 0 && x < display->pixel_buffer.width && y >= 0 && y < display->pixel_buffer.height) {
                u32* display_pixel = (u32*)display->pixel_buffer.pixels + (y * display->pixel_buffer.width) + x;
                u32* texture_pixel = (u32*)texture->pixels + (ty * texture->width) + tx;
                if(*texture_pixel != TRANSPARENT_COLOUR) {
                    if(*texture_pixel == REPLACEMENT_COLOUR_1) {
                        *display_pixel = replacement_colour1;
                    }
                    else {
                        *display_pixel = *texture_pixel;
                    }
                }
            }
        }
    }
}

void RenderString(Display* display, Font* font, Vec2* pos, char* string, u32 colour) {
    i32 char_num = 0;
    i32 x_min = pos->x * display->pixels_per_meter;
    i32 y_min = pos->y * display->pixels_per_meter;

    SDL_assert(font->texture_sheet.width % font->char_width == 0 && font->texture_sheet.height % font->char_height == 0);

    i32 characters_wide = font->texture_sheet.width / font->char_width;
    i32 characters_high = font->texture_sheet.height / font->char_height;
    for(char* s = string; *s != '\0'; s++) {
        if(*s == ' ') {
            char_num++;
            continue;
        }

        Vec2 render_pos = {x_min + (char_num * (font->char_width + font->char_spacing)), y_min};
        bool found = false;
        i32 i = 0;
        for(; i < font->char_set_size; i++) {
            if(*s == font->char_set[i]) {
                found = true;
                break;
            }
        }

        if(found) {
            if(*s == 'p' || *s == 'y' || *s == 'q' || *s == 'j' || *s == 'g') {
                render_pos.y += font->tailed_char_offset;
            }

            i32 sub_tex_y = (i / characters_wide) * font->char_height;
            i32 sub_tex_x = (i % characters_wide) * font->char_width;
            // NOTE (Mathew): Revert back to meters because RenderSubTexture will convert from meters to pixels.
            render_pos.x /= display->pixels_per_meter;
            render_pos.y /= display->pixels_per_meter;

            RenderSubTextureEx(display, &render_pos, &font->texture_sheet, sub_tex_x, sub_tex_y, font->char_width, font->char_height, colour);
        }

        char_num++;
    }
}

void LoadTexture(char* file_path, Texture* dst_texture, SDL_PixelFormat* pixel_format) {
    SDL_assert(!dst_texture->pixels);

    SDL_Surface* loaded_texture = NULL;

    loaded_texture = IMG_Load(file_path);
    if(!loaded_texture) {
        printf("Warning - could not load image '%s'.\n", file_path);
        return;
    }

    SDL_Surface* corrected_texture = SDL_ConvertSurface(loaded_texture, pixel_format, 0);
    if(!corrected_texture) {
        printf("Warning - could not convert image. %s.\n", SDL_GetError());
        return;
    }

    dst_texture->width = corrected_texture->w;
    dst_texture->height = corrected_texture->h;
    i32 tex_size = dst_texture->width * dst_texture->height;
    dst_texture->pixels = malloc(tex_size * pixel_format->BytesPerPixel);
    for(i32 i = 0; i < tex_size; i++) {
        ((u32*)dst_texture->pixels)[i] = ((u32*)corrected_texture->pixels)[i];
    }

    SDL_FreeSurface(loaded_texture);
    SDL_FreeSurface(corrected_texture);
}

void FreeTexture(Texture* texture) {
    free(texture->pixels);
    texture->pixels = 0;
}

void ReadEntireFile(char* path, File_Read_Result* read_result) {
    SDL_assert(!read_result->data);
    read_result->data = NULL;

    SDL_RWops* file_handle = SDL_RWFromFile(path, "r");

    if(file_handle) {
        SDL_RWseek(file_handle, 0, RW_SEEK_END);
        // NOTE (Mathew): +1 for EOF at the end of the file
        read_result->size = SDL_RWtell(file_handle) + 1;
        SDL_RWseek(file_handle, 0, RW_SEEK_SET);
        read_result->data = malloc(read_result->size);

        // NOTE (Mathew): Because we are reading the whole file a 0 means an error
        i32 read_read_result = SDL_RWread(file_handle, read_result->data, read_result->size - 1, 1);

        if(!read_read_result) { 
            // TODO (Mathew): Log Error!
            printf("Couldn't read the file: %s\n", path);
        }

        *((char*)read_result->data + read_result->size - 1) = EOF;
        SDL_RWclose(file_handle);
    }
    else {
        printf("Could Not Read The File: %s\n", path);
    }
}

void WriteEntireFile(char* path, void* data, u64 size) {
    SDL_RWops* file_handle = SDL_RWFromFile(path, "w");

    if(file_handle) {
        SDL_RWwrite(file_handle, data, size, 1);

        SDL_RWclose(file_handle);

        printf("File Written %s\n", path);
     }
     else {
        // Log Error
        printf("Couldn't write to file\n");
     }
}

void FreeFileReadResult(File_Read_Result* file_read_result) {
    free(file_read_result->data);
    file_read_result->size = 0;
}

char char_set[67] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '/', '.', ',', '!', ':'};

void LoadFont(Font* font, SDL_PixelFormat* format) {
    font->char_set_size = 67;
    font->char_set = char_set;
    font->char_width = 8;
    font->char_height = 12;
    font->tailed_char_offset = 5;
    font->char_spacing = 1;

    LoadTexture("../res/textures/font_sheet.png", &font->texture_sheet, format);
}

void FreeFont(Font* font) {
    font->char_set = 0;
    FreeTexture(&font->texture_sheet);
}

#endif 