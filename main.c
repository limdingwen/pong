#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>
#include <stdio.h>

// Vector data types

typedef struct vector2i {
	int x;
	int y;
} vector2i;

// Geometry data types

typedef struct recti {
	int x;
	int y;
	int width;
	int height;
} recti;

// Functions

int inRect(vector2i point, recti rect) {
	return
		point.x >= rect.x && point.x <= rect.x + rect.width &&
		point.y >= rect.y && point.y <= rect.y + rect.height;
}

recti spriteRect(vector2i position, vector2i size) {
	return (recti){
		position.x - size.x / 2, 
		position.y - size.y / 2,
		size.x,
		size.y};
}

SDL_Rect sdlRect(recti rect) {
	return (SDL_Rect){rect.x, rect.y, rect.width, rect.height};
}

// Main

int main(int argc, char** argv) {
	const int EXIT_OK = 0;
	const int EXIT_ERR = -1;

	const double FRAME_MIN_DELTA = 1/60;

	const vector2i SCREEN_SIZE = (vector2i){640, 400};
	
	const vector2i SEPARATOR_SIZE = (vector2i){2, 400};
	const vector2i PADDLE_SIZE = (vector2i){8, 32};
	const vector2i BALL_SIZE = (vector2i){8, 8};
	
	const int PADDLE_PIXELS_FROM_BOUNDARY = 20;
	
	const vector2i BALL_INITIAL_POSITION = (vector2i){
		SCREEN_SIZE.x / 2,
		SCREEN_SIZE.y / 2};
	const vector2i BALL_INITIAL_SPEED = (vector2i){-1, 0}; // Pixels per frame

	// ----------
	// INITIALIZATION
	// ----------

	// Initialize SDL and create window

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Window* window = SDL_CreateWindow(
		"Pong", 
		SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, 
		SCREEN_SIZE.x, 
		SCREEN_SIZE.y, 
		SDL_WINDOW_SHOWN);
	if (!window) {
		printf("Couldn't create window: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	SDL_Surface* screenSurface = SDL_GetWindowSurface(window);
	
	// Initialize scene surfaces
	
	SDL_Surface* separatorSurface = IMG_Load("resources/separator.png");
	if (!separatorSurface) {
		printf("Couldn't create separator: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Surface* leftPaddleSurface = IMG_Load("resources/left_pallete.png");
	if (!leftPaddleSurface) {
		printf("Couldn't create left paddle: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Surface* rightPaddleSurface = IMG_Load("resources/right_pallete.png");
	if (!rightPaddleSurface) {
		printf("Couldn't create right paddle: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Surface* ballSurface = IMG_Load("resources/ball.png");
	if (!ballSurface) {
		printf("Couldn't create separator: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	// Initalize scene variables
	
	// Positions are centered
	
	vector2i ballPosition = BALL_INITIAL_POSITION;
	vector2i ballSpeed = BALL_INITIAL_SPEED;
	
	// ----------
	// MAIN LOOP
	// ----------
	
	// Initialize loop
	
	int running = 1;
	
	while (running) {
		
		// ----------
		// HANDLE EVENTS
		// ----------
		
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				running = 0;
			}
		}
		
		// ----------
		// UPDATE FRAME
		// ----------
		
		// Update ball position
		
		ballPosition.x += ballSpeed.x;
		ballPosition.y += ballSpeed.y;
		
		// Check if ball is out of bounds horizontally
		// If so, reset position, speed and add score
		
		if (ballPosition.x < 0 || ballPosition.x > SCREEN_SIZE.x) {
			ballPosition = BALL_INITIAL_POSITION;
		}
		
		// Check if ball is out of bounds vertically
		// If so, bounce ball vertically
		
		if (ballPosition.y < 0 || ballPosition.y > SCREEN_SIZE.y) {
			ballSpeed.y = -ballSpeed.y;
		}
		
		// Check if ball is hitting a paddle
		// If so, set ball's horizontal direction
		
		if (inRect(ballPosition, spriteRect((vector2i){
			PADDLE_PIXELS_FROM_BOUNDARY, 
			SCREEN_SIZE.y / 2}, PADDLE_SIZE)) && ballSpeed.x < 0) {
			ballSpeed.x = -ballSpeed.x;
		}
		
		if (inRect(ballPosition, spriteRect((vector2i){
			SCREEN_SIZE.x - PADDLE_PIXELS_FROM_BOUNDARY, 
			SCREEN_SIZE.y / 2}, PADDLE_SIZE)) && ballSpeed.x > 0) {
			ballSpeed.x = -ballSpeed.x;
		}

		// ----------
		// RENDER FRAME
		// ----------
	
		// Clear screen
	
		SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
	
		// Blit surfaces to screen
	
		SDL_Rect dstRect;
	
		dstRect = sdlRect(spriteRect((vector2i){
			SCREEN_SIZE.x / 2,
			SCREEN_SIZE.y / 2}, SEPARATOR_SIZE));
		SDL_BlitSurface(separatorSurface, NULL, screenSurface, &dstRect);
		
		dstRect = sdlRect(spriteRect((vector2i){
			PADDLE_PIXELS_FROM_BOUNDARY, 
			SCREEN_SIZE.y / 2}, PADDLE_SIZE));
		SDL_BlitSurface(leftPaddleSurface, NULL, screenSurface, &dstRect);
		
		dstRect = sdlRect(spriteRect((vector2i){
			SCREEN_SIZE.x - PADDLE_PIXELS_FROM_BOUNDARY, 
			SCREEN_SIZE.y / 2}, PADDLE_SIZE));
		SDL_BlitSurface(rightPaddleSurface, NULL, screenSurface, &dstRect);
		
		dstRect = sdlRect(spriteRect(ballPosition, BALL_SIZE));
		SDL_BlitSurface(ballSurface, NULL, screenSurface, &dstRect);
	
		// Swap back screen buffer to front (present frame)
	
		SDL_UpdateWindowSurface(window);
		
	}
	
	// ----------
	// RELEASE AND EXIT
	// ----------
	
	SDL_FreeSurface(separatorSurface);
	SDL_FreeSurface(leftPaddleSurface);
	SDL_FreeSurface(rightPaddleSurface);
	SDL_FreeSurface(ballSurface);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return EXIT_OK;
}
