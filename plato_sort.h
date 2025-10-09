#ifndef PLATO_SORT_H
#define PLATO_SORT_H

#include <stddef.h>

void pl_qsort_r(
    void *base, 
    size_t num, 
    size_t size, 
    int (*cmp)(const void *, const void *, void *), 
    void *arg
);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_SORT_IMPLEMENTATION)

static void _pl_sort_swap_internal(void *a, void *b, size_t size) {
    unsigned char *pa = (unsigned char*)a;
    unsigned char *pb = (unsigned char*)b; 
    unsigned char *tmp;
    while(size--) {
        tmp = *pa;
        *pa++ = *pb;
        *pb++ = tmp;
    }
}

static void *_pl_sort_partition_internal(
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
            _pl_sort_swap_internal(arr + i * size, arr + j * size, size);
            i++;
        }
    }
    _pl_sort_swap_internal(arr + i * size, pivot, size);
    return arr + i * size;
}

void pl_qsort_r(
    void *base, 
    size_t num, 
    size_t size, 
    int (*cmp)(const void *, const void *, void *), 
    void *arg
) {
    if(num < 2) return;
    unsigned char *arr = (unsigned char*)base;
    void *p = _pl_sort_partition_internal(arr, num, size, cmp, arg);
    size_t pivot_index = (p - (void *)arr) / size;
    pl_qsort_r(arr, pivot_index, size, cmp, arg);
    pl_qsort_r(arr + (pivot_index + 1) * size, num - pivot_index - 1, size, cmp, arg);
}

#endif // PLATO_SORT_IMPLEMENTATION
#endif // PLATO_SORT_H