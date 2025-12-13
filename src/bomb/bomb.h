#ifndef BOMB_H
#define BOMB_H

#include "raylib.h"
#include <vector>

class Bomber;
class Map;
class Baf;

// Объявляем функцию для установки глобального вектора баффов
void SetGlobalBafs(std::vector<Baf*>* bafs);

class Bomb {
private:
    Vector2 position;
    Vector2 size;
    float timer;
    float explosionTime;
    int explosionRange;
    bool isExploded;
    bool shouldRemove;
    Bomber* owner;
    float explosionDisplayTime;

public:
    Bomb(float x, float y, int range, Bomber* bombOwner);
    
    void Update();
    void Draw();
    void Explode();
    void DestroyWallsInRange(Map* map); // Оставляем, но не будем начислять очки
    
    Vector2 GetPosition() const;
    Vector2 GetSize() const;
    Rectangle GetRect() const;
    bool IsExploded() const;
    bool ShouldRemove() const;
    int GetExplosionRange() const;
    Bomber* GetOwner() const;
    
    Rectangle GetExplosionArea() const;
    
    bool CheckCollision(const Rectangle& rect) const;
};

#endif