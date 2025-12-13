#ifndef BUTTON_H
#define BUTTON_H

#include "raylib.h"

class Button {
private:
    Rectangle rect;
    const char* text;
    Color color;
    Color textColor;
    Color hoverColor;
    bool isHovered;

public:
    Button(float x, float y, float width, float height, const char* btnText);
    
    void Update();
    void Draw();
    bool IsClicked();
};

#endif