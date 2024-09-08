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

	Init(&context);

	while (!quit)
	{
		context.LeftPressed = 0;
		context.RightPressed = 0;
		context.DownPressed = 0;
		context.UpPressed = 0;

		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			if (evt.type == SDL_EVENT_QUIT) {
				Quit(&context);
				quit = 1;
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

		if (Draw(&context) < 0)
		{
			SDL_Log("Draw failed!");
			return 1;
		}
		
		SDL_Delay(1000 / 60);
	}

	return 0;
}
