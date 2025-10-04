#ifndef PLATO_INI_H
#define PLATO_INI_H

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define START_COMMENT_PREFIXES ";#"
#define INLINE_COMMENT_PREFIXES ";"

#define MAX_SECTION 64
#define MAX_NAME 64
#define MAX_LINE 256

typedef int (*ini_handler)(void* user, const char* section, const char* name, const char* value);
typedef char* (*ini_reader)(char* str, int num, void* stream);

int pl_ini_parse(char* filepath, void *parser, void *dest);

#ifdef PLATO_INI_IMPLEMENTATION

// The 'end' paramenter must be a pointer to the null terminator at the end of 'str'
static char* pl_ini_stripws(char* str, char* end) {
    while(end > str && isspace((unsigned char)(*--end))) *end = '\0';
    return str;
}

// Returns pointer to first non-whitespace char
static char* pl_ini_skipws(const char* str) {
    while(*str && isspace((unsigned char)(*str))) str++;
    return (char*)str;
}

// Returns pointer to first char (of 'chars'), inline comment, or null terminator if none found
// Inline comments must be prefixed by whitespace
static char* pl_ini_find_chars_or_comment(const char* str, const char* chars) {
    int was_space = 0;
    while(*str && (!chars || !strchr(chars, *str)) &&
    !(was_space && strchr(INLINE_COMMENT_PREFIXES, *str))) {
        was_space = isspace((unsigned char)(*str));
        str++;
    }
    return (char*)str;
}

// strncpy but guarantees dest is null terminated and doesn't pad with null terminators
static char* pl_ini_strncpy_np(char* dest, const char* src, size_t size) {
    size_t i;
    for(i = 0; i < size - 1 && src[i]; i++) dest[i] = src[i];
    dest[i] = '\0';
    return dest;
}

int pl_ini_parse_stream(ini_reader reader, void* stream, ini_handler handler, void* user) {
    char line[MAX_LINE];
    size_t max_line = MAX_LINE;
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    size_t offset;
    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;
    char abyss[16]; // Holds input when line is too long

    while(reader(line, (int)max_line, stream) != NULL) {
        offset = strlen(line);
        lineno++;

        if(offset == max_line - 1 && line[offset - 1] != '\n') {
            while (reader(abyss, sizeof(abyss), stream) != NULL) {
                if (!error)
                    error = lineno;
                if (abyss[strlen(abyss) - 1] == '\n')
                    break;
            }
        }

        start = line;

        if(lineno == 1 && (unsigned char)start[0] == 0xEF &&
            (unsigned char)start[1] == 0xBB &&
            (unsigned char)start[2] == 0xBF) {
            start += 3;
        }

        start = pl_ini_stripws(pl_ini_skipws(start), line + offset);

        if(strchr(START_COMMENT_PREFIXES, *start)) {
            // Start-of-line comment
        }
        else if(*prev_name && *start && start > line) {
            end = pl_ini_find_chars_or_comment(start, NULL);
            *end = '\0';
            pl_ini_stripws(start, end);
            if(!handler(user, section, prev_name, start) && !error) error = lineno;
        }
        else if(*start == '[') {
            // [section] line
            end = pl_ini_find_chars_or_comment(start + 1, "]");
            if(*end == ']') {
                *end = '\0';
                pl_ini_strncpy_np(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if(!error) {
                // No ']' found on section line
                error = lineno;
            }
        }
        else if(*start) {
            // Not a comment, must be a name[=:]value pair
            end = pl_ini_find_chars_or_comment(start, "=:");
            if(*end == '=' || *end == ':') {
                *end = '\0';
                name = pl_ini_stripws(start, end);
                value = end + 1;

                end = pl_ini_find_chars_or_comment(value, NULL);
                *end = '\0';

                value = pl_ini_skipws(value);
                pl_ini_stripws(value, end);
                pl_ini_strncpy_np(prev_name, name, sizeof(prev_name));

                // Valid name[=:]value pair found
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
            else {
                // No '=' or ':' found on name[=:]value line
                if (!error) error = lineno;
            }
        }
    }

    return error;
}

void pl_ini_normalize_path_separators(char *filepath) {
    #ifdef _WIN32
        for(int i = 0; filepath[i] != '\0'; i++) {
            if(filepath[i] == '/') filepath[i] = '\\';
        }
    #endif
}

int pl_ini_parse(char* filepath, void *parser, void *dest) {
    pl_ini_normalize_path_separators(filepath);
    FILE *file = fopen(filepath, "r");
    if(!file) return 1;

    int error = pl_ini_parse_stream((ini_reader)fgets, file, (ini_handler)parser, dest);
    fclose(file);

    if(error < 0) return 1;
    return 0;
}

#endif // PLATO_INI_IMPLEMENTATION
#endif // PLATO_INI_H