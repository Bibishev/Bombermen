#ifndef MOB_H
#define MOB_H

#include "raylib.h"
#include <vector>

class Map;
class Bomb;

// ИЗМЕНЕНО: Обновляем enum для типов мобов
enum MobType {
    MOB_NORMAL,
    MOB_FAST,      // В 2 раза быстрее
    MOB_SUPER_FAST // В 3 раза быстрее (только на 3 уровне)
};

class Mob {
private:
    Vector2 position;
    Vector2 size;
    Color color;
    float baseSpeed;        // ИЗМЕНЕНО: базовая скорость
    float currentSpeed;     // ИЗМЕНЕНО: текущая скорость с модификаторами
    Vector2 direction;
    Map* map;
    bool isAlive;
    float directionChangeTimer;
    MobType type;           // ИЗМЕНЕНО: тип моба

public:
    // ИЗМЕНЕНО: добавляем параметр типа
    Mob(float startX, float startY, MobType mobType = MOB_NORMAL);
    
    void Update();
    void Draw();
    void SetMap(Map* gameMap);
    void CheckBombDamage(const std::vector<Bomb*>& bombs);
    
    Vector2 GetPosition() const;
    Vector2 GetSize() const;
    Rectangle GetRect() const;
    bool IsAlive() const;
    MobType GetType() const; // ИЗМЕНЕНО: геттер типа
    
    bool CheckCollision(const Rectangle& rect) const;
    
private:
    bool CanMove(Vector2 newPosition) const;
    void ChangeDirection();
};

#endif