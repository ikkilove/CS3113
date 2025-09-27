/**
 * Author: Yuxiang Weng
 * Assignment: Simple 2D Scene
 * Date due: 2025-09-27, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#include "CS3113/cs3113.h"
#include <map>
#include <array>
#include <string>
#include <math.h>
#include <cstring>

// cat image enum
enum CAT
{
    ANGRY,
    HAPPY,
    SAD
};

// Global Constants
constexpr int SCREEN_WIDTH = 1600 / 4 * 3,
              SCREEN_HEIGHT = 900 / 4 * 3,
              FPS = 60,
              SIZE = 100,
              FRAME_LIMIT = 100;
constexpr float MAX_AMP = 50.0f;
constexpr Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
constexpr Vector2 BASE_SIZE = {(float)SIZE, (float)SIZE};
constexpr const char* BG_COLOURS[] = {
    "#000000", // black
    "#FFFFFF", // white
    "#FF0000", // red
    "#00FF00", // green
    "#0000FF", // blue
    "#FFFF00", // yellow
    "#00FFFF", // cyan
    "#FF00FF"  // magenta
};

// image assets for 3 cats
constexpr std::array<const char *, 3> CATS_SOURCE = {
    "assets/1.jpg",
    "assets/2.jpg",
    "assets/3.jpg"};

// Global Variables
AppStatus gAppStatus = RUNNING;
float gPulseTime = 0.0f;
float gScaleFactor[] = {SIZE, SIZE, SIZE};
float gAngle[] = {0.0f, 0.0f, 0.0f};
Vector2 gPosition = ORIGIN;
Vector2* gScale = new Vector2[3]{BASE_SIZE, BASE_SIZE, BASE_SIZE};
// global rotate condition
float gRotate = 0.0f;

float gPreviousTicks = 0.0f;
int gFrameCounter = 0;
int currentColorIndex = 0;

Texture2D* gTextures = nullptr;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Textures");

    // load 3 cat textures
    gTextures = new Texture2D[CATS_SOURCE.size()];
    for (int i = 0; i < CATS_SOURCE.size(); i++)
    {
        gTextures[i] = LoadTexture(CATS_SOURCE[i]);
    }

    SetTargetFPS(FPS);
}

void processInput()
{
    if (WindowShouldClose())
        gAppStatus = TERMINATED;
}

void update()
{
    // Delta time
    float ticks = (float)GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    // heartbeat
    gPulseTime += 1.0f * deltaTime;

    // global rotate
    gRotate += 0.5f * deltaTime;

    // cat 1 rotate by self
    gAngle[0] += 40.0f * deltaTime;
    // cat 2 scale
    gScale[1] = {
    BASE_SIZE.x + MAX_AMP * cos(gPulseTime),
    BASE_SIZE.y + MAX_AMP * cos(gPulseTime)};
    // cat 3 rotate by left up corner
    gAngle[2] -= 80.0f * deltaTime;

    // update bg color every FRAME_LIMIT frames
    if (gFrameCounter++ % FRAME_LIMIT == 0)
    {
        currentColorIndex = (currentColorIndex + 1) % (sizeof(BG_COLOURS) / sizeof(BG_COLOURS[0]));
    }
}

void render()
{
    ClearBackground(ColorFromHex(BG_COLOURS[currentColorIndex]));

    // render 3 cats textures
    for (int i = 0; i < 3; i++)
    {
        // UV coordinates of one picture
        Rectangle textureArea = {0.0f, 0.0f,
                                static_cast<float>(gTextures[i].width),
                                static_cast<float>(gTextures[i].height)};

        // Origin inside the source texture (centre of the texture)
        Vector2 objectOrigin = {
            static_cast<float>(gScale[i].x) / 2.0f,
            static_cast<float>(gScale[i].y) / 2.0f};

        // 3 cats position
        Rectangle destinationArea = {
            gPosition.x + SIZE * 1.5f * cos(gRotate + PI * 2 / 3 * i),
            gPosition.y + SIZE * 1.5f * sin(gRotate + PI * 2 / 3 * i),
            static_cast<float>(gScale[i].x),
            static_cast<float>(gScale[i].y)
        };

        // cat 3 rotate by left up corner
        if (i == 2) {
            objectOrigin = {0.0f, 0.0f};
        }

        DrawTexturePro(
            gTextures[i],
            textureArea,
            destinationArea,
            objectOrigin,
            gAngle[i],
            WHITE);
    }

    EndDrawing();
}

void shutdown() { CloseWindow(); }

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