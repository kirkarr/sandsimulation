#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <SDL2/SDL.h>

// Размеры окна и клеток
#define WIDTH 800
#define HEIGHT 600
#define CELL_SIZE 4

// Размеры сетки
#define GRID_WIDTH (WIDTH / CELL_SIZE)
#define GRID_HEIGHT (HEIGHT / CELL_SIZE)

// Типы клеток
#define EMPTY 0
#define SAND 1
#define WALL 2

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int grid[GRID_HEIGHT][GRID_WIDTH] = {0};
int new_grid[GRID_HEIGHT][GRID_WIDTH] = {0};

// Цвета для разных типов клеток
SDL_Color colors[] = {
    {0, 0, 0, 255},       // EMPTY - черный
    {194, 178, 128, 255}, // SAND - песочный
    {100, 100, 100, 255}   // WALL - серый
};

// Инициализация SDL
bool init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    window = SDL_CreateWindow("Sand Simulation", SDL_WINDOWPOS_UNDEFINED, 
                             SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    
    return true;
}

// Инициализация поля
void init_grid() {
    // Заполняем нижнюю строку стенами
    for (int x = 0; x < GRID_WIDTH; x++) {
        grid[GRID_HEIGHT-1][x] = WALL;
    }
    
    // Добавляем случайные стены
    for (int i = 0; i < 50; i++) {
        int x = rand() % GRID_WIDTH;
        int y = rand() % (GRID_HEIGHT/2) + GRID_HEIGHT/2;
        grid[y][x] = WALL;
    }
}

// Отрисовка поля
void draw_grid() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            SDL_Color color = colors[grid[y][x]];
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            
            SDL_Rect rect = {
                x * CELL_SIZE,
                y * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE
            };
            
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    
    SDL_RenderPresent(renderer);
}

// Обновление состояния песка
void update_sand() {
    // Копируем статичные элементы (стены)
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == WALL) {
                new_grid[y][x] = WALL;
            } else {
                new_grid[y][x] = EMPTY;
            }
        }
    }
    
    // Обрабатываем песок
    for (int y = GRID_HEIGHT-1; y >= 0; y--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == SAND) {
                // Пытаемся двигаться вниз
                if (y+1 < GRID_HEIGHT && new_grid[y+1][x] == EMPTY) {
                    new_grid[y+1][x] = SAND;
                } 
                // Если внизу занято, пробуем вниз-влево или вниз-вправо
                else {
                    int direction = rand() % 2;
                    int new_x = x;
                    
                    if (direction == 0) {
                        new_x = x - 1; // Влево
                    } else {
                        new_x = x + 1; // Вправо
                    }
                    
                    if (new_x >= 0 && new_x < GRID_WIDTH && y+1 < GRID_HEIGHT && new_grid[y+1][new_x] == EMPTY) {
                        new_grid[y+1][new_x] = SAND;
                    } else {
                        new_grid[y][x] = SAND;
                    }
                }
            }
        }
    }
    
    // Копируем новое состояние в основную сетку
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x] = new_grid[y][x];
        }
    }
}

// Добавление песка в указанную позицию
void add_sand(int x, int y) {
    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT && grid[y][x] == EMPTY) {
        grid[y][x] = SAND;
    }
}

// Добавление стены в указанную позицию
void add_wall(int x, int y) {
    if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
        grid[y][x] = WALL;
    }
}

int main() {
    srand(time(NULL));
    
    if (!init_sdl()) {
        return 1;
    }
    
    init_grid();
    
    bool running = true;
    bool mouse_down = false;
    bool placing_wall = false;
    SDL_Event event;
    
    while (running) {
        // Обработка событий
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                    
                case SDL_MOUSEBUTTONDOWN:
                    mouse_down = true;
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        placing_wall = false;
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        placing_wall = true;
                    }
                    break;
                    
                case SDL_MOUSEBUTTONUP:
                    mouse_down = false;
                    break;
                    
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_c) {
                        // Очистка экрана (кроме нижней стены)
                        for (int y = 0; y < GRID_HEIGHT-1; y++) {
                            for (int x = 0; x < GRID_WIDTH; x++) {
                                grid[y][x] = EMPTY;
                            }
                        }
                    }
                    break;
            }
        }
        
        // Добавление песка/стен с помощью мыши
        if (mouse_down) {
            int mouse_x, mouse_y;
            SDL_GetMouseState(&mouse_x, &mouse_y);
            
            int grid_x = mouse_x / CELL_SIZE;
            int grid_y = mouse_y / CELL_SIZE;
            
            if (placing_wall) {
                add_wall(grid_x, grid_y);
            } else {
                add_sand(grid_x, grid_y);
            }
        }
        
        // Добавляем случайные песчинки сверху
        int x = rand() % GRID_WIDTH;
        add_sand(x, 0);

        
        update_sand();
        draw_grid();
        SDL_Delay(16); // ~60 FPS
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}