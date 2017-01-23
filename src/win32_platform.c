// win32_platform.c

long
platform_executable_directory(char* destination, long destination_size)
{
    HMODULE module = GetModuleHandle(NULL);

    WCHAR module_path[MAX_PATH];
    int got_path = GetModuleFileNameW(module, module_path, MAX_PATH);
    if(got_path == 0)
    {
        return 0;
    }

    int check_index = got_path;
    while(--check_index)
    {
        WCHAR character = *(module_path + check_index);
        if(character == '/' || character == '\\')
        {
            *(module_path + check_index) = 0;
            break;
        }
    }

    destination_size = (long)wcstombs(destination, module_path, destination_size);
    if(destination_size == -1)
    {
        return 0;
    }

    return destination_size;
}

void 
platform_set_working_directory(char* directory)
{
    WCHAR wide_directory[MAX_PATH];
    int converted = MultiByteToWideChar(CP_UTF8, MB_PRECOMPOSED, directory, -1, wide_directory, MAX_PATH);
    if(converted != 0)
    {
        SetCurrentDirectoryW(wide_directory);
    }
}

void 
platform_sleep(int milliseconds)
{
    // TODO: Implement more precise sleep
    Sleep(milliseconds);
}

read_file 
platform_read_file(char* path, int append_null)
{
    read_file result = (read_file){0};

    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, NULL);
    if(file == INVALID_HANDLE_VALUE)
    {
        return result;
    }

    DWORD file_size = GetFileSize(file, NULL);
    long data_size = file_size + (append_null ? 1 : 0);

    result.size = file_size;
    result.data = malloc(data_size);

    DWORD READ_CHUNK_SIZE = 4096;

    DWORD total_read_bytes = 0;
    while(total_read_bytes < file_size)
    {
        DWORD read_bytes = READ_CHUNK_SIZE;
        DWORD bytes_left = (file_size - total_read_bytes);
        if(read_bytes > bytes_left)
        {
            read_bytes = bytes_left;
        }

        DWORD read;
        if(!ReadFile(file, result.data + total_read_bytes, read_bytes, &read, NULL))
        {   
            break;
        }

        total_read_bytes += read;
    }

    CloseHandle(file);

    if(append_null)
    {
        *(result.data + total_read_bytes) = 0;
    }

    return result;
}

void 
platform_free_file(read_file* file)
{
    if(file->data)
    {
        free(file->data);
    }

    *file = (read_file){0};
}

void 
platform_random_seed(int seed)
{
    srand(seed);
}

int 
platform_random(int min, int max)
{
    float value = (float)rand() / RAND_MAX;
    return min + (int)value * (max - min);
}

float 
platform_randomf(float min, float max)
{
    float value = (float)rand() / RAND_MAX;
    return min + value * (max - min);
}

void 
platform_copy_memory(void* destination, void* source, long size)
{
    CopyMemory(destination, source, size);
}
