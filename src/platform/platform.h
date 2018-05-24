#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#include <stdint.h>
typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8  bool8;
typedef uint32 bool32;

typedef float  real32;
typedef double real64;

#define internal static

#define assert(expr) if(!(expr)) { platform_log("assertion! %s:%d\n", __FILE__, __LINE__); *(int*)0 = 0; }

#define array_count(array) (sizeof(array) / sizeof(array[0]))
#define for_range(n, count) for(n = 0; n < count; ++n)

#define KB(kilo_bytes) (kilo_bytes * 1024)
#define MB(mega_bytes) (KB(mega_bytes) * 1024)
#define GB(giga_bytes) (MB(giga_bytes) * 1024)

#define unused_arg(arg) (void)arg;
#define array_count(array) (sizeof(array) / sizeof(array[0]))
#define invalid_code *(int*)0 = 0

typedef struct read_file read_file;

struct read_file
{
    unsigned long size;
    unsigned char* data;
};

long
platform_executable_directory(char* destination, long destination_size);

void
platform_set_working_directory(char* directory);

void
platform_sleep(int milliseconds);

uint64
platform_time();

read_file
platform_read_file(char* path, int append_null);

void
platform_free_file(read_file* file);

void
platform_random_seed(int seed);

int
platform_random(int min, int max);

float
platform_randomf(float min, float max);

void
platform_copy_memory(void* destination, void* source, long size);

void
platform_log(char* format, ...);

long
platform_format(char* destination, long size, char* format, ...);

void
platform_copy_string(char* destination, char* source, long max_length);

#endif // PLATFORM_H_INCLUDED
