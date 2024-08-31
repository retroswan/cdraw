#include "Common.h"
#include <SDL3/SDL_main.h>

extern int Init(Context* context);
extern int Update(Context* context);
extern int Draw(Context* context);
extern void Quit(Context* context);

int main(int argc, char **argv)
{
	Context context = {
		.ExampleName = "cdraw",
	};
	int quit = 0;
	float lastTime = 0;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
		return 1;
	}

	InitializeAssetLoader();

	SDL_bool canDraw = SDL_TRUE;
	
	Init(&context);

	while (!quit)
	{
		context.LeftPressed = 0;
		context.RightPressed = 0;
		context.DownPressed = 0;
		context.UpPressed = 0;

		SDL_Event evt;
		while (SDL_PollEvent(&evt))
		{
			if (evt.type == SDL_EVENT_QUIT)
			{
				Quit(&context);
				quit = 1;
			}
			else if (evt.type == SDL_EVENT_USER)
			{
				if (evt.user.code == 0)
				{
#ifdef SDL_PLATFORM_GDK
					SDL_GDKSuspendGPU(context.Device);
					canDraw = SDL_FALSE;
					SDL_GDKSuspendComplete();
#endif
				}
				else if (evt.user.code == 1)
				{
#ifdef SDL_PLATFORM_GDK
					SDL_GDKResumeGPU(context.Device);
					canDraw = SDL_TRUE;
#endif
				}
			}
			else if (evt.type == SDL_EVENT_KEY_DOWN)
			{
				if (evt.key.key == SDLK_LEFT)
				{
					context.LeftPressed = SDL_TRUE;
				}
				else if (evt.key.key == SDLK_RIGHT)
				{
					context.RightPressed = SDL_TRUE;
				}
				else if (evt.key.key == SDLK_DOWN)
				{
					context.DownPressed = SDL_TRUE;
				}
				else if (evt.key.key == SDLK_UP)
				{
					context.UpPressed = SDL_TRUE;
				}
			}
		}
		if (quit)
		{
			break;
		}

		float newTime = SDL_GetTicks() / 1000.0f;
		context.DeltaTime = newTime - lastTime;
		lastTime = newTime;

		if (Update(&context) < 0)
		{
			SDL_Log("Update failed!");
			return 1;
		}

		if (canDraw)
		{
			if (Draw(&context) < 0)
			{
				SDL_Log("Draw failed!");
				return 1;
			}
		}
	}

	return 0;
}
