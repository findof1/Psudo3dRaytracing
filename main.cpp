#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <array>
#include "textures.h"

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

enum SpriteType
{
    Key,
    Light,
    Enemy
};

struct Sprite
{
    SpriteType type;
    int state;
    float x, y, z;
    float width, height;
};

Sprite sprites[1];

const float rayStep = 0.25;

float distances[241];

int map[] = {
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 3, 3, 3, 0, 4, 0, 4, 4, 6, 6, 6, 6, 0, 2,
    2, 0, 3, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 6, 0, 2,
    2, 0, 3, 0, 4, 4, 4, 4, 4, 4, 6, 0, 0, 6, 0, 2,
    2, 0, 0, 0, 4, 0, 0, 0, 0, 0, 6, 0, 0, 6, 0, 2,
    2, 0, 4, 4, 4, 0, 1, 1, 1, 0, 6, 6, 6, 6, 0, 2,
    2, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    2, 1, 1, 1, 1, 0, 1, 0, 6, 6, 6, 6, 6, 6, 6, 2,
    2, 0, 0, 0, 0, 0, 1, 0, 6, 0, 0, 0, 0, 0, 0, 2,
    2, 0, 1, 1, 1, 1, 1, 0, 4, 4, 4, 4, 4, 4, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 2,
    2, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 4, 0, 2,
    2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

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

void hexToRGB(int hexColor, Uint8 &r, Uint8 &g, Uint8 &b)
{
    b = (hexColor >> 16) & 0xFF;
    g = (hexColor >> 8) & 0xFF;
    r = hexColor & 0xFF;
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

    for (float i = 0; i < player->FOV; i += rayStep)
    {
        float rayX = player->pos.x;
        float rayY = player->pos.y;
        float dy;
        float dx;

        float distanceHorizontal = 10000000;
        int cellIndexX;
        int cellIndexY = floor(rayY / cellWidth);
        int depth = 0;

        int mappedPosHorizontal;
        int hitTypeHorizontal;

        while (depth < 16)
        {

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

            rayY = rayY + dy;
            rayX = rayX + dx;
            cellIndexX = floor(rayX / cellWidth);
            cellIndexY = floor(rayY / cellWidth);

            int mapCellIndex = getCell(cellIndexX, cellIndexY);

            if (mapCellIndex == -1)
            {
                depth = 16;
            }
            if (map[mapCellIndex] != 0)
            {
                hitTypeHorizontal = map[mapCellIndex];
                depth = 16;
                mappedPosHorizontal = static_cast<int>((rayX - cellIndexX * cellWidth) / 2.0f);
                distanceHorizontal = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
            }
            if (map[getCell(cellIndexX, cellIndexY - 1)] != 0)
            {
                hitTypeHorizontal = map[getCell(cellIndexX, cellIndexY - 1)];
                depth = 16;
                mappedPosHorizontal = static_cast<int>((rayX - cellIndexX * cellWidth) / 2.0f);
                distanceHorizontal = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
            }
            depth++;
        }

        float horizontalRayX = rayX;
        float horizontalRayY = rayY;
        // vertical

        int mappedPosVertical;
        int hitTypeVertical;

        rayX = player->pos.x;
        rayY = player->pos.y;

        float distanceVertical = 10000000;

        cellIndexX = floor(rayX / cellWidth);

        depth = 0;
        while (depth < 16)
        {

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

            rayY = rayY + dy;
            rayX = rayX + dx;
            cellIndexX = floor(rayX / cellWidth);
            cellIndexY = floor(rayY / cellWidth);
            int mapCellIndex = getCell(cellIndexX, cellIndexY);

            if (mapCellIndex == -1)
            {
                depth = 16;
            }

            if (map[mapCellIndex] != 0)
            {
                hitTypeVertical = map[mapCellIndex];
                depth = 16;
                mappedPosVertical = static_cast<int>((rayY - cellIndexY * cellWidth) / 2.0f);
                distanceVertical = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
            }
            if (map[getCell(cellIndexX - 1, cellIndexY)] != 0)
            {
                hitTypeVertical = map[getCell(cellIndexX - 1, cellIndexY)];
                depth = 16;
                mappedPosVertical = static_cast<int>((rayY - cellIndexY * cellWidth) / 2.0f);
                distanceVertical = sqrt(pow(rayX - player->pos.x, 2) + pow(rayY - player->pos.y, 2));
            }
            depth++;
        }

        rayAngle = FixAngle(rayAngle + rayStep);
        /*
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, horizontalRayX, horizontalRayY);
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, rayX, rayY);
        */
        int mappedPos;
        int hitType;
        // SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        if (distanceVertical < distanceHorizontal)
        {
            // SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, horizontalRayX, horizontalRayY);
            mappedPos = mappedPosVertical;
            hitType = hitTypeVertical;
            SDL_SetRenderDrawColor(renderer, 0, 155, 0, 255);
        }
        else
        {
            mappedPos = mappedPosHorizontal;
            hitType = hitTypeHorizontal;
            SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
            // SDL_RenderDrawLine(renderer, player->pos.x, player->pos.y, rayX, rayY);
        }

        float distance = std::min(distanceHorizontal, distanceVertical);
        float correctedDistance = distance * cos(degToRad(FixAngle(player->angle - rayAngle)));
        distances[static_cast<int>(i / rayStep)] = distance;
        SDL_FRect rectangle;
        rectangle.x = i * (1024 / (player->FOV));
        rectangle.h = (64 * 512) / correctedDistance;
        rectangle.y = (512 / 2) - (rectangle.h / 2);
        rectangle.w = (1024 / (player->FOV)) * rayStep;
        // SDL_RenderFillRect(renderer, &rectangle);

        float smallRectHeight = rectangle.h / 32;

        for (int j = 0; j < 32; j++)
        {
            uint32_t textureColorHex;
            textureColorHex = textures[hitType - 1][mappedPos + j * 32];
            Uint8 r, g, b;
            hexToRGB(textureColorHex, r, g, b);
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            float smallRectY = rectangle.y + j * smallRectHeight;

            SDL_FRect smallRect = rectangle;
            smallRect.y = smallRectY;
            smallRect.h = smallRectHeight;

            SDL_RenderFillRectF(renderer, &smallRect);
        }
    }
}

void drawSprites(SDL_Renderer *renderer, Player *player)
{
    for (int i = 0; i < 1; i++)
    {
        float spriteX = sprites[i].x - player->pos.x;
        float spriteY = sprites[i].y - player->pos.y;
        float spriteZ = sprites[i].z;

        float angleRad = -degToRad(player->angle);
        float rotatedX = spriteY * cos(angleRad) + spriteX * sin(angleRad);
        float rotatedY = spriteX * cos(angleRad) - spriteY * sin(angleRad);

        if (rotatedY > 0)
        {

            float fovFactor = 512.0f / tan(degToRad(player->FOV / 2));

            float projectedX = (rotatedX * fovFactor / rotatedY) + (1024 / 2);
            float projectedY = (spriteZ * fovFactor / rotatedY) + (512 / 2);

            float distance = sqrt(pow(spriteX, 2) + pow(spriteY, 2));

            float preCalculatedWidth = (1024 / (player->FOV)) * rayStep + (1024.f / distance);
            float preCalculatedHeight = (1024 / (player->FOV)) * rayStep + (512.f / distance);

            for (int x = 0; x < sprites[i].width; x++)
            {
                float recX = projectedX + ((x * 256) / distance);

                if (recX >= 0 && distance < distances[static_cast<int>((recX) / (1024 / 240))])
                {

                    for (int y = 0; y < sprites[i].height; y++)
                    {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                        SDL_FRect rectangle;
                        rectangle.x = recX;
                        rectangle.y = projectedY - ((y * 256) / distance);
                        rectangle.w = preCalculatedWidth;
                        rectangle.h = preCalculatedHeight;
                        SDL_RenderFillRectF(renderer, &rectangle);
                    }
                }
            }
        }
    }
}

void handleInput(Player *player)
{
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);

    if (keystate[SDL_SCANCODE_W])
    {
        int cellIndexX = floor(((player->pos.x + (moveSpeed * cos(degToRad(player->angle)))) * 1.0) / cellWidth);
        int cellIndexY = floor(((player->pos.y + (moveSpeed * sin(degToRad(player->angle)))) * 1.0) / cellWidth);

        int mapCellIndex = getCell(cellIndexX, cellIndexY);

        if (map[mapCellIndex] == 0)
        {
            player->pos.x += moveSpeed * cos(degToRad(player->angle));
            player->pos.y += moveSpeed * sin(degToRad(player->angle));
        }
    }
    if (keystate[SDL_SCANCODE_S])
    {
        int cellIndexX = floor(((player->pos.x - (moveSpeed * cos(degToRad(player->angle)) * 1.1))) / cellWidth);
        int cellIndexY = floor(((player->pos.y - (moveSpeed * sin(degToRad(player->angle)) * 1.1))) / cellWidth);
        int mapCellIndex = getCell(cellIndexX, cellIndexY);

        if (map[mapCellIndex] == 0)
        {
            player->pos.x -= moveSpeed * cos(degToRad(player->angle));
            player->pos.y -= moveSpeed * sin(degToRad(player->angle));
        }
    }

    if (keystate[SDL_SCANCODE_A])
    {
        player->angle -= rotateSpeed;
    }
    if (keystate[SDL_SCANCODE_D])
    {
        player->angle += rotateSpeed;
    }

    if (keystate[SDL_SCANCODE_E])
    {
        int cellIndexX = floor(((player->pos.x + (moveSpeed * cos(degToRad(player->angle)) * 4))) / cellWidth);
        int cellIndexY = floor(((player->pos.y + (moveSpeed * sin(degToRad(player->angle)) * 4))) / cellWidth);

        int mapCellIndex = getCell(cellIndexX, cellIndexY);

        if (map[mapCellIndex] == 5)
        {
            map[mapCellIndex] = 0;
        }
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

    Player player = {{80.0f, 80.0f}, 0.0f, 60};
    Sprite key;
    key.state = 1;
    key.type = Key;
    key.x = 85;
    key.y = 85;
    key.z = 20;
    key.width = 20;
    key.height = 60;
    sprites[0] = key;

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
        drawSprites(renderer, &player);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}