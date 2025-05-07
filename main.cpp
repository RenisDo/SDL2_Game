#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <time.h>
#include "tile.h"
#include "stopwatch.h"
#include "button.h"
#include "startMenu.h"

typedef std::vector<std::vector<Tile>> tileArray;

Mix_Chunk* gMoveSound = nullptr;
Mix_Chunk* gVictorySound = nullptr;

static void forEachTile(tileArray& tiles, std::function<void(tileArray&, const int, const int)>&& func) {
    for (int row = 0; row < tiles.size(); ++row) {
        for (int col = 0; col < tiles[row].size(); ++col) {
            func(tiles, row, col);
        }
    }
}

static inline bool inBounds(const int row, const int col, const int maxRow, const int maxCol) {
    return !(row < 0 || row > maxRow || col < 0 || col > maxCol);
}

static bool isEmptyTileInNeighbours(tileArray& tiles, const int row, const int col, Tile* emptyTile) {
    static const int deltas[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

    for (int i = 0; i < 4; ++i) {
        if (inBounds(row + deltas[i][0], col + deltas[i][1], tiles.size() - 1, tiles[0].size() - 1)) {
            if (emptyTile == &tiles[row + deltas[i][0]][col + deltas[i][1]]) {
                return true;
            }
        }
    }

    return false;
}

unsigned int playMenu(SDL_Renderer* renderer, bool* exit, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT) {
    const unsigned int NUMBER_OF_ROW_ELEMENTS = 1;
    const unsigned int NUMBER_OF_COL_ELEMENTS = 3;
    const unsigned int NUMBER_OF_ROW_BORDERS = NUMBER_OF_ROW_ELEMENTS + 1;
    const unsigned int NUMBER_OF_COL_BORDERS = NUMBER_OF_COL_ELEMENTS + 1;
    const unsigned int BORDER_THICKNESS = 20;

    const unsigned int BUTTON_WIDTH = SCREEN_WIDTH - 2 * BORDER_THICKNESS;
    const unsigned int BUTTON_HEIGHT = (SCREEN_HEIGHT - NUMBER_OF_COL_BORDERS * BORDER_THICKNESS) / NUMBER_OF_COL_ELEMENTS;

    const SDL_Color FONT_COLOUR = {0, 0, 0, 255};
    const SDL_Color BUTTON_COLOUR = {255, 255, 102, 255};
    const SDL_Color BUTTON_DOWN_COLOUR = {50, 255, 100, 255};

    const int fontSize = BUTTON_HEIGHT - 40;
    TTF_Font* font = TTF_OpenFont("assets/ARCADECLASSIC.ttf", fontSize);
    if (font == nullptr) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
    }

    const char* buttonTexts[3] = {"3x3", "4x4", "5x5"};

    int startX = BORDER_THICKNESS;
    int startY = 0;

    std::vector<Button> buttons;
    for (int row = 0; row < NUMBER_OF_COL_ELEMENTS; ++row) {
        startY += BORDER_THICKNESS;
        SDL_Rect rect = {startX, startY, (int)BUTTON_WIDTH, (int)BUTTON_HEIGHT};
        Button button(rect, BUTTON_COLOUR, font, FONT_COLOUR);
        button.loadTexture(renderer, buttonTexts[row]);
        buttons.push_back(button);
        startY += BUTTON_HEIGHT;
    }

    const unsigned int FPS = 60;
    const float milliSecondsPerFrame = 1000 / FPS;
    float lastTimeRendered = SDL_GetTicks();
    float deltaTimeRendered;

    bool stop = false;
    SDL_Event event;
    unsigned int difficulty = 0;

    while (!stop) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                *exit = true;
                stop = true;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                for (int i = 0; i < NUMBER_OF_COL_ELEMENTS; ++i) {
                    if (buttons[i].isMouseInside(x, y)) {
                        buttons[i].changeColourTo(BUTTON_DOWN_COLOUR);
                        difficulty = i + 3;
                    }
                }
            } else if (event.type == SDL_MOUSEBUTTONUP){
                for (auto& button : buttons) {
                    button.changeColourTo(BUTTON_COLOUR);
                }
                if (difficulty != 0) {
                    stop = true;
                }
            }
        }

        deltaTimeRendered = SDL_GetTicks() - lastTimeRendered;
        if (deltaTimeRendered > milliSecondsPerFrame) {
            lastTimeRendered = SDL_GetTicks();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            for (const auto& button : buttons) {
                button.render(renderer);
            }

            SDL_RenderPresent(renderer);

        } else {
            SDL_Delay(milliSecondsPerFrame - deltaTimeRendered);
        }
    }

    for (auto& button : buttons) {
        button.free();
    }

    TTF_CloseFont(font);
    font = nullptr;

    return difficulty;
}

void playPuzzle(SDL_Renderer* renderer, bool* exit, const unsigned int DIFFICULTY, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT) {
    const unsigned int NUMBER_OF_ROW_ELEMENTS = DIFFICULTY;
    const unsigned int NUMBER_OF_COL_ELEMENTS = DIFFICULTY + 2;
    const unsigned int NUMBER_OF_ROW_BORDERS = NUMBER_OF_ROW_ELEMENTS + 1;
    const unsigned int NUMBER_OF_COL_BORDERS = NUMBER_OF_COL_ELEMENTS + 1;
    const unsigned int BORDER_THICKNESS = 6;
    const unsigned int TILE_WIDTH = (SCREEN_WIDTH - NUMBER_OF_ROW_BORDERS * BORDER_THICKNESS) / NUMBER_OF_ROW_ELEMENTS;
    const unsigned int TILE_HEIGHT = (SCREEN_HEIGHT - NUMBER_OF_COL_BORDERS * BORDER_THICKNESS) / NUMBER_OF_COL_ELEMENTS;

    const unsigned int STOPWATCH_WIDTH = SCREEN_WIDTH - 2 * BORDER_THICKNESS;
    const unsigned int STOPWATCH_HEIGHT = TILE_HEIGHT;

    const unsigned int BUTTON_WIDTH = SCREEN_WIDTH - 2 * BORDER_THICKNESS;
    const unsigned int BUTTON_HEIGHT = TILE_HEIGHT;

    const SDL_Color TILE_COLOUR = {0, 191, 255, 255};
    const SDL_Color TILE_COMPLETION_COLOUR = {50, 255, 100, 255};
    const SDL_Color EMPTY_TILE_COLOUR = {0, 0, 0, 255};
    const SDL_Color FONT_COLOUR = {0, 0, 0, 255};
    const SDL_Color STOPWATCH_COLOUR = {255, 50, 50, 255};
    const SDL_Color BUTTON_COLOUR = {255, 255, 102, 255};
    const SDL_Color BUTTON_DOWN_COLOUR = {50, 255, 100, 255};

    const int fontSize = TILE_HEIGHT - 40;
    TTF_Font* font = TTF_OpenFont("assets/ARCADECLASSIC.ttf", fontSize);
    if (font == nullptr) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
    }

    int startX = BORDER_THICKNESS;
    int startY = BORDER_THICKNESS;
    SDL_Rect rect = {startX, startY, (int)STOPWATCH_WIDTH, (int)STOPWATCH_HEIGHT};
    Stopwatch stopwatch(rect, STOPWATCH_COLOUR, font, FONT_COLOUR);

    tileArray tiles;
    startY += TILE_HEIGHT;
    for (int row = 0; row < DIFFICULTY; ++row) {
        std::vector<Tile> tileRow;
        startY += BORDER_THICKNESS;
        startX = 0;
        for (int col = 0; col < DIFFICULTY; ++col) {
            startX += BORDER_THICKNESS;
            rect = {startX, startY, (int)TILE_WIDTH, (int)TILE_HEIGHT};
            SDL_Color colour;
            if (row == DIFFICULTY - 1 && col == DIFFICULTY - 1) {
                colour = EMPTY_TILE_COLOUR;
            } else {
                colour = TILE_COLOUR;
            }
            int number = row * DIFFICULTY + col + 1;

            Tile tile(rect, colour, font, FONT_COLOUR, number);
            const char* numberStr = std::to_string(number).c_str();
            tile.loadTexture(renderer, numberStr);
            tileRow.push_back(tile);

            startX += TILE_WIDTH;
        }
        startY += TILE_HEIGHT;
        tiles.push_back(tileRow);
    }

    startX = BORDER_THICKNESS;
    startY += BORDER_THICKNESS;
    rect = {startX, startY, (int)BUTTON_WIDTH, (int)BUTTON_HEIGHT};
    Button menuButton(rect, BUTTON_COLOUR, font, FONT_COLOUR);
    menuButton.loadTexture(renderer, "Menu");

    const unsigned int FPS = 60;
    const float milliSecondsPerFrame = 1000 / FPS;
    float lastTimeRendered = SDL_GetTicks();
    float deltaTimeRendered;

    const unsigned int pixelsPerSecond = 500;
    const float milliSecondsPerPixel = 1000 / pixelsPerSecond;
    float lastTimeMoved;
    float deltaTimeMoved;

    Tile* movingTile = nullptr;
    bool doneMoving = true;
    Tile* emptyTile = &tiles[DIFFICULTY - 1][DIFFICULTY - 1];
    bool selected = false;
    int tempXPosition;
    int tempYPosition;

    const unsigned int TOTAL_SWAPS = 1000;
    int emptyTileRow = DIFFICULTY - 1;
    int emptyTileCol = DIFFICULTY - 1;
    std::vector<std::vector<int>> neighbours;
    srand(time(NULL));
    const int deltas[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};
    for (int swap = 0; swap < TOTAL_SWAPS; ++swap) {
        for (int i = 0; i < 4; ++i) {
            const int deltaRow = deltas[i][0];
            const int deltaCol = deltas[i][1];
            if (inBounds(emptyTileRow + deltaRow, emptyTileCol + deltaCol, tiles.size() - 1, tiles[0].size() - 1)) {
                std::vector<int> neighbour = {emptyTileRow + deltaRow, emptyTileCol + deltaCol};
                neighbours.push_back(neighbour);
            }
        }
        const int randomIndex = rand() % neighbours.size();
        const int row = neighbours[randomIndex][0];
        const int col = neighbours[randomIndex][1];
        tempXPosition = emptyTile->getXPosition();
        tempYPosition = emptyTile->getYPosition();
        emptyTile->setPositionTo(tiles[row][col].getXPosition(), tiles[row][col].getYPosition());
        tiles[row][col].setPositionTo(tempXPosition, tempYPosition);
        std::iter_swap(&tiles[row][col], emptyTile);
        emptyTile = &tiles[row][col];
        emptyTileRow = row;
        emptyTileCol = col;
        neighbours.clear();
    }

    bool stop = false;
    SDL_Event event;
    bool checkSolved = false;
    bool solved = false;
    bool menuButtonPressed = false;
    bool isPaused = false;

    stopwatch.start();

    while (!stop) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                stop = true;
                *exit = true;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isPaused = !isPaused;
                    std::cout << "Game " << (isPaused ? "paused" : "resumed") << std::endl;
                    if (isPaused) {
                        stopwatch.pause();
                    } else {
                        stopwatch.resume();
                    }
                }
            }
            if (doneMoving && !isPaused) {
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    if (!solved) {
                        forEachTile(tiles, [x, y, &movingTile, emptyTile, &selected, &doneMoving, &lastTimeMoved](tileArray& tiles, const int row, const int col) {
                            if (tiles[row][col].isMouseInside(x, y)) {
                                if (isEmptyTileInNeighbours(tiles, row, col, emptyTile)) {
                                    movingTile = &tiles[row][col];
                                    selected = true;
                                    doneMoving = false;
                                    lastTimeMoved = SDL_GetTicks();
                                }
                            }
                        });
                    }
                    if (menuButton.isMouseInside(x, y)) {
                        menuButton.changeColourTo(BUTTON_DOWN_COLOUR);
                        menuButtonPressed = true;
                    }
                } else if (event.type == SDL_MOUSEBUTTONUP) {
                    menuButton.changeColourTo(BUTTON_COLOUR);
                    if (menuButtonPressed) {
                        stop = true;
                    }
                }
            }
        }

        if (selected && movingTile != nullptr) {
            tempXPosition = movingTile->getXPosition();
            tempYPosition = movingTile->getYPosition();
            selected = false;
        }

        if (movingTile != nullptr && !isPaused) {
            deltaTimeMoved = SDL_GetTicks() - lastTimeMoved;
            if (deltaTimeMoved > milliSecondsPerPixel) {
                int pixelsToMove = deltaTimeMoved / milliSecondsPerPixel;
                for (int i = 0; i < pixelsToMove; ++i) {
                    doneMoving = movingTile->moveTo(emptyTile->getXPosition(), emptyTile->getYPosition());
                    if (doneMoving) {
                        emptyTile->setPositionTo(tempXPosition, tempYPosition);
                        std::iter_swap(movingTile, emptyTile);
                        emptyTile = movingTile;
                        movingTile = nullptr;
                        checkSolved = true;
                        if (gMoveSound) {
                            Mix_PlayChannel(-1, gMoveSound, 0);
                        }
                        break;
                    }
                }
                lastTimeMoved = SDL_GetTicks();
            }
        }

        if (checkSolved) {
            solved = true;
            forEachTile(tiles, [&solved, DIFFICULTY](tileArray& tiles, const int row, const int col) {
                const int number = row * DIFFICULTY + col + 1;
                if (tiles[row][col].getNumber() != number) {
                    solved = false;
                }
            });
            if (solved && gVictorySound) {
                Mix_PlayChannel(-1, gVictorySound, 0);
            }
            checkSolved = false;
        }

        if (solved) {
            forEachTile(tiles, [emptyTile, &TILE_COMPLETION_COLOUR](tileArray& tiles, const int row, const int col) {
                if (emptyTile != &tiles[row][col]) {
                    tiles[row][col].changeColourTo(TILE_COMPLETION_COLOUR);
                }
            });

            TTF_Font* victoryFont = TTF_OpenFont("assets/ARCADECLASSIC.ttf", 60);
            if (victoryFont) {
                SDL_Color textColor = {255, 215, 0, 255};
                SDL_Surface* textSurface = TTF_RenderText_Solid(victoryFont, "You Did It!", textColor);
                if (textSurface) {
                    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                    if (textTexture) {
                        SDL_Rect textRect = {
                            (SCREEN_WIDTH - textSurface->w) / 2,
                            (SCREEN_HEIGHT - textSurface->h) / 2,
                            textSurface->w,
                            textSurface->h
                        };
                        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                        SDL_DestroyTexture(textTexture);
                    }
                    SDL_FreeSurface(textSurface);
                }
                TTF_CloseFont(victoryFont);
            }
        } else if (!isPaused) {
            stopwatch.calculateTime(renderer);
        }

        deltaTimeRendered = SDL_GetTicks() - lastTimeRendered;
        if (deltaTimeRendered > milliSecondsPerFrame) {
            lastTimeRendered = SDL_GetTicks();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            stopwatch.render(renderer);

            forEachTile(tiles, [renderer, emptyTile](tileArray& tiles, const int row, const int col) {
                if (emptyTile != &tiles[row][col]) {
                    tiles[row][col].render(renderer);
                }
            });

            menuButton.render(renderer);

            if (isPaused) {
                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128);
                SDL_Rect pauseRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
                SDL_RenderFillRect(renderer, &pauseRect);

                TTF_Font* pauseFont = TTF_OpenFont("assets/ARCADECLASSIC.ttf", 40);
                if (pauseFont) {
                    SDL_Color textColor = {255, 255, 255, 255};
                    const char* pauseText = "PAUSED - Press ESC to continue";
                    SDL_Surface* textSurface = TTF_RenderText_Solid(pauseFont, pauseText, textColor);
                    if (textSurface) {
                        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                        if (textTexture) {
                            SDL_Rect textRect = {
                                (SCREEN_WIDTH - textSurface->w) / 2,
                                (SCREEN_HEIGHT - textSurface->h) / 2,
                                textSurface->w,
                                textSurface->h
                            };
                            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
                            SDL_DestroyTexture(textTexture);
                        }
                        SDL_FreeSurface(textSurface);
                    }
                    TTF_CloseFont(pauseFont);
                } else {
                    std::cout << "Failed to load pause font! Error: " << TTF_GetError() << std::endl;
                }
            }

            SDL_RenderPresent(renderer);

        } else {
            SDL_Delay(milliSecondsPerFrame - deltaTimeRendered);
        }
    }

    if (solved) {
        std::cout << "Solved!" << std::endl;
    }

    forEachTile(tiles, [](tileArray& tiles, const int row, const int col) {
        tiles[row][col].free();
    });
    stopwatch.free();

    TTF_CloseFont(font);
    font = nullptr;
}

int main( int argc, char* args[] ) {
    const unsigned int SCREEN_WIDTH = 410;
    const unsigned int SCREEN_HEIGHT = 600;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL could not initialise! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (TTF_Init() == -1) {
        std::cout << "SDL_ttf could not initialise! Error: " << TTF_GetError() << std::endl;
        return -1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cout << "SDL_mixer could not initialize! Error: " << Mix_GetError() << std::endl;
        return -1;
    }

    gMoveSound = Mix_LoadWAV("assets/move.wav");
    if (!gMoveSound) {
        std::cout << "Failed to load move sound effect! Error: " << Mix_GetError() << std::endl;
    }

    gVictorySound = Mix_LoadWAV("assets/victory.wav");
    if (!gVictorySound) {
        std::cout << "Failed to load victory sound effect! Error: " << Mix_GetError() << std::endl;
    }

    SDL_Window* window = SDL_CreateWindow("Puzzle Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "SDL could not create window! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == nullptr) {
        std::cout << "SDL could not create renderer! Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    StartMenu startMenu(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    bool exit = false;
    unsigned int difficulty;
    const unsigned int FPS = 60;
    const float milliSecondsPerFrame = 1000 / FPS;
    float lastTimeRendered = SDL_GetTicks();
    float deltaTimeRendered;

    while (!exit) {
        SDL_Event event;
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                exit = true;
            }

            int menuAction = startMenu.handleInput(event);
            if (menuAction == 0) {
                difficulty = playMenu(renderer, &exit, SCREEN_WIDTH, SCREEN_HEIGHT);
                if (!exit) {
                    playPuzzle(renderer, &exit, difficulty, SCREEN_WIDTH, SCREEN_HEIGHT);
                }
            } else if (menuAction == 2) {
                exit = true;
            }
        }

        deltaTimeRendered = SDL_GetTicks() - lastTimeRendered;
        if (deltaTimeRendered > milliSecondsPerFrame) {
            lastTimeRendered = SDL_GetTicks();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            startMenu.render(renderer);

            SDL_RenderPresent(renderer);
        } else {
            SDL_Delay(milliSecondsPerFrame - deltaTimeRendered);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    renderer = nullptr;
    window = nullptr;

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();

    if (gMoveSound) {
        Mix_FreeChunk(gMoveSound);
        gMoveSound = nullptr;
    }
    if (gVictorySound) {
        Mix_FreeChunk(gVictorySound);
        gVictorySound = nullptr;
    }


    std::cout << "Exiting program..." << std::endl;
    return 0;
}
