#ifndef PROFILER_H_INCLUDED
#define PROFILER_H_INCLUDED

#include "platform/platform.h"

#define PROFILER_FUNC() profiler_record_block_begin((char*)__FUNCTION__, __FILE__, \
						    (char*)__FUNCTION__, __LINE__)
#define PROFILER_EVENT(label) profiler_record_event(label, __FILE__, (char*)__FUNCTION__, __LINE__)
#define PROFILER_BEGIN(label) profiler_record_block_begin(label, __FILE__, (char*)__FUNCTION__, __LINE__)
#define PROFILER_END() profiler_record_block_end()

typedef struct profiler_entry {
    char label[64];
    char* file;
    char* function;
    int32 line;

    uint64 min;
    uint64 max;
    uint64 avg;
    uint64 frm;
    uint32 cnt;
    real32 prc;
} profiler_entry;

typedef struct profiler_frame {
    real32              frame_delta;
    real32              frames_per_second;
    uint64              frame_count;
    uint64              frame_count_real;
    uint32		frame_index;
    uint32		entry_count;
    profiler_entry*	entries;
} profiler_frame;

void
profiler_record_event(char* label, char* file, char* function, int32 line);

void
profiler_record_block_begin(char* label, char* file, char* function, int32 line);

void
profiler_record_block_end();

void
profiler_set_paused(bool32 paused);

profiler_frame
profiler_process_frame();

#endif // PROFILER_H_INCLUDED
