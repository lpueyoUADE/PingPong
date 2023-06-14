#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
using namespace std;

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

void init(const char* window_title, int width, int height)
{
	// Hide console Window
	ShowWindow(GetConsoleWindow(), SW_HIDE); //SW_RESTORE to bring back

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	//Create window
	window = SDL_CreateWindow(
		window_title,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width,
		height,
		SDL_WINDOW_SHOWN
	);

	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	//Get window surface
	screenSurface = SDL_GetWindowSurface(window);

	//Fill the surface white
	SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));

	//Update the surface
	SDL_UpdateWindowSurface(window);
}

void main_loop()
{
	SDL_Event e;
	bool running = true;

	while(running)
	{
		while(SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				running = false;
		}
	}
}

void quit()
{
	//Destroy window
	SDL_DestroyWindow(window);

	//Quit SDL subsystems
	SDL_Quit();
}
int main(int argc, char* args[])
{
	init("SDL Tutorial", 960, 540);
	main_loop();
	quit();

	exit(EXIT_SUCCESS);
}
