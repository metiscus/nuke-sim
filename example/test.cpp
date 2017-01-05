#include "window.h"
#include "input.h"
#include <pthread.h>
#include "scheduler.h"

void game_frame(void *pArg, struct scheduler *s, sched_uint begin, sched_uint end, sched_uint thread);

int main(int argc, char** argv)
{
	Window::create_window(0, 0, 1024, 768, "Hello Test Window");
	Input::initialize();

    void *memory;
    sched_size needed_memory;

    struct scheduler sched;
    scheduler_init(&sched, &needed_memory, SCHED_DEFAULT, 0);
    memory = calloc(needed_memory, 1);
    scheduler_start(&sched, memory);
    
    bool keep_running = true;

    while(keep_running)
    {
        struct sched_task task;
        scheduler_add(&task, &sched, game_frame, 0, 1);
        scheduler_join(&sched, &task);
    }
    scheduler_stop(&sched);
    free(memory);
}

void game_frame(void *pArg, struct scheduler *s, sched_uint begin, sched_uint end, sched_uint thread)
{
	Input::update();
	Window::swap_buffer();
}