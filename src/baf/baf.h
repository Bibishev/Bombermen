#ifndef BAF_H
#define BAF_H

#include "raylib.h"

enum BafType {
    BAF_EXPLOSION_RANGE,  // +1 к радиусу взрыва
    BAF_MOVEMENT_SPEED,   // +10% к скорости
    BAF_BOMB_COUNT        // +1 к количеству бомб
};

class Baf {
private:
    Vector2 position;
    Vector2 size;
    BafType type;
    bool isActive;
    
public:
    Baf(float x, float y, BafType bafType);
    
    void Update();
    void Draw();
    
    BafType GetType() const;
    Vector2 GetPosition() const;
    Vector2 GetSize() const;
    Rectangle GetRect() const;
    bool IsActive() const;
    void Collect();
    
    const char* GetSymbol() const;
};

#endif