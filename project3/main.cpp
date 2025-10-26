/**
 * Author: Yuxiang Weng
 * Assignment: Lunar Lander
 * Date due: 2025-10-25, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/
#include "CS3113/Entity.h"

// Global Constants
constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr char    BG_COLOUR[] = "#000000";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

constexpr int   NUMBER_OF_TILES         = 11,
                NUMBER_OF_BLOCKS        = 9,
                NUMBER_OF_MOVING        = 2,
                NUMBER_OF_LAND_SEGMENTS = 20;
constexpr float TILE_DIMENSION          = 50.0f,
                ACCELERATION_OF_GRAVITY = 10.0f,
                FIXED_TIMESTEP          = 1.0f / 60.0f,
                END_GAME_THRESHOLD      = 800.0f;
constexpr float MOVING_SPEED = 10.0f;
constexpr float LAND_SPEED_THRESHOLD = 20.0f;

// Global Variables
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;
float lastVec = 0.0f;
float maxXAcceleration = 100.0f;
bool successfullyLand =  false;
bool gameEnd = false;
int fuel = 1500;

Entity *lander = nullptr;
Entity *gTiles   = nullptr;
Entity *gBlocks  = nullptr;

bool movingLeft = false;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lunar Lander");

    float sizeRatio  = 48.0f / 64.0f;

    // Assets from @see https://sscary.itch.io/the-adventurer-female
    lander = new Entity();
    lander->setPosition({ORIGIN.x - 300.0f, ORIGIN.y - 200.0f});
    lander->setScale({50.0f * sizeRatio, 50.0f});
    lander->setTexture("assets/game/lander.png");
    lander->setEntityType(PLAYER);
    lander->setColliderDimensions({
        lander->getScale().x,
        lander->getScale().y
    });
    lander->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    /*
       land generate
    */
    gTiles = new Entity[NUMBER_OF_TILES + NUMBER_OF_MOVING];
    gBlocks = new Entity[NUMBER_OF_BLOCKS];

    float leftMostX = TILE_DIMENSION / 2.0f;
    int tileIndex = 0;
    int blockIndex = 0;

    // start from left, every 3 blocks and 3 tiles
    for (int i = 0; i < NUMBER_OF_TILES + NUMBER_OF_BLOCKS; i++)
    {
        // Alternating pattern: 3 safe tiles, 3 dangerous blocks
        if ((i / 3) % 2 == 0)
        {
            if (tileIndex < NUMBER_OF_TILES)
            {
                gTiles[tileIndex].setTexture("assets/game/tile_0000.png");
                gTiles[tileIndex].setEntityType(PLATFORM);
                gTiles[tileIndex].setScale({TILE_DIMENSION, TILE_DIMENSION});
                gTiles[tileIndex].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
                gTiles[tileIndex].setPosition({leftMostX + i * TILE_DIMENSION, SCREEN_HEIGHT - TILE_DIMENSION / 2});
                tileIndex++;
            }
        }
        else
        {
            if (blockIndex < NUMBER_OF_BLOCKS)
            {
                gBlocks[blockIndex].setTexture("assets/game/tile_0061.png");
                gBlocks[blockIndex].setEntityType(BLOCK);
                gBlocks[blockIndex].setScale({TILE_DIMENSION, TILE_DIMENSION});
                gBlocks[blockIndex].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
                gBlocks[blockIndex].setPosition({leftMostX + i * TILE_DIMENSION, SCREEN_HEIGHT - TILE_DIMENSION});
                blockIndex++;
            }
        }
    }

    // moving tiles start from the center of the screen
    for (int i = 0; i < NUMBER_OF_MOVING; i++) {
        gTiles[NUMBER_OF_TILES + i].setTexture("assets/game/tile_0000.png");
        gTiles[NUMBER_OF_TILES + i].setEntityType(PLATFORM);
        gTiles[NUMBER_OF_TILES + i].setScale({TILE_DIMENSION, TILE_DIMENSION});
        gTiles[NUMBER_OF_TILES + i].setColliderDimensions({TILE_DIMENSION, TILE_DIMENSION});
        gTiles[NUMBER_OF_TILES + i].setPosition({ORIGIN.x + i * TILE_DIMENSION, SCREEN_HEIGHT - TILE_DIMENSION * 3});
    }

    SetTargetFPS(FPS);
}

void processInput() 
{
    bool accOnX = false;
    // update lander acc, limit to max acceleration
    Vector2 currentAcc = lander->getAcceleration();
    float targetX = currentAcc.x;
    if (IsKeyDown(KEY_A) && fuel > 0) {
        targetX -= 0.5f;
        accOnX = true;
        fuel--;
    }
    else if (IsKeyDown(KEY_D) && fuel > 0) {
        targetX += 0.5f;
        accOnX = true;
        fuel--;
    }
    if (targetX > maxXAcceleration)
        targetX = maxXAcceleration;
    else if (targetX < -maxXAcceleration)
        targetX = -maxXAcceleration;

    // is user isn't currently pressing a or d, give a inverse acc to the velocity to 
    // slowly stop the ship
    Vector2 currentVec = lander->getVelocity();
    if (!accOnX) {
        targetX = 0;
    }

    if (IsKeyDown(KEY_W) && fuel > 0) {
        float targetY = lander->getVelocity().y;
        fuel--;
        // avoid back
        if (targetY < 0) {
            lander->setAcceleration(Vector2{targetX, 0});
        } else {
            lander->setAcceleration(Vector2{targetX, currentAcc.y - 0.3f});
        }
    } else {
        lander->setAcceleration(Vector2{targetX, ACCELERATION_OF_GRAVITY});
    }

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    // Fixed timestep
    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        lander->update(FIXED_TIMESTEP, gTiles, NUMBER_OF_TILES + NUMBER_OF_MOVING, gBlocks,
            NUMBER_OF_BLOCKS);

        // update moving tiles, start 
        if (movingLeft) {
            // check the left most moving tile
            if (gTiles[NUMBER_OF_TILES + 0].getPosition().x < TILE_DIMENSION / 2) {
                movingLeft = false;
            } else {
                // update moving tiles
                for (int i = 0; i < NUMBER_OF_MOVING; i++) {
                    gTiles[NUMBER_OF_TILES + i].setPosition({
                        gTiles[NUMBER_OF_TILES + i].getPosition().x - MOVING_SPEED * deltaTime,
                        gTiles[NUMBER_OF_TILES + i].getPosition().y
                    });
                }
            }
        } else {
            // check the right most moving tile
            if (gTiles[NUMBER_OF_TILES + NUMBER_OF_MOVING - 1].getPosition().x > SCREEN_WIDTH - TILE_DIMENSION / 2) {
                movingLeft = true;
            } else {
                // update moving tiles
                for (int i = 0; i < NUMBER_OF_MOVING; i++) {
                    gTiles[NUMBER_OF_TILES + i].setPosition({
                        gTiles[NUMBER_OF_TILES + i].getPosition().x + MOVING_SPEED * deltaTime,
                        gTiles[NUMBER_OF_TILES + i].getPosition().y
                    });
                }
            }
        }

        for (int i = 0; i < NUMBER_OF_BLOCKS; i++)
        {
            gBlocks[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }

        for (int i = 0; i < NUMBER_OF_TILES + NUMBER_OF_MOVING; i++) 
        {
            gTiles[i].update(FIXED_TIMESTEP, nullptr, 0, nullptr, 0);
        }

        deltaTime -= FIXED_TIMESTEP;

        // check if lander is on the any of the PLATFORM
        EntityType collidingType = lander->getCollidingEntityType();
        if (collidingType == EntityType::PLATFORM) {
            gameEnd = true;
            // if speed is above threshold, game end
            successfullyLand = lastVec <= LAND_SPEED_THRESHOLD;
        } else if (collidingType == EntityType::BLOCK) {
            gameEnd = true;
            successfullyLand = false;
        }
    }

    // make a copy of 
    Vector2 speed = lander->getVelocity();
    lastVec = sqrt(speed.x * speed.x + speed.y * speed.y);

    if (lander->getPosition().y > END_GAME_THRESHOLD) 
        gAppStatus = TERMINATED;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOUR));

    if (!gameEnd) {
        lander->render();

        for (int i = 0; i < NUMBER_OF_TILES + NUMBER_OF_MOVING;  i++) gTiles[i].render();
        for (int i = 0; i < NUMBER_OF_BLOCKS; i++) gBlocks[i].render();

        // draw lander acc and vec and fuel
        DrawText(TextFormat("Acc: (%.2f, %.2f)", lander->getAcceleration().x, lander->getAcceleration().y), 10, 10, 20, WHITE);
        DrawText(TextFormat("Vec: (%.2f, %.2f)", lander->getVelocity().x, lander->getVelocity().y), 10, 40, 20, WHITE);
        DrawText(TextFormat("Fuel: %d", fuel), 10, 70, 20, WHITE);
        // land on the yellow block
        // DrawText()
    } else {
        DrawText(successfullyLand ? "Successfully Land" : "Your ship crashed, try land slower on the yellow block", 100, 300, 20, WHITE);
        DrawText("Press Q to Quit", 100, 400, 20, WHITE);
    }

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow();
}

int main(void)
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}