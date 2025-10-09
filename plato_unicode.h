#ifndef PLATO_UNICODE_H
#define PLATO_UNICODE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t pl_utf8_parse(const char **_str, const size_t slen);
uint32_t pl_utf8_step(const char **pstr, size_t *pslen);
size_t pl_utf8_strlen(const char *str);
int pl_u8_mbtoucr(uint32_t *puc, const uint8_t *s, size_t n);
uint32_t *pl_u8_u32(const uint8_t *s, size_t n, uint32_t *resultbuf, size_t *lengthp);
uint32_t *pl_utf8_codepoints(const char *utf8_str, size_t *num_codepoints);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_UNICODE_IMPLEMENTATION)

uint32_t pl_utf8_parse(const char **_str, const size_t slen) {
    /*
     *   Char. number range  |        UTF-8 octet sequence
     *      (hexadecimal)    |              (binary)
     *   --------------------+------------------------------------
     *   0000 0000-0000 007F | 0xxxxxxx
     *   0000 0080-0000 07FF | 110xxxxx 10xxxxxx
     *   0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
     *   0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
     */

    const uint8_t *str = (const uint8_t *)*_str;
    const uint32_t octet = (uint32_t)(slen ? *str : 0);

    if(octet == 0) {
        return 0;
    } 
    else if((octet & 0x80) == 0) {
        (*_str)++;
        return octet;
    } 
    else if(((octet & 0xE0) == 0xC0) && (slen >= 2)) {
        const uint8_t str1 = str[1];
        if((str1 & 0xC0) == 0x80) {
            const uint32_t result = ((octet & 0x1F) << 6) | (str1 & 0x3F);
            if(result >= 0x0080) {
                *_str += 2;
                return result;
            }
        }
    } 
    else if(((octet & 0xF0) == 0xE0) && (slen >= 3)) {
        const uint8_t str1 = str[1];
        const uint8_t str2 = str[2];
        if(((str1 & 0xC0) == 0x80) && ((str2 & 0xC0) == 0x80)) {
            const uint32_t octet2 = ((uint32_t)(str1 & 0x3F)) << 6;
            const uint32_t octet3 = ((uint32_t)(str2 & 0x3F));
            const uint32_t result = ((octet & 0x0F) << 12) | octet2 | octet3;
            if(result >= 0x800) {
                if((result < 0xD800) || (result > 0xDFFF)) {
                    *_str += 3;
                    return result;
                }
            }
        }
    } 
    else if(((octet & 0xF8) == 0xF0) && (slen >= 4)) {
        const uint8_t str1 = str[1];
        const uint8_t str2 = str[2];
        const uint8_t str3 = str[3];
        if(((str1 & 0xC0) == 0x80) && ((str2 & 0xC0) == 0x80) && ((str3 & 0xC0) == 0x80)) {
            const uint32_t octet2 = ((uint32_t)(str1 & 0x1F)) << 12;
            const uint32_t octet3 = ((uint32_t)(str2 & 0x3F)) << 6;
            const uint32_t octet4 = ((uint32_t)(str3 & 0x3F));
            const uint32_t result = ((octet & 0x07) << 18) | octet2 | octet3 | octet4;
            if(result >= 0x10000) {
                *_str += 4;
                return result;
            }
        }
    }

    (*_str)++;
    return 0xFFFD;
}

uint32_t pl_utf8_step(const char **pstr, size_t *pslen) {
    if(!pslen) {
        return pl_utf8_parse(pstr, 4);
    }
    const char *ogstr = *pstr;
    const uint32_t result = pl_utf8_parse(pstr, *pslen);
    *pslen -= (size_t)(*pstr - ogstr);
    return result;
}

size_t pl_utf8_strlen(const char *str) {
    size_t result = 0;
    while(pl_utf8_step(&str, NULL)) {
        result++;
    }
    return result;
}

int pl_u8_mbtoucr(uint32_t *puc, const uint8_t *s, size_t n) {
    uint8_t c = *s;

    if(c < 0x80) {
        *puc = c;
        return 1;
    }
    else if(c >= 0xc2) {
        if(c < 0xe0) {
            if(n >= 2) {
                if((s[1] ^ 0x80) < 0x40) {
                    *puc = ((unsigned int)(c & 0x1f) << 6) | (unsigned int)(s[1] ^ 0x80);
                    return 2;
                }
            }
            else {
                *puc = 0xfffd;
                return -2;
            }
        }
        else if(c < 0xf0) {
            if (n >= 2) {
                if((s[1] ^ 0x80) < 0x40
                    && (c >= 0xe1 || s[1] >= 0xa0)
                    && (c != 0xed || s[1] < 0xa0)
                ) {
                    if(n >= 3) {
                        if((s[2] ^ 0x80) < 0x40) {
                            *puc = ((unsigned int)(c & 0x0f) << 12)
                                    | ((unsigned int)(s[1] ^ 0x80) << 6)
                                    | (unsigned int)(s[2] ^ 0x80);
                            return 3;
                        }
                    }
                    else {
                        *puc = 0xfffd;
                        return -2;
                    }
                }
            }
            else {
                *puc = 0xfffd;
                return -2;
            }
        }
        else if(c <= 0xf4) {
            if(n >= 2) {
                if((s[1] ^ 0x80) < 0x40
                  && (c >= 0xf1 || s[1] >= 0x90)
                  && (c < 0xf4 || (s[1] < 0x90))
                ) {
                    if(n >= 3) {
                        if((s[2] ^ 0x80) < 0x40) {
                            if(n >= 4) {
                                if((s[3] ^ 0x80) < 0x40) {
                                    *puc = ((unsigned int) (c & 0x07) << 18)
                                            | ((unsigned int) (s[1] ^ 0x80) << 12)
                                            | ((unsigned int) (s[2] ^ 0x80) << 6)
                                            | (unsigned int) (s[3] ^ 0x80);
                                    return 4;
                                }
                            }
                            else {
                                *puc = 0xfffd;
                                return -2;
                            }
                        }
                    }
                    else {
                        *puc = 0xfffd;
                        return -2;
                    }
                }
            }
            else {
                *puc = 0xfffd;
                return -2;
            }
        }
    }
    *puc = 0xfffd;
    return -1;
}

uint32_t *pl_u8_u32(const uint8_t *s, size_t n, uint32_t *resultbuf, size_t *lengthp) {
    const uint8_t *s_end = s + n;
    uint32_t *result;
    size_t allocated;
    size_t length;

    if(resultbuf != NULL) {
        result = resultbuf;
        allocated = *lengthp;
    }
    else {
        result = NULL;
        allocated = 0;
    }
    length = 0;

    while(s < s_end) {
        uint32_t uc;
        int count;

        count = pl_u8_mbtoucr(&uc, s, s_end - s);
        if(count < 0) {
            if(!(result == resultbuf || result == NULL)) free(result);
            return NULL;
        }
        s += count;

        if(length + 1 > allocated) {
            uint32_t *memory;

            allocated = (allocated > 0 ? 2 * allocated : 12);
            if(length + 1 > allocated) 
                allocated = length + 1;
            if(result == resultbuf || result == NULL)
                memory = (uint32_t*)malloc(allocated * sizeof(uint32_t));
            else
                memory = (uint32_t*)realloc(result, allocated * sizeof(uint32_t));

            if (memory == NULL) {
                if(!(result == resultbuf || result == NULL)) free(result);
                return NULL;
            }
            if(result == resultbuf && length > 0)
                memcpy((char*) memory, (char*) result, length * sizeof(uint32_t));

            result = memory;
        }
        result[length++] = uc;
    }

    if(length == 0) {
        if(result == NULL) {
            result = (uint32_t*)malloc(1);
            if(result == NULL) {
                return NULL;
            }
        }
    }
    else if(result != resultbuf && length < allocated) {
        uint32_t *memory;
        memory = (uint32_t*)realloc(result, length * sizeof(uint32_t));
        if(memory != NULL) result = memory;
    }

    *lengthp = length;
    return result;
}

// User responsible for freeing returned array of uint32_t codepoints
uint32_t *pl_utf8_codepoints(const char *utf8_str, size_t *num_codepoints) {
    if(utf8_str == NULL || num_codepoints == NULL) return NULL;

    size_t u8_length = strlen(utf8_str);
    size_t u32_length = 0;
    uint32_t *u32_str = NULL;

    if(pl_u8_u32((uint8_t*)utf8_str, u8_length, NULL, &u32_length) == NULL) return NULL;
    u32_str = (uint32_t*)malloc(u32_length * sizeof(uint32_t));
    if(u32_str == NULL) return NULL;

    if(pl_u8_u32((uint8_t*)utf8_str, u8_length, u32_str, &u32_length) == NULL) {
        free(u32_str);
        return NULL;
    }

    *num_codepoints = pl_utf8_strlen(utf8_str);
    return u32_str;
}

#endif // PLATO_UNICODE_IMPLEMENTATION
#endif // PLATO_UNICODE_H