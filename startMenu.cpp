#include "startMenu.h"
#include <SDL_mixer.h>

StartMenu::StartMenu(SDL_Renderer* renderer, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT) 
    : mSelectedButton(0), mMusicEnabled(true), mBGM(nullptr) {
    
    mBGM = Mix_LoadMUS("assets/music.mp3");
    if (mBGM) {
        Mix_PlayMusic(mBGM, -1);
    }

    const unsigned int BORDER_THICKNESS = 20;
    const unsigned int BUTTON_WIDTH = SCREEN_WIDTH - 2 * BORDER_THICKNESS;
    const unsigned int BUTTON_HEIGHT = 80;
    const unsigned int BUTTON_SPACING = 20;

    const SDL_Color BUTTON_COLOUR = {255, 123, 43, 255};
    const SDL_Color SELECTED_COLOUR = {50, 255, 100, 255};
    const SDL_Color FONT_COLOUR = {0, 0, 0, 255};

    TTF_Font* font = TTF_OpenFont("assets/ARCADECLASSIC.ttf", 40);
    if (!font) {
        std::cout << "Failed to load font! Error: " << TTF_GetError() << std::endl;
        return;
    }

    const char* buttonTexts[3] = {"PLAY GAME", "MUSIC: ON", "QUIT"};
    int startY = (SCREEN_HEIGHT - (3 * BUTTON_HEIGHT + 2 * BUTTON_SPACING)) / 2;

    for (int i = 0; i < 3; ++i) {
        SDL_Rect rect = {
            BORDER_THICKNESS,
            startY + i * (BUTTON_HEIGHT + BUTTON_SPACING),
            BUTTON_WIDTH,
            BUTTON_HEIGHT
        };
        Button button(rect, BUTTON_COLOUR, font, FONT_COLOUR);
        button.loadTexture(renderer, buttonTexts[i]);
        mButtons.push_back(button);
    }

    mButtons[0].changeColourTo(SELECTED_COLOUR);

    TTF_CloseFont(font);
}

StartMenu::~StartMenu() {
    if (mBGM) {
        Mix_FreeMusic(mBGM);
        mBGM = nullptr;
    }

    for (auto& button : mButtons) {
        button.free();
    }
}

int StartMenu::handleInput(SDL_Event& event) {
    if (event.type == SDL_KEYDOWN) {
        const SDL_Color BUTTON_COLOUR = {255, 123, 43, 255};
        const SDL_Color SELECTED_COLOUR = {50, 255, 100, 255};

        switch (event.key.keysym.sym) {
            case SDLK_w:
            case SDLK_UP:
                mButtons[mSelectedButton].changeColourTo(BUTTON_COLOUR);
                mSelectedButton = (mSelectedButton - 1 + mButtons.size()) % mButtons.size();
                mButtons[mSelectedButton].changeColourTo(SELECTED_COLOUR);
                break;

            case SDLK_s:
            case SDLK_DOWN:
                mButtons[mSelectedButton].changeColourTo(BUTTON_COLOUR);
                mSelectedButton = (mSelectedButton + 1) % mButtons.size();
                mButtons[mSelectedButton].changeColourTo(SELECTED_COLOUR);
                break;

            case SDLK_RETURN:
                if (mSelectedButton == 1) {
                    toggleMusic();
                    return -1;
                }
                return mSelectedButton;
        }
    }
    return -1;
}

void StartMenu::render(SDL_Renderer* renderer) {
    for (auto& button : mButtons) {
        button.render(renderer);
    }
}

void StartMenu::toggleMusic() {
    mMusicEnabled = !mMusicEnabled;
    if (mMusicEnabled) {
        Mix_ResumeMusic();
        mButtons[1].loadTexture(nullptr, "MUSIC: ON");
    } else {
        Mix_PauseMusic();
        mButtons[1].loadTexture(nullptr, "MUSIC: OFF");
    }
} 