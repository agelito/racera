// linux_time.c

#define SECONDS_TO_NANOSECONDS 1000000000

uint64
platform_time()
{
    struct timespec timespec;
    clock_gettime(CLOCK_MONOTONIC, &timespec);

    uint64 nanoseconds = timespec.tv_sec * SECONDS_TO_NANOSECONDS;
    nanoseconds += timespec.tv_nsec;
    return nanoseconds;
}
