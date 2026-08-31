#ifndef CANVAS_H
#define CANVAS_H

#include "mysdl2.h"
#include "sweetie-16.h"

typedef struct {
    int frames;
    int w,h;
    uint8_t *pixels;
} Canvas;

Canvas *Canvas_Load(char *path);
void Canvas_Free(Canvas **canvas);
void Canvas_Draw(MySDL *app,Canvas *canvas,int frame,int x,int y,int size);
void Canvas_Animate(MySDL *app, Canvas *canvas, Uint64 *anim_timer, int *current_index, const int *frame_list, const Uint64 *delay_list, int num_frames, int x, int y, int size);

#ifdef CANVAS_IMPLEMENTATION

Canvas *Canvas_Load(char *path) {
    FILE *fin = fopen(path, "r");
    if (!fin) return NULL;

    Canvas *canvas = malloc(sizeof(*canvas));
    if (!canvas) {
        fclose(fin);
        return NULL;
    }

    if (fscanf(fin, "%d,%d,%d", &canvas->frames, &canvas->w, &canvas->h) != 3) {
        fclose(fin);
        free(canvas);
        return NULL;
    }

    size_t total_pixels = (size_t)canvas->frames * canvas->w * canvas->h;
    canvas->pixels = malloc(total_pixels);
    if (!canvas->pixels) {
        fclose(fin);
        free(canvas);
        return NULL;
    }

    // Pre-calculate an O(1) ASCII lookup table
    uint8_t ascii_map[256];
    for (int i = 0; i < 256; i++) ascii_map[i] = 255;
    for (int i = 0; i < 16; i++) {
        ascii_map[(int)"0123456789ABCDEF"[i]] = i;
        ascii_map[(int)"0123456789abcdef"[i]] = i;
    }
    ascii_map[(int)'.'] = 254; 
    ascii_map[(int)'\n'] = 253; 
    ascii_map[(int)'\r'] = 253; 
    ascii_map[(int)' '] = 252;  
    ascii_map[(int)'\t'] = 252; 

    // Buffer-based block reading to maximize I/O throughput
    char buffer[8192];
    size_t bytes_read;
    size_t i = 0;
    int line = 1, col = 1;
    int in_line_comment = 0;
    int in_block_comment = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fin)) > 0) {
        char *p = buffer;
        char *end = buffer + bytes_read;

        while (p < end) {
            int ch = (unsigned char)*p++;

            if (ch == '\n') {
                line++;
                col = 1;
                in_line_comment = 0;
                continue;
            }

            if (in_line_comment) {
                col++;
                continue;
            }

            // Handle comments using inline lookahead within the buffer
            if (ch == '/') {
                if (p < end) {
                    int next_ch = (unsigned char)*p;
                    if (next_ch == '/') {
                        in_line_comment = 1;
                        p++;
                        col += 2;
                        continue;
                    } else if (next_ch == '*') {
                        in_block_comment = 1;
                        p++;
                        col += 2;
                        continue;
                    }
                } else {
                    // Handle edge case where '/' sits right at the buffer boundary
                    int next_ch = fgetc(fin);
                    if (next_ch == '/') {
                        in_line_comment = 1;
                        col += 2;
                        continue;
                    } else if (next_ch == '*') {
                        in_block_comment = 1;
                        col += 2;
                        continue;
                    } else if (next_ch != EOF) {
                        ungetc(next_ch, fin);
                    }
                }
            }

            if (in_block_comment) {
                if (ch == '*') {
                    int next_ch = (p < end) ? (unsigned char)*p : fgetc(fin);
                    if (next_ch == '/') {
                        in_block_comment = 0;
                        if (p < end) p++;
                        col += 2;
                        continue;
                    } else if (p >= end && next_ch != EOF) {
                        ungetc(next_ch, fin);
                    }
                }
                col++;
                continue;
            }

            uint8_t val = ascii_map[ch];
            if (val == 255) {
                fprintf(stderr, "Error: Invalid character '%c' (0x%02X) at LINE:%d COLUMN:%d in %s\n", 
                        (ch >= 32 && ch <= 126) ? ch : '?', ch, line, col, path);
                fclose(fin);
                free(canvas->pixels);
                free(canvas);
                return NULL;
            }

            if (val != 252 && val != 253) { 
                if (i < total_pixels) {
                    canvas->pixels[i++] = (val == 254) ? 255 : val;
                }
            }
            col++;
        }
    }

    if (in_block_comment) {
        fprintf(stderr, "Error: Unclosed block comment at end of file in %s\n", path);
        fclose(fin);
        free(canvas->pixels);
        free(canvas);
        return NULL;
    }

    fclose(fin);
    return canvas;
}

void Canvas_Free(Canvas **canvas) {
    free((*canvas)->pixels);
    (*canvas)->pixels=NULL;
    free(*canvas);
    (*canvas)=NULL;
}

void Canvas_Draw(MySDL *app,Canvas *canvas,int frame,int x,int y,int size) {
    for(int j=0;j<canvas->h;j++) {
        for(int i=0;i<canvas->w;i++) {
            int k=canvas->pixels[frame*canvas->w*canvas->h+j*canvas->w+i];
            if(k!=255) {
                mysdl_fill_rect(app,x+i*size,y+j*size,size,size,SWEETIE_16_PALETTE[k]);
            }
        }
    }
}

void Canvas_Animate(MySDL *app, Canvas *canvas, Uint64 *anim_timer, int *current_index, const int *frame_list, const Uint64 *delay_list, int num_frames, int x, int y, int size) {
    Uint64 current_ticks = SDL_GetTicks64();
    if (*anim_timer == 0) {
        *anim_timer = current_ticks;
    }

    Uint64 elapsed_ms = current_ticks - *anim_timer;
    Uint64 delay_ms = delay_list[*current_index] / 1000;

    if (elapsed_ms >= delay_ms) {
        *anim_timer = current_ticks;
        *current_index = (*current_index + 1) % num_frames;
    }

    int active_frame = frame_list[*current_index];
    Canvas_Draw(app, canvas, active_frame, x, y, size);
}

#endif /* CANVAS_IMPLEMENTATION */

#endif /* CANVAS_H */
