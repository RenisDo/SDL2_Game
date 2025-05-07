#include "stopwatch.h"

Stopwatch::Stopwatch(const SDL_Rect& rect, const SDL_Color& colour, TTF_Font* const font, const SDL_Color& fontColour) 
    :  UserInterface(rect, colour, font, fontColour),
    mStartTime(0), mElapsedTime(""), mIsPaused(false), mPauseTime(0), mTotalPausedTime(0) {
    
}

void Stopwatch::start() {
    time(&mStartTime);
    mIsPaused = false;
    mTotalPausedTime = 0;
}

void Stopwatch::pause() {
    if (!mIsPaused) {
        time(&mPauseTime);
        mIsPaused = true;
    }
}

void Stopwatch::resume() {
    if (mIsPaused) {
        time_t currentTime;
        time(&currentTime);
        mTotalPausedTime += currentTime - mPauseTime;
        mIsPaused = false;
    }
}

void Stopwatch::calculateTime(SDL_Renderer* const renderer) {
    if (!mIsPaused) {
        time_t currentTime;
        time(&currentTime);
        time_t difference = currentTime - mStartTime - mTotalPausedTime;
        struct tm* timeinfo = gmtime(&difference);
        strftime(mElapsedTime, sizeof(mElapsedTime), "%H:%M:%S", timeinfo);

        loadTexture(renderer, mElapsedTime);
    }
}
