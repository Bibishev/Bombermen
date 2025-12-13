#include <iostream>
#include <cmath>
#include <algorithm>
#include <fstream>
#include "raylib.h"
#include "bomber.h"
#include "wiu/button.h"
#include "map/map.h"
#include "mob/mob.h"
#include "baf.h"
#include "bomb/bomb.h"
#include <queue>
#include <set>
#include <utility>
#include <random>

using namespace std;

// Текстуры для игры
Texture2D bomberTexture;
Texture2D mobTexture;
Texture2D bombTexture;
Texture2D boomTexture;
Texture2D bafExplosionTexture;
Texture2D bafSpeedTexture;
Texture2D bafBombTexture;
Texture2D wallTexture;
Texture2D destructibleWallTexture;


enum GameState {
    MENU,
    PLAYING,
    SETTINGS,
    RECORDS,
    GAME_OVER,
    VICTORY
};

// Глобальные переменные для счёта
int totalScore = 0;               // Общий счёт за всю игру
int currentLevelScore = 0;        // Счёт на текущем уровне
vector<int> highScores;           // Рекорды
const string HIGHSCORES_FILE = "highscores.txt"; // Файл для сохранения рекордов

// Функции для работы с рекордами
void LoadHighScores() {
    highScores.clear();
    ifstream file(HIGHSCORES_FILE);
    if (file.is_open()) {
        int score;
        while (file >> score) {
            highScores.push_back(score);
        }
        file.close();
        // Сортируем по убыванию
        sort(highScores.rbegin(), highScores.rend());
        // Ограничиваем до 10 лучших результатов
        if (highScores.size() > 10) {
            highScores.resize(10);
        }
    } else {
        // Если файла нет, создаём пустой список
        highScores = {5000, 4000, 3000, 2000, 1000};
    }
}

void SaveHighScores() {
    ofstream file(HIGHSCORES_FILE);
    if (file.is_open()) {
        // Добавляем текущий счёт в список рекордов
        highScores.push_back(totalScore);
        // Сортируем по убыванию
        sort(highScores.rbegin(), highScores.rend());
        // Ограничиваем до 10 лучших результатов
        if (highScores.size() > 10) {
            highScores.resize(10);
        }
        // Сохраняем в файл
        for (int score : highScores) {
            file << score << endl;
        }
        file.close();
    }
}

void AddHighScore(int score) {
    highScores.push_back(score);
    sort(highScores.rbegin(), highScores.rend());
    if (highScores.size() > 10) {
        highScores.resize(10);
    }
    SaveHighScores();
}

bool IsValidSpawnPosition(float x, float y, Map* map) {
    if (!map) return false;
    
    Vector2 gridPos = map->GetGridPosition(x, y);
    return map->IsWalkable(gridPos.x, gridPos.y);
}

bool HasMinimumPath(Map* map, float startX, float startY, int minPathLength) {
    if (!map) return false;
    
    Vector2 startGrid = map->GetGridPosition(startX, startY);
    int startXGrid = static_cast<int>(startGrid.x);
    int startYGrid = static_cast<int>(startGrid.y);
    
    if (!map->IsWalkable(startXGrid, startYGrid)) {
        return false;
    }
    
    std::queue<std::pair<int, int>> q;
    std::set<std::pair<int, int>> visited;
    
    q.push(std::make_pair(startXGrid, startYGrid));
    visited.insert(std::make_pair(startXGrid, startYGrid));
    
    int reachableCount = 0;
    std::vector<std::pair<int, int>> directions;
    directions.push_back(std::make_pair(0, -1));
    directions.push_back(std::make_pair(0, 1));
    directions.push_back(std::make_pair(-1, 0));
    directions.push_back(std::make_pair(1, 0));
    
    while (!q.empty() && reachableCount < minPathLength) {
        std::pair<int, int> current = q.front();
        q.pop();
        int x = current.first;
        int y = current.second;
        reachableCount++;
        
        for (size_t i = 0; i < directions.size(); i++) {
            int dx = directions[i].first;
            int dy = directions[i].second;
            int nx = x + dx;
            int ny = y + dy;
            
            std::pair<int, int> neighbor = std::make_pair(nx, ny);
            if (visited.find(neighbor) == visited.end() && 
                nx >= 0 && nx < map->GetWidth() && 
                ny >= 0 && ny < map->GetHeight() && 
                map->IsWalkable(nx, ny)) {
                
                q.push(neighbor);
                visited.insert(neighbor);
            }
        }
    }
    
    return reachableCount >= minPathLength;
}

Baf* CreateRandomBaf(float x, float y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 2);
    
    int type = dis(gen);
    return new Baf(x, y, static_cast<BafType>(type));
}

void CreateMobs(std::vector<Mob*>& mobs, Map* map, int currentLevel) {
    int normalCount = 0;
    int fastCount = 0;
    int superFastCount = 0;
    
    if (currentLevel == 1) {
        normalCount = 3;
    } else if (currentLevel == 2) {
        normalCount = 2;
        fastCount = 2;
    } else if (currentLevel == 3) {
        fastCount = 4;
        superFastCount = 1;
    }
    
    int totalMobs = normalCount + fastCount + superFastCount;
    int created = 0;
    int attempts = 0;
    const int maxAttempts = 500;
    
    const int minDistanceFromPlayer = 10 + (currentLevel - 1) * 2;
    Vector2 playerSpawnGrid = map->GetGridPosition(80.0f, 80.0f);
    
    std::vector<std::pair<int, int>> occupiedPositions;
    
    for (int i = 0; i < normalCount; i++) {
        while (created < totalMobs && attempts < maxAttempts) {
            attempts++;
            
            float x = GetRandomValue(3, map->GetWidth() - 4) * 64.0f + 5.0f;
            float y = GetRandomValue(3, map->GetHeight() - 4) * 64.0f + 5.0f;
            
            Vector2 mobGrid = map->GetGridPosition(x, y);
            int mobXGrid = static_cast<int>(mobGrid.x);
            int mobYGrid = static_cast<int>(mobGrid.y);
            
            float distanceFromPlayer = abs(mobGrid.x - playerSpawnGrid.x) + 
                                      abs(mobGrid.y - playerSpawnGrid.y);
            
            if (distanceFromPlayer < minDistanceFromPlayer) {
                continue;
            }
            
            bool positionOccupied = false;
            for (size_t j = 0; j < occupiedPositions.size(); j++) {
                int ox = occupiedPositions[j].first;
                int oy = occupiedPositions[j].second;
                if (mobXGrid == ox && mobYGrid == oy) {
                    positionOccupied = true;
                    break;
                }
            }
            
            if (positionOccupied) {
                continue;
            }
            
            if (HasMinimumPath(map, x, y, 4)) {
                mobs.push_back(new Mob(x, y, MOB_NORMAL));
                mobs.back()->SetMap(map);
                occupiedPositions.push_back(std::make_pair(mobXGrid, mobYGrid));
                created++;
                break;
            }
        }
    }
    
    for (int i = 0; i < fastCount; i++) {
        while (created < totalMobs && attempts < maxAttempts) {
            attempts++;
            
            float x = GetRandomValue(3, map->GetWidth() - 4) * 64.0f + 5.0f;
            float y = GetRandomValue(3, map->GetHeight() - 4) * 64.0f + 5.0f;
            
            Vector2 mobGrid = map->GetGridPosition(x, y);
            int mobXGrid = static_cast<int>(mobGrid.x);
            int mobYGrid = static_cast<int>(mobGrid.y);
            
            float distanceFromPlayer = abs(mobGrid.x - playerSpawnGrid.x) + 
                                      abs(mobGrid.y - playerSpawnGrid.y);
            
            if (distanceFromPlayer < minDistanceFromPlayer) {
                continue;
            }
            
            bool positionOccupied = false;
            for (size_t j = 0; j < occupiedPositions.size(); j++) {
                int ox = occupiedPositions[j].first;
                int oy = occupiedPositions[j].second;
                if (mobXGrid == ox && mobYGrid == oy) {
                    positionOccupied = true;
                    break;
                }
            }
            
            if (positionOccupied) {
                continue;
            }
            
            if (HasMinimumPath(map, x, y, 4)) {
                mobs.push_back(new Mob(x, y, MOB_FAST));
                mobs.back()->SetMap(map);
                occupiedPositions.push_back(std::make_pair(mobXGrid, mobYGrid));
                created++;
                break;
            }
        }
    }
    
    for (int i = 0; i < superFastCount; i++) {
        while (created < totalMobs && attempts < maxAttempts) {
            attempts++;
            
            float x = GetRandomValue(3, map->GetWidth() - 4) * 64.0f + 5.0f;
            float y = GetRandomValue(3, map->GetHeight() - 4) * 64.0f + 5.0f;
            
            Vector2 mobGrid = map->GetGridPosition(x, y);
            int mobXGrid = static_cast<int>(mobGrid.x);
            int mobYGrid = static_cast<int>(mobGrid.y);
            
            float distanceFromPlayer = abs(mobGrid.x - playerSpawnGrid.x) + 
                                      abs(mobGrid.y - playerSpawnGrid.y);
            
            if (distanceFromPlayer < minDistanceFromPlayer) {
                continue;
            }
            
            bool positionOccupied = false;
            for (size_t j = 0; j < occupiedPositions.size(); j++) {
                int ox = occupiedPositions[j].first;
                int oy = occupiedPositions[j].second;
                if (mobXGrid == ox && mobYGrid == oy) {
                    positionOccupied = true;
                    break;
                }
            }
            
            if (positionOccupied) {
                continue;
            }
            
            if (HasMinimumPath(map, x, y, 4)) {
                mobs.push_back(new Mob(x, y, MOB_SUPER_FAST));
                mobs.back()->SetMap(map);
                occupiedPositions.push_back(std::make_pair(mobXGrid, mobYGrid));
                created++;
                break;
            }
        }
    }
    
    if (created < totalMobs) {
        std::cout << "Warning: Only created " << created << " out of " << totalMobs << " mobs" << std::endl;
    }
    
    std::cout << "Created " << normalCount << " normal, " 
              << fastCount << " fast (2x), " 
              << superFastCount << " super-fast (3x) mobs" << std::endl;
}

// Функция для подсчёта бонусных очков за сохранённые жизни
int CalculateLifeBonus(int startLives, int endLives) {
    if (endLives <= 0) return 0;
    return (endLives * 300);
}

// Передаём totalScore по ссылке
void GoToNextLevel(Bomber*& player, Map*& gameMap, std::vector<Mob*>& mobs, 
                   std::vector<Baf*>& bafs, int& currentLevel, bool& allMobsKilled,
                   int& totalScore, int& currentLevelScore, int& levelStartLives) {
    
    std::cout << "Moving to level " << (currentLevel + 1) << "..." << std::endl;
    
    // Начисляем очки за сохранённые жизни
    int currentLives = player->GetLives();
    int lifeBonus = CalculateLifeBonus(levelStartLives, currentLives);
    if (lifeBonus > 0) {
        totalScore += lifeBonus;
        currentLevelScore += lifeBonus;
        std::cout << "Life bonus: +" << lifeBonus << " points for " << currentLives << " lives saved" << std::endl;
        std::cout << "Total score after level " << currentLevel << ": " << totalScore << std::endl;
    }
    
    // Сохраняем баффы и очки перед удалением игрока
    int savedLives = currentLives;
    int explosionBuff = player->GetExplosionRangeBuff();
    int speedBuff = player->GetMovementSpeedBuff();
    int bombBuff = player->GetBombCountBuff();
    
    // Сохраняем ОБЩИЙ счёт перед удалением
    int savedTotalScore = totalScore; // Это уже включает бонус за жизни
    
    if (gameMap != nullptr) {
        delete gameMap;
        gameMap = nullptr;
    }
    
    for (auto mob : mobs) {
        delete mob;
    }
    mobs.clear();
    
    for (auto baf : bafs) {
        delete baf;
    }
    bafs.clear();
    
    SetGlobalBafs(nullptr);
    
    currentLevel++;
    
    const int tileSize = 64;
    const int mapWidth = 30;
    const int mapHeight = 22;
    gameMap = new Map(mapWidth, mapHeight, tileSize);
    
    delete player;
    
    // Создаём нового игрока с сохранением общего счёта
    player = new Bomber(80.0f, 80.0f);
    player->SetMap(gameMap);
    player->SetLives(savedLives);
    
    // Восстанавливаем общий счёт через AddScore
    player->AddScore(savedTotalScore); // Передаём ВЕСЬ накопленный счёт
    
    // Обновляем глобальные переменные
    totalScore = savedTotalScore;
    
    // Сбрасываем счёт текущего уровня
    currentLevelScore = 0;
    
    // Сохраняем информацию для подсчёта бонусов
    levelStartLives = savedLives;
    
    // Восстанавливаем баффы
    for (int i = 0; i < explosionBuff; i++) {
        player->ApplyBaf(0);
    }
    for (int i = 0; i < speedBuff; i++) {
        player->ApplyBaf(1);
    }
    for (int i = 0; i < bombBuff; i++) {
        player->ApplyBaf(2);
    }
    
    CreateMobs(mobs, gameMap, currentLevel);
    
    SetGlobalBafs(&bafs);
    
    allMobsKilled = false;
    
    std::cout << "Level " << currentLevel << " started" << std::endl;
    std::cout << "Carrying over total score: " << totalScore << std::endl;
    std::cout << "Player score after creation: " << player->GetScore() << std::endl;
}

// Функция загрузки текстур
void LoadGameTextures() {
    // Загружаем текстуры из текущей директории или папки assets
    bomberTexture = LoadTexture("bomber.png");
    mobTexture = LoadTexture("mob.png");
    bombTexture = LoadTexture("bomb.png");
    boomTexture = LoadTexture("boom.png");
    bafExplosionTexture = LoadTexture("rad_b.png");      // Радиус взрыва
    bafSpeedTexture = LoadTexture("speed_b.png");        // Скорость
    bafBombTexture = LoadTexture("caint_b.png");         // Количество бомб
    wallTexture = LoadTexture("vol_n.png");              // Неразрушаемые стены
    destructibleWallTexture = LoadTexture("vol_r.png");  // Разрушаемые стены
    
    // Проверяем загрузку текстур
    if (bomberTexture.id == 0) std::cout << "Failed to load bomber.png" << std::endl;
    if (mobTexture.id == 0) std::cout << "Failed to load mob.png" << std::endl;
    if (bombTexture.id == 0) std::cout << "Failed to load bomb.png" << std::endl;
    if (boomTexture.id == 0) std::cout << "Failed to load boom.png" << std::endl;
    if (bafExplosionTexture.id == 0) std::cout << "Failed to load rad_b.png" << std::endl;
    if (bafSpeedTexture.id == 0) std::cout << "Failed to load speed_b.png" << std::endl;
    if (bafBombTexture.id == 0) std::cout << "Failed to load caint_b.png" << std::endl;
    if (wallTexture.id == 0) std::cout << "Failed to load vol_n.png" << std::endl;
    if (destructibleWallTexture.id == 0) std::cout << "Failed to load vol_r.png" << std::endl;
}

// Функция выгрузки текстур
void UnloadGameTextures() {
    UnloadTexture(bomberTexture);
    UnloadTexture(mobTexture);
    UnloadTexture(bombTexture);
    UnloadTexture(boomTexture);
    UnloadTexture(bafExplosionTexture);
    UnloadTexture(bafSpeedTexture);
    UnloadTexture(bafBombTexture);
    UnloadTexture(wallTexture);
    UnloadTexture(destructibleWallTexture);
}

int main()
{
    const int screenWidth = 1920;
    const int screenHeight = 1440;
    const int tileSize = 64;
    
    const int mapWidth = 30;
    const int mapHeight = 22;
    
    InitWindow(screenWidth, screenHeight, "Bombermen");
    SetTargetFPS(60);
    
    // Загружаем текстуры
    LoadGameTextures();

    // Загружаем рекорды при старте
    LoadHighScores();

    GameState currentState = MENU;

    Button playButton(710, 500, 500, 120, "PLAY");
    Button settingsButton(710, 700, 500, 120, "SETTINGS");
    Button recordsButton(710, 900, 500, 120, "RECORDS");
    Button menuButton(710, 700, 500, 120, "BACK TO MENU");

    Bomber* player = nullptr;
    Map* gameMap = nullptr;
    std::vector<Mob*> mobs;
    std::vector<Baf*> bafs;
    
    bool allMobsKilled = false;
    int currentLevel = 1;
    bool levelTransition = false;
    float levelTransitionTimer = 0.0f;
    
    // Переменные для отслеживания счёта
    int levelStartLives = 3;
    currentLevelScore = 0;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_ENTER)) {
            if (currentState == MENU) {
                break;
            } else if (currentState == VICTORY || currentState == GAME_OVER) {
                currentState = MENU;
            } else {
                currentState = MENU;
                if (player != nullptr) {
                    delete player;
                    player = nullptr;
                }
                if (gameMap != nullptr) {
                    delete gameMap;
                    gameMap = nullptr;
                }
                for (auto mob : mobs) {
                    delete mob;
                }
                mobs.clear();
                for (auto baf : bafs) {
                    delete baf;
                }
                bafs.clear();
                SetGlobalBafs(nullptr);
                allMobsKilled = false;
                currentLevel = 1;
                levelTransition = false;
                totalScore = 0;
                currentLevelScore = 0;
                levelStartLives = 3;
            }
        }

        switch (currentState) {
            case MENU:
                playButton.Update();
                settingsButton.Update();
                recordsButton.Update();
                
                if (playButton.IsClicked()) {
                    currentState = PLAYING;
                    
                    if (player != nullptr) {
                        delete player;
                        player = nullptr;
                    }
                    if (gameMap != nullptr) {
                        delete gameMap;
                        gameMap = nullptr;
                    }
                    for (auto mob : mobs) {
                        delete mob;
                    }
                    mobs.clear();
                    for (auto baf : bafs) {
                        delete baf;
                    }
                    bafs.clear();
                    
                    currentLevel = 1;
                    totalScore = 0;
                    currentLevelScore = 0;
                    gameMap = new Map(mapWidth, mapHeight, tileSize);
                    player = new Bomber(80.0f, 80.0f);
                    player->SetMap(gameMap);
                    
                    CreateMobs(mobs, gameMap, currentLevel);
                    
                    SetGlobalBafs(&bafs);
                    allMobsKilled = false;
                    levelTransition = false;
                    
                    levelStartLives = 3;
                    
                    std::cout << "New game started. Level " << currentLevel << std::endl;
                    std::cout << "Player spawned at (80, 80)" << std::endl;
                }
                if (settingsButton.IsClicked()) {
                    currentState = SETTINGS;
                }
                if (recordsButton.IsClicked()) {
                    currentState = RECORDS;
                }
                break;
                
            case PLAYING:
                if (player != nullptr && gameMap != nullptr) {
                    if (levelTransition) {
                        levelTransitionTimer += GetFrameTime();
                        if (levelTransitionTimer >= 2.0f) {
                            levelTransition = false;
                            levelTransitionTimer = 0.0f;
                            
                            if (currentLevel > 3) {
                                // Начисляем финальные бонусы за жизни
                                int finalLifeBonus = CalculateLifeBonus(levelStartLives, player->GetLives());
                                if (finalLifeBonus > 0) {
                                    player->AddScore(finalLifeBonus);
                                    totalScore += finalLifeBonus;
                                    currentLevelScore += finalLifeBonus;
                                    std::cout << "Final life bonus: +" << finalLifeBonus << " points" << std::endl;
                                }
                                
                                // Получаем финальный счёт от игрока
                                totalScore = player->GetScore();
                                
                                // Добавляем результат в таблицу рекордов
                                AddHighScore(totalScore);
                                
                                currentState = VICTORY;
                                std::cout << "Game completed! All 3 levels finished!" << std::endl;
                                std::cout << "Final score: " << totalScore << std::endl;
                            }
                        }
                    } else {
                        player->Update();
                        
                        player->CheckBafCollisions(bafs);
                        
                        player->CheckMobCollisions(mobs);
                        
                        for (auto baf : bafs) {
                            baf->Update();
                        }
                        
                        if (!allMobsKilled && mobs.empty()) {
                            allMobsKilled = true;
                            gameMap->ActivateBlinkingWalls();
                            std::cout << "All mobs killed! Blinking walls activated. Find the portal!" << std::endl;
                        }
                        
                        if (allMobsKilled) {
                            gameMap->UpdateBlinkingWalls();
                        }
                        
                        if (allMobsKilled && gameMap->HasPortal()) {
                            auto portalPos = gameMap->GetPortalPosition();
                            Vector2 portalWorldPos = gameMap->GetTilePosition(portalPos.first, portalPos.second);
                            Rectangle portalRect = {portalWorldPos.x, portalWorldPos.y, tileSize, tileSize};
                            
                            if (player->CheckCollision(portalRect)) {
                                std::cout << "Portal reached! Transitioning to next level..." << std::endl;
                                levelTransition = true;
                                levelTransitionTimer = 0.0f;
                                
                                GoToNextLevel(player, gameMap, mobs, bafs, currentLevel, 
                                            allMobsKilled, totalScore, currentLevelScore, levelStartLives);
                            }
                        }
                        
                        for (auto it = mobs.begin(); it != mobs.end();) {
                            Mob* mob = *it;
                            if (mob->IsAlive()) {
                                mob->Update();
                                mob->CheckBombDamage(player->GetBombs());
                                
                                if (mob->IsAlive()) {
                                    ++it;
                                } else {
                                    // Начисляем очки за убийство моба
                                    MobType type = mob->GetType();
                                    int mobPoints = 0;
                                    
                                    switch (type) {
                                        case MOB_NORMAL:
                                            mobPoints = 100;
                                            break;
                                        case MOB_FAST:
                                            mobPoints = 300;
                                            break;
                                        case MOB_SUPER_FAST:
                                            mobPoints = 500;
                                            break;
                                    }
                                    
                                    if (mobPoints > 0) {
                                        player->AddScore(mobPoints);
                                        totalScore += mobPoints;
                                        currentLevelScore += mobPoints;
                                        std::cout << "Mob killed! +" << mobPoints << " points" << std::endl;
                                        std::cout << "Player score: " << player->GetScore() << std::endl;
                                        std::cout << "Total score: " << totalScore << std::endl;
                                    }
                                    
                                    std::random_device rd;
                                    std::mt19937 gen(rd());
                                    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
                                    
                                    if (dis(gen) < 0.35f) {
                                        Baf* newBaf = CreateRandomBaf(mob->GetPosition().x, mob->GetPosition().y);
                                        bafs.push_back(newBaf);
                                        std::cout << "Baf dropped from mob!" << std::endl;
                                    }
                                    
                                    delete mob;
                                    it = mobs.erase(it);
                                }
                            } else {
                                delete mob;
                                it = mobs.erase(it);
                            }
                        }
                        
                        for (auto it = bafs.begin(); it != bafs.end();) {
                            Baf* baf = *it;
                            if (!baf->IsActive()) {
                                delete baf;
                                it = bafs.erase(it);
                            } else {
                                ++it;
                            }
                        }
                        
                        if (!player->IsAlive()) {
                            // Получаем счёт от игрока (должен содержать счёт ВСЕХ уровней)
                            totalScore = player->GetScore();
                            
                            // Проверяем, что счёт действительно общий
                            std::cout << "Player died at level " << currentLevel << std::endl;
                            std::cout << "Player final score: " << player->GetScore() << std::endl;
                            std::cout << "Total score variable: " << totalScore << std::endl;
                            
                            // Добавляем результат в таблицу рекордов
                            if (totalScore > 0) {
                                AddHighScore(totalScore);
                            }
                            currentState = GAME_OVER;
                            std::cout << "Game over! Final score: " << totalScore << std::endl;
                        }
                    }
                }
                break;
                
            case SETTINGS:
                break;
                
            case RECORDS:
                // Отображаем таблицу рекордов
                if (IsKeyPressed(KEY_ENTER)) {
                    currentState = MENU;
                }
                break;
                
            case VICTORY:
                menuButton.Update();
                if (menuButton.IsClicked() || IsKeyPressed(KEY_ENTER)) {
                    currentState = MENU;
                    if (player != nullptr) {
                        delete player;
                        player = nullptr;
                    }
                    if (gameMap != nullptr) {
                        delete gameMap;
                        gameMap = nullptr;
                    }
                    for (auto mob : mobs) {
                        delete mob;
                    }
                    mobs.clear();
                    for (auto baf : bafs) {
                        delete baf;
                    }
                    bafs.clear();
                    SetGlobalBafs(nullptr);
                    currentLevel = 1;
                    totalScore = 0;
                    currentLevelScore = 0;
                    levelStartLives = 3;
                }
                break;
                
            case GAME_OVER:
                menuButton.Update();
                if (menuButton.IsClicked() || IsKeyPressed(KEY_ENTER)) {
                    currentState = MENU;
                    if (player != nullptr) {
                        delete player;
                        player = nullptr;
                    }
                    if (gameMap != nullptr) {
                        delete gameMap;
                        gameMap = nullptr;
                    }
                    for (auto mob : mobs) {
                        delete mob;
                    }
                    mobs.clear();
                    for (auto baf : bafs) {
                        delete baf;
                    }
                    bafs.clear();
                    SetGlobalBafs(nullptr);
                    currentLevel = 1;
                    totalScore = 0;
                    currentLevelScore = 0;
                    levelStartLives = 3;
                }
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        
        switch (currentState) {
            case MENU:
            {
                int screenCenterX = screenWidth / 2;
                
                // Центрируем заголовок
                const char* titleText = "BOMBERMEN";
                int titleWidth = MeasureText(titleText, 120);
                DrawText(titleText, screenCenterX - titleWidth/2, 200, 120, GREEN);
                
                // Центрируем кнопки (они уже центрированы по кнопкам, но можно выровнять текст)
                playButton.Draw();
                settingsButton.Draw();
                recordsButton.Draw();
                
                // Центрируем нижние подсказки
                const char* mouseText = "Use mouse to navigate";
                int mouseTextWidth = MeasureText(mouseText, 40);
                DrawText(mouseText, screenCenterX - mouseTextWidth/2, 1100, 40, GRAY);
                
                const char* exitText = "ENTER - Exit Game";
                int exitTextWidth = MeasureText(exitText, 40);
                DrawText(exitText, screenCenterX - exitTextWidth/2, 1200, 40, GRAY);
                
                break;
            }
                
            case PLAYING:
                if (levelTransition) {
                    DrawRectangle(0, 0, screenWidth, screenHeight, ColorAlpha(BLACK, levelTransitionTimer / 2.0f));
                    DrawText("LEVEL COMPLETE!", screenWidth/2 - 200, screenHeight/2 - 50, 60, ORANGE);
                    DrawText(TextFormat("Moving to Level %d...", currentLevel), screenWidth/2 - 150, screenHeight/2 + 50, 40, GREEN);
        
                    if (currentLevel > 3) {
                        DrawText("FINAL LEVEL COMPLETED!", screenWidth/2 - 250, screenHeight/2 + 120, 50, YELLOW);
                    }
                } else {
                    if (gameMap != nullptr) {
                        gameMap->Draw();
                    }
        
                    for (auto mob : mobs) {
                        mob->Draw();
                    }
        
                    for (auto baf : bafs) {
                        baf->Draw();
                    }
        
                    if (player != nullptr) {
                        player->Draw();
                    }
        
                    DrawText(TextFormat("Game Level %d/3", currentLevel), 40, 40, 60, WHITE);
                    DrawText("ENTER - Back to Menu", 40, 120, 40, GRAY);
        
                    if (player != nullptr) {
                        DrawText(TextFormat("Lives: %d", player->GetLives()), 40, 180, 40, RED);
                        DrawText(TextFormat("Bombs: %zu/%zu", player->GetBombs().size(), player->GetBombLimit()), 
                                40, 230, 40, ORANGE);
            
                        // Отображаем общий счёт и счёт уровня
                        DrawText(TextFormat("Total Score: %d", totalScore), 40, 280, 40, YELLOW);
                        DrawText(TextFormat("Level Score: %d", currentLevelScore), 40, 330, 30, YELLOW);
            
                        // Отображаем количество подобраных баффов
                        int yOffset = 400;
            
                        int totalBafs = player->GetExplosionRangeBuff() + 
                                    player->GetMovementSpeedBuff() + 
                                    player->GetBombCountBuff();
            
                        DrawText(TextFormat("Bafs Collected: %d/9", totalBafs), 40, yOffset, 40, GREEN);
                        yOffset += 50;
            
                        DrawText(TextFormat("Explosion Range: +%d", player->GetExplosionRangeBuff()), 
                                60, yOffset, 25, GREEN);
                        yOffset += 30;
            
                        DrawText(TextFormat("Movement Speed: +%d", player->GetMovementSpeedBuff()), 
                                60, yOffset, 25, GREEN);
                        yOffset += 30;
            
                        DrawText(TextFormat("Bomb Count: +%d", player->GetBombCountBuff()), 
                                60, yOffset, 25, GREEN);
                        yOffset += 50;
            
                        if (allMobsKilled) {
                            DrawText("FIND THE PORTAL!", 40, yOffset, 40, ORANGE);
                            yOffset += 50;
                            DrawText("Destroy PINK blinking walls to find it", 40, yOffset, 25, PINK);
                            yOffset += 30;
                            DrawText("Blinking walls: 5% baf chance", 40, yOffset, 25, GREEN);
                            yOffset += 30;
                            DrawText("Portal is under one of the pink walls", 40, yOffset, 25, ORANGE);
                        }
            
                        // Правая панель - только управление
                        DrawText("Controls:", screenWidth - 300, 40, 40, WHITE);
                        DrawText("WASD - Move", screenWidth - 300, 90, 30, GRAY);
                        DrawText("SPACE - Place Bomb", screenWidth - 300, 130, 30, GRAY);
                        DrawText("ENTER - Back to Menu", screenWidth - 300, 170, 30, GRAY);
                    }
                }
                break;
                
            case SETTINGS:
            {
                int screenCenterX = screenWidth / 2;
                
                // Центрируем заголовок
                const char* titleText = "SETTINGS";
                int titleWidth = MeasureText(titleText, 120);
                DrawText(titleText, screenCenterX - titleWidth/2, 200, 120, YELLOW);
                
                // Центрируем сообщение
                const char* devText = "Settings screen is under development";
                int devTextWidth = MeasureText(devText, 60);
                DrawText(devText, screenCenterX - devTextWidth/2, 600, 60, WHITE);
                
                // Центрируем подсказку
                const char* backText = "ENTER - Back to Menu";
                int backTextWidth = MeasureText(backText, 40);
                DrawText(backText, screenCenterX - backTextWidth/2, 1000, 40, GRAY);
                
                break;
            }
                
            case RECORDS:
{
                int screenCenterX = screenWidth / 2;
    
                // Центрируем заголовок
                const char* titleText = "HIGH SCORE";
                int titleWidth = MeasureText(titleText, 80);
                DrawText(titleText, screenCenterX - titleWidth/2, 200, 80, YELLOW);
    
                // Центрируем подзаголовок
                const char* subtitleText = "Best Result";
                int subtitleWidth = MeasureText(subtitleText, 50);
                DrawText(subtitleText, screenCenterX - subtitleWidth/2, 300, 50, ORANGE);
    
                if (!highScores.empty()) {
                    // Отображаем лучший результат
                    int yPos = 450;
        
                    // Золотая рамка для лучшего результата
                    int boxWidth = 400;
                    int boxHeight = 100;
                    DrawRectangle(screenCenterX - boxWidth/2 - 10, yPos - 10, 
                                boxWidth + 20, boxHeight + 20, Color{139, 69, 19, 255}); // Коричневый
                    DrawRectangle(screenCenterX - boxWidth/2, yPos, 
                                boxWidth, boxHeight, Color{255, 215, 0, 200}); // Золотой
        
                    // Лучший результат
                    char scoreText[50];
                    sprintf(scoreText, "TOP SCORE: %d", highScores[0]);
                    int textWidth = MeasureText(scoreText, 50);
                    DrawText(scoreText, screenCenterX - textWidth/2, yPos + 25, 50, BLACK);
        
                    // Если есть текущий счёт, показываем его для сравнения
                    if (totalScore > 0) {
                        yPos += 150;
            
                        char currentText[50];
                        sprintf(currentText, "Your Score: %d", totalScore);
                        int currentWidth = MeasureText(currentText, 40);
                        DrawText(currentText, screenCenterX - currentWidth/2, yPos, 40, WHITE);
            
                        // Показываем разницу с лучшим результатом
                        if (totalScore < highScores[0]) {
                            int difference = highScores[0] - totalScore;
                            char diffText[50];
                            sprintf(diffText, "Need %d more points", difference);
                            int diffWidth = MeasureText(diffText, 30);
                            DrawText(diffText, screenCenterX - diffWidth/2, yPos + 50, 30, YELLOW);
                        } else if (totalScore == highScores[0]) {
                            const char* tieText = "Tied with best!";
                            int tieWidth = MeasureText(tieText, 30);
                            DrawText(tieText, screenCenterX - tieWidth/2, yPos + 50, 30, GREEN);
                        } else {
                            const char* newBestText = "NEW BEST!";
                            int newBestWidth = MeasureText(newBestText, 30);
                            DrawText(newBestText, screenCenterX - newBestWidth/2, yPos + 50, 30, GOLD);
                        }
                    }
                } else {
                    // Если нет рекордов
                    const char* noScoresText = "No scores yet!";
                    int noScoresWidth = MeasureText(noScoresText, 50);
                    DrawText(noScoresText, screenCenterX - noScoresWidth/2, 450, 50, WHITE);
                }
    
                // Центрируем подсказку
                const char* backText = "Press ENTER to return to menu";
                int backTextWidth = MeasureText(backText, 30);
                DrawText(backText, screenCenterX - backTextWidth/2, 1000, 30, GRAY);
    
                break;
            }
                
            case VICTORY:
                DrawText("VICTORY!", 760, 200, 120, GREEN);
                DrawText("You have completed all 3 levels!", 620, 400, 60, WHITE);
                DrawText("Congratulations!", 760, 500, 80, YELLOW);
                
                DrawText(TextFormat("Final Score: %d", totalScore), 760, 600, 60, GOLD);
                
                // Проверяем, побил ли игрок рекорд
                if (!highScores.empty() && totalScore >= highScores[0]) {
                    DrawText("NEW HIGH SCORE!", 660, 670, 50, Color{255, 215, 0, 255}); // Золотой цвет
                }
                
                if (player != nullptr) {
                    DrawText(TextFormat("Final Lives: %d", player->GetLives()), 760, 740, 50, RED);
                    DrawText(TextFormat("Final Buffs:"), 760, 810, 40, GREEN);
                    DrawText(TextFormat("Explosion Range: +%d", player->GetExplosionRangeBuff()), 
                            760, 860, 30, GREEN);
                    DrawText(TextFormat("Movement Speed: +%d", player->GetMovementSpeedBuff()), 
                            760, 900, 30, GREEN);
                    DrawText(TextFormat("Bomb Count: +%d", player->GetBombCountBuff()), 
                            760, 940, 30, GREEN);
                }
                
                menuButton.Draw();
                DrawText("Press ENTER or click button to return to menu", 660, 1050, 30, GRAY);
                break;
                
            case GAME_OVER:
                DrawText("GAME OVER", 760, 200, 120, RED);
                DrawText("You died!", 860, 400, 60, WHITE);
                DrawText(TextFormat("Reached Level: %d", currentLevel), 760, 500, 50, YELLOW);
                DrawText(TextFormat("Final Score: %d", totalScore), 760, 570, 60, YELLOW);
                
                menuButton.Draw();
                DrawText("Press ENTER or click button to return to menu", 660, 1000, 30, GRAY);
                break;
        }
        
        EndDrawing();
    }

    UnloadGameTextures();
    
    if (player != nullptr) {
        delete player;
    }
    if (gameMap != nullptr) {
        delete gameMap;
    }
    for (auto mob : mobs) {
        delete mob;
    }
    for (auto baf : bafs) {
        delete baf;
    }

    CloseWindow();
    return 0;
}