#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
using namespace std;

SDL_Window* window = NULL;
SDL_Texture* texture = NULL;
SDL_Renderer* renderer = NULL;

// Mix_Music* music = NULL;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

void text_exmaple()
{
	SDL_Color color = { 255, 255, 255 };

	TTF_Font* font = TTF_OpenFont("resources/fonts/work_sans/static/WorkSans-Regular.ttf", 24);
	SDL_Surface* surface = TTF_RenderText_Blended(font, "Hello, SDL!", color);

	// Create texture from surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);  // Free the surface

	// Set position of the text
	SDL_Rect destinationRect;
	destinationRect.x = 200;
	destinationRect.y = 200;
	SDL_QueryTexture(texture, nullptr, nullptr, &destinationRect.w, &destinationRect.h);

	// Render the text texture
	SDL_RenderCopy(renderer, texture, nullptr, &destinationRect);

	// Update screen
	SDL_RenderPresent(renderer);
}

int image_sample()
{
	// Load PNG image
	SDL_Surface* surface = IMG_Load("resources/img/ball.png");
	if (!surface) {
		// Handle image loading error
		return 1;
	}

	// Create texture from surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);  // Free the surface

	 // Set initial position of the image
	SDL_Rect destinationRect;
	destinationRect.x = 100;
	destinationRect.y = 100;
	SDL_QueryTexture(texture, nullptr, nullptr, &destinationRect.w, &destinationRect.h);

	// Render the image texture
	SDL_RenderCopy(renderer, texture, nullptr, &destinationRect);

	// Update screen
	SDL_RenderPresent(renderer);
}

bool init(const char* window_title, int width, int height)
{
	// Hide console Window
	ShowWindow(GetConsoleWindow(), SW_HIDE); //SW_RESTORE to bring back

	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	// Initialize TTF
	if (TTF_Init() < 0) {
		printf("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
		exit(EXIT_FAILURE);
	}

	// Initialize IMG
	if (IMG_Init(IMG_INIT_PNG) < 0)
	{
		printf("Error initializing SDL_image: %s\n", IMG_GetError());
		exit(EXIT_FAILURE);
	}

	/*
	// Initialize SDL_mixer with our audio format
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
	{
		printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
		exit(EXIT_FAILURE);
	}
	*/

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
	
	// Create Renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	
	if (renderer == NULL)
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}


	// Set drawing color to black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	// rectangle example
	SDL_Rect rect;
	rect.x = 100;
	rect.y = 100;
	rect.w = 200;
	rect.h = 150;

	//Fill the surface black
	SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
	SDL_RenderFillRect(renderer, &rect);
	
	// Update window
	SDL_RenderPresent(renderer);


	// image Test
	image_sample();	

	// text Test
	text_exmaple();

	return true;
}

void main_loop()
{
	SDL_Event e;
	bool running = true;
	Uint32 frameStart, frameTime;

	while(running)
	{
		frameStart = SDL_GetTicks();
		
		// Event Loop
		while(SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				running = false;
		}

		frameTime = SDL_GetTicks() - frameStart;
		// Delay if necessary to cap the frame rate
		if (frameTime < SCREEN_TICKS_PER_FRAME) {
			SDL_Delay(SCREEN_TICKS_PER_FRAME - frameTime);
		}
	}
}

void quit()
{
	//Destroy window
	SDL_DestroyWindow(window);
	window = NULL;

	// Destroy Renderer
	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	// Destroy texture
	SDL_DestroyTexture(texture);
	texture = NULL;

	// Destroy Music 
	// Mix_FreeMusic(music);
	// music = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	// Mix_Quit();
	SDL_Quit();
}
int main(int argc, char* args[])
{
	init("SDL Tutorial", 960, 540);
	main_loop();
	quit();

	exit(EXIT_SUCCESS);
}
