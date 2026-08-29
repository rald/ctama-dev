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
    Canvas *canvas=malloc(sizeof(*canvas));
    char *hex="0123456789ABCDEF.";
    FILE *fin=fopen(path,"r");
    int ch;
    int i,j,k;

    fscanf(fin,"%d,%d,%d",&canvas->frames,&canvas->w,&canvas->h);
    canvas->pixels=calloc(canvas->frames*canvas->w*canvas->h,sizeof(*canvas->pixels));

    i=0;
    while((ch=fgetc(fin))!=EOF) {
        j=-1;
        for(k=0;hex[k];k++) {
            if(ch==hex[k]) {
                j=(k==16?255:k);
                break;
            }
        }
        if(j!=-1) {
            canvas->pixels[i++]=j;
        }
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
