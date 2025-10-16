#ifndef PLATO_EASING_H
#define PLATO_EASING_H

/*
    C implementation of https://github.com/ai/easings.net
*/

#include <math.h>

float pl_ease_in_sine(float x);
float pl_ease_in_quad(float x);
float pl_ease_in_cubic(float x);
float pl_ease_in_quart(float x);
float pl_ease_in_quint(float x);
float pl_ease_in_exp(float x);
float pl_ease_in_circ(float x);
float pl_ease_in_back(float x);
float pl_ease_in_elastic(float x);
float pl_ease_in_bounce(float x);

float pl_ease_out_sine(float x);
float pl_ease_out_quad(float x);
float pl_ease_out_cubic(float x);
float pl_ease_out_quart(float x);
float pl_ease_out_quint(float x);
float pl_ease_out_exp(float x);
float pl_ease_out_circ(float x);
float pl_ease_out_back(float x);
float pl_ease_out_elastic(float x);
float pl_ease_out_bounce(float x);

float pl_ease_in_out_sine(float x);
float pl_ease_in_out_quad(float x);
float pl_ease_in_out_cubic(float x);
float pl_ease_in_out_quart(float x);
float pl_ease_in_out_quint(float x);
float pl_ease_in_out_exp(float x);
float pl_ease_in_out_circ(float x);
float pl_ease_in_out_back(float x);
float pl_ease_in_out_elastic(float x);
float pl_ease_in_out_bounce(float x);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_EASING_IMPLEMENTATION)

#define _PI 3.14159265359f
#define _TAU (2.0f * _PI)
#define _EPSILON 1e-7f
#define _INV_EPSILON (1.0f - _EPSILON)

float pl_ease_in_sine(float x) {
    return 1.0f - cosf((x * _PI) / 2.0f);
}

float pl_ease_in_quad(float x) {
    return x * x;
}

float pl_ease_in_cubic(float x) {
    return x * x * x;
}

float pl_ease_in_quart(float x) {
    return x * x * x * x;
}

float pl_ease_in_quint(float x) {
    return x * x * x * x * x;
}

float pl_ease_in_exp(float x){
    return (x <= _EPSILON) ? 0.0f : powf(2.0f, 10.0f * x - 10.0f);
}

float pl_ease_in_circ(float x) {
    return 1.0f - sqrtf(1.0f - x * x);
}

float pl_ease_in_back(float x) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * x * x * x - c1 * x * x;
}

float pl_ease_in_elastic(float x) {
    const float c4 = _TAU / 3.0f;
    return (x <= _EPSILON) ? 0.0f : (x >= _INV_EPSILON) 
        ? 1.0f : powf(2.0f, 10.0f * x - 10.0f) * sinf((x * 10.0f - 10.75f) * c4);
}

float pl_ease_in_bounce(float x) {
    return 1.0f - pl_ease_out_bounce(1.0f - x);
}


float pl_ease_out_sine(float x) {
    return sinf((x * _PI) / 2.0f);
}

float pl_ease_out_quad(float x) {
    const float ix = 1.0f - x;
    return 1.0f - ix * ix;
}

float pl_ease_out_cubic(float x) {
    return 1.0f - powf(1.0f - x, 3.0f);
}

float pl_ease_out_quart(float x) {
    return 1.0f - powf(1.0f - x, 4.0f);
}

float pl_ease_out_quint(float x) {
    return 1.0f - powf(1.0f - x, 5.0f);
}

float pl_ease_out_exp(float x) {
    return (x >= _INV_EPSILON) ? 1.0f : 1.0f - powf(2.0f, -10.0f * x);
}

float pl_ease_out_circ(float x) {
    return sqrtf(1.0f - powf(x - 1.0f, 2.0f));
}

float pl_ease_out_back(float x) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(x - 1.0f, 3.0f) + c1 * powf(x - 1.0f, 2.0f);
}

float pl_ease_out_elastic(float x) {
    const float c4 = _TAU / 3.0f;
    return (x <= _EPSILON) ? 0.0f : (x >= _INV_EPSILON)
        ? 1.0f : -powf(2.0f, 10.0f * x - 10.0f) * sinf((x * 10.0f - 10.75f) * c4); 
}

float pl_ease_out_bounce(float x) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if     (x < 1.0f / d1) return n1 * x * x;
    else if(x < 2.0f / d1) return n1 * (x -= 1.5f  / d1) * x + 0.75f;
    else if(x < 2.5f / d1) return n1 * (x -= 2.25f / d1) * x + 0.9375f;
    else                   return n1 * (x -= 2.625 / d1) * x + 0.984375f;
}


float pl_ease_in_out_sine(float x) {
    return -(cosf(_PI * x) - 1.0f) / 2.0f;
}

float pl_ease_in_out_quad(float x) {
    return (x < 0.5f) ? 2.0f * x * x : 1.0f - powf(-2.0f * x + 2.0f, 2.0f) / 2.0f;
}

float pl_ease_in_out_cubic(float x) {
    return (x < 0.5f) ? 4.0f * x * x * x : 1.0f - powf(-2.0f * x + 2.0f, 3.0f) / 2.0f;
}

float pl_ease_in_out_quart(float x) {
    return (x < 0.5f) ? 8.0f * x * x * x * x : 1.0f - powf(-2.0f * x + 2.0f, 4.0f) / 2.0f;
}

float pl_ease_in_out_quint(float x) {
    return (x < 0.5f) ? 16.0f * x * x * x * x * x : 1.0f - powf(-2.0f * x + 2.0f, 5.0f) / 2.0f;
}

float pl_ease_in_out_exp(float x) {
    return (x <= _EPSILON) ? 0.0f : (x >= _INV_EPSILON) ? 1.0f : (x < 0.5f)
        ? powf(2.0f, 20.0f * x - 10.0f) / 2.0f : (2.0f - powf(2.0f, -20.0f * x + 10.0f)) / 2.0f;
}

float pl_ease_in_out_circ(float x) {
    return (x < 0.5f) 
        ? (1.0f - sqrtf(1.0f - powf(2.0f * x, 2.0f))) / 2.0f 
        : (sqrtf(1.0f - powf(-2.0f * x + 2.0f, 2.0f)) + 1.0f) / 2.0f;
}

float pl_ease_in_out_back(float x) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    return (x < 0.5f)
        ? (powf(2.0f * x, 2.0f) * ((c2 + 1.0f) * 2.0f * x - c2)) / 2.0f
        : (powf(2.0f * x - 2.0f, 2.0f) * ((c2 + 1.0f) * (x * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
}

float pl_ease_in_out_elastic(float x) {
    const float c5 = _TAU / 4.5f;
    return (x <= _EPSILON) ? 0.0f : (x >= _INV_EPSILON) ? 1.0f : (x < 0.5f)
        ? -(powf(2.0f,  20.0f * x - 10.0f) * sinf((20.0f * x - 11.125f) * c5)) / 2.0f
        :  (powf(2.0f, -20.0f * x + 10.0f) * sinf((20.0f * x - 11.125f) * c5)) / 2.0f + 1.0f;
}

float pl_ease_in_out_bounce(float x) {
    return (x < 0.5f) 
        ? (1.0f - pl_ease_out_bounce(1.0f - 2.0f * x)) / 2.0f
        : (1.0f + pl_ease_out_bounce(2.0f * x - 1.0f)) / 2.0f;
}

#endif // PLATO_EASING_IMPLEMENTATION
#endif // PLATO_EASING_H