#ifndef BOMBER_H
#define BOMBER_H

#include "raylib.h"
#include "bomb.h"
#include <vector>

class Map;
class Mob;
class Baf;

class Bomber {
private:
    Vector2 position;
    Vector2 size;
    Color color;
    float speed;
    std::size_t bombLimit;
    int bombRange;
    bool isAlive;
    int lives;
    float invincibilityTimer;
    Map* map;
    
    std::vector<Bomb*> bombs;
    
    // Система баффов
    int explosionRangeBuff;  // Текущий бонус к радиусу (0-3)
    int movementSpeedBuff;   // Текущий бонус к скорости (0-3)
    int bombCountBuff;       // Текущий бонус к количеству бомб (0-3)
    float baseSpeed;         // Базовая скорость (без баффов)
    
    // ИЗМЕНЕНО: Добавляем переменную для очков
    int score;
    
public:
    Bomber(float startX, float startY);
    ~Bomber();
    
    void Update();
    void Draw();
    void PlaceBomb();
    void SetMap(Map* gameMap);
    void TakeDamage();
    void CheckMobCollisions(const std::vector<Mob*>& mobs);
    void CheckBafCollisions(std::vector<Baf*>& bafs);
    
    Vector2 GetPosition() const;
    Vector2 GetSize() const;
    Rectangle GetRect() const;
    bool IsAlive() const;
    std::size_t GetBombLimit() const;
    int GetBombRange() const;
    int GetLives() const;
    bool IsInvincible() const;
    Map* GetMap() const { return map; }
    
    // Методы для баффов
    void ApplyBaf(int bafType);
    int GetExplosionRangeBuff() const;
    int GetMovementSpeedBuff() const;
    int GetBombCountBuff() const;
    float GetCurrentSpeed() const;
    
    // ИЗМЕНЕНО: Методы для работы с очками
    int GetScore() const;
    void AddScore(int points);
    void ResetScore();
    
    // Новый метод для установки жизней
    void SetLives(int newLives);
    
    void SetPosition(Vector2 newPos);
    void SetBombLimit(std::size_t limit);
    void SetBombRange(int range);
    void SetSpeed(float newSpeed);
    
    bool CheckCollision(const Rectangle& rect) const;
    
    void Kill();
    void Respawn(float x, float y);
    
    void RemoveBomb(Bomb* bomb);
    const std::vector<Bomb*>& GetBombs() const;
};

#endif