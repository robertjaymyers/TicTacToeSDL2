﻿// TicTacToeSDL2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <SDL.h>
#include <SDL_image.h>

struct sdlCleanupWindow
{
	void operator()(SDL_Window* window) const
	{
		SDL_DestroyWindow(window);
		SDL_Log("SDL_Window destroyed"); // for debugging
	}
};

struct sdlCleanupRenderer
{
	void operator()(SDL_Renderer* renderer) const
	{
		SDL_DestroyRenderer(renderer);
		SDL_Log("SDL_Renderer destroyed"); // for debugging
	}
};

struct sdlCleanupTexture
{
	void operator()(SDL_Texture* texture) const
	{
		SDL_DestroyTexture(texture);
		SDL_Log("SDL_Texture destroyed"); // for debugging
	}
};

std::unique_ptr<SDL_Window, sdlCleanupWindow> window;
std::unique_ptr<SDL_Renderer, sdlCleanupRenderer> renderer;

std::unique_ptr<SDL_Texture, sdlCleanupTexture> bgBoardTex;
SDL_Rect bgBoardRect;

std::unique_ptr<SDL_Texture, sdlCleanupTexture> bgUiTex;
SDL_Rect bgUiRect;

std::unique_ptr<SDL_Texture, sdlCleanupTexture> txtUiYourTurnTex;
SDL_Rect txtUiYourTurnTokenRect;
std::unique_ptr<SDL_Texture, sdlCleanupTexture> txtUiRoundEndTex;
SDL_Rect txtUiRoundEndTokenRect;

SDL_Rect btnUiPlayRect;

std::unique_ptr<SDL_Texture, sdlCleanupTexture> catsGameTex;
SDL_Rect catsGameRect;


std::unique_ptr<SDL_Texture, sdlCleanupTexture> tokenXTex;
std::unique_ptr<SDL_Texture, sdlCleanupTexture> tokenOTex;

const int boardOffset = 5;
const int boardSize = 390; // 390x390
const int squareSize = 130; // 130x130

struct token
{
	SDL_Rect rectDst;
	enum class Type { BLANK, X, O };
	Type type;

	token()
	{
		type = Type::BLANK;
		rectDst.w = squareSize;
		rectDst.h = squareSize;
	}
};

// Slots are laid out as follows:
// 0 1 2
// 3 4 5
// 6 7 8
//std::vector<token> tokensPlaced(9);
const int boardVecSize = 3;
std::vector<std::vector<token>> boardSquares(boardVecSize, std::vector<token>(boardVecSize));

SDL_Rect tokenXRect;
SDL_Rect tokenORect;

enum class TurnState { PLAY_Xs, PLAY_Os };
TurnState turnState = TurnState::PLAY_Xs; // X always plays first, so game starts with X's turn.

enum class ProgramState { START, IDLE, ROUND_END_X, ROUND_END_O, ROUND_END_TIE, QUIT };
ProgramState programState = ProgramState::START;

const int fpsCap = 60;
const int fpsDelay = 1000 / fpsCap;
Uint32 fpsTimerStart;
int fpsTimerElapsed;

void programStartup();
void programShutdown();
void eventCheck();
void eventCheckRound();
void renderUpdate();
bool mouseWithinRectBound(const SDL_MouseButtonEvent &btn, const SDL_Rect &rect);
bool gameWon(enum class token::Type &passedType);
bool gameOver();

int main(int arg, char *argc[])
{
	while (programState != ProgramState::QUIT)
	{
		switch (programState)
		{
		case (ProgramState::START):
			programStartup();
			break;
		case (ProgramState::IDLE):
			fpsTimerStart = SDL_GetTicks();
			eventCheck();
			renderUpdate();
			fpsTimerElapsed = SDL_GetTicks() - fpsTimerStart;
			if (fpsDelay > fpsTimerElapsed)
			{
				SDL_Delay(fpsDelay - fpsTimerElapsed);
			}
			break;
		case (ProgramState::ROUND_END_X):
			fpsTimerStart = SDL_GetTicks();
			eventCheckRound();
			renderUpdate();
			fpsTimerElapsed = SDL_GetTicks() - fpsTimerStart;
			if (fpsDelay > fpsTimerElapsed)
			{
				SDL_Delay(fpsDelay - fpsTimerElapsed);
			}
			break;
		case (ProgramState::ROUND_END_O):
			fpsTimerStart = SDL_GetTicks();
			eventCheckRound();
			renderUpdate();
			fpsTimerElapsed = SDL_GetTicks() - fpsTimerStart;
			if (fpsDelay > fpsTimerElapsed)
			{
				SDL_Delay(fpsDelay - fpsTimerElapsed);
			}
			break;
		case (ProgramState::ROUND_END_TIE):
			fpsTimerStart = SDL_GetTicks();
			eventCheckRound();
			renderUpdate();
			fpsTimerElapsed = SDL_GetTicks() - fpsTimerStart;
			if (fpsDelay > fpsTimerElapsed)
			{
				SDL_Delay(fpsDelay - fpsTimerElapsed);
			}
			break;
		}
	}

	programShutdown();

	return 0;
}

void programStartup()
{
	SDL_Init(SDL_INIT_EVERYTHING);
	window.reset(SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 550, false));
	renderer.reset(SDL_CreateRenderer(window.get(), -1, 0));
	SDL_SetRenderDrawColor(renderer.get(), 255, 255, 255, 255);

	SDL_Surface *tempSurface;
	tempSurface = IMG_Load("textures/tictactoe-bg.png");
	bgBoardTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	tempSurface = IMG_Load("textures/bg-ui.png");
	bgUiTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	tempSurface = IMG_Load("textures/ui-txt-your-turn.png");
	txtUiYourTurnTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	tempSurface = IMG_Load("textures/ui-txt-round-end.png");
	txtUiRoundEndTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	tempSurface = IMG_Load("textures/tokenX.png");
	tokenXTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	tempSurface = IMG_Load("textures/tokenO.png");
	tokenOTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	tempSurface = IMG_Load("textures/cats-game-pic.png");
	catsGameTex.reset(SDL_CreateTextureFromSurface(renderer.get(), tempSurface));
	SDL_FreeSurface(tempSurface);

	bgBoardRect.w = 390;
	bgBoardRect.h = 390;
	bgBoardRect.x = boardOffset;
	bgBoardRect.y = boardOffset;

	bgUiRect.w = 400;
	bgUiRect.h = 150;
	bgUiRect.x = 0;
	bgUiRect.y = 400;

	txtUiYourTurnTokenRect.w = 44;
	txtUiYourTurnTokenRect.h = 44;
	txtUiYourTurnTokenRect.x = 300;
	txtUiYourTurnTokenRect.y = bgUiRect.y + 10;

	txtUiRoundEndTokenRect.w = 44;
	txtUiRoundEndTokenRect.h = 44;
	txtUiRoundEndTokenRect.x = 16;
	txtUiRoundEndTokenRect.y = bgUiRect.y + 10;

	btnUiPlayRect.w = 81;
	btnUiPlayRect.h = 36;
	btnUiPlayRect.x = 296;
	btnUiPlayRect.y = bgUiRect.y + 83;

	catsGameRect.w = 300;
	catsGameRect.h = 300;
	catsGameRect.x = boardOffset + 50;
	catsGameRect.y = boardOffset + 50;

	int xOffset = 0;
	int yOffset = 0;
	for (int row = 0; row < boardVecSize; row++)
	{
		for (int col = 0; col < boardVecSize; col++)
		{
			boardSquares[row][col].rectDst.x = boardOffset + xOffset;
			boardSquares[row][col].rectDst.y = boardOffset + yOffset;

			xOffset += squareSize;

			//std::cout << "boardSquaresX " << row << col << " " << boardSquares[row][col].rectDst.x << std::endl;
			//std::cout << "boardSquaresY " << row << col << " " << boardSquares[row][col].rectDst.y << std::endl;
		}
		xOffset = 0;
		yOffset += squareSize;
	}

	programState = ProgramState::IDLE;
}

void programShutdown()
{
	SDL_Quit();
}

void eventCheck()
{
	SDL_Event sdlEvent;
	SDL_PollEvent(&sdlEvent);

	switch (sdlEvent.type)
	{
	case SDL_QUIT:
		programState = ProgramState::QUIT;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (sdlEvent.button.button == SDL_BUTTON_LEFT)
		{
			for (int row = 0; row < boardVecSize; row++)
			{
				for (int col = 0; col < boardVecSize; col++)
				{
					if (mouseWithinRectBound(sdlEvent.button, boardSquares[row][col].rectDst))
					{
						if (boardSquares[row][col].type == token::Type::BLANK)
						{
							if (turnState == TurnState::PLAY_Xs)
							{
								boardSquares[row][col].type = token::Type::X;
								turnState = TurnState::PLAY_Os;
								if (gameWon(boardSquares[row][col].type))
								{
									programState = ProgramState::ROUND_END_X;
								}
								else if (gameOver())
								{
									programState = ProgramState::ROUND_END_TIE;
								}
								break;
							}
							else if (turnState == TurnState::PLAY_Os)
							{
								boardSquares[row][col].type = token::Type::O;
								turnState = TurnState::PLAY_Xs;
								if (gameWon(boardSquares[row][col].type))
								{
									programState = ProgramState::ROUND_END_O;
								}
								else if (gameOver())
								{
									programState = ProgramState::ROUND_END_TIE;
								}
								break;
							}
						}
					}
				}
			}
		}
		break;
	}
}

void eventCheckRound()
{
	SDL_Event sdlEvent;
	SDL_PollEvent(&sdlEvent);

	switch (sdlEvent.type)
	{
	case SDL_QUIT:
		programState = ProgramState::QUIT;
		break;
	case SDL_MOUSEBUTTONDOWN:
		if (sdlEvent.button.button == SDL_BUTTON_LEFT)
		{
			if (mouseWithinRectBound(sdlEvent.button, btnUiPlayRect))
			{
				for (int row = 0; row < boardVecSize; row++)
				{
					for (int col = 0; col < boardVecSize; col++)
					{
						boardSquares[row][col].type = token::Type::BLANK;
					}
				}
				programState = ProgramState::IDLE;
			}
		}
		break;
	}
}

void renderUpdate()
{
	SDL_RenderClear(renderer.get());
	SDL_RenderCopy(renderer.get(), bgBoardTex.get(), NULL, &bgBoardRect);
	SDL_RenderCopy(renderer.get(), bgUiTex.get(), NULL, &bgUiRect);

	for (int row = 0; row < boardVecSize; row++)
	{
		for (int col = 0; col < boardVecSize; col++)
		{
			switch (boardSquares[row][col].type)
			{
			case (token::Type::X):
				SDL_RenderCopy(renderer.get(), tokenXTex.get(), NULL, &boardSquares[row][col].rectDst);
				break;
			case (token::Type::O):
				SDL_RenderCopy(renderer.get(), tokenOTex.get(), NULL, &boardSquares[row][col].rectDst);
				break;
			}
		}
	}

	if (programState == ProgramState::IDLE)
	{
		SDL_RenderCopy(renderer.get(), txtUiYourTurnTex.get(), NULL, &bgUiRect);
		if (turnState == TurnState::PLAY_Xs)
		{
			SDL_RenderCopy(renderer.get(), tokenXTex.get(), NULL, &txtUiYourTurnTokenRect);
		}
		else if (turnState == TurnState::PLAY_Os)
		{
			SDL_RenderCopy(renderer.get(), tokenOTex.get(), NULL, &txtUiYourTurnTokenRect);
		}
	}
	else if (programState == ProgramState::ROUND_END_X)
	{
		SDL_RenderCopy(renderer.get(), txtUiRoundEndTex.get(), NULL, &bgUiRect);
		SDL_RenderCopy(renderer.get(), tokenXTex.get(), NULL, &txtUiRoundEndTokenRect);
	}
	else if (programState == ProgramState::ROUND_END_O)
	{
		SDL_RenderCopy(renderer.get(), txtUiRoundEndTex.get(), NULL, &bgUiRect);
		SDL_RenderCopy(renderer.get(), tokenOTex.get(), NULL, &txtUiRoundEndTokenRect);
	}
	else if (programState == ProgramState::ROUND_END_TIE)
	{
		SDL_RenderCopy(renderer.get(), txtUiRoundEndTex.get(), NULL, &bgUiRect);
		SDL_RenderCopy(renderer.get(), catsGameTex.get(), NULL, &txtUiRoundEndTokenRect);
		SDL_RenderCopy(renderer.get(), catsGameTex.get(), NULL, &catsGameRect);
	}

	SDL_RenderPresent(renderer.get());
}

bool mouseWithinRectBound(const SDL_MouseButtonEvent &btn, const SDL_Rect &rect)
{
	if (btn.x >= rect.x &&
		btn.x <= rect.x + rect.w &&
		btn.y >= rect.y &&
		btn.y <= rect.y + rect.h)
	{
		return true;
	}
	return false;
}

bool gameWon(enum class token::Type &passedType)
{
	// Horizontal/Vertical win condition check.
	int acrossCount = 0;
	std::vector<int> downCount{ 0,0,0 };
	for (int row = 0; row < boardVecSize; row++)
	{
		for (int col = 0; col < boardVecSize; col++)
		{
			if (boardSquares[row][col].type == passedType)
			{
				downCount[col]++;
				acrossCount++;
				if (acrossCount == 3 || downCount[col] == 3)
				{
					return true;
				}
			}
		}
		acrossCount = 0;
	}

	// Diagonal win condition check.
	if (boardSquares[0][0].type == passedType &&
		boardSquares[1][1].type == passedType &&
		boardSquares[2][2].type == passedType)
	{
		return true;
	}
	else if (boardSquares[0][2].type == passedType &&
		boardSquares[1][1].type == passedType &&
		boardSquares[2][0].type == passedType)
	{
		return true;
	}

	return false;
}

bool gameOver()
{
	for (int row = 0; row < boardVecSize; row++)
	{
		for (int col = 0; col < boardVecSize; col++)
		{
			if (boardSquares[row][col].type == token::Type::BLANK)
			{
				return false;
			}
		}
	}
	return true;
}