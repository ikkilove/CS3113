/**
 * Author: Yuxiang Weng
 * Assignment: Pong Clone
 * Date due: 2025-10-13, 11:59pm
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

// Global Constants
constexpr int SCREEN_WIDTH = 800,
              SCREEN_HEIGHT = 450,
              FPS = 60,
              BALL_SPEED = 200,
              PADDLE_SPEED = 300,
              SHRINK_RATE = 100;

constexpr Vector2 ORIGIN = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
constexpr int FRAME_LIMIT = 120;

// bg color loop container
constexpr const char *BG_COLOURS[] = {
    "#000000", // black
    "#FF0000", // red
    "#0000FF", // blue
};

// image assets for 3 cats
constexpr std::array<const char *, 3> CATS_SOURCE = {
    "assets/1.jpg",
    "assets/2.jpg",
    "assets/3.jpg"};

// Global Variables
AppStatus gAppStatus = RUNNING;
float gPreviousTicks = 0.0f;
float gPulseTime = 0.0f;

// paddle & ball
Vector2 leftPaddle = {50.0f, SCREEN_HEIGHT / 2.0f};
Vector2 rightPaddle = {SCREEN_WIDTH - 50.0f, SCREEN_HEIGHT / 2.0f};
Vector2 paddleScale = {10.0f, 100.0f};
float paddleVelocity = PADDLE_SPEED;

// ball always starts from the left paddle
Vector2 ballScale = {10.0f, 10.0f};
// at most 3 ball position
Vector2 *ballPositions = new Vector2[3];
Vector2 *ballVelocity = new Vector2[3];
int ballCount = 1;

// top and bottom walls
Vector2 topWall = {SCREEN_WIDTH / 2.0f, 20.0f};
Vector2 bottomWall = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT - 20.0f};
Vector2 wallScale = {SCREEN_WIDTH, 10.0f};

// whether single player mode is on
bool singlePlayer = false;
bool moveUp = true;
bool gameOver = false;
bool leftWin = false;

int gFrameCounter = 0;
int currentColorIndex = 0;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();
bool isColliding(const Vector2 *postionA, const Vector2 *scaleA, const Vector2 *positionB, const Vector2 *scaleB);

/**
 * @brief Checks for a square collision between 2 Rectangle objects.
 *
 * @see
 *
 * @param postionA The position of the first object
 * @param scaleA The scale of the first object
 * @param positionB The position of the second object
 * @param scaleB The scale of the second object
 * @return true if a collision is detected,
 * @return false if a collision is not detected
 */
bool isColliding(const Vector2 *postionA, const Vector2 *scaleA,
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(postionA->x - positionB->x) - ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(postionA->y - positionB->y) - ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f)
        return true;

    return false;
}

// Function Definitions
void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Textures");

    // init ball positions and velocities
    ballPositions[0] = (Vector2){leftPaddle.x + (paddleScale.x / 2.0f) + 5.0f, leftPaddle.y};
    ballPositions[1] = (Vector2){leftPaddle.x + (paddleScale.x / 2.0f) + 5.0f, leftPaddle.y + 10};
    ballPositions[2] = (Vector2){leftPaddle.x + (paddleScale.x / 2.0f) + 5.0f, leftPaddle.y - 10};

    ballVelocity[0] = (Vector2){BALL_SPEED, BALL_SPEED / 2.0f};
    ballVelocity[1] = (Vector2){BALL_SPEED, BALL_SPEED / 3.0f};
    ballVelocity[2] = (Vector2){BALL_SPEED, BALL_SPEED / 4.0f};

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

    // if game is running
    if (!gameOver)
    {
        // if t is pressed toggle single player mode
        if (IsKeyPressed(KEY_T))
            singlePlayer = !singlePlayer;

        // check if ball collides with any thing
        for (int i = 0; i < ballCount; i++)
        {
            // update ball position
            ballPositions[i] = (Vector2){
                ballPositions[i].x + ballVelocity[i].x * deltaTime,
                ballPositions[i].y + ballVelocity[i].y * deltaTime};

            // check if ball collides with top wall or bottom wall
            if (isColliding(&ballPositions[i], &ballScale, &topWall, &wallScale))
            {
                ballVelocity[i].y = -ballVelocity[i].y;
                ballPositions[i].y = topWall.y + (wallScale.y / 2.0f) + (ballScale.y / 2.0f);
            }
            if (isColliding(&ballPositions[i], &ballScale, &bottomWall, &wallScale))
            {
                ballVelocity[i].y = -ballVelocity[i].y;
                ballPositions[i].y = bottomWall.y - (wallScale.y / 2.0f) - (ballScale.y / 2.0f);
            }

            // check if ball collides with left paddle or right paddle
            if (isColliding(&ballPositions[i], &ballScale, &leftPaddle, &paddleScale))
            {
                ballVelocity[i].x = -ballVelocity[i].x;
                ballPositions[i].x = leftPaddle.x + (paddleScale.x / 2.0f) + (ballScale.x / 2.0f);
            }
            if (isColliding(&ballPositions[i], &ballScale, &rightPaddle, &paddleScale))
            {
                ballVelocity[i].x = -ballVelocity[i].x;
                ballPositions[i].x = rightPaddle.x - (paddleScale.x / 2.0f) - (ballScale.x / 2.0f);
            }
        }

        // update left paddle position
        if (IsKeyDown(KEY_W))
        {
            leftPaddle.y -= paddleVelocity * deltaTime;
            // limit paddle movement to top wall
            if (leftPaddle.y - (paddleScale.y / 2.0f) < 0.0f)
                leftPaddle.y = paddleScale.y / 2.0f;
        }
        if (IsKeyDown(KEY_S))
        {
            leftPaddle.y += paddleVelocity * deltaTime;
            if (leftPaddle.y + (paddleScale.y / 2.0f) > SCREEN_HEIGHT)
                leftPaddle.y = SCREEN_HEIGHT - (paddleScale.y / 2.0f);
        }

        // update ball count based on user input, 1, 2, or 3 balls
        if (IsKeyPressed(KEY_ONE))
            ballCount = 1;
        if (IsKeyPressed(KEY_TWO))
            ballCount = 2;
        if (IsKeyPressed(KEY_THREE))
            ballCount = 3;

        // check current mode to update right paddle position
        if (singlePlayer)
        {
            if (moveUp)
            {
                rightPaddle.y -= paddleVelocity * deltaTime;
                if (rightPaddle.y - (paddleScale.y / 2.0f) < 0.0f)
                {
                    rightPaddle.y = paddleScale.y / 2.0f;
                    moveUp = false;
                }
            }
            else
            {
                rightPaddle.y += paddleVelocity * deltaTime;
                if (rightPaddle.y + (paddleScale.y / 2.0f) > SCREEN_HEIGHT)
                {
                    rightPaddle.y = SCREEN_HEIGHT - (paddleScale.y / 2.0f);
                    moveUp = true;
                }
            }
        }
        else
        {
            if (IsKeyDown(KEY_UP))
            {
                rightPaddle.y -= paddleVelocity * deltaTime;
                if (rightPaddle.y - (paddleScale.y / 2.0f) < 0.0f)
                    rightPaddle.y = paddleScale.y / 2.0f;
            }
            if (IsKeyDown(KEY_DOWN))
            {
                rightPaddle.y += paddleVelocity * deltaTime;
                if (rightPaddle.y + (paddleScale.y / 2.0f) > SCREEN_HEIGHT)
                    rightPaddle.y = SCREEN_HEIGHT - (paddleScale.y / 2.0f);
            }
        }

        // if any ball goes out of the screen, game over
        for (int i = 0; i < ballCount; i++)
        {
            if (ballPositions[i].x < 0.0f)
            {
                gameOver = true;
                leftWin = false;
            }
            else if (ballPositions[i].x > SCREEN_WIDTH)
            {
                gameOver = true;
                leftWin = true;
            }
        }
    }

    // update bg color every FRAME_LIMIT frames
    if (gFrameCounter++ % FRAME_LIMIT == 0)
    {
        currentColorIndex = (currentColorIndex + 1) % (sizeof(BG_COLOURS) / sizeof(BG_COLOURS[0]));
    }
}

void drawGame()
{
    // draw top wall and bottom wall
    DrawRectangleV(
        (Vector2){topWall.x - (wallScale.x / 2.0f), topWall.y - (wallScale.y / 2.0f)},
        wallScale,
        DARKGRAY);
    DrawRectangleV(
        (Vector2){bottomWall.x - (wallScale.x / 2.0f), bottomWall.y - (wallScale.y / 2.0f)},
        wallScale,
        DARKGRAY);

    // draw left paddle and right paddle
    DrawRectangleV(
        (Vector2){leftPaddle.x - (paddleScale.x / 2.0f), leftPaddle.y - (paddleScale.y / 2.0f)},
        paddleScale,
        WHITE);

    DrawRectangleV(
        (Vector2){rightPaddle.x - (paddleScale.x / 2.0f), rightPaddle.y - (paddleScale.y / 2.0f)},
        paddleScale,
        WHITE);

    // draw ball
    for (int i = 0; i < ballCount; i++)
    {
        DrawRectangleV(
            (Vector2){ballPositions[i].x - (ballScale.x / 2.0f), ballPositions[i].y - (ballScale.y / 2.0f)},
            ballScale,
            WHITE);
    }

    // draw middle line
    for (int y = 0; y < SCREEN_HEIGHT; y += 40)
    {
        DrawRectangle(SCREEN_WIDTH / 2 - 5, y, 10, 20, LIGHTGRAY);
    }
}

void drawEnding()
{
    const char *title = "Game Over!";
    const char *content = leftWin ? "Left Player Wins!" : "Right Player Wins!";
    int titleFontSize = 60;
    int contentFontSize = 30;

    int titleWidth = MeasureText(title, titleFontSize);
    int contentWidth = MeasureText(content, contentFontSize);

    DrawText(title, ORIGIN.x - (titleWidth / 2), ORIGIN.y - 100, titleFontSize, WHITE);
    DrawText(content, ORIGIN.x - (contentWidth / 2), ORIGIN.y, contentFontSize, WHITE);
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex(BG_COLOURS[currentColorIndex]));

    if (!gameOver)
        drawGame();
    else
        drawEnding();

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