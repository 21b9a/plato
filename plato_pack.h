#ifndef PLATO_PACK_H
#define PLATO_PACK_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct pl_pack_rect_t;

int pl_pack_rects(
    pl_pack_rect_t *rects, int rect_count, 
    int container_w, int container_h, 
    int padding
);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_PACK_IMPLEMENTATION)

typedef struct pl_pack_rect_s {
    int x, y;
    int w, h;
    int obj_idx;
} pl_pack_rect_t;

int _pl_pack_compare_rect_h_internal(const void *a, const void *b) {
    return ((pl_pack_rect_t*)b)->h - ((pl_pack_rect_t*)a)->h;
}

int pl_pack_rects(
    pl_pack_rect_t *rects, int rect_count, 
    int container_w, int container_h, 
    int padding
) {
    qsort(rects, rect_count, sizeof(pl_pack_rect_t), _pl_pack_compare_rect_h_internal);

    int pos_x = padding;
    int pos_y = padding;
    int max_row_h = 0;

    for(int i = 0; i < rect_count; i++) {
        pl_pack_rect_t *rect = &rects[i];

        if(pos_x + rect->w > container_w) {
            pos_y += max_row_h + padding;
            pos_x = padding;
            max_row_h = 0;
        }

        if(pos_y + rect->h > container_h) return 1;

        rect->x += pos_x;
        rect->y += pos_y;

        pos_x += rect->w + padding;
        if(rect->h > max_row_h) max_row_h = rect->h;
    }

    return 0;
}

#endif // PLATO_PACK_IMPLEMENTATION
#endif // PLATO_PACK_H