#ifndef MAP_H
#define MAP_H

#include "raylib.h"
#include <vector>
#include <utility>

class Map {
private:
    int width, height;
    int tileSize;
    std::vector<std::vector<int>> tiles;
    
    // Моргающие стены и портал
    std::vector<std::pair<int, int>> blinkingWalls;
    std::pair<int, int> portalPosition;
    bool hasPortal;
    float blinkTimer;
    bool showBlinkingWalls;
    
public:
    Map(int w, int h, int tileSize);
    void GenerateLevel();
    void Draw();
    bool IsWalkable(int gridX, int gridY) const;
    bool IsDestructible(int gridX, int gridY) const;
    bool IsWall(int gridX, int gridY) const;
    void DestroyWall(int gridX, int gridY);
    Vector2 GetTilePosition(int gridX, int gridY) const;
    Vector2 GetGridPosition(float worldX, float worldY) const;
    int GetTileSize() const { return tileSize; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    void GenerateDestructibleWalls(float density = 0.4f);
    
    // Методы для моргающих стен и портала
    void ActivateBlinkingWalls(); // Активировать моргание стен
    void UpdateBlinkingWalls();   // Обновить таймер моргания
    void DrawBlinkingWallsAndPortal(); // Нарисовать моргающие стены и портал
    bool HasPortal() const { return hasPortal; }
    std::pair<int, int> GetPortalPosition() const { return portalPosition; }
    std::vector<std::pair<int, int>> GetBlinkingWalls() const { return blinkingWalls; }
    
    // Методы для отладки
    int CountWalls() const;
    int CountDestructibleWalls() const;
};

#endif