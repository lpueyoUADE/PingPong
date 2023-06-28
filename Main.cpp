#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <iostream>

// Main Structs
typedef struct Position
{
	int x, y;
} Position;

// Ball and paddle Struct
typedef struct Component
{
	SDL_Rect rect;
	SDL_Surface* imageSurface;

	int velocity;
	int xDirection;
	int yDirection;
} Component;

// Text Component
typedef struct TextComponent
{
	SDL_Rect rect;
	std::string text;
	const char* font;
	int fontSize;
	SDL_Color fontColor;
	SDL_Surface* surface;
	void (*placement)(SDL_Rect&, int); // Pointer to a placement Function
} TextComponent;

enum class Screen {
	MAIN_MENU,
	GAMEPLAY,
	RESULT_MENU,
	SAME_SCREEN,
	EXIT
};

enum class Button {
	NEW_GAME,
	MAIN_MENU,
	QUIT
};

// GAME SETTINGS
const char* WINDOW_TITLE = "Ping Pong classic 1.0";
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 768;
const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;
const int MATCH_DURATION = 120;

// Difficulty
int TOO_YOUNG_TO_DIE = (int)(WINDOW_WIDTH / 3);
int ULTRA_VIOLENCE = (int)(WINDOW_WIDTH / 2);
int NIGHTMARE = (int)(WINDOW_WIDTH / 1.1);

const int DIFFICULTY_LEVEL = ULTRA_VIOLENCE;

// Initial Screen
const Screen FIRST_SCREEN = Screen::MAIN_MENU;

//SDL classes
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Mix_Music* music = NULL;
Mix_Chunk* pongSound = NULL;
Mix_Chunk* selectSound = NULL;
Mix_Chunk* navigateSound = NULL;

// Images
const char* BALL_IMAGE_PATH = "resources/img/ball.png";
const char* PADDLE_IMAGE_PATH = "resources/img/paddle.png";
const char* ICON_IMAGE_PATH = "resources/img/icon/icon.png";

// Fonts
const char* WORK_SANS_THIN = "resources/fonts/work_sans/static/WorkSans-Thin.ttf";
const char* WORK_SANS_REGULAR = "resources/fonts/work_sans/static/WorkSans-Regular.ttf";
const char* WORK_SANS_EXTRABOLD = "resources/fonts/work_sans/static/WorkSans-ExtraBold.ttf";

// Music
const char* MAIN_MENU_MUSIC_PATH = "resources/Sounds/main_menu.wav";
const char* GAMEPLAY_MUSIC_PATH = "resources/Sounds/gameplay.mp3";
const char* RESULT_MENU_MUSIC_PATH = "resources/Sounds/gameplay.mp3";

// Sounds
const char* NAVIGATE_SOUND_PATH = "resources/Sounds/navigate.mp3";
const char* PONG_SOUND_PATH = "resources/Sounds/pong.mp3";
const char* SELECT_SOUND_PATH = "resources/Sounds/select.mp3";

// Directions
const int DIRECTION_STOP = 0;
const int DIRECTION_UP = -1;
const int DIRECTION_DOWN = 1;
const int DIRECTION_LEFT = -1;
const int DIRECTION_RIGHT = 1;

void ClearMusic()
{
	Mix_FreeMusic(music);
	music = NULL;
}

void LoadAndPlayMusic(const char* path, int volume = 64)
{
	ClearMusic();
	music = Mix_LoadMUS(path);

	Mix_VolumeMusic(volume);

	Mix_PlayMusic(music, -1);
}

void PlaySoundOnce(Mix_Chunk* sound)
{
	Mix_PlayChannel(-1, sound, 0);
}

void DrawRectangle(SDL_Rect rect, SDL_Color color, bool filled = true)
{
	//Fill the surface black
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	filled ? SDL_RenderFillRect(renderer, &rect) : SDL_RenderDrawRect(renderer, &rect);
}

void DrawImage(SDL_Surface* image, int x, int y)
{
	// Create texture from surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);

	// Set position of the image
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);

	// Render the image texture
	SDL_RenderCopy(renderer, texture, nullptr, &rect);

	// Destroy Texture
	SDL_DestroyTexture(texture);
	texture = NULL;
}

void DrawTextFont(SDL_Surface* surface, int x, int y)
{
	// Create texture from surface
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	// Set position of the text
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	SDL_QueryTexture(texture, nullptr, nullptr, &rect.w, &rect.h);

	// Render the text texture
	SDL_RenderCopy(renderer, texture, nullptr, &rect);

	// Destroy texture
	SDL_DestroyTexture(texture);
	texture = NULL;
}

void PlaceMiddle(SDL_Rect& rect, int padding = 0)
{
	rect.x = (WINDOW_WIDTH - rect.w) / 2;
	rect.y = (WINDOW_HEIGHT - rect.h) / 2;
}

void PlaceLeftMiddle(SDL_Rect& rect, int padding = 0)
{
	rect.x = 0 + padding;
	rect.y = (WINDOW_HEIGHT - rect.h) / 2;
}

void PlaceRightMiddle(SDL_Rect& rect, int padding = 0)
{
	rect.x = WINDOW_WIDTH - rect.w - padding;
	rect.y = (WINDOW_HEIGHT - rect.h) / 2;
}

void PlaceRightBottom(SDL_Rect& rect, int padding = 0)
{
	rect.x = WINDOW_WIDTH - rect.w - padding;
	rect.y = WINDOW_HEIGHT - rect.h - padding;
}
void PlaceMiddleBottom(SDL_Rect& rect, int padding = 0)
{
	rect.x = (WINDOW_WIDTH - rect.w) / 2;
	rect.y = WINDOW_HEIGHT - rect.h - padding;
}

void PlaceMiddleTop(SDL_Rect& rect, int padding = 0)
{
	rect.x = (WINDOW_WIDTH - rect.w) / 2;
	rect.y = 0 + padding;
}

Component CreateComponent(Position position, const char* imagePath) {

	SDL_Surface* imageSurface = IMG_Load(imagePath);
	return {
		{
			position.x,
			position.y,
			imageSurface->clip_rect.w,
			imageSurface->clip_rect.h,
		},
		imageSurface
	};
}

TextComponent CreateTextComponent(Position position, std::string text, const char* font, int size, SDL_Color color, void (*placement)(SDL_Rect&, int)) {

	TTF_Font* ttfFont = TTF_OpenFont(font, size);
	SDL_Surface* surface = TTF_RenderText_Blended(ttfFont, text.c_str(), color);
	TTF_CloseFont(ttfFont);
	ttfFont = NULL;
	return {
		{
			position.x,
			position.y,
			surface->clip_rect.w,
			surface->clip_rect.h,
		},
		text,
		font,
		size,
		color,
		surface,
		placement,
	};
}

void FreeTextComponent(TextComponent& c)
{
	SDL_FreeSurface(c.surface);
	c.surface = NULL;
}

void DrawComponent(Component c) {
	DrawImage(c.imageSurface, c.rect.x, c.rect.y);
}

void DrawTextComponent(TextComponent& c, int padding) {
	FreeTextComponent(c);
	c = CreateTextComponent({ c.rect.x, c.rect.y }, c.text, c.font, c.fontSize, c.fontColor, c.placement);
	c.placement(c.rect, padding);
	DrawTextFont(c.surface, c.rect.x, c.rect.y);
}

void FreeComponent(Component c)
{
	SDL_FreeSurface(c.imageSurface);
	c.imageSurface = NULL;
}

void MoveComponent(Component& c)
{
	c.rect.x += c.velocity * c.xDirection;
	c.rect.y += c.velocity * c.yDirection;
}
bool Init()
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

	// Initialize SDL_mixer with our audio format
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0)
	{
		printf("Error initializing SDL_mixer: %s\n", Mix_GetError());
		exit(EXIT_FAILURE);
	}

	//Create window
	window = SDL_CreateWindow(
		WINDOW_TITLE,
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN
	);

	if (window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// Icon
	SDL_Surface* icon = IMG_Load(ICON_IMAGE_PATH);
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);
	icon = NULL;

	// Create Renderer
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);

	if (renderer == NULL)
	{
		printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		exit(EXIT_FAILURE);
	}

	// Set drawing color to black
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	return true;
}

typedef struct MainMenuState
{
	// Main conditions
	bool initialized = false;

	// Labels
	TextComponent titleLabel;
	TextComponent subTitleLabel;
	TextComponent newGameLabel;
	TextComponent quitLabel;
	TextComponent signatureLabel;

	// Highlights
	int highlighedFontSize;
	SDL_Color highlightedColor;

	int regularFontSize;
	SDL_Color regularColor;

	Button selectedButton;

	// Window Padding
	int padding;

	// Screen Swap
	Screen nextScreen;

}MainMenuState;

typedef struct GameplayMenuState
{
	// Main conditions
	bool newMatch; // A new match takes place.
	bool newRound; // The player or the enemy Scored.
	bool waitingToBegin; // Waiting for user input to start the round.

	// Score
	int playerPoints = 0;
	int enemyPoints = 0;

	// Timer
	Uint32 timeAccumulated;
	Uint32 timeParcial;
	Uint32 currentTime;

	// Ball and Paddles
	Component ball;
	Component player;
	Component enemy;

	// Labels
	TextComponent helpLabel;
	TextComponent scoreLabel;
	TextComponent timeLabel;

	std::string WAIT_TO_BEGIN_MESSAGE = "Presione ENTER para comenzar";
	std::string PLAYING_MESSAGE = "Jugando";

	// Window Padding
	int padding;

	// Frame Borders
	SDL_Rect TOP;
	SDL_Rect RIGHT;
	SDL_Rect BOTTOM;
	SDL_Rect LEFT;

	// Enemy ""AI""
	int gameTicks;
	int actionDelay;
	
	// Ball Velocity
	int intialBallVelocity;

	int movementActivationDistance;

	// Screen Swap
	Screen nextScreen;

}GameplayMenuState;

typedef struct ResultMenuState
{
	// Main conditions
	bool initialized = false;
	int playerPoints;
	int enemyPoints;

	// Labels
	TextComponent resultScoreLabel;
	TextComponent resultTextLabel;
	TextComponent mainMenuLabel;
	TextComponent quitLabel;

	// Highlights
	int highlighedFontSize;
	SDL_Color highlightedColor;

	int regularFontSize;
	SDL_Color regularColor;

	SDL_Color winColor;
	SDL_Color drawColor;
	SDL_Color loseColor;

	Button selectedButton;

	// Window Padding
	int padding;

	// Screen Swap
	Screen nextScreen;

}ResultMenuState;

bool CheckCollision(const SDL_Rect& a, const SDL_Rect& b)
{
	// The sides of the rectangles
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	// Calculate the sides of rectA
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	// Calculate the sides of rectB
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	// Check for collision
	if (bottomA <= topB || topA >= bottomB || rightA <= leftB || leftA >= rightB)
	{
		return false;
	}

	return true;
}

int getMiddleHeight(SDL_Rect rect)
{
	return (rect.y + rect.h) / 2;
}

void EnemyMovement(GameplayMenuState& state)
{
	state.gameTicks = (state.gameTicks + 1) % state.actionDelay;
	if (state.gameTicks == state.actionDelay - 1)
	{
		if (state.ball.rect.x < state.movementActivationDistance)
		{
			state.enemy.yDirection = state.ball.yDirection == 1 ? 1 : -1;
		}

		else {
			state.enemy.yDirection = 0;
		}
	}
}

std::string RenderPoints(int enemyPoints, int playerPoints)
{
	return std::to_string(enemyPoints) + " - " + std::to_string(playerPoints);
}

void InitMainMenu(MainMenuState& state)
{
	// Initial States
	state.initialized = true;
	state.regularFontSize = 50;
	state.regularColor = { 255,255,255 };
	state.highlighedFontSize = 70;
	state.highlightedColor = { 0,255,0 };

	state.selectedButton = Button::NEW_GAME;

	// Padding
	state.padding = 15;

	// Create Text Components
	state.titleLabel = CreateTextComponent(
		{ 0,0 },
		"PONG",
		WORK_SANS_EXTRABOLD,
		100,
		{ 255,255,255,255 },
		&PlaceMiddleTop
	);
	state.subTitleLabel = CreateTextComponent(
		{ 0,0 },
		"Classic V1.0",
		WORK_SANS_THIN,
		24,
		{ 255,255,255,255 },
		&PlaceMiddleTop
	);

	state.newGameLabel = CreateTextComponent(
		{ 0,0 },
		"JUGAR",
		WORK_SANS_EXTRABOLD,
		state.regularFontSize,
		state.regularColor,
		&PlaceMiddle
	);
	state.quitLabel = CreateTextComponent(
		{ 0,0 },
		"SALIR",
		WORK_SANS_EXTRABOLD,
		state.regularFontSize,
		state.regularColor,
		&PlaceMiddleBottom
	);

	state.signatureLabel = CreateTextComponent(
		{ 0,0 },
		"Pueyo Luciano - Introducción a la Programación - UADE 1er Cuatrimestre 2023",
		WORK_SANS_THIN,
		14,
		{ 255,255,255,255 },
		&PlaceRightBottom
	);

	// Load Music
	LoadAndPlayMusic(MAIN_MENU_MUSIC_PATH, 32);

	// Screen Swap
	state.nextScreen = Screen::SAME_SCREEN;
}

void InitGamePlay(GameplayMenuState& state)
{
	// Initial States
	state.newMatch = false;
	state.newRound = false;
	state.waitingToBegin = true;
	state.playerPoints = 0;
	state.enemyPoints = 0;
	state.intialBallVelocity = 5;

	// timer
	state.timeAccumulated = 0;
	state.timeParcial = 0;
	state.currentTime = 0;

	// Enemy AI
	state.actionDelay = 5;
	state.gameTicks = 0;
	state.movementActivationDistance = DIFFICULTY_LEVEL;

	// window Padding
	state.padding = 15;

	// Frame Borders
	state.TOP = { 0,0,WINDOW_WIDTH, state.padding };
	state.RIGHT = { WINDOW_WIDTH - state.padding, 0,state.padding, WINDOW_HEIGHT };
	state.BOTTOM = { 0,WINDOW_HEIGHT - state.padding ,WINDOW_WIDTH, WINDOW_HEIGHT };
	state.LEFT = { 0,0,state.padding, WINDOW_HEIGHT };

	// Create Components
	state.ball = CreateComponent({ 0, 0 }, BALL_IMAGE_PATH);
	state.player = CreateComponent({ 0, 0 }, PADDLE_IMAGE_PATH);
	state.enemy = CreateComponent({ 0, 0 }, PADDLE_IMAGE_PATH);

	// Ball properties
	state.ball.velocity = state.intialBallVelocity;
	state.ball.xDirection = DIRECTION_LEFT;
	state.ball.yDirection = DIRECTION_UP;

	// paddles properties
	state.player.velocity = 7;
	state.player.xDirection = DIRECTION_STOP;
	state.player.yDirection = DIRECTION_STOP;

	state.enemy.velocity = 5;
	state.enemy.xDirection = DIRECTION_STOP;
	state.enemy.yDirection = DIRECTION_STOP;

	// Create Text Components
	state.helpLabel = CreateTextComponent(
		{ 0,0 },
		state.WAIT_TO_BEGIN_MESSAGE,
		WORK_SANS_REGULAR,
		15,
		{ 255,255,255,255 },
		&PlaceMiddleBottom
	);
	state.scoreLabel = CreateTextComponent(
		{ 0,0 },
		RenderPoints(state.enemyPoints, state.playerPoints),
		WORK_SANS_EXTRABOLD,
		50,
		{ 255,255,255,255 },
		&PlaceMiddleTop
	);

	state.timeLabel = CreateTextComponent(
		{ 0,0 },
		std::to_string(MATCH_DURATION),
		WORK_SANS_THIN,
		32,
		{ 200,200,200,255 },
		&PlaceMiddleTop
	);

	// Place components
	PlaceMiddle(state.ball.rect);
	PlaceRightMiddle(state.player.rect, state.padding);
	PlaceLeftMiddle(state.enemy.rect, state.padding);

	// Load Music
	LoadAndPlayMusic(GAMEPLAY_MUSIC_PATH, 32);

	// Load sounds
	pongSound = Mix_LoadWAV(PONG_SOUND_PATH);
	selectSound = Mix_LoadWAV(SELECT_SOUND_PATH);
	navigateSound = Mix_LoadWAV(NAVIGATE_SOUND_PATH);

	// Screen Swap
	state.nextScreen = Screen::SAME_SCREEN;
}

void SetNewRoundGamePlay(GameplayMenuState& state)
{
	state.helpLabel.text = state.WAIT_TO_BEGIN_MESSAGE;
	state.newRound = false;
	state.waitingToBegin = true;
	// timer

	// Ball properties
	state.ball.velocity = state.intialBallVelocity;
	state.ball.xDirection = DIRECTION_RIGHT;
	state.ball.yDirection = DIRECTION_UP;

	// Paddles properties
	state.player.yDirection = DIRECTION_STOP;
	state.enemy.yDirection = DIRECTION_STOP;

	// Enemy AI
	state.gameTicks = 0;

	state.scoreLabel.text = RenderPoints(state.enemyPoints, state.playerPoints);

	PlaceMiddle(state.ball.rect);
	PlaceRightMiddle(state.player.rect, state.padding);
	PlaceLeftMiddle(state.enemy.rect, state.padding);
}

void InitResultMenu(ResultMenuState& state)
{
	// Initial States
	state.initialized = true;

	// HighLights
	state.regularFontSize = 50;
	state.regularColor = { 255,255,255 };
	state.highlighedFontSize = 70;
	state.highlightedColor = { 0,255,0 };

	state.winColor = { 0,255,0 };
	state.drawColor = { 200,200,200 };
	state.loseColor = { 255,0,0 };

	state.selectedButton = Button::NEW_GAME;

	// Padding
	state.padding = 15;

	// Render results
	std::string result = RenderPoints(state.enemyPoints, state.playerPoints);

	std::string resultText;
	SDL_Color resultColor;

	if (state.enemyPoints > state.playerPoints)
	{
		resultText = "PERDISTE :(";
		resultColor = state.loseColor;
	}
	else if (state.enemyPoints < state.playerPoints)
	{
		resultText = "GANASTE :D";
		resultColor = state.winColor;
	}
	else
	{
		resultText = "EMPATE (._.)";
		resultColor = state.drawColor;
	}

	// Create Text Components
	state.resultScoreLabel = CreateTextComponent(
		{ 0,0 },
		result,
		WORK_SANS_EXTRABOLD,
		100,
		{ 255,255,255,255 },
		&PlaceMiddleTop
	);
	state.resultTextLabel = CreateTextComponent(
		{ 0,0 },
		resultText,
		WORK_SANS_REGULAR,
		24,
		resultColor,
		&PlaceMiddleTop
	);

	state.mainMenuLabel = CreateTextComponent(
		{ 0,0 },
		"MENU PRINCIPAL",
		WORK_SANS_EXTRABOLD,
		state.regularFontSize,
		state.regularColor,
		&PlaceMiddle
	);

	state.quitLabel = CreateTextComponent(
		{ 0,0 },
		"SALIR",
		WORK_SANS_EXTRABOLD,
		state.regularFontSize,
		state.regularColor,
		&PlaceMiddleBottom
	);

	// Load Music
	LoadAndPlayMusic(MAIN_MENU_MUSIC_PATH, 32);

	// Screen Swap
	state.nextScreen = Screen::SAME_SCREEN;
}

void ExitMainMenu(MainMenuState& state)
{
	// Free text Components
	FreeTextComponent(state.titleLabel);
	FreeTextComponent(state.subTitleLabel);
	FreeTextComponent(state.newGameLabel);
	FreeTextComponent(state.quitLabel);
	FreeTextComponent(state.signatureLabel);
}

void ExitGamePlay(GameplayMenuState& state)
{
	// Free Components
	FreeComponent(state.ball);
	FreeComponent(state.player);
	FreeComponent(state.enemy);

	// Free text Components
	FreeTextComponent(state.helpLabel);
	FreeTextComponent(state.scoreLabel);
	FreeTextComponent(state.timeLabel);
}

void ExitResultMenu(ResultMenuState& state)
{
	// Free text Components
	FreeTextComponent(state.resultScoreLabel);
	FreeTextComponent(state.resultTextLabel);
	FreeTextComponent(state.mainMenuLabel);
	FreeTextComponent(state.quitLabel);
}

void MainMenuHandleEvent(SDL_Event event, MainMenuState& state) {
	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_RETURN:
			switch (state.selectedButton)
			{
			case Button::NEW_GAME:
				state.nextScreen = Screen::GAMEPLAY;
				break;

			case Button::QUIT:
				state.nextScreen = Screen::EXIT;
				break;
			}
			break;

		case SDLK_UP:
		case SDLK_DOWN:
			state.selectedButton = state.selectedButton == Button::NEW_GAME ? Button::QUIT : Button::NEW_GAME;
			break;
		}
	default:
		break;
	}
}

void GamePlayHandleEvent(SDL_Event event, GameplayMenuState& state) {
	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_RETURN:
			if (state.waitingToBegin) {
				state.waitingToBegin = false;
				state.helpLabel.text = state.PLAYING_MESSAGE;
			}
			break;
		case SDLK_UP:
			if (!state.waitingToBegin) {
				state.player.yDirection = DIRECTION_UP;

			}
			break;
		case SDLK_DOWN:
			if (!state.waitingToBegin) {
				state.player.yDirection = DIRECTION_DOWN;
			}
			break;
		}
		break;

	case SDL_KEYUP:
		switch (event.key.keysym.sym) {
		case SDLK_UP:
			if (!state.waitingToBegin) {
				state.player.yDirection = DIRECTION_STOP;

			}
			break;
		case SDLK_DOWN:
			if (!state.waitingToBegin) {
				state.player.yDirection = DIRECTION_STOP;
			}
			break;
		}
		break;

	default:
		break;
	}
}

void ResultMenuHandleEvent(SDL_Event event, ResultMenuState& state) {
	switch (event.type)
	{
	case SDL_KEYDOWN:
		switch (event.key.keysym.sym) {
		case SDLK_RETURN:
			switch (state.selectedButton)
			{
				case Button::MAIN_MENU:
				state.nextScreen = Screen::MAIN_MENU;
				break;

			case Button::QUIT:
				state.nextScreen = Screen::EXIT;
				break;
			}
			break;

		case SDLK_UP:
		case SDLK_DOWN:
			state.selectedButton = state.selectedButton == Button::MAIN_MENU ? Button::QUIT : Button::MAIN_MENU;
			break;
		}
	default:
		break;
	}
}

void HighlightSelectedOptionMainMenu(MainMenuState& state)
{
	switch (state.selectedButton)
	{
	case Button::NEW_GAME:
		state.newGameLabel.fontSize = state.highlighedFontSize;
		state.newGameLabel.fontColor = state.highlightedColor;
		state.quitLabel.fontSize = state.regularFontSize;
		state.quitLabel.fontColor = state.regularColor;
		break;

	case Button::QUIT:
		state.newGameLabel.fontSize = state.regularFontSize;
		state.newGameLabel.fontColor = state.regularColor;
		state.quitLabel.fontSize = state.highlighedFontSize;
		state.quitLabel.fontColor = state.highlightedColor;
		break;
	}
}

void HighlightSelectedOptionResultMenu(ResultMenuState& state)
{
	switch (state.selectedButton)
	{
	case Button::MAIN_MENU:
		state.mainMenuLabel.fontSize = state.highlighedFontSize;
		state.mainMenuLabel.fontColor = state.highlightedColor;
		state.quitLabel.fontSize = state.regularFontSize;
		state.quitLabel.fontColor = state.regularColor;
		break;

	case Button::QUIT:
		state.mainMenuLabel.fontSize = state.regularFontSize;
		state.mainMenuLabel.fontColor = state.regularColor;
		state.quitLabel.fontSize = state.highlighedFontSize;
		state.quitLabel.fontColor = state.highlightedColor;
		break;
	}
}

Screen MainMenuLogic(MainMenuState& state)
{
	if (!state.initialized)
	{
		InitMainMenu(state);
	}

	HighlightSelectedOptionMainMenu(state);

	// Draw text components
	DrawTextComponent(state.titleLabel, state.padding);
	DrawTextComponent(state.subTitleLabel, state.padding + state.titleLabel.rect.h);
	DrawTextComponent(state.newGameLabel, state.padding);
	DrawTextComponent(state.quitLabel, state.padding + WINDOW_HEIGHT / 3);
	DrawTextComponent(state.signatureLabel, state.padding);

	return state.nextScreen;
}

Screen GamePlayLogic(GameplayMenuState& state)
{
	if (state.newMatch)
	{
		InitGamePlay(state);
		state.currentTime = SDL_GetTicks();
	}

	if (state.newRound)
	{
		SetNewRoundGamePlay(state);
		state.timeAccumulated = state.timeParcial;
	}

	DrawComponent(state.ball);
	DrawComponent(state.player);
	DrawComponent(state.enemy);
	DrawTextComponent(state.helpLabel, state.padding);
	DrawTextComponent(state.scoreLabel, state.padding);
	DrawTextComponent(state.timeLabel, state.padding + state.scoreLabel.rect.h);

	// Show Frame Window Colliders

	/*
	DrawRectangle(state.TOP, {255,0,0,255}, false);
	DrawRectangle(state.RIGHT, { 255,0,0,255 }, false);
	DrawRectangle(state.BOTTOM, { 255,0,0,255 }, false);
	DrawRectangle(state.LEFT, { 255,0,0,255 }, false);
	*/

	if (state.waitingToBegin)
	{
		state.currentTime = SDL_GetTicks();
		return state.nextScreen;
	}

	// Timer
	Uint32 elapsedTicks = SDL_GetTicks() - state.currentTime;
	state.timeParcial = (int)(elapsedTicks * 0.001f) + state.timeAccumulated;

	int timeLeft = MATCH_DURATION - state.timeParcial;
	state.timeLabel.text = std::to_string(timeLeft);

	if (timeLeft <= 0) {
		state.nextScreen = Screen::RESULT_MENU;
		return state.nextScreen;
	}

	// Move ball and Paddles
	MoveComponent(state.ball);
	MoveComponent(state.player);
	EnemyMovement(state);
	MoveComponent(state.enemy);

	// Check collisions
	// Ball and Borders
	if (CheckCollision(state.ball.rect, state.player.rect))
	{
		state.ball.xDirection = DIRECTION_LEFT;
		state.ball.rect.x = state.player.rect.x - state.ball.rect.w;
		state.ball.velocity++;
		PlaySoundOnce(pongSound);
	}

	if (CheckCollision(state.ball.rect, state.enemy.rect))
	{
		state.ball.xDirection = DIRECTION_RIGHT;
		state.ball.rect.x = state.enemy.rect.x + state.enemy.rect.w + 1;
		state.ball.velocity++;
		PlaySoundOnce(pongSound);
	}

	// Frames
	if (CheckCollision(state.ball.rect, state.TOP))
	{
		state.ball.yDirection = DIRECTION_DOWN;
		state.ball.rect.y = state.TOP.y + state.TOP.h + 1;
		PlaySoundOnce(pongSound);
	}

	if (CheckCollision(state.ball.rect, state.RIGHT))
	{
		state.ball.xDirection = DIRECTION_LEFT;
		state.ball.rect.x = state.RIGHT.x - state.ball.rect.w;
		state.newRound = true;
		state.enemyPoints++;
		PlaySoundOnce(pongSound);
	}

	if (CheckCollision(state.ball.rect, state.BOTTOM))
	{
		state.ball.yDirection = DIRECTION_UP;
		state.ball.rect.y = state.BOTTOM.y - state.ball.rect.h;
		PlaySoundOnce(pongSound);
	}

	if (CheckCollision(state.ball.rect, state.LEFT))
	{
		state.ball.xDirection = DIRECTION_DOWN;
		state.ball.rect.x = state.LEFT.x + state.LEFT.w + 1;
		state.newRound = true;
		state.playerPoints++;
		PlaySoundOnce(pongSound);
	}

	// Player Paddle and Borders
	if (CheckCollision(state.player.rect, state.TOP))
	{
		state.player.yDirection = DIRECTION_STOP;
		state.player.rect.y = state.TOP.h;
	}
	if (CheckCollision(state.player.rect, state.BOTTOM))
	{
		state.player.yDirection = DIRECTION_STOP;
		state.player.rect.y = state.BOTTOM.y - state.player.rect.h;
	}

	// enemy Paddle and Borders
	if (CheckCollision(state.enemy.rect, state.TOP))
	{
		state.enemy.yDirection = DIRECTION_STOP;
		state.enemy.rect.y = state.TOP.h;
	}
	if (CheckCollision(state.enemy.rect, state.BOTTOM))
	{
		state.enemy.yDirection = DIRECTION_STOP;
		state.enemy.rect.y = state.BOTTOM.y - state.enemy.rect.h;
	}

	return state.nextScreen;
}

Screen ResultMenuLogic(ResultMenuState& state)
{
	if (!state.initialized)
	{
		InitResultMenu(state);
	}

	HighlightSelectedOptionResultMenu(state);

	// Draw text components
	DrawTextComponent(state.resultScoreLabel, state.padding);
	DrawTextComponent(state.resultTextLabel, state.padding + state.resultScoreLabel.rect.h);
	DrawTextComponent(state.mainMenuLabel, state.padding);
	DrawTextComponent(state.quitLabel, state.padding + WINDOW_HEIGHT / 3);

	return state.nextScreen;
}

bool HandleScreenSwap(Screen& currentScreen, Screen& nextScreen, MainMenuState& mmState, GameplayMenuState& gpState,ResultMenuState& rmState)
{
	if (currentScreen == nextScreen || nextScreen == Screen::SAME_SCREEN)
	{
		return true;
	}

	if (nextScreen == Screen::EXIT)
	{
		return false;
	}

	switch (currentScreen)
	{
	case Screen::MAIN_MENU:
		ExitMainMenu(mmState);
		break;

	case Screen::GAMEPLAY:
		ExitGamePlay(gpState);
		break;

	case Screen::RESULT_MENU:
		ExitResultMenu(rmState);
		break;

	default:
		break;
	}
	switch (nextScreen)
	{
	case Screen::MAIN_MENU:
		mmState.initialized = false;
		break;

	case Screen::GAMEPLAY:
		gpState.newMatch = true;
		break;

	case Screen::RESULT_MENU:
		rmState.initialized = false;
		rmState.playerPoints = gpState.playerPoints;
		rmState.enemyPoints = gpState.enemyPoints;
		break;

	default:
		break;
	}

	currentScreen = nextScreen;

	return true;
}

void MainLoop()
{
	SDL_Event e;
	bool running = true;

	// Menu Selection
	Screen currentScreen = FIRST_SCREEN;

	// Screen's states
	GameplayMenuState gameplayState;
	MainMenuState mainMenuState;
	ResultMenuState resultMenuState;

	while (running)
	{
		// Clear the window to white
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		Screen nextScreen;

		// Event Loop
		while (SDL_PollEvent(&e))
		{
			
			if (e.type == SDL_QUIT)
			{
				running = false;
				break;
			}

			switch (currentScreen)
			{
			case Screen::MAIN_MENU:
				MainMenuHandleEvent(e, mainMenuState);
				break;

			case Screen::GAMEPLAY:
				GamePlayHandleEvent(e, gameplayState);
				break;

			case Screen::RESULT_MENU:
				ResultMenuHandleEvent(e, resultMenuState);
				break;

			default:
				break;
			}
		}
		switch (currentScreen)
		{
		case Screen::MAIN_MENU:
			nextScreen = MainMenuLogic(mainMenuState);
			break;

		case Screen::GAMEPLAY:
			nextScreen = GamePlayLogic(gameplayState);
			break;

		case Screen::RESULT_MENU:
			nextScreen = ResultMenuLogic(resultMenuState);
			break;
		}
		
		running = running && HandleScreenSwap(currentScreen, nextScreen, mainMenuState, gameplayState, resultMenuState);

		SDL_RenderPresent(renderer);
	}
}

void Quit()
{
	//Destroy window
	SDL_DestroyWindow(window);
	window = NULL;

	// Destroy Renderer
	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	// Destroy Music 
	ClearMusic();

	// Destroy Sounds
	Mix_FreeChunk(pongSound);
	pongSound = NULL;

	Mix_FreeChunk(selectSound);
	selectSound = NULL;

	Mix_FreeChunk(navigateSound);
	navigateSound = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	Mix_Quit();
	SDL_Quit();
}
int main(int argc, char* args[])
{
	Init();
	MainLoop();
	Quit();

	exit(EXIT_SUCCESS);
}
