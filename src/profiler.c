// profiler.c

#include "profiler.h"

#include <stdlib.h>

#define PROFILER_MAX_SAMPLE_COUNT   20000
#define PROFILER_SAMPLES_PER_BLOCK  1024
#define PROFILER_MAX_UNIQUE_SAMPLES 2000
#define PROFILER_MAX_FRAMES         120
#define PROFILER_ENTRIES_PER_FRAME  5000

#define NANOSECOND 1000000000

typedef struct profiler_sample profiler_sample;
typedef struct profiler_sample_block profiler_sample_block;
typedef struct profiler_sample_list profiler_sample_list;

struct profiler_sample
{
    profiler_sample*		parent;
    char*			file;
    char*			function;
    int32			line;
    uint64			timestamp_begin;
    uint64			timestamp_end;
    uint64                      frame_number;
    char			label[64];
    profiler_sample_list*	list;
    profiler_sample*		next;
};

struct profiler_sample_block
{
    uint32			sample_count;
    profiler_sample		samples[PROFILER_SAMPLES_PER_BLOCK];
    profiler_sample_block*	next;
};

struct profiler_sample_list
{
    char* file;
    int32 line;

    profiler_sample_list*	parent;
    profiler_sample*		first_sample;
};

typedef struct profiler_state {
    profiler_sample* current_sample;

    profiler_sample_block* blocks;
    profiler_sample_block* free_blocks;
    
    uint32 sample_list_count;
    profiler_sample_list* sample_list;

    uint64 frame_timestamp;
    uint32 frame_index;
    uint64 frame_count;
    uint64 frame_count_real;
    profiler_entry* frame_data;

    bool32 paused;

    uint64 profiler_memory_overhead;
} profiler_state;

// TODO: One state per thread, so the threads doesn't
// have to synchronize when adding samples. Preparing
// samples would still require synchronizing.
static profiler_state state = (profiler_state){0};

internal void
sample_list_insert(profiler_sample_list* list, profiler_sample* sample)
{
    profiler_sample* next = list->first_sample;
    sample->next = next;
    
    list->first_sample = sample;
}

internal void
sample_list_reset(profiler_state* state)
{
    uint32 entry_index;
    for(entry_index = 0; entry_index < state->sample_list_count; ++entry_index)
    {
	profiler_sample_list* list = (state->sample_list + entry_index);
	list->file	   = 0;
	list->line	   = 0;
	list->parent	   = 0;
	list->first_sample = 0;
    }
    state->sample_list_count = 0;
}

internal profiler_sample_list*
sample_list_find(profiler_state* state, char* file, int32 line, profiler_sample_list* parent)
{
    profiler_sample_list* result = 0;

    uint32 i;
    for_range(i, state->sample_list_count)
    {
	profiler_sample_list* list = (state->sample_list + i);
	if(file	  == list->file &&
	   line	  == list->line &&
	   parent == list->parent)
	{
	    result = list;
	    break;
	}
    }

    return result;
}

void
profiler_init_state()
{
    state.paused = 0;

    uint64 sample_list_size = sizeof(profiler_sample_list) * PROFILER_MAX_UNIQUE_SAMPLES;
    state.sample_list = (profiler_sample_list*)malloc(sample_list_size);

    sample_list_reset(&state);

    uint64 frame_data_size =
	sizeof(profiler_entry) * PROFILER_MAX_FRAMES * PROFILER_ENTRIES_PER_FRAME;
    state.frame_data = (profiler_entry*)malloc(frame_data_size);

    state.profiler_memory_overhead = (sample_list_size + frame_data_size + sizeof(profiler_state));
    platform_log("profiler memory: %.02f MB\n", (float)state.profiler_memory_overhead / 1024 / 1024);
}

internal profiler_sample*
profiler_insert_sample(profiler_state* profiler)
{
    profiler_sample_block* sample_block = profiler->blocks;
    if(!sample_block || sample_block->sample_count >= PROFILER_SAMPLES_PER_BLOCK)
    {
	profiler_sample_block* new_block = 0;
	if(profiler->free_blocks)
	{
	    new_block = profiler->free_blocks;
	    profiler->free_blocks = new_block->next;
	}
	else
	{
	    new_block = (profiler_sample_block*)malloc(sizeof(profiler_sample_block));
	    profiler->profiler_memory_overhead += sizeof(profiler_sample_block);
	}

	
	new_block->sample_count = 0;
	new_block->next		= sample_block;
	
	sample_block	 = new_block;
	profiler->blocks = sample_block;
    }

    return (sample_block->samples + sample_block->sample_count++);
}

void
profiler_record_event(char* label, char* file, char* function, int32 line)
{
    if(state.sample_list == 0)
    {
	profiler_init_state();
    }
    
    profiler_sample* sample = profiler_insert_sample(&state);
    sample->parent = state.current_sample;
    
    sample->file     = file;
    sample->function = function;
    sample->line     = line;

    sample->frame_number = state.frame_count_real;

    uint64 timestamp = platform_time();
    sample->timestamp_begin = timestamp;
    sample->timestamp_end   = timestamp;

    sample->next = 0;

    platform_copy_string(sample->label, label, array_count(sample->label));
}

void
profiler_record_block_begin(char* label, char* file, char* function, int32 line)
{
    if(state.sample_list == 0)
    {
	profiler_init_state();
    }

    profiler_sample* sample = profiler_insert_sample(&state);
    sample->parent = state.current_sample;
    
    sample->file     = file;
    sample->function = function;
    sample->line     = line;

    sample->frame_number = state.frame_count_real;

    uint64 timestamp = platform_time();
    sample->timestamp_begin = timestamp;
    sample->timestamp_end   = 0;

    platform_copy_string(sample->label, label, array_count(sample->label));

    state.current_sample = sample;
}

void
profiler_record_block_end()
{
    profiler_sample* sample = state.current_sample;
    assert(sample != 0);

    uint64 timestamp = platform_time();
    sample->timestamp_end = timestamp;

    state.current_sample = sample->parent;
}

void
profiler_set_paused(bool32 paused)
{
    state.paused = paused;
}

profiler_frame
profiler_process_frame()
{
    profiler_sample_list* list = 0;

    uint64 timestamp = platform_time();
    uint32 delta = (uint32)(timestamp - state.frame_timestamp);
    state.frame_timestamp = timestamp;

    real32 frame_delta = (real32)((real64)delta / NANOSECOND);
    real32 frames_per_second = (real32)(NANOSECOND / delta);

    if(!state.paused)
    {
	state.frame_count = state.frame_count_real;
	
	sample_list_reset(&state);

	profiler_sample_block* block = state.blocks;
	while(block)
	{
	    int sample_count = block->sample_count;
	    for(int i = 0; i < sample_count; ++i)
	    {
		profiler_sample* sample        = (block->samples + i);
		profiler_sample* sample_parent = sample->parent;

		profiler_sample_list* parent = 0;
		if(sample_parent != 0)
		{
		    parent = sample_parent->list;
		}

		if(sample->frame_number == state.frame_count)
		{
		    if(list == 0 || list->file != sample->file || list->line != sample->line ||
		       list->parent != parent)
		    {
			list = sample_list_find(&state, sample->file, sample->line, parent);
		    }

		    if(!list)
		    {
			assert(state.sample_list_count < PROFILER_MAX_UNIQUE_SAMPLES);
			list		   = (state.sample_list + state.sample_list_count++);
			list->file	   = sample->file;
			list->line	   = sample->line;
			list->first_sample = 0;
			list->parent       = parent;
		    }

		    sample_list_insert(list, sample);
		    sample->list = list;
		}
	    }

	    profiler_sample_block* next = block->next;
	    block->next = state.free_blocks;
	    state.free_blocks = block;

	    block = next;
	}

	state.blocks = 0;
	
	++state.frame_count;
    }

    ++state.frame_count_real;

    uint64 frame_data_offset = state.frame_index * PROFILER_ENTRIES_PER_FRAME;
    
    uint32 entry_count = 0;
    uint32 entry_index;
    for(entry_index = 0; entry_index < state.sample_list_count; ++entry_index)
    {
	assert(entry_count < PROFILER_ENTRIES_PER_FRAME);
	
	profiler_entry* entry = (state.frame_data + frame_data_offset + entry_count);
	profiler_sample_list* list = (state.sample_list + entry_index);
	profiler_sample* sample = list->first_sample;

	int32 is_first = 1;
	
	while(sample)
	{
	    uint64 time = (sample->timestamp_end - sample->timestamp_begin);
	    
	    if(is_first)
	    {
		platform_copy_string(entry->label, sample->label, array_count(entry->label));
		entry->file	= sample->file;
		entry->function = sample->function;
		entry->line	= sample->line;
		entry->min	= time;
		entry->max	= time;
		entry->avg	= time;
		entry->frm      = sample->frame_number;
		entry->cnt	= 1;
		is_first = 0;

		entry_count++;
	    }
	    else
	    {
		if(time < entry->min)
		{
		    entry->min = time;
		}
		
		if(time > entry->max)
		{
		    entry->max = time;
		}

		entry->avg = (entry->avg + time) / 2;
	    
		++entry->cnt;
	    }

	    sample = sample->next;
	}
    }

    profiler_frame frame = (profiler_frame){0};
    
    frame.frame_count	   = state.frame_count;
    frame.frame_count_real = state.frame_count_real;
    frame.frame_index	   = state.frame_index;

    frame.memory_overhead = state.profiler_memory_overhead;

    frame.entry_count = entry_count;
    frame.entries = (state.frame_data + frame_data_offset);

    frame.frame_delta = frame_delta;
    frame.frames_per_second = frames_per_second;
    
    state.frame_index = (state.frame_count % PROFILER_MAX_FRAMES);

    return frame;
}
