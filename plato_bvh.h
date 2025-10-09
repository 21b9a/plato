#ifndef PLATO_BVH_H
#define PLATO_BVH_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <math.h>

#define PL_BVH_LEAFNODE            -1
#define PL_BVH_INTERNALNODE_OBJIDX -1

typedef struct pl_bvh_s pl_bvh_t;
typedef struct pl_bvh_node_s pl_bvh_node_t;
typedef struct pl_bvh_aabb_s pl_bvh_aabb_t;

void pl_bvh_init(pl_bvh_t *bvh, pl_bvh_aabb_t *aabbs, size_t aabb_count);
void pl_bvh_print(pl_bvh_t *bvh);
int pl_bvh_ray_intersection(
    pl_bvh_t *bvh, 
    float ray_origin[3], 
    float ray_dir[3], 
    float ray_range,
    float padding[3],
    int *dest, 
    size_t dest_sz
);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_BVH_IMPLEMENTATION)

typedef struct pl_bvh_aabb_s {
    float min[3];
    float max[3];
} pl_bvh_aabb_t;

typedef struct _pl_bvh_indexed_aabb_s {
    pl_bvh_aabb_t aabb;
    int idx;
} _pl_bvh_indexed_aabb_t;

typedef struct pl_bvh_node_s {
    pl_bvh_aabb_t aabb;
    int left;
    int right;
    int obj_idx;
} pl_bvh_node_t;

typedef struct pl_bvh_s {
    pl_bvh_node_t *nodes;
    int node_count;
} pl_bvh_t;

static void _pl_bvh_swap_internal(void *a, void *b, size_t size) {
    unsigned char *pa = (unsigned char*)a;
    unsigned char *pb = (unsigned char*)b; 
    unsigned char tmp;
    while(size--) {
        tmp = *pa;
        *pa++ = *pb;
        *pb++ = tmp;
    }
}

static void *_pl_bvh_partition_internal(
    void *base, 
    size_t num, 
    size_t size, 
    int (*cmp)(const void*, const void*, void*), 
    void *arg
) {
    unsigned char *arr = (unsigned char*)base;
    void *pivot = arr + (num - 1) * size;
    size_t i = 0;

    for(size_t j = 0; j < num - 1; j++) {
        if (cmp(arr + j * size, pivot, arg) < 0) {
            _pl_bvh_swap_internal(arr + i * size, arr + j * size, size);
            i++;
        }
    }
    _pl_bvh_swap_internal(arr + i * size, pivot, size);
    return arr + i * size;
}

static void _pl_bvh_qsortr_internal(
    void *base, 
    size_t num, 
    size_t size, 
    int (*cmp)(const void*, const void*, void*), 
    void *arg
) {
    if(num < 2) return;

    unsigned char *arr = (unsigned char*)base;
    void *p = _pl_bvh_partition_internal(arr, num, size, cmp, arg);
    size_t pivot_index = (p - (void *)arr) / size;

    _pl_bvh_qsortr_internal(arr, pivot_index, size, cmp, arg);
    _pl_bvh_qsortr_internal(arr + (pivot_index + 1) * size, num - pivot_index - 1, size, cmp, arg);
}

static int _pl_bvh_compare_aabbs_internal(const void *a, const void *b, void *axis_ptr) {
    int axis = *(int*)axis_ptr;
    const _pl_bvh_indexed_aabb_t *aabb0 = (const _pl_bvh_indexed_aabb_t*)a;
    const _pl_bvh_indexed_aabb_t *aabb1 = (const _pl_bvh_indexed_aabb_t*)b;
    float centerA = (aabb0->aabb.min[axis] + aabb0->aabb.max[axis]) * 0.5f;
    float centerB = (aabb1->aabb.min[axis] + aabb1->aabb.max[axis]) * 0.5f;
    return (centerA > centerB) - (centerA < centerB);
}

static void _pl_bvh_print_internal(pl_bvh_t *bvh, int node_idx, int depth) {
    if(!bvh || node_idx < 0 || depth < 0) return;

    for(int i = 0; i < depth; i++) printf("  ");
    pl_bvh_node_t *node = &bvh->nodes[node_idx];
    printf(
        "node_idx: %d, min(%.2f, %.2f, %.2f), max(%.2f, %.2f, %.2f), obj_idx: %d\n",
        node_idx, 
        node->aabb.min[0], node->aabb.min[1], node->aabb.min[2], 
        node->aabb.max[0], node->aabb.max[1], node->aabb.max[2], 
        node->obj_idx
    );
    _pl_bvh_print_internal(bvh, node->left,  depth + 1);
    _pl_bvh_print_internal(bvh, node->right, depth + 1);
}

void pl_bvh_print(pl_bvh_t *bvh) {
    _pl_bvh_print_internal(bvh, 0, 0);
}

static int _pl_build_bvh_internal(
    pl_bvh_node_t *nodes, 
    _pl_bvh_indexed_aabb_t *indexed_aabbs, 
    int start, 
    int end, 
    int *node_count
) {
    int count = end - start;
    int node_idx = (*node_count)++;
    pl_bvh_node_t *node = &nodes[node_idx];

    node->aabb.min[0] = node->aabb.min[1] = node->aabb.min[2] =  FLT_MAX;
    node->aabb.max[0] = node->aabb.max[1] = node->aabb.max[2] = -FLT_MAX;
    for(int i = start; i < end; i++) {
        for(int j = 0; j < 3; j++) {
            if(indexed_aabbs[i].aabb.min[j] < node->aabb.min[j]) 
                node->aabb.min[j] = indexed_aabbs[i].aabb.min[j];
            if(indexed_aabbs[i].aabb.max[j] > node->aabb.max[j]) 
                node->aabb.max[j] = indexed_aabbs[i].aabb.max[j];
        }
    }

    if(count == 1) {
        node->left =  PL_BVH_LEAFNODE;
        node->right = PL_BVH_LEAFNODE;
        node->obj_idx = indexed_aabbs[start].idx;
    }
    else {
        float extent[3] = {
            node->aabb.max[0] - node->aabb.min[0],
            node->aabb.max[1] - node->aabb.min[1],
            node->aabb.max[2] - node->aabb.min[2]
        };

        int axis = (extent[0] > extent[1] && extent[0] > extent[2]) ? 0 : (extent[1] > extent[2] ? 1 : 2);
        _pl_bvh_qsortr_internal(
            indexed_aabbs + start, 
            count, 
            sizeof(_pl_bvh_indexed_aabb_t),
            _pl_bvh_compare_aabbs_internal,
            &axis
        );

        int mid = start + count / 2;
        node->left  = _pl_build_bvh_internal(nodes, indexed_aabbs, start, mid, node_count);
        node->right = _pl_build_bvh_internal(nodes, indexed_aabbs, mid, end, node_count);
        node->obj_idx = PL_BVH_INTERNALNODE_OBJIDX;
    }

    return node_idx;
}

void pl_bvh_init(pl_bvh_t *bvh, pl_bvh_aabb_t *aabbs, size_t aabb_count) {
    if(aabb_count <= 0) {
        bvh->node_count = 0;
        return;
    }

    _pl_bvh_indexed_aabb_t *indexed_aabbs = malloc(sizeof(_pl_bvh_indexed_aabb_t) * aabb_count);
    for(size_t i = 0; i < aabb_count; i++) {
        indexed_aabbs[i].idx = (int)i;
        indexed_aabbs[i].aabb = aabbs[i];
    }

    _pl_build_bvh_internal(bvh->nodes, indexed_aabbs, 0, aabb_count, &bvh->node_count);
    free(indexed_aabbs);
}

static int _pl_bvh_ray_aabb_intersection_internal(
    float origin[3],
    float dir[3],
    float max_dist,
    float min_aabb[3],
    float max_aabb[3],
    float padding[3]
) {
    float inv_dir[3];
    inv_dir[0] = (dir[0] == 0.0f) ? copysignf(FLT_MAX, 1.0f) : 1.0f / dir[0];
    inv_dir[1] = (dir[1] == 0.0f) ? copysignf(FLT_MAX, 1.0f) : 1.0f / dir[1];
    inv_dir[2] = (dir[2] == 0.0f) ? copysignf(FLT_MAX, 1.0f) : 1.0f / dir[2];

    float padded_min[3], padded_max[3];
    padded_min[0] = min_aabb[0] - padding[0];
    padded_min[1] = min_aabb[1] - padding[1];
    padded_min[2] = min_aabb[2] - padding[2];
    padded_max[0] = max_aabb[0] + padding[0];
    padded_max[1] = max_aabb[1] + padding[1];
    padded_max[2] = max_aabb[2] + padding[2];

    float tmin = 0.0f;
    float tmax = max_dist;

    for(int i = 0; i < 3; i++) {
        float t1 = (padded_min[i] - origin[i]) * inv_dir[i];
        float t2 = (padded_max[i] - origin[i]) * inv_dir[i];

        if(inv_dir[i] < 0.0f) {
            float temp = t1;
            t1 = t2;
            t2 = temp;
        }

        if(t1 > tmin) tmin = t1;
        if(t2 < tmax) tmax = t2;

        if(tmin > tmax) return 0;
    }

    return 1;
}

int pl_bvh_ray_intersection(
    pl_bvh_t *bvh, 
    float ray_origin[3], 
    float ray_dir[3], 
    float ray_range,
    float padding[3],
    int *dest, 
    size_t dest_sz
) {
    if(!bvh || !dest || bvh->node_count == 0) return 0;

    int hit_count = 0;
    int stack[64];
    int stack_ptr = 0;

    stack[stack_ptr++] = 0;

    while(stack_ptr > 0) {
        int node_idx = stack[--stack_ptr];
        pl_bvh_node_t *node = &bvh->nodes[node_idx];

        if(_pl_bvh_ray_aabb_intersection_internal(
            ray_origin, ray_dir, ray_range,
            node->aabb.min, node->aabb.max, padding)){
            if(node->left == PL_BVH_LEAFNODE) {
                if((size_t)hit_count < dest_sz) dest[hit_count] = node->obj_idx;
                hit_count++;
            }
            else {
                if(stack_ptr + 2 <= 64) {
                    stack[stack_ptr++] = node->left;
                    stack[stack_ptr++] = node->right;
                }
            }
        }
    }

    return hit_count;
}


#endif // PLATO_BVH_IMPLEMENTATION
#endif // PLATO_BVH_H