#include "window.h"
#include "input.h"
#include <pthread.h>
#include "scheduler.h"

#include <GL/gl.h>

#define RMT_ENABLED 1
#define RMT_USE_OPENGL 1
#include "remotery.h"

void game_frame(void *pArg, struct scheduler *s, sched_uint begin, sched_uint end, sched_uint thread);

int main(int argc, char** argv)
{

    // Create profiler
    Remotery *rmt;
    rmt_CreateGlobalInstance(&rmt);

	Window::create_window(0, 0, 1024, 768, "Hello Test Window");
	Input::initialize();

	rmt_BindOpenGL();

    void *memory;
    sched_size needed_memory;

    struct scheduler sched;
    scheduler_init(&sched, &needed_memory, SCHED_DEFAULT, 0);
    memory = calloc(needed_memory, 1);
    scheduler_start(&sched, memory);
    
    bool keep_running = true;

    while(keep_running)
    {
    	//rmt_LogText("start profiling");
        struct sched_task task;
    	glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scheduler_add(&task, &sched, game_frame, 0, 1);
        scheduler_join(&sched, &task);
        //rmt_LogText("end profiling");
        
    }
    scheduler_stop(&sched);
	free(memory);


    rmt_DestroyGlobalInstance(rmt);

    return 0;    
}

void game_frame(void *pArg, struct scheduler *s, sched_uint begin, sched_uint end, sched_uint thread)
{
	rmt_BeginCPUSample(game_frame, 0);
	Input::update();
	
	Window::swap_buffer();
	rmt_EndCPUSample();
}