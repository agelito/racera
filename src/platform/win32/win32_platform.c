// win32_platform.c

#include "../platform.h"

long
platform_executable_directory(char* destination, long destination_size)
{
    return 0;
}

void
platform_set_working_directory(char* directory)
{
    
}

uint64
platform_time()
{
    return 0;
}

read_file
platform_read_file(char* path, int append_null)
{
    read_file file = (read_file){0};

    return file;
}

void
platform_free_file(read_file* file)
{

}

void
platform_random_seed(int seed)
{

}

int
platform_random(int min, int max)
{
    return 0;
}

float
platform_randomf(float min, float max)
{
    return 0.0f;
}

void
platform_copy_memory(void* destination, void* source, long size)
{

}

void
platform_log(char* format, ...)
{

}

long
platform_format(char* destination, long size, char* format, ...)
{
    return 0;
}

void
platform_copy_string(char* destination, char* source, long max_length)
{

}
