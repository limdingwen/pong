#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <time.h>
#include <stdio.h>
#include <math.h>

// Constants

#define EXIT_OK 0
#define EXIT_ERR -1

#define FRAME_MIN_DELTA (1./60.)

#define SCREEN_SIZE (vector2i){640, 400}

#define SEPARATOR_SIZE (vector2i){2, 400}
#define PADDLE_SIZE (vector2i){8, 32}
#define BALL_SIZE (vector2i){8, 8}

#define PADDLE_BOUNDARY_DIST 20

#define SCORE_CENTER_DIST 20
#define SCORE_TOP_DIST 20
#define SCORE_FONT_SIZE (vector2i){15, 24}

#define BALL_INITIAL_POSITION (vector2i){\
	SCREEN_SIZE.x / 2,\
	SCREEN_SIZE.y / 2}
#define BALL_INITIAL_VELOCITY (vector2i){-8, 0}

#define BALL_VERTICAL_SPEED 2
#define PADDLE_SPEED 3

#define PADDLE_AI_DEADZONE (PADDLE_SIZE.y / 3)

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

double absd(double x) {
	return (x >= 0) ? x : -x;
}

int verticalBallVelocity(int ballPositionY, int paddlePosition) {
	int verticalSpeed = (int) (
		log(
			absd(
				(ballPositionY - paddlePosition) / 
				(PADDLE_SIZE.y / 2.)
			) + 1.
		) * 3.33 * BALL_VERTICAL_SPEED
	);
	
	return ((ballPositionY - paddlePosition) >= 0) ? verticalSpeed : -verticalSpeed;
}

// Main

int main(int argc, char** argv) {

	// ----------
	// INITIALIZATION
	// ----------

	char* buffer = malloc(128);

	// Initialize SDL and create window

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	if (TTF_Init() < 0) {
		printf("Couldn't initialize SDL TTF: %s\n", SDL_GetError());
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
		printf("Couldn't load separator: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Surface* leftPaddleSurface = IMG_Load("resources/left_pallete.png");
	if (!leftPaddleSurface) {
		printf("Couldn't load left paddle: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Surface* rightPaddleSurface = IMG_Load("resources/right_pallete.png");
	if (!rightPaddleSurface) {
		printf("Couldn't load right paddle: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	SDL_Surface* ballSurface = IMG_Load("resources/ball.png");
	if (!ballSurface) {
		printf("Couldn't load separator: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	// Initialize scene variables
	
	vector2i ballPosition = BALL_INITIAL_POSITION;
	vector2i ballVelocity = BALL_INITIAL_VELOCITY;
	int ballPaused = 1;
	
	int keyDownPressed = 0;
	int keyUpPressed = 0;
	
	int leftPaddlePosition = SCREEN_SIZE.y / 2;
	int rightPaddlePosition = SCREEN_SIZE.y / 2;
	
	int leftScore = 0;
	int rightScore = 0;
	
	// Initialize font
	
	TTF_Font* fpsFont = TTF_OpenFont("resources/LCD_Solid.ttf", 9);
	if (!fpsFont) {
		printf("Couldn't load FPS font: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	TTF_Font* scoreFont = TTF_OpenFont("resources/LCD_Solid.ttf", 24);
	if (!scoreFont) {
		printf("Couldn't load score font: %s\n", SDL_GetError());
		return EXIT_ERR;
	}
	
	// ----------
	// MAIN LOOP
	// ----------
	
	// Initialize loop
	
	int running = 1;
	double frameDeltaTime = 0.;
	
	while (running) {
		time_t frameStartTime = clock();
		
		// ----------
		// HANDLE EVENTS
		// ----------
		
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				running = 0;
			}
			else if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_DOWN:
						keyDownPressed = 1;
						break;
					case SDLK_UP:
						keyUpPressed = 1;
						break;
					case SDLK_SPACE:
						if (ballPaused)
							ballPaused = 0;
						break;
				}
			}
			else if (e.type == SDL_KEYUP) {
				switch (e.key.keysym.sym) {
					case SDLK_DOWN:
						keyDownPressed = 0;
						break;
					case SDLK_UP:
						keyUpPressed = 0;
						break;
				}
			}
		}
		
		// ----------
		// UPDATE FRAME
		// ----------
		
		// Update left paddle position
		
		if (keyDownPressed) leftPaddlePosition += PADDLE_SPEED;
		if (keyUpPressed) leftPaddlePosition -= PADDLE_SPEED;
		
		if (leftPaddlePosition < 0) leftPaddlePosition = 0;
		else if (leftPaddlePosition > SCREEN_SIZE.y) leftPaddlePosition = SCREEN_SIZE.y;
		
		// Update right paddle position
		
		if (ballPosition.y > rightPaddlePosition + PADDLE_AI_DEADZONE) rightPaddlePosition += PADDLE_SPEED;
		else if (ballPosition.y < rightPaddlePosition - PADDLE_AI_DEADZONE) rightPaddlePosition -= PADDLE_SPEED;
		
		if (rightPaddlePosition < 0) rightPaddlePosition = 0;
		else if (rightPaddlePosition > SCREEN_SIZE.y) rightPaddlePosition = SCREEN_SIZE.y;
		
		if (!ballPaused) {
			// Update ball position
		
			ballPosition.x += ballVelocity.x;
			ballPosition.y += ballVelocity.y;
		
			// Check if ball is out of bounds horizontally
			// If so, reset ball
		
			if (ballPosition.x < 0 || ballPosition.x > SCREEN_SIZE.x) {
				if (ballPosition.x < 0) {
					rightScore++;
				}
				else {
					leftScore++;
				}
			
				ballPosition = BALL_INITIAL_POSITION;
				ballVelocity = BALL_INITIAL_VELOCITY;
				ballPaused = 1;
			}
		
			// Check if ball is out of bounds vertically
			// If so, bounce ball vertically
		
			if (ballPosition.y < 0 || ballPosition.y > SCREEN_SIZE.y) {
				ballVelocity.y = -ballVelocity.y;
			}
		
			// Check if ball is hitting a paddle
			// If so, set ball's horizontal direction
		
			if (inRect(ballPosition, spriteRect((vector2i){
				PADDLE_BOUNDARY_DIST, 
				leftPaddlePosition}, PADDLE_SIZE)) && ballVelocity.x < 0) {
				ballVelocity.x = -ballVelocity.x;
				ballVelocity.y = verticalBallVelocity(ballPosition.y, leftPaddlePosition);
			}
		
			if (inRect(ballPosition, spriteRect((vector2i){
				SCREEN_SIZE.x - PADDLE_BOUNDARY_DIST, 
				rightPaddlePosition}, PADDLE_SIZE)) && ballVelocity.x > 0) {
				ballVelocity.x = -ballVelocity.x;
				ballVelocity.y = verticalBallVelocity(ballPosition.y, rightPaddlePosition);
			}
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
			PADDLE_BOUNDARY_DIST, 
			leftPaddlePosition}, PADDLE_SIZE));
		SDL_BlitSurface(leftPaddleSurface, NULL, screenSurface, &dstRect);
		
		dstRect = sdlRect(spriteRect((vector2i){
			SCREEN_SIZE.x - PADDLE_BOUNDARY_DIST, 
			rightPaddlePosition}, PADDLE_SIZE));
		SDL_BlitSurface(rightPaddleSurface, NULL, screenSurface, &dstRect);
		
		dstRect = sdlRect(spriteRect(ballPosition, BALL_SIZE));
		SDL_BlitSurface(ballSurface, NULL, screenSurface, &dstRect);

		sprintf(buffer, "%.1f (%.1fms)", 1.0/frameDeltaTime, frameDeltaTime * 1000.0);
		SDL_Surface* fpsSurface = TTF_RenderText_Solid(
			fpsFont, buffer, (SDL_Color){255, 255, 255});
		dstRect = (SDL_Rect){5, 5, 0, 0};
		SDL_BlitSurface(fpsSurface, NULL, screenSurface, &dstRect);
		SDL_FreeSurface(fpsSurface);
		
		sprintf(buffer, "%i", leftScore);
		SDL_Surface* leftScoreSurface = TTF_RenderText_Solid(
			scoreFont, buffer, (SDL_Color){255, 255, 255});
		dstRect = sdlRect(spriteRect(
			(vector2i){SCREEN_SIZE.x / 2 - SCORE_CENTER_DIST, SCORE_TOP_DIST},
			SCORE_FONT_SIZE));
		SDL_BlitSurface(leftScoreSurface, NULL, screenSurface, &dstRect);
		SDL_FreeSurface(leftScoreSurface);
		
		sprintf(buffer, "%i", rightScore);
		SDL_Surface* rightScoreSurface = TTF_RenderText_Solid(
			scoreFont, buffer, (SDL_Color){255, 255, 255});
		dstRect = sdlRect(spriteRect(
			(vector2i){SCREEN_SIZE.x / 2 + SCORE_CENTER_DIST, SCORE_TOP_DIST},
			SCORE_FONT_SIZE));
		SDL_BlitSurface(rightScoreSurface, NULL, screenSurface, &dstRect);
		SDL_FreeSurface(rightScoreSurface);
	
		// Swap back screen buffer to front (present frame)
	
		SDL_UpdateWindowSurface(window);
		
		// ----------
		// CAP FPS
		// ----------
		
		time_t frameEndTime = clock();
		frameDeltaTime = (frameEndTime - frameStartTime) / CLOCKS_PER_SEC;
		if (frameDeltaTime < FRAME_MIN_DELTA) {
			SDL_Delay((int) ((FRAME_MIN_DELTA - frameDeltaTime) * 1000));
			frameDeltaTime = FRAME_MIN_DELTA;
		}
	}
	
	// ----------
	// RELEASE AND EXIT
	// ----------
	
	SDL_FreeSurface(separatorSurface);
	SDL_FreeSurface(leftPaddleSurface);
	SDL_FreeSurface(rightPaddleSurface);
	SDL_FreeSurface(ballSurface);
	TTF_CloseFont(fpsFont);
	TTF_CloseFont(scoreFont);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	free(buffer);
	
	return EXIT_OK;
}
