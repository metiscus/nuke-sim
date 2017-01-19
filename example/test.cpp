#include "window.h"
#include "input.h"
#include <pthread.h>
#include "scheduler.h"

#include <GL/gl.h>

#define RMT_ENABLED 1
#define RMT_USE_OPENGL 1
#include "remotery.h"

#include "logging.h"

#include "core.h"

#include "color.h"
#include "renderer_gl.h"

#include "resourcemanager.h"


void game_frame(void *pArg, struct scheduler *s, sched_uint begin, sched_uint end, sched_uint thread);

RendererGL *renderer;
void draw_reactor(RendererGL& renderer);

Core core;

int main(int argc, char** argv)
{

	int time_scale = 20;

	ResourceManager::initialize();

	Resource::Guid guid = Resource::random_guid();
	auto future_guid = ResourceManager::load_file_as_resource("qgit.png", guid, ImageResourceType);

    // Create profiler
    Remotery *rmt;
    rmt_CreateGlobalInstance(&rmt);

	Window::create_window(0, 0, 1024, 768, "Hello Test Window");
	
    bool keep_running = true;
	Input::on_keyboard_down = [&keep_running, &time_scale, &core](Input::InputEvent event)
	{
		if(event.button == Input::Key_Space)
		{
			keep_running = false;
		}

		if(event.button == Input::Key_Q)
		{
			time_scale += 1;
		}

		if(event.button == Input::Key_E)
		{
			time_scale += 10;
		}

		if(event.button == Input::Key_R)
		{
			time_scale = 1;
		}

		if(event.button == Input::Key_P)
		{
			time_scale = 0;
		}		

		if(event.button == Input::Key_S)
		{
			time_scale -= 1;
		}

		if(event.button == Input::Key_U)
		{
			core.set_rod_position(core.get_rod_position() - 0.1);
		}

		if(event.button == Input::Key_D)
		{
			core.set_rod_position(core.get_rod_position() + 0.1);
		}
	};

	Input::initialize();

	rmt_BindOpenGL();

    void *memory;
    sched_size needed_memory;

    struct scheduler sched;
    scheduler_init(&sched, &needed_memory, SCHED_DEFAULT, 0);
    memory = calloc(needed_memory, 1);
    scheduler_start(&sched, memory);

    renderer = new RendererGL(1024, 768);
    renderer->load_font("font.ttf", {10, 12, 14});

    double ttime = 0.0;

    while(keep_running)
    {
    	//rmt_LogText("start profiling");
        struct sched_task task;
    	glClearColor(0.8, 0.8, 0.8, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		core.simulate((float)time_scale * 1.0 / 60.0);

		ttime += (float)time_scale * 1.0 / 60.0;

		renderer->begin();

		LOG_F(INFO, "Time: %lf\tReactor flux: %f\n", ttime, core.get_flux());

        scheduler_add(&task, &sched, game_frame, 0, 1);
        scheduler_join(&sched, &task);

    	renderer->end();
		Window::swap_buffer();
        //rmt_LogText("end profiling");
    }
    scheduler_stop(&sched);
	free(memory);


    rmt_DestroyGlobalInstance(rmt);

    ResourceManager::shutdown();

    return 0;    
}

void game_frame(void *pArg, struct scheduler *s, sched_uint begin, sched_uint end, sched_uint thread)
{
	rmt_BeginCPUSample(game_frame, 0);
	Input::update();

	draw_reactor(*renderer);

	rmt_EndCPUSample();
}


void draw_reactor(RendererGL& renderer)
{
	auto bright_green      = Color::FromRGBA(60, 200, 60, 255);
	auto bright_red        = Color::FromRGBA(200, 60, 60, 255);
	auto bright_yellow     = Color::FromRGBA(200, 200, 60, 255);
	auto light_gray        = Color::FromRGBA(200, 200, 200, 255);
	auto light_blue        = Color::FromRGBA(170, 170, 200, 255);
	auto light_blue_trans  = Color::FromRGBA(170, 170, 200, 128);
	auto med_blue          = Color::FromRGBA(80, 80, 180, 255);
	auto dark_gray         = Color::FromRGBA(50, 50, 50, 255);
	auto med_gray          = Color::FromRGBA(120, 120, 120, 255);
	auto brown             = Color::FromRGBA(0x8b, 0x45, 0x13, 255);

	//auto pump_map = reactor.get_pump_map();

	// draw containment building
	renderer.draw_rectangle(light_gray, 50, 50, 500, 500, false);
	renderer.draw_rectangle(med_gray, 50, 50, 500, 500, true, 1);

	// draw the reactor containment vessel
	int containment_height = 200;
	renderer.draw_rectangle(light_gray, 100, 100, 200, 100+containment_height, true);
	renderer.draw_rectangle(dark_gray, 100, 100, 200, 100+containment_height, false);
	//float fluid_coverage = reactor.get_core_fluid_fill_percent();
	float fluid_coverage = 1.0;
	LOG_F(INFO, "Fluid coverage: %f", fluid_coverage);
	renderer.draw_rectangle(light_blue_trans, 100, 100, 200, 100+containment_height*fluid_coverage, true, -5);

	// draw the reactor core
	//auto core = reactor.get_core();
	//float temperature = 450.0;
	//float core_color_factor = std::min(1.0f, temperature / 800.0f);

	float core_color_factor = core.get_flux() / 1000.0;
	auto core_color = Color::FromRGBA( core_color_factor * 255.0, 0, 0, 255);
	renderer.draw_rectangle(core_color, 110, 120, 190, 200, true, -1);
	renderer.draw_rectangle(dark_gray, 100, 100, 200, 300, false, -1);

	// draw the control rods
	float rod_start = 120;
	float rod_height = 200-120;
	float rod_position = 0.25;

	rod_start += rod_height * (1.0 - (core.get_rod_position() / 4.0));
	renderer.draw_rectangle(brown, 120, rod_start, 130, rod_start + rod_height, true, -2);
	renderer.draw_rectangle(brown, 140, rod_start, 150, rod_start + rod_height, true, -2);
	renderer.draw_rectangle(brown, 160, rod_start, 170, rod_start + rod_height, true, -2);
	
	// draw the pressurizer
	renderer.draw_rectangle(dark_gray, 240, 200, 280, 300, false, -1);
	renderer.draw_rectangle(light_blue, 240, 200, 280, 300, true);

	// draw the steam generator
	renderer.draw_rectangle(dark_gray, 320, 100, 400, 300, false, -1);
	renderer.draw_rectangle(light_blue, 320, 100, 400, 300, true);

	// draw the reactor coolant pump
	//auto pump = pump_map["Reactor Coolant Return"];
	//if(pump)
	{
//		if(pump->get_is_powered() && !pump->get_is_failed())
//		{
//			renderer.draw_rectangle(bright_green, 250, 100, 300, 120, false, -3);
//		}
//		else if(pump->get_is_powered() && pump->get_is_failed())
//		{
//			renderer.draw_rectangle(bright_red, 250, 100, 300, 120, false, -3);
//			time_scale = 15.0;
//		}
//		else
//		{
			renderer.draw_rectangle(bright_yellow, 250, 100, 300, 120, false, -3);
//		}
		
	}
	renderer.draw_rectangle(light_gray, 250, 100, 300, 120, true, -2);
	
	
	// draw the pipe from the steam generator to the reactor
	renderer.draw_rectangle(med_blue, 200, 280, 320, 300, true, 0);

	// draw the pipe from the reactor to the steam generator
	renderer.draw_rectangle(med_blue, 200, 100, 320, 120, true, 0);

	// draw the pipe from the reactor to the steam generator
	renderer.draw_rectangle(med_blue, 200, 100, 320, 120, true, 0);

	// draw a label
	renderer.draw_text("Containment Building", 14, 20, 20, -9);

//	renderer.draw_text(reactor.get_date_string().c_str(), 14, 500, 700, -9);
}