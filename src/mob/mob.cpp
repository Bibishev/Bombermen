#include "mob.h"
#include "map/map.h"
#include "bomb/bomb.h"
#include <iostream>
#include <random>

// Внешние текстуры (объявлены в main.cpp)
extern Texture2D mobTexture;

Mob::Mob(float startX, float startY, MobType mobType) {
    position = { startX, startY };
    size = { 55, 55 };
    type = mobType;
    map = nullptr;
    isAlive = true;
    directionChangeTimer = 0.0f;
    
    // ИЗМЕНЕНО: настройки в зависимости от типа
    switch (type) {
        case MOB_NORMAL:
            baseSpeed = 2.67f;
            currentSpeed = baseSpeed;
            color = PURPLE;
            break;
        case MOB_FAST:
            baseSpeed = 2.67f * 2.0f; // В 2 раза быстрее
            currentSpeed = baseSpeed;
            color = Color{255, 50, 50, 255}; // Красный
            break;
        case MOB_SUPER_FAST:
            baseSpeed = 2.67f * 3.0f; // В 3 раза быстрее
            currentSpeed = baseSpeed;
            color = Color{255, 20, 147, 255}; // Розовый/фиолетовый
            break;
    }
    
    direction = { 1, 0 };
}

void Mob::SetMap(Map* gameMap) {
    map = gameMap;
}

void Mob::Update() {
    if (!isAlive) return;
    if (map == nullptr) return;
    
    directionChangeTimer -= GetFrameTime();
    
    if (directionChangeTimer <= 0) {
        if (GetRandomValue(0, 100) < 10) {
            ChangeDirection();
        }
        directionChangeTimer = GetRandomValue(10, 30) / 10.0f;
    }
    
    Vector2 newPosition = { 
        position.x + direction.x * currentSpeed, 
        position.y + direction.y * currentSpeed 
    };
    
    if (CanMove(newPosition)) {
        position = newPosition;
    } else {
        ChangeDirection();
    }
}

void Mob::Draw() {
    if (!isAlive) return;
    
    // Если текстура загружена, рисуем её
    if (mobTexture.id != 0) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)mobTexture.width, (float)mobTexture.height };
        Rectangle destRec = { position.x, position.y, size.x, size.y };
        Vector2 origin = { 0, 0 };
        
        // Меняем цвет оттенка в зависимости от типа моба
        Color tint = WHITE;
        switch (type) {
            case MOB_NORMAL:
                tint = PURPLE;
                break;
            case MOB_FAST:
                tint = RED;
                break;
            case MOB_SUPER_FAST:
                tint = Color{255, 20, 147, 255}; // Розовый
                break;
        }
        
        DrawTexturePro(mobTexture, sourceRec, destRec, origin, 0.0f, tint);
        
        // Для отладки - рисуем рамку
        // DrawRectangleLinesEx(destRec, 2, GREEN);
    } else {
        // Резервный вариант: рисуем цветной прямоугольник
        DrawRectangleRec(GetRect(), color);
        
        Color darkerColor = {
            static_cast<unsigned char>(color.r / 2),
            static_cast<unsigned char>(color.g / 2),
            static_cast<unsigned char>(color.b / 2),
            255
        };
        DrawRectangleLinesEx(GetRect(), 3, darkerColor);
        
        int eyeSize = 8;
        int eyeOffset = 20;
        
        if (direction.x > 0) {
            DrawRectangle(position.x + size.x - eyeOffset, position.y + eyeOffset, eyeSize, eyeSize, WHITE);
            DrawRectangle(position.x + size.x - eyeOffset, position.y + size.y - eyeOffset - eyeSize, eyeSize, eyeSize, WHITE);
        } else if (direction.x < 0) {
            DrawRectangle(position.x + eyeOffset, position.y + eyeOffset, eyeSize, eyeSize, WHITE);
            DrawRectangle(position.x + eyeOffset, position.y + size.y - eyeOffset - eyeSize, eyeSize, eyeSize, WHITE);
        } else if (direction.y > 0) {
            DrawRectangle(position.x + eyeOffset, position.y + size.y - eyeOffset, eyeSize, eyeSize, WHITE);
            DrawRectangle(position.x + size.x - eyeOffset - eyeSize, position.y + size.y - eyeOffset, eyeSize, eyeSize, WHITE);
        } else if (direction.y < 0) {
            DrawRectangle(position.x + eyeOffset, position.y + eyeOffset, eyeSize, eyeSize, WHITE);
            DrawRectangle(position.x + size.x - eyeOffset - eyeSize, position.y + eyeOffset, eyeSize, eyeSize, WHITE);
        }
        
        int mouthY = position.y + size.y / 2 + 10;
        DrawLine(position.x + 20, mouthY, position.x + size.x - 20, mouthY, DARKPURPLE);
    }
}


bool Mob::CanMove(Vector2 newPosition) const {
    if (map == nullptr) return false;
    
    Vector2 gridPos = map->GetGridPosition(newPosition.x, newPosition.y);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        return false;
    }
    
    gridPos = map->GetGridPosition(newPosition.x + size.x - 1, newPosition.y);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        return false;
    }
    
    gridPos = map->GetGridPosition(newPosition.x, newPosition.y + size.y - 1);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        return false;
    }
    
    gridPos = map->GetGridPosition(newPosition.x + size.x - 1, newPosition.y + size.y - 1);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        return false;
    }
    
    return true;
}

void Mob::ChangeDirection() {
    if (map == nullptr) return;
    
    std::vector<Vector2> possibleDirections;
    possibleDirections.push_back({0, -1});
    possibleDirections.push_back({0, 1});
    possibleDirections.push_back({-1, 0});
    possibleDirections.push_back({1, 0});
    
    std::vector<Vector2> availableDirections;
    for (size_t i = 0; i < possibleDirections.size(); i++) {
        Vector2 dir = possibleDirections[i];
        if (!(dir.x == -direction.x && dir.y == -direction.y)) {
            Vector2 testPosition = {
                position.x + dir.x * currentSpeed,
                position.y + dir.y * currentSpeed
            };
            if (CanMove(testPosition)) {
                availableDirections.push_back(dir);
            }
        }
    }
    
    if (!availableDirections.empty()) {
        int randomIndex = GetRandomValue(0, availableDirections.size() - 1);
        direction = availableDirections[randomIndex];
    } else {
        direction.x = -direction.x;
        direction.y = -direction.y;
    }
}

void Mob::CheckBombDamage(const std::vector<Bomb*>& bombs) {
    if (!isAlive) return;
    
    for (auto bomb : bombs) {
        if (bomb->IsExploded()) {
            if (bomb->CheckCollision(GetRect())) {
                // Шанс выпадения баффа 35% (ИЗМЕНЕНО с 15%)
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_real_distribution<float> dis(0.0f, 1.0f);
                
                if (dis(gen) < 0.35f) {
                    std::cout << "Baf should drop from mob death at (" 
                              << position.x << ", " << position.y << ")" << std::endl;
                }
                
                isAlive = false;
                std::cout << "Mob died from bomb explosion! Type: " << type << std::endl;
                return;
            }
        }
    }
}

Vector2 Mob::GetPosition() const {
    return position;
}

Vector2 Mob::GetSize() const {
    return size;
}

Rectangle Mob::GetRect() const {
    return { position.x, position.y, size.x, size.y };
}

bool Mob::IsAlive() const {
    return isAlive;
}

MobType Mob::GetType() const {
    return type;
}

bool Mob::CheckCollision(const Rectangle& rect) const {
    return CheckCollisionRecs(GetRect(), rect);
}