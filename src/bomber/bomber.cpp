#include "bomber.h"
#include "map/map.h"
#include "mob/mob.h"
#include "baf.h"
#include <algorithm>
#include <iostream>

// Внешние текстуры (объявлены в main.cpp)
extern Texture2D bomberTexture;

Bomber::Bomber(float startX, float startY) {
    position = { startX, startY };
    size = { 32, 32 };
    color = BLUE;
    baseSpeed = 4.0f;
    speed = baseSpeed;
    bombLimit = 1;
    bombRange = 1;
    isAlive = true;
    lives = 3;
    invincibilityTimer = 0.0f;
    map = nullptr;
    
    // Инициализация баффов
    explosionRangeBuff = 0;
    movementSpeedBuff = 0;
    bombCountBuff = 0;
    
    // Инициализация очков
    score = 0;
}

Bomber::~Bomber() {
    for (auto bomb : bombs) {
        delete bomb;
    }
    bombs.clear();
}

void Bomber::SetMap(Map* gameMap) {
    map = gameMap;
}

void Bomber::Update() {
    if (!isAlive) return;
    if (map == nullptr) return;
    
    if (invincibilityTimer > 0) {
        invincibilityTimer -= GetFrameTime();
    }
    
    Vector2 movement = { 0, 0 };
    
    if (IsKeyDown(KEY_W)) movement.y = -speed;
    if (IsKeyDown(KEY_S)) movement.y = speed;
    if (IsKeyDown(KEY_A)) movement.x = -speed;
    if (IsKeyDown(KEY_D)) movement.x = speed;
    
    if (movement.x != 0 && movement.y != 0) {
        movement.x *= 0.7071f;
        movement.y *= 0.7071f;
    }
    
    Vector2 newPosition = { position.x + movement.x, position.y + movement.y };
    
    bool canMove = true;
    
    Vector2 gridPos = map->GetGridPosition(newPosition.x, newPosition.y);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        canMove = false;
    }
    
    gridPos = map->GetGridPosition(newPosition.x + size.x - 1, newPosition.y);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        canMove = false;
    }
    
    gridPos = map->GetGridPosition(newPosition.x, newPosition.y + size.y - 1);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        canMove = false;
    }
    
    gridPos = map->GetGridPosition(newPosition.x + size.x - 1, newPosition.y + size.y - 1);
    if (!map->IsWalkable(gridPos.x, gridPos.y)) {
        canMove = false;
    }
    
    if (canMove) {
        position = newPosition;
    }
    
    // Проверка столкновения с взрывами бомб
    for (auto bomb : bombs) {
        if (bomb->IsExploded()) {
            // Используем новый метод проверки столкновения
            if (bomb->CheckCollision(GetRect())) {
                TakeDamage();
            }
            
            // Разрушаем стены взрывом
            bomb->DestroyWallsInRange(map);
        }
    }
    
    for (auto it = bombs.begin(); it != bombs.end();) {
        Bomb* bomb = *it;
        bomb->Update();
        
        if (bomb->ShouldRemove()) {
            delete bomb;
            it = bombs.erase(it);
        } else {
            ++it;
        }
    }
    
    if (IsKeyPressed(KEY_SPACE)) {
        PlaceBomb();
    }
}

void Bomber::Draw() {
    if (!isAlive) return;
    
    // Если текстура загружена, рисуем её
    if (bomberTexture.id != 0) {
        Rectangle sourceRec = { 0.0f, 0.0f, (float)bomberTexture.width, (float)bomberTexture.height };
        Rectangle destRec = { position.x, position.y, size.x, size.y };
        Vector2 origin = { 0, 0 };
        
        // Если игрок неуязвим - мигаем
        if (IsInvincible()) {
            if (static_cast<int>(GetTime() * 10) % 2 == 0) {
                DrawTexturePro(bomberTexture, sourceRec, destRec, origin, 0.0f, WHITE);
            }
        } else {
            DrawTexturePro(bomberTexture, sourceRec, destRec, origin, 0.0f, WHITE);
        }
        
        // Для отладки - рисуем рамку
        // DrawRectangleLinesEx(destRec, 2, RED);
    } else {
        // Иначе рисуем цветной прямоугольник (резервный вариант)
        if (IsInvincible()) {
            if (static_cast<int>(GetTime() * 10) % 2 == 0) {
                DrawRectangleRec(GetRect(), color);
            }
        } else {
            DrawRectangleRec(GetRect(), color);
        }
    }
    
    for (auto bomb : bombs) {
        bomb->Draw();
    }
}


void Bomber::PlaceBomb() {
    if (!isAlive) return;
    
    // Учитываем бафф к количеству бомб
    std::size_t currentBombLimit = bombLimit + bombCountBuff;
    if (bombs.size() >= currentBombLimit) return;
    
    int gridSize = 64;
    float bombX = static_cast<int>((position.x + size.x / 2) / gridSize) * gridSize;
    float bombY = static_cast<int>((position.y + size.y / 2) / gridSize) * gridSize;
    
    // Учитываем бафф к радиусу взрыва
    int currentBombRange = bombRange + explosionRangeBuff;
    
    Bomb* newBomb = new Bomb(bombX, bombY, currentBombRange, this);
    bombs.push_back(newBomb);
}

void Bomber::TakeDamage() {
    if (IsInvincible()) return;
    
    lives--;
    std::cout << "Bomber took damage! Lives remaining: " << lives << std::endl;
    
    if (lives <= 0) {
        Kill();
        std::cout << "Bomber is dead!" << std::endl;
    } else {
        invincibilityTimer = 2.0f;
    }
}

void Bomber::CheckMobCollisions(const std::vector<Mob*>& mobs) {
    if (!isAlive || IsInvincible()) return;
    
    for (auto mob : mobs) {
        if (mob->IsAlive() && CheckCollision(mob->GetRect())) {
            TakeDamage();
            break;
        }
    }
}

// Новый метод для проверки столкновений с баффами
void Bomber::CheckBafCollisions(std::vector<Baf*>& bafs) {
    if (!isAlive) return;
    
    for (auto it = bafs.begin(); it != bafs.end();) {
        Baf* baf = *it;
        if (baf->IsActive() && CheckCollision(baf->GetRect())) {
            ApplyBaf(static_cast<int>(baf->GetType()));
            baf->Collect();
            delete baf;
            it = bafs.erase(it);
        } else {
            ++it;
        }
    }
}

// Метод применения баффа
void Bomber::ApplyBaf(int bafType) {
    switch (bafType) {
        case 0: // BAF_EXPLOSION_RANGE
            if (explosionRangeBuff < 3) {
                explosionRangeBuff++;
                std::cout << "Explosion range increased to: " << (bombRange + explosionRangeBuff) << std::endl;
            }
            break;
            
        case 1: // BAF_MOVEMENT_SPEED
            if (movementSpeedBuff < 3) {
                movementSpeedBuff++;
                // Пересчитываем скорость: +20% за каждый бафф
                speed = baseSpeed * (1.0f + 0.2f * movementSpeedBuff);
                std::cout << "Movement speed increased to: " << speed << " (+" << (movementSpeedBuff * 20) << "%)" << std::endl;
            }
            break;
            
        case 2: // BAF_BOMB_COUNT
            if (bombCountBuff < 3) {
                bombCountBuff++;
                std::cout << "Bomb limit increased to: " << (bombLimit + bombCountBuff) << std::endl;
            }
            break;
    }
}

Vector2 Bomber::GetPosition() const {
    return position;
}

Vector2 Bomber::GetSize() const {
    return size;
}

Rectangle Bomber::GetRect() const {
    return { position.x, position.y, size.x, size.y };
}

bool Bomber::IsAlive() const {
    return isAlive;
}

std::size_t Bomber::GetBombLimit() const {
    return bombLimit + bombCountBuff;
}

int Bomber::GetBombRange() const {
    return bombRange + explosionRangeBuff;
}

int Bomber::GetLives() const {
    return lives;
}

bool Bomber::IsInvincible() const {
    return invincibilityTimer > 0;
}

// Геттеры для баффов
int Bomber::GetExplosionRangeBuff() const {
    return explosionRangeBuff;
}

int Bomber::GetMovementSpeedBuff() const {
    return movementSpeedBuff;
}

int Bomber::GetBombCountBuff() const {
    return bombCountBuff;
}

float Bomber::GetCurrentSpeed() const {
    return speed;
}

// Методы для работы с очками
int Bomber::GetScore() const {
    return score;
}

void Bomber::AddScore(int points) {
    score += points;
    std::cout << "Bomber::AddScore(" << points << ") called. New score: " << score << std::endl;
}

void Bomber::ResetScore() {
    score = 0;
}

// Метод для установки жизней
void Bomber::SetLives(int newLives) {
    if (newLives <= 0) {
        lives = 0;
        isAlive = false;
    } else {
        lives = newLives;
        isAlive = true;
    }
}

void Bomber::SetPosition(Vector2 newPos) {
    position = newPos;
}

void Bomber::SetBombLimit(std::size_t limit) {
    bombLimit = limit;
}

void Bomber::SetBombRange(int range) {
    bombRange = range;
}

void Bomber::SetSpeed(float newSpeed) {
    baseSpeed = newSpeed;
    // Пересчитываем с учетом текущих баффов скорости
    speed = baseSpeed * (1.0f + 0.2f * movementSpeedBuff);
}

bool Bomber::CheckCollision(const Rectangle& rect) const {
    return CheckCollisionRecs(GetRect(), rect);
}

void Bomber::Kill() {
    isAlive = false;
    lives = 0;
}

void Bomber::Respawn(float x, float y) {
    position = { x, y };
    isAlive = true;
    lives = 3;
    invincibilityTimer = 2.0f;
    // Сбрасываем баффы при респавне
    explosionRangeBuff = 0;
    movementSpeedBuff = 0;
    bombCountBuff = 0;
    speed = baseSpeed;
    // не сбрасываем очки при респавне
}

void Bomber::RemoveBomb(Bomb* bomb) {
    auto it = std::find(bombs.begin(), bombs.end(), bomb);
    if (it != bombs.end()) {
        bombs.erase(it);
    }
}

const std::vector<Bomb*>& Bomber::GetBombs() const {
    return bombs;
}