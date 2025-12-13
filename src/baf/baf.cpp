#include "baf.h"
#include <iostream>
#include <cmath>

// Внешние текстуры (объявлены в main.cpp)
extern Texture2D bafExplosionTexture;
extern Texture2D bafSpeedTexture;
extern Texture2D bafBombTexture;

Baf::Baf(float x, float y, BafType bafType) {
    position = { x, y };
    size = { 40, 40 };
    type = bafType;
    isActive = true;
}

void Baf::Update() {
    // Бафф не двигается, только проверяем сбор
}

void Baf::Draw() {
    if (!isActive) return;
    
    // Получаем текстуру для данного типа баффа
    Texture2D texture;
    switch (type) {
        case BAF_EXPLOSION_RANGE:
            texture = bafExplosionTexture;
            break;
        case BAF_MOVEMENT_SPEED:
            texture = bafSpeedTexture;
            break;
        case BAF_BOMB_COUNT:
            texture = bafBombTexture;
            break;
        default:
            texture = Texture2D{0}; // Пустая текстура
            break;
    }
    
    // Если текстура загружена, рисуем её
    if (texture.id != 0) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
        Rectangle destRec = { position.x, position.y, size.x, size.y };
        Vector2 origin = { 0, 0 };
        
        DrawTexturePro(texture, sourceRec, destRec, origin, 0.0f, WHITE);
        
        // Легкое мерцание (пульсация)
        float time = static_cast<float>(GetTime());
        float pulse = sin(time * 5.0f) * 0.2f + 1.0f;
        DrawRectangleLinesEx(
            Rectangle{
                position.x - 2.0f * pulse,
                position.y - 2.0f * pulse,
                size.x + 4.0f * pulse,
                size.y + 4.0f * pulse
            },
            2,
            ColorAlpha(GREEN, 0.5f)
        );
        
        // Для отладки - рисуем рамку
        // DrawRectangleLinesEx(destRec, 2, YELLOW);
    } else {
        // Резервный вариант: рисуем простую графику
        DrawRectangleRec(GetRect(), GREEN);
        DrawRectangleLinesEx(GetRect(), 3, DARKGREEN);
        
        int fontSize = 20;
        const char* symbol = GetSymbol();
        int textWidth = MeasureText(symbol, fontSize);
        
        DrawText(symbol,
                 position.x + (size.x - textWidth) / 2,
                 position.y + (size.y - fontSize) / 2,
                 fontSize, WHITE);
        
        float time = static_cast<float>(GetTime());
        float pulse = sin(time * 5.0f) * 0.2f + 1.0f;
        DrawRectangleLinesEx(
            Rectangle{
                position.x - 2.0f * pulse,
                position.y - 2.0f * pulse,
                size.x + 4.0f * pulse,
                size.y + 4.0f * pulse
            },
            2,
            ColorAlpha(GREEN, 0.5f)
        );
    }
}

BafType Baf::GetType() const {
    return type;
}

Vector2 Baf::GetPosition() const {
    return position;
}

Vector2 Baf::GetSize() const {
    return size;
}

Rectangle Baf::GetRect() const {
    return { position.x, position.y, size.x, size.y };
}

bool Baf::IsActive() const {
    return isActive;
}

void Baf::Collect() {
    isActive = false;
    std::cout << "Baf collected: ";
    switch (type) {
        case BAF_EXPLOSION_RANGE: std::cout << "Explosion Range"; break;
        case BAF_MOVEMENT_SPEED: std::cout << "Movement Speed"; break;
        case BAF_BOMB_COUNT: std::cout << "Bomb Count"; break;
    }
    std::cout << std::endl;
}

const char* Baf::GetSymbol() const {
    switch (type) {
        case BAF_EXPLOSION_RANGE: return "R";
        case BAF_MOVEMENT_SPEED: return "S";
        case BAF_BOMB_COUNT: return "B";
        default: return "?";
    }
}