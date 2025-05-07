#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <vector>
#include "button.h"

class StartMenu {
private:
    std::vector<Button> mButtons;
    int mSelectedButton;
    bool mMusicEnabled;
    Mix_Music* mBGM;

public:
    StartMenu(SDL_Renderer* renderer, const unsigned int SCREEN_WIDTH, const unsigned int SCREEN_HEIGHT);
    ~StartMenu();

    int handleInput(SDL_Event& event);
    void render(SDL_Renderer* renderer);
    bool isMusicEnabled() const { return mMusicEnabled; }
    void toggleMusic();
};
