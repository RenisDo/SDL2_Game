#include "button.h"

Button::Button(const SDL_Rect& rect, const SDL_Color& colour, TTF_Font* const font, const SDL_Color& fontColour)
    : UserInterface(rect, colour, font, fontColour) {

}

bool Button::isMouseInside(const int x, const int y) const {
    bool inside = true;

	if (x < mRect.x)
	{
		inside = false;
	}
	else if (x > mRect.x + mRect.w)
	{
		inside = false;
	}
	else if (y < mRect.y)
	{
		inside = false;
	}
	else if (y > mRect.y + mRect.h)
	{
		inside = false;
	}

    return inside;
}

void Button::changeColourTo(const SDL_Color& colour) {
    mColour = colour;
}
