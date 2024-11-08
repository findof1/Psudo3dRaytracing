#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>

const float moveSpeed = 2.f;
const float rotateSpeed = 3.f;

struct Player
{
    glm::vec2 pos;
    float angle;
    float FOV;
};
#define mapX 16
#define mapY 16
#define mapS 256
#define cellWidth 64

int map[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
    1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1,
    1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
/*
#define mapX 8
#define mapY 8
#define mapS 64
#define cellWidth 64
int map[] =
    {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 1, 0, 1,
        1, 0, 0, 0, 0, 0, 0, 1,
        1, 1, 1, 1, 1, 1, 1, 1};*/

float FixAngle(float a)
{
    if (a > 359)
    {
        a -= 360;
    }
    if (a < 0)
    {
        a += 360;
    }
    return a;
}

float degToRad(float angle) { return angle * M_PI / 180.0; }

int getCell(int x, int y)
{
    if (x >= 0 && x < mapX && y >= 0 && y < mapY)
    {
        int cellIndex = y * mapX + x;
        return cellIndex;
    }
    else
    {
        return -1;
    }
}

void drawMap(SDL_Renderer *renderer)
{
    for (int y = 0; y < mapY; y++)
    {
        for (int x = 0; x < mapX; x++)
        {
            int cell = getCell(x, y);

            if (map[cell] == 1)
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }

            SDL_Rect rect;
            rect.x = x * cellWidth;
            rect.y = y * cellWidth;
            rect.w = cellWidth;
            rect.h = cellWidth;

            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void raycast(Player *player, SDL_Renderer *renderer)
{
    float rayAngle = FixAngle(player->angle - (player->FOV / 2));

    for (float i = 0; i < player->FOV; i += 0.25)
    {
        int rayX = player->pos.x;
        int rayY = player->pos.y;
        int cellIndexY = floor(rayY / cellWidth);
        float dy;
        float dx;

        if (sin(degToRad(rayAngle)) > 0)
        {
            dy = cellWidth - (rayY - (cellWidth * cellIndexY));
            if (dy == 0)
            {
                dy = cellWidth;
            }
        }
        else
        {
            dy = -(rayY - (cellWidth * cellIndexY));
            if (dy == 0)
            {
                dy = -cellWidth;
            }
        }
        dx = dy / tan(degToRad(rayAngle));

        float distanceHorizontal = 10000000;
        rayY = rayY + dy;
        rayX = rayX + dx;
        int cellIndexX = floor(rayX / cellWidth);
        cellIndexY = floor(rayY / cellWidth);
        int depth = 0;
        int mapCellIndex = getCell(cellIndexX, cellIndexY);

        if (mapCellIndex == -1)
        {
            depth = 16;
        }
        if (map[mapCellIndex] == 1 || map[getCell(cellIndexX, cellIndexY - 1)] == 1)
        {
            depth = 16;
            distanceHorizontal = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
        }
        while (depth < 16)
        {

            if (sin(degToRad(rayAngle)) > 0)
            {
                dy = cellWidth;
            }
            else
            {
                dy = -cellWidth;
            }
            dx = dy / tan(degToRad(rayAngle));

            rayY = rayY + dy;
            rayX = rayX + dx;
            cellIndexX = floor(rayX / cellWidth);
            cellIndexY = floor(rayY / cellWidth);
            int mapCellIndex = getCell(cellIndexX, cellIndexY);

            if (mapCellIndex == -1)
            {
                depth = 16;
            }

            if (map[mapCellIndex] == 1 || map[getCell(cellIndexX, cellIndexY - 1)] == 1)
            {
                depth = 16;
                distanceHorizontal = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
            }
            depth++;
        }

        int horizontalRayX = rayX;
        int horizontalRayY = rayY;
        // vertical

        rayX = player->pos.x;
        rayY = player->pos.y;
        cellIndexX = floor(rayX / cellWidth);

        if (cos(degToRad(rayAngle)) > 0)
        {
            dx = cellWidth - (rayX - (cellWidth * cellIndexX));
            if (dx == 0)
            {
                dx = cellWidth;
            }
        }
        else
        {
            dx = -(rayX - (cellWidth * cellIndexX));
            if (dx == 0)
            {
                dx = -cellWidth;
            }
        }
        dy = dx / (1 / tan(degToRad(rayAngle)));

        float distanceVertical = 10000000;
        rayY = rayY + dy;
        rayX = rayX + dx;
        cellIndexX = floor(rayX / cellWidth);
        cellIndexY = floor(rayY / cellWidth);
        depth = 0;
        mapCellIndex = getCell(cellIndexX, cellIndexY);

        if (mapCellIndex == -1)
        {
            depth = 16;
        }
        if (map[mapCellIndex] == 1 || map[getCell(cellIndexX - 1, cellIndexY)] == 1)
        {
            depth = 16;
            distanceVertical = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
        }
        while (depth < 16)
        {

            if (cos(degToRad(rayAngle)) > 0)
            {
                dx = cellWidth;
            }
            else
            {
                dx = -cellWidth;
            }
            dy = dx / (1 / tan(degToRad(rayAngle)));

            rayY = rayY + dy;
            rayX = rayX + dx;
            cellIndexX = floor(rayX / cellWidth);
            cellIndexY = floor(rayY / cellWidth);
            int mapCellIndex = getCell(cellIndexX, cellIndexY);

            if (mapCellIndex == -1)
            {
                depth = 16;
            }

            if (map[mapCellIndex] == 1 || map[getCell(cellIndexX - 1, cellIndexY)] == 1)
            {
                depth = 16;
                distanceVertical = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
            }
            depth++;
        }

        rayAngle = FixAngle(rayAngle + 0.25);
        /*
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, horizontalRayX, horizontalRayY);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, rayX, rayY);
        */

        // SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        if (distanceVertical < distanceHorizontal)
        {
            // SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, horizontalRayX, horizontalRayY);
            SDL_SetRenderDrawColor(renderer, 0, 155, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
            // SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, rayX, rayY);
        }

        float distance = std::min(distanceHorizontal, distanceVertical);
        float correctedDistance = distance * cos(degToRad(FixAngle(player->angle - rayAngle)));
        SDL_Rect rectangle;
        rectangle.x = i * (1024 / (player->FOV));
        rectangle.h = (64 * 512) / correctedDistance;
        rectangle.y = (512 / 2) - (rectangle.h / 2);
        rectangle.w = ((1024 / (player->FOV)) + 3) * 0.25;
        SDL_RenderFillRect(renderer, &rectangle);
    }
}

void handleInput(Player *player)
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_W])
    {
        player->pos.x += moveSpeed * cos(degToRad(player->angle));
        player->pos.y += moveSpeed * sin(degToRad(player->angle));
    }
    if (keystate[SDL_SCANCODE_S])
    {
        player->pos.x -= moveSpeed * cos(degToRad(player->angle));
        player->pos.y -= moveSpeed * sin(degToRad(player->angle));
    }

    if (keystate[SDL_SCANCODE_A])
    {
        player->angle -= rotateSpeed;
    }
    if (keystate[SDL_SCANCODE_D])
    {
        player->angle += rotateSpeed;
    }

    if (keystate[SDL_SCANCODE_LEFT])
    {
        player->pos.x += moveSpeed * cos(degToRad(player->angle) + M_PI / 2);
        player->pos.y += moveSpeed * sin(degToRad(player->angle) + M_PI / 2);
    }
    if (keystate[SDL_SCANCODE_RIGHT])
    {
        player->pos.x -= moveSpeed * cos(degToRad(player->angle) + M_PI / 2);
        player->pos.y -= moveSpeed * sin(degToRad(player->angle) + M_PI / 2);
    }
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Psudo 3d Raytracer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 512, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Player player = {{400.0f, 300.0f}, 0.0f, 60};

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        handleInput(&player);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // drawMap(renderer);

        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_Rect bottomBackground;
        bottomBackground.x = 0;
        bottomBackground.h = 256;
        bottomBackground.y = 256;
        bottomBackground.w = 1024;
        SDL_RenderFillRect(renderer, &bottomBackground);

        SDL_SetRenderDrawColor(renderer, 51, 197, 255, 255);
        SDL_Rect topBackground;
        topBackground.x = 0;
        topBackground.h = 256;
        topBackground.y = 0;
        topBackground.w = 1024;
        SDL_RenderFillRect(renderer, &topBackground);

        raycast(&player, renderer);
        // SDL_RenderDrawPoint(renderer, static_cast<int>(player.pos.x), static_cast<int>(player.pos.y));

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}