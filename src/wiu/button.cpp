#include "button.h"

Button::Button(float x, float y, float width, float height, const char* btnText) {
    rect = { x, y, width, height };
    text = btnText;
    color = BLUE;
    textColor = WHITE;
    hoverColor = SKYBLUE;
    isHovered = false;
}

void Button::Update() {
    Vector2 mousePoint = GetMousePosition();
    isHovered = CheckCollisionPointRec(mousePoint, rect);
}

void Button::Draw() {
    Color btnColor = isHovered ? hoverColor : color;
    DrawRectangleRec(rect, btnColor);
    DrawRectangleLinesEx(rect, 2, DARKBLUE);
    
    int fontSize = 24;
    int textWidth = MeasureText(text, fontSize);
    int textX = rect.x + (rect.width - textWidth) / 2;
    int textY = rect.y + (rect.height - fontSize) / 2;
    
    DrawText(text, textX, textY, fontSize, textColor);
}

bool Button::IsClicked() {
    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}