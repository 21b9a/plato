#ifndef PLATO_PATH_H
#define PLATO_PATH_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

#if defined(__linux__)
    #define _DEFAULT_SOURCE
#elif defined(__APPLE__)
    #define _DARWIN_C_SOURCE
    #define _DARWIN_BETTER_REALPATH
#endif

int pl_path_executable(char *dest, size_t capacity);
bool pl_file_exists(const char *filename);
long pl_file_modtime(const char *filename);
void pl_path_updir(const char *filepath, char *dest, int iter);
void pl_path_normalize_separators(char *filepath);

#if defined(PLATO_IMPLEMENTATION) || defined(PLATO_PATH_IMPLEMENTATION)

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <intrin.h>
#include <io.h>

#define access _access
#ifndef F_OK
    #define F_OK 0
#endif

int pl_path_executable(char* dest, size_t capacity) {
    wchar_t buffer1[MAX_PATH];
    wchar_t buffer2[MAX_PATH];
    wchar_t* path = NULL;
    int length = -1;
    bool valid = false;

    DWORD size = GetModuleFileNameW(NULL, buffer1, sizeof(buffer1) / sizeof(buffer1[0]));
    
    if(size == 0) {
        return -1;
    }
    
    if(size == (DWORD)(sizeof(buffer1) / sizeof(buffer1[0]))) {
        DWORD size_ = size;
        do {
            wchar_t* path_ = (wchar_t*)realloc(path, sizeof(wchar_t) * size_ * 2);
            if(!path_) break;
            size_ *= 2;
            path = path_;
            size = GetModuleFileNameW(NULL, path, size_);
        } while(size == size_);
        
        if(size == size_) {
            free(path);
            return -1;
        }
    } 
    else {
        path = buffer1;
    }
    
    if(_wfullpath(buffer2, path, MAX_PATH)) {
        int length_ = (int)wcslen(buffer2);
        
        int required_length = WideCharToMultiByte(CP_UTF8, 0, buffer2, length_, NULL, 0, NULL, NULL);
        
        if(required_length > 0) {
            if(required_length < capacity) {
                WideCharToMultiByte(CP_UTF8, 0, buffer2, length_, dest, capacity, NULL, NULL);
                dest[required_length] = '\0';
            }
            length = required_length;
            valid = true;
        }
    }
    
    if(path != buffer1) {
        free(path);
    }
    
    return valid ? length : -1;
}

#elif defined(__linux__)

#include <stdio.h>
#include <limits.h>
#include <unistd.h>

int pl_path_executable(char* dest, size_t capacity) {
    char buffer[PATH_MAX];
    char* resolved = realpath("/proc/self/exe", buffer);
    
    if(!resolved) return -1;
    
    int length = (int)strlen(resolved);
    if(length < capacity) {
        memcpy(dest, resolved, length);
        dest[length] = '\0';
    }
    
    return length;
}

#elif defined(__APPLE__)

#include <mach-o/dyld.h>
#include <limits.h>
#include <dlfcn.h>
#include <unistd.h>

int pl_path_executable(char* dest, size_t capacity) {
    char buffer1[PATH_MAX];
    char buffer2[PATH_MAX];
    char* path = buffer1;
    int length = -1;
    
    uint32_t size = (uint32_t)sizeof(buffer1);
    if(_NSGetExecutablePath(path, &size) == -1) {
        path = (char*)malloc(size);
        if(!path || _NSGetExecutablePath(path, &size) != 0) {
            if(path != buffer1) free(path);
            return -1;
        }
    }
    
    char* resolved = realpath(path, buffer2);
    if(resolved) {
        length = (int)strlen(resolved);
        if(length <= capacity) {
            memcpy(dest, resolved, length);
            dest[length] = '\0';
        }
    }
    
    if(path != buffer1) free(path);
    
    return length;
}

#else
    #error "Unsupported platform - Windows, Linux, and macOS are supported."
#endif

bool pl_file_exists(const char *filename) {
    return (access(filename, F_OK) == 0);
}

long pl_file_modtime(const char *filename) {
    struct stat file_info;
    if(stat(filename, &file_info) != 0) return -1;
    return (long)file_info.st_mtime;
}

void pl_path_updir(const char *filepath, char *dest, int iter) {
    strcpy(dest, filepath);
    pl_path_normalize_separators(dest);
    for(int i = 0; i < iter; i++) {
        #if defined(_WIN32)
            char *last_sep = strrchr(dest, '\\');
        #else
            char *last_sep = strrchr(dest, '/');
        #endif
        if(!last_sep) {
            dest = NULL; 
            return;
        }
        *last_sep = '\0';
    }
}

void pl_path_normalize_separators(char *filepath) {
    #ifdef _WIN32
        for(int i = 0; filepath[i] != '\0'; i++) {
            if(filepath[i] == '/') filepath[i] = '\\';
        }
    #endif
}

#endif // PLATO_PATH_IMPLEMENTATION
#endif // PLATO_PATH_H