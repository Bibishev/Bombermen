#include "bomb.h"
#include "bomber.h"
#include "map/map.h"
#include "baf.h"
#include <iostream>
#include <random>

// Внешние текстуры (объявлены в main.cpp)
extern Texture2D bombTexture;
extern Texture2D boomTexture;

std::vector<Baf*>* globalBafs = nullptr;

void SetGlobalBafs(std::vector<Baf*>* bafs) {
    globalBafs = bafs;
}

Bomb::Bomb(float x, float y, int range, Bomber* bombOwner) {
    position = { x, y };
    size = { 60, 60 };
    explosionTime = 3.0f;
    timer = explosionTime;
    explosionRange = range;
    isExploded = false;
    shouldRemove = false;
    owner = bombOwner;
    explosionDisplayTime = 0.5f;
}

void Bomb::Update() {
    if (isExploded) {
        explosionDisplayTime -= GetFrameTime();
        if (explosionDisplayTime <= 0) {
            shouldRemove = true;
        }
        return;
    }
    
    timer -= GetFrameTime();
    
    if (timer <= 0) {
        Explode();
    }
}

void Bomb::Draw() {
    if (shouldRemove) return;
    
    if (!isExploded) {
        // Рисуем бомбу с текстурой
        if (bombTexture.id != 0) {
            Rectangle sourceRec = { 0.0f, 0.0f, (float)bombTexture.width, (float)bombTexture.height };
            Rectangle destRec = { position.x, position.y, size.x, size.y };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(bombTexture, sourceRec, destRec, origin, 0.0f, WHITE);
            
            // Таймер на бомбе
            int secondsLeft = static_cast<int>(timer) + 1;
            DrawText(TextFormat("%d", secondsLeft), 
                     position.x + size.x/2 - 5, 
                     position.y + size.y/2 - 10, 
                     20, BLACK);
            
            // Для отладки - рисуем рамку
            // DrawRectangleLinesEx(destRec, 2, BLUE);
        } else {
            DrawCircle(position.x + size.x/2, position.y + size.y/2, size.x/2, RED);
            int secondsLeft = static_cast<int>(timer) + 1;
            DrawText(TextFormat("%d", secondsLeft), 
                     position.x + size.x/2 - 5, 
                     position.y + size.y/2 - 10, 
                     20, WHITE);
        }
    } else {
        // Рисуем взрыв с текстурой
        if (boomTexture.id != 0) {
            // Центральный взрыв
            Rectangle sourceRec = { 0.0f, 0.0f, (float)boomTexture.width, (float)boomTexture.height };
            Rectangle destRec = { position.x, position.y, size.x, size.y };
            Vector2 origin = { 0, 0 };
            DrawTexturePro(boomTexture, sourceRec, destRec, origin, 0.0f, WHITE);
            
            // Лучи взрыва
            float cellSize = 64.0f;
            for (int i = 1; i <= explosionRange; i++) {
                // Вверх
                Rectangle upDest = { 
                    position.x + size.x/2 - 10.0f, 
                    position.y - i * cellSize + size.y/2 - 10.0f, 
                    20.0f, cellSize 
                };
                DrawTexturePro(boomTexture, sourceRec, upDest, origin, 0.0f, WHITE);
                
                // Вниз
                Rectangle downDest = { 
                    position.x + size.x/2 - 10.0f,
                    position.y + i * cellSize + size.y/2 - 10.0f,
                    20.0f, cellSize 
                };
                DrawTexturePro(boomTexture, sourceRec, downDest, origin, 0.0f, WHITE);
                
                // Влево
                Rectangle leftDest = { 
                    position.x - i * cellSize + size.x/2 - 10.0f,
                    position.y + size.y/2 - 10.0f,
                    cellSize, 20.0f 
                };
                DrawTexturePro(boomTexture, sourceRec, leftDest, origin, 0.0f, WHITE);
                
                // Вправо
                Rectangle rightDest = { 
                    position.x + i * cellSize + size.x/2 - 10.0f,
                    position.y + size.y/2 - 10.0f,
                    cellSize, 20.0f 
                };
                DrawTexturePro(boomTexture, sourceRec, rightDest, origin, 0.0f, WHITE);
            }
        } else {
            // Резервный вариант
            DrawCircle(position.x + size.x/2, position.y + size.y/2, size.x/2, YELLOW);
            
            float cellSize = 64.0f;
            for (int i = 1; i <= explosionRange; i++) {
                DrawRectangle(position.x + size.x/2 - 10.0f, 
                             position.y - i * cellSize + size.y/2 - 10.0f, 
                             20.0f, cellSize, YELLOW);
                
                DrawRectangle(position.x + size.x/2 - 10.0f,
                             position.y + i * cellSize + size.y/2 - 10.0f,
                             20.0f, cellSize, YELLOW);
                
                DrawRectangle(position.x - i * cellSize + size.x/2 - 10.0f,
                             position.y + size.y/2 - 10.0f,
                             cellSize, 20.0f, YELLOW);
                
                DrawRectangle(position.x + i * cellSize + size.x/2 - 10.0f,
                             position.y + size.y/2 - 10.0f,
                             cellSize, 20.0f, YELLOW);
            }
        }
    }
}

void Bomb::Explode() {
    if (isExploded) return;
    
    isExploded = true;
    std::cout << "Bomb exploded!" << std::endl;
}

Vector2 Bomb::GetPosition() const {
    return position;
}

Vector2 Bomb::GetSize() const {
    return size;
}

Rectangle Bomb::GetRect() const {
    return { position.x, position.y, size.x, size.y };
}

bool Bomb::IsExploded() const {
    return isExploded;
}

bool Bomb::ShouldRemove() const {
    return shouldRemove;
}

int Bomb::GetExplosionRange() const {
    return explosionRange;
}

Bomber* Bomb::GetOwner() const {
    return owner;
}

Rectangle Bomb::GetExplosionArea() const {
    float cellSize = 64.0f;
    float rangeSize = explosionRange * cellSize;
    
    float minX = position.x;
    float minY = position.y;
    float maxX = position.x + size.x;
    float maxY = position.y + size.y;
    
    minX -= rangeSize;
    maxX += rangeSize;
    minY -= rangeSize;
    maxY += rangeSize;
    
    return { minX, minY, maxX - minX, maxY - minY };
}

bool Bomb::CheckCollision(const Rectangle& rect) const {
    if (!isExploded) return false;
    
    Rectangle bombRect = GetRect();
    if (CheckCollisionRecs(rect, bombRect)) {
        return true;
    }
    
    float cellSize = 64.0f;
    float centerX = position.x + size.x / 2;
    float centerY = position.y + size.y / 2;
    
    for (int i = 1; i <= explosionRange; i++) {
        Rectangle upRect = { centerX - 32, centerY - i * cellSize - 32, 64, cellSize };
        if (CheckCollisionRecs(rect, upRect)) {
            return true;
        }
        
        Rectangle downRect = { centerX - 32, centerY + i * cellSize - 32, 64, cellSize };
        if (CheckCollisionRecs(rect, downRect)) {
            return true;
        }
        
        Rectangle leftRect = { centerX - i * cellSize - 32, centerY - 32, cellSize, 64 };
        if (CheckCollisionRecs(rect, leftRect)) {
            return true;
        }
        
        Rectangle rightRect = { centerX + i * cellSize - 32, centerY - 32, cellSize, 64 };
        if (CheckCollisionRecs(rect, rightRect)) {
            return true;
        }
    }
    
    return false;
}

void Bomb::DestroyWallsInRange(Map* map) {
    if (!isExploded || !map) return;
    
    Vector2 gridPos = map->GetGridPosition(position.x, position.y);
    int centerX = static_cast<int>(gridPos.x);
    int centerY = static_cast<int>(gridPos.y);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    std::uniform_int_distribution<> typeDis(0, 2);
    
    std::vector<std::pair<int, int>> blinkingWalls = map->GetBlinkingWalls();
    
    // ИЗМЕНЕНО: Убираем подсчёт очков за стены
    // int destroyedWalls = 0; // Убрали
    
    // Проверяем центральную клетку
    if (map->IsDestructible(centerX, centerY)) {
        bool isBlinkingWall = false;
        for (size_t j = 0; j < blinkingWalls.size(); j++) {
            int bx = blinkingWalls[j].first;
            int by = blinkingWalls[j].second;
            if (bx == centerX && by == centerY) {
                isBlinkingWall = true;
                break;
            }
        }
        
        float bafChance = isBlinkingWall ? 0.05f : 0.015f;
        
        if (dis(gen) < bafChance && globalBafs != nullptr) {
            float bafX = centerX * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
            float bafY = centerY * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
            
            int type = typeDis(gen);
            Baf* newBaf = new Baf(bafX, bafY, static_cast<BafType>(type));
            globalBafs->push_back(newBaf);
        }
        map->DestroyWall(centerX, centerY);
    }
    
    // Проверяем во всех направлениях
    for (int i = 1; i <= explosionRange; i++) {
        // Вверх
        if (centerY - i >= 0 && map->IsDestructible(centerX, centerY - i)) {
            bool isBlinkingWall = false;
            for (size_t j = 0; j < blinkingWalls.size(); j++) {
                int bx = blinkingWalls[j].first;
                int by = blinkingWalls[j].second;
                if (bx == centerX && by == centerY - i) {
                    isBlinkingWall = true;
                    break;
                }
            }
            
            float bafChance = isBlinkingWall ? 0.05f : 0.015f;
            
            if (dis(gen) < bafChance && globalBafs != nullptr) {
                float bafX = centerX * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                float bafY = (centerY - i) * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                
                int type = typeDis(gen);
                Baf* newBaf = new Baf(bafX, bafY, static_cast<BafType>(type));
                globalBafs->push_back(newBaf);
            }
            map->DestroyWall(centerX, centerY - i);
        }
        
        // Вниз
        if (centerY + i < map->GetHeight() && map->IsDestructible(centerX, centerY + i)) {
            bool isBlinkingWall = false;
            for (size_t j = 0; j < blinkingWalls.size(); j++) {
                int bx = blinkingWalls[j].first;
                int by = blinkingWalls[j].second;
                if (bx == centerX && by == centerY + i) {
                    isBlinkingWall = true;
                    break;
                }
            }
            
            float bafChance = isBlinkingWall ? 0.05f : 0.015f;
            
            if (dis(gen) < bafChance && globalBafs != nullptr) {
                float bafX = centerX * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                float bafY = (centerY + i) * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                
                int type = typeDis(gen);
                Baf* newBaf = new Baf(bafX, bafY, static_cast<BafType>(type));
                globalBafs->push_back(newBaf);
            }
            map->DestroyWall(centerX, centerY + i);
        }
        
        // Влево
        if (centerX - i >= 0 && map->IsDestructible(centerX - i, centerY)) {
            bool isBlinkingWall = false;
            for (size_t j = 0; j < blinkingWalls.size(); j++) {
                int bx = blinkingWalls[j].first;
                int by = blinkingWalls[j].second;
                if (bx == centerX - i && by == centerY) {
                    isBlinkingWall = true;
                    break;
                }
            }
            
            float bafChance = isBlinkingWall ? 0.05f : 0.015f;
            
            if (dis(gen) < bafChance && globalBafs != nullptr) {
                float bafX = (centerX - i) * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                float bafY = centerY * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                
                int type = typeDis(gen);
                Baf* newBaf = new Baf(bafX, bafY, static_cast<BafType>(type));
                globalBafs->push_back(newBaf);
            }
            map->DestroyWall(centerX - i, centerY);
        }
        
        // Вправо
        if (centerX + i < map->GetWidth() && map->IsDestructible(centerX + i, centerY)) {
            bool isBlinkingWall = false;
            for (size_t j = 0; j < blinkingWalls.size(); j++) {
                int bx = blinkingWalls[j].first;
                int by = blinkingWalls[j].second;
                if (bx == centerX + i && by == centerY) {
                    isBlinkingWall = true;
                    break;
                }
            }
            
            float bafChance = isBlinkingWall ? 0.05f : 0.015f;
            
            if (dis(gen) < bafChance && globalBafs != nullptr) {
                float bafX = (centerX + i) * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                float bafY = centerY * map->GetTileSize() + map->GetTileSize() / 2.0f - 20.0f;
                
                int type = typeDis(gen);
                Baf* newBaf = new Baf(bafX, bafY, static_cast<BafType>(type));
                globalBafs->push_back(newBaf);
            }
            map->DestroyWall(centerX + i, centerY);
        }
    }
}   