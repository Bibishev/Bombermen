#include "map.h"
#include <iostream>
#include <random>
#include <algorithm>

// Внешние текстуры (объявлены в main.cpp)
extern Texture2D wallTexture;
extern Texture2D destructibleWallTexture;

Map::Map(int w, int h, int tileSize) : width(w), height(h), tileSize(tileSize) {
    tiles.resize(height, std::vector<int>(width, 0));
    hasPortal = false;
    blinkTimer = 0.0f;
    showBlinkingWalls = false;
    GenerateLevel();
}

void Map::GenerateLevel() {
    // Инициализируем все клетки как проходы
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            tiles[y][x] = 1; // Все клетки - проходы по умолчанию
        }
    }
    
    // НЕРАЗРУШАЕМЫЕ СТЕНЫ:
    
    // 1. Верхний ряд (y = 0) - ВСЯ ширина
    for (int x = 0; x < width; x++) {
        tiles[0][x] = 0;
    }
    
    // 2. Левый столбец (x = 0) - ВСЯ высота
    for (int y = 0; y < height; y++) {
        tiles[y][0] = 0;
    }
    
    // 3. ПРАВАЯ ГРАНИЦА: два самых правых ряда (x = width-1 и x = width-2) - ВСЯ высота
    for (int y = 0; y < height; y++) {
        tiles[y][width - 1] = 0;    // Самый правый ряд
        tiles[y][width - 2] = 0;    // Предпоследний правый ряд
    }
    
    // 4. НИЖНЯЯ ГРАНИЦА: два самых нижних ряда (y = height-1 и y = height-2) - ВСЯ ширина
    for (int x = 0; x < width; x++) {
        tiles[height - 1][x] = 0;   // Самый нижний ряд
        tiles[height - 2][x] = 0;   // Предпоследний нижний ряд
    }
    
    // 5. Шахматный порядок неразрушаемых стен внутри (игровая зона)
    for (int y = 1; y < height - 2; y++) {    // -2 чтобы не задеть нижнюю границу
        for (int x = 1; x < width - 2; x++) { // -2 чтобы не задеть правую границу
            // Проверяем что это не граничная клетка
            if (y < height - 2 && x < width - 2) {
                if (x % 2 == 0 && y % 2 == 0) {
                    tiles[y][x] = 0; // Неразрушаемая стена
                }
            }
        }
    }
    
    // 6. Стена на 2x2 (внутри игровой зоны)
    tiles[2][2] = 0;
    
    // 7. Спавн игрока на 1x1 и вокруг (оставляем проходы)
    tiles[1][1] = 1;
    tiles[1][2] = 1;
    tiles[2][1] = 1;
    
    std::cout << "Map generated: " << width << "x" << height << std::endl;
    std::cout << "Borders: top(1), left(1), right(2), bottom(2) rows" << std::endl;
    
    // Генерация разрушаемых стен с плотностью 30%
    GenerateDestructibleWalls(0.30f);
    
    std::cout << "Destructible walls: " << CountDestructibleWalls() << " (30% density)" << std::endl;
}

void Map::GenerateDestructibleWalls(float density) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    int destructibleCount = 0;
    
    // Генерируем разрушаемые стены только внутри игровой зоны
    // Игровая зона: от (1,1) до (width-3, height-3)
    for (int y = 1; y < height - 2; y++) {    // -2 чтобы не задеть нижнюю границу
        for (int x = 1; x < width - 2; x++) { // -2 чтобы не задеть правую границу
            // Пропускаем уже занятые клетки
            if (tiles[y][x] != 1) continue;
            
            // Запрещаем разрушаемые стены только в клетках спавна игрока
            bool isSpawnArea = 
                (x == 1 && y == 1) ||  // клетка 1,1
                (x == 1 && y == 2) ||  // клетка 1,2  
                (x == 2 && y == 1);    // клетка 2,1
            
            if (isSpawnArea) {
                continue; // Пропускаем только клетки спавна
            }
            
            // Генерация разрушаемой стены с плотностью 30%
            if (dis(gen) < density) {
                tiles[y][x] = 2; // Разрушаемая стена
                destructibleCount++;
            }
        }
    }
    
    std::cout << "Generated " << destructibleCount << " destructible walls in play area" << std::endl;
}

void Map::ActivateBlinkingWalls() {
    if (hasPortal) return; // Уже активировано
    
    std::cout << "Activating blinking walls and portal..." << std::endl;
    
    // Собираем все разрушаемые стены
    std::vector<std::pair<int, int>> destructibleWalls;
    for (int y = 1; y < height - 2; y++) {
        for (int x = 1; x < width - 2; x++) {
            if (tiles[y][x] == 2) {
                destructibleWalls.push_back(std::make_pair(x, y));
            }
        }
    }
    
    // Перемешиваем случайным образом
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(destructibleWalls.begin(), destructibleWalls.end(), gen);
    
    // Выбираем 5 случайных стен для моргания
    int wallsToSelect = std::min(5, static_cast<int>(destructibleWalls.size()));
    for (int i = 0; i < wallsToSelect; i++) {
        blinkingWalls.push_back(destructibleWalls[i]);
        std::cout << "Blinking wall added at random position" << std::endl;
        // ИЗМЕНЕНО: не выводим конкретные координаты портала
    }
    
    // Выбираем случайную стену для портала (из моргающих)
    if (!blinkingWalls.empty()) {
        std::uniform_int_distribution<> dis(0, blinkingWalls.size() - 1);
        int portalIndex = dis(gen);
        portalPosition = blinkingWalls[portalIndex];
        hasPortal = true;
        
        std::cout << "Portal hidden in one of the blinking walls" << std::endl;
        // ИЗМЕНЕНО: не выводим конкретные координаты портала
    }
    
    showBlinkingWalls = true;
    blinkTimer = 0.0f;
}

void Map::UpdateBlinkingWalls() {
    if (!showBlinkingWalls) return;
    
    blinkTimer += GetFrameTime();
    if (blinkTimer >= 0.5f) { // Моргаем каждые 0.5 секунды
        showBlinkingWalls = !showBlinkingWalls;
        blinkTimer = 0.0f;
    }
}

void Map::DrawBlinkingWallsAndPortal() {
    if (!showBlinkingWalls && !hasPortal) return;
    
    // Рисуем моргающие стены - розовые с черными точками
    if (showBlinkingWalls) {
        for (size_t i = 0; i < blinkingWalls.size(); i++) {
            int x = blinkingWalls[i].first;
            int y = blinkingWalls[i].second;
            Vector2 position = GetTilePosition(x, y);
            
            // Розовый цвет для моргающих стен
            Color wallColor = PINK;
            if (static_cast<int>(GetTime() * 10) % 2 == 0) {
                wallColor = ColorAlpha(PINK, 0.7f); // Полупрозрачный при моргании
            }
            
            // Рисуем стену
            DrawRectangle(position.x, position.y, tileSize, tileSize, wallColor);
            
            // Черные точки для текстуры
            DrawCircle(position.x + tileSize/4, position.y + tileSize/4, 3, BLACK);
            DrawCircle(position.x + 3*tileSize/4, position.y + tileSize/4, 3, BLACK);
            DrawCircle(position.x + tileSize/4, position.y + 3*tileSize/4, 3, BLACK);
            DrawCircle(position.x + 3*tileSize/4, position.y + 3*tileSize/4, 3, BLACK);
            
            // Черная рамка
            DrawRectangleLines(position.x, position.y, tileSize, tileSize, BLACK);
        }
    }
    
    // Рисуем портал (оранжевый квадрат)
    if (hasPortal) {
        Vector2 portalPos = GetTilePosition(portalPosition.first, portalPosition.second);
        
        // Оранжевый квадрат с анимацией
        Color portalColor = ORANGE;
        float pulse = sin(GetTime() * 3.0f) * 0.3f + 0.7f;
        DrawRectangle(portalPos.x, portalPos.y, tileSize, tileSize, 
                     ColorAlpha(portalColor, pulse));
        
        // Яркая золотая рамка
        DrawRectangleLinesEx(
            {portalPos.x - 2.0f, portalPos.y - 2.0f, 
             static_cast<float>(tileSize) + 4.0f, 
             static_cast<float>(tileSize) + 4.0f},
            3,
            GOLD
        );
        
        // Текст "PORTAL" в центре
        const char* portalText = "PORTAL";
        int fontSize = 20;
        int textWidth = MeasureText(portalText, fontSize);
        DrawText(portalText,
                 portalPos.x + (tileSize - textWidth) / 2,
                 portalPos.y + (tileSize - fontSize) / 2,
                 fontSize, WHITE);
    }
}

void Map::Draw() {
    // Сначала рисуем проходы (фон)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector2 position = GetTilePosition(x, y);
            
            // Фон для проходов (темно-серый)
            if (tiles[y][x] == 1) {
                DrawRectangle(position.x, position.y, (float)tileSize, (float)tileSize, Color{30, 30, 30, 255});
            }
        }
    }
    
    // Затем рисуем стены поверх фона
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Vector2 position = GetTilePosition(x, y);
            
            // Пропускаем моргающие стены - они рисуются отдельно
            bool isBlinkingWall = false;
            for (size_t i = 0; i < blinkingWalls.size(); i++) {
                int bx = blinkingWalls[i].first;
                int by = blinkingWalls[i].second;
                if (bx == x && by == y) {
                    isBlinkingWall = true;
                    break;
                }
            }
            if (isBlinkingWall) continue;
            
            // Пропускаем клетку с порталом
            if (hasPortal && x == portalPosition.first && y == portalPosition.second) {
                continue;
            }
            
            // Рисуем стены с текстурами или цветами
            switch (tiles[y][x]) {
                case 0: // Неразрушаемая стена
                    if (wallTexture.id != 0) {
                        // Рисуем текстуру стены
                        Rectangle sourceRec = { 0.0f, 0.0f, (float)wallTexture.width, (float)wallTexture.height };
                        float tileSizeFloat = static_cast<float>(tileSize);
                        Rectangle destRec = { position.x, position.y, tileSizeFloat, tileSizeFloat };
                        Vector2 origin = { 0, 0 };
                        DrawTexturePro(wallTexture, sourceRec, destRec, origin, 0.0f, WHITE);
                        
                        // Для отладки - рисуем рамку
                        // DrawRectangleLinesEx(destRec, 1, GRAY);
                    } else {
                        // Границы рисуем более темным цветом
                        if (x == 0 || y == 0 || x >= width - 2 || y >= height - 2) {
                            DrawRectangle(position.x, position.y, tileSize, tileSize, GRAY); // Границы
                        } else {
                            DrawRectangle(position.x, position.y, tileSize, tileSize, LIGHTGRAY); // Внутренние
                        }
                    }
                    break;
                    
                case 2: // Разрушаемая стена
                    if (destructibleWallTexture.id != 0) {
                        // Рисуем текстуру разрушаемой стены
                        Rectangle sourceRec = { 0.0f, 0.0f, (float)destructibleWallTexture.width, (float)destructibleWallTexture.height };
                        float tileSizeFloat = static_cast<float>(tileSize);
                        Rectangle destRec = { position.x, position.y, tileSizeFloat, tileSizeFloat };
                        Vector2 origin = { 0, 0 };
                        DrawTexturePro(destructibleWallTexture, sourceRec, destRec, origin, 0.0f, WHITE);
                        
                        // Для отладки - рисуем рамку
                        // DrawRectangleLinesEx(destRec, 1, BROWN);
                    } else {
                        // Разрушаемая стена - коричневый
                        DrawRectangle(position.x, position.y, tileSize, tileSize, BROWN);
                        // Простая текстура
                        DrawRectangleLines(position.x, position.y, tileSize, tileSize, DARKBROWN);
                        DrawLine(position.x, position.y + tileSize/2, 
                                position.x + tileSize, position.y + tileSize/2, DARKBROWN);
                        DrawLine(position.x + tileSize/2, position.y,
                                position.x + tileSize/2, position.y + tileSize, DARKBROWN);
                    }
                    break;
            }
            
            // Рисуем тонкую сетку
            DrawRectangleLines(position.x, position.y, tileSize, tileSize, Color{20, 20, 20, 100});
        }
    }
    
    // Рисуем моргающие стены и портал поверх всего
    DrawBlinkingWallsAndPortal();
}

bool Map::IsWalkable(int gridX, int gridY) const {
    if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height) 
        return false;
    
    return tiles[gridY][gridX] == 1;
}

bool Map::IsDestructible(int gridX, int gridY) const {
    if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height) 
        return false;
    
    return tiles[gridY][gridX] == 2;
}

bool Map::IsWall(int gridX, int gridY) const {
    if (gridX < 0 || gridX >= width || gridY < 0 || gridY >= height) 
        return false;
    
    return tiles[gridY][gridX] == 0 || tiles[gridY][gridX] == 2;
}

void Map::DestroyWall(int gridX, int gridY) {
    if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
        if (tiles[gridY][gridX] == 2) {
            tiles[gridY][gridX] = 1;
        }
    }
}

Vector2 Map::GetTilePosition(int gridX, int gridY) const {
    return { static_cast<float>(gridX * tileSize), static_cast<float>(gridY * tileSize) };
}

Vector2 Map::GetGridPosition(float worldX, float worldY) const {
    return { 
        static_cast<float>(static_cast<int>(worldX) / tileSize),
        static_cast<float>(static_cast<int>(worldY) / tileSize)
    };
}

int Map::CountWalls() const {
    int count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (tiles[y][x] == 0 || tiles[y][x] == 2) {
                count++;
            }
        }
    }
    return count;
}

int Map::CountDestructibleWalls() const {
    int count = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (tiles[y][x] == 2) {
                count++;
            }
        }
    }
    return count;
}