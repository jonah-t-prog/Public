//g++ Tetris.cpp -o Tetris.exe -I"C:/Users/Jonah/code/projects/raylib projects/Tetris/include" -L"C:/Users/Jonah/code/projects/raylib projects/Tetris/lib" -lraylib -lopengl32 -lgdi32 -lwinmm

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define square_size 20

#define grid_horizontal_size 12
#define grid_vertical_size 20

#define lateral_speed 10
#define turning_speed 12
#define fast_fall_await_counter 30

#define fading_time 33

typedef enum gridsquare {EMPTY, MOVING, FULL, BLOCK, FADING} gridsquare;

static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;

static gridsquare grid [grid_horizontal_size][grid_vertical_size];
static gridsquare piece [4][4];
static gridsquare incomingPiece [4][4];

static int piecePositionX = 0;
static int piecePositionY = 0;

static Color fadingColor;

static bool beginPlay = true;
static bool pieceActive = false;
static bool detection = false;
static bool lineToDelete = false;

static int level = 1;
static int lines = 0;

static int gravityMovementCounter = 0;
static int lateralMovementCounter = 0;
static int turnMovementCounter = 0;
static int fastFallMovementCounter = 0;

static int fadeLineCounter = 0;

static int gravitySpeed = 30;

static Texture2D backgroundTexture; 


static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);
static void DrawTexture(void); 

static bool Createpiece();
static void GetRandompiece();
static void ResolveFallingMovement(bool *detection, bool *pieceActive);
static bool ResolveLateralMovement();
static bool ResolveTurnMovement();
static void CheckDetection(bool *detection);
static void CheckCompletion(bool *lineToDelete);
static int DeleteCompleteLines();

int main()
{
    InitWindow(screenWidth, screenHeight, "classic game: tetris");

    Image image = LoadImage("E:/Code/c++/Tetris/ocean_life.jpg");
    backgroundTexture = LoadTextureFromImage(image);
    UnloadImage(image);
    
    InitGame();
    
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }


    UnloadGame();
    UnloadTexture(backgroundTexture);
    CloseWindow();

    return 0;
}

void InitGame(void)
{
    level = 1;
    lines = 0;

    fadingColor = BLACK;

    piecePositionX = 0;
    piecePositionY = 0;
    pause = false;

    beginPlay = true;
    pieceActive = false;
    detection = false;
    lineToDelete = false;

    gravityMovementCounter = 0;
    lateralMovementCounter = 0;
    turnMovementCounter = 0;
    fastFallMovementCounter = 0;

    fadeLineCounter = 0;
    gravitySpeed = 30;

    //Initialize grid matrices
    for (int i = 0; i < grid_horizontal_size; i++)
    {
            for (int j = 0 ; j < grid_vertical_size; j++) 
            {
                if ((j == grid_vertical_size - 1) || (i == 0) || (i == grid_horizontal_size - 1)) 
                {
                    grid[i][j] = BLOCK;
                }
                else grid[i][j] = EMPTY;
            }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            incomingPiece[i][j] = EMPTY;
        }
    }
}

void UpdateGame(void)
{
      if (!gameOver)
    {
        if (IsKeyPressed('P')) 
        {
            pause = !pause;
        }

        if (!pause)
        {
            if (!lineToDelete)
            {
                if (!pieceActive)
                {
                    CheckCompletion(&lineToDelete);

                    pieceActive = Createpiece();

                    fastFallMovementCounter = 0;

                }
                else
                {
                    fastFallMovementCounter++;
                    gravityMovementCounter++;
                    lateralMovementCounter++;
                    turnMovementCounter++;

                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) 
                    {
                        lateralMovementCounter = lateral_speed;
                    }

                    if (IsKeyPressed(KEY_UP)) 
                    {
                        turnMovementCounter = turning_speed;
                    }
                    //block gravity
                    if (IsKeyDown(KEY_DOWN) && (fastFallMovementCounter >= fast_fall_await_counter))
                    {
                        gravityMovementCounter += gravitySpeed;
                    }

                    if (gravityMovementCounter >= gravitySpeed)
                    {
                        CheckDetection(&detection);

                        ResolveFallingMovement(&detection, &pieceActive);

                        gravityMovementCounter = 0;
                    }

                    if (lateralMovementCounter >= lateral_speed)
                    {
                        if (!ResolveLateralMovement())
                        {
                            lateralMovementCounter = 0;
                        }
                    }

                    if (turnMovementCounter >= turning_speed)
                    {
                        if (ResolveTurnMovement())
                        {
                            turnMovementCounter = 0;
                        }
                    }
                }

                //Game over cal
                for (int j = 0; j < 2; j++)
                {
                    for (int i = 1; i < grid_horizontal_size - 1; i++) 
                    {
                        if (grid[i][j] == FULL)
                        {
                            gameOver = true;
                        }
                    }
                }
            }
            else
            {
                fadeLineCounter++;

                if (fadeLineCounter%8 < 4) 
                {
                    fadingColor = WHITE;
                }
                else
                {
                    fadingColor = GRAY;
                }

                if (fadeLineCounter >= fading_time)
                {
                    int deletedLines = 0;
                    deletedLines = DeleteCompleteLines();
                    fadeLineCounter = 0;
                    lineToDelete = false;

                    lines +=deletedLines;
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

static bool Createpiece()
{
    piecePositionX = (int)((grid_horizontal_size - 4)/2);
    piecePositionY = 0;

    if (beginPlay)
    {
        GetRandompiece();
        beginPlay = false;
    }
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            piece[i][j] = incomingPiece[i][j];
        }
    }
    GetRandompiece();
    for (int i = piecePositionX; i < piecePositionX + 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (piece[i - (int)piecePositionX][j] == MOVING)
            {
                grid[i][j] = MOVING;
            }
        }
    }
    return true;
}

static void GetRandompiece()
{
    int random = GetRandomValue(0, 6);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            incomingPiece[i][j] = EMPTY;
        }
    }
    switch (random)
    {
        case 0: { incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; } 
        break;    //Cube
        case 1: { incomingPiece[1][0] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; } 
        break;    //L
        case 2: { incomingPiece[1][2] = MOVING; incomingPiece[2][0] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[2][2] = MOVING; } 
        break;    //L inversa
        case 3: { incomingPiece[0][1] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[3][1] = MOVING; } 
        break;    //Recta
        case 4: { incomingPiece[1][0] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][1] = MOVING; } 
        break;    //Creu tallada
        case 5: { incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[2][2] = MOVING; incomingPiece[3][2] = MOVING; } 
        break;    //S
        case 6: { incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[3][1] = MOVING; } 
        break;    //S inversa
    }
}

void DrawGame(void)
{
    BeginDrawing();
        
       
        ClearBackground(GetColor(0x000639ff));
        Rectangle src = { 0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height };
        Rectangle dst = { 0, 0, (float)screenWidth, (float)screenHeight };
        DrawTexturePro(backgroundTexture, src, dst, Vector2{0, 0}, 0.0f, WHITE); 
        if (!gameOver)
        {
            Vector2 offset;
            offset.x = screenWidth/2 - (grid_horizontal_size*square_size/2) - 50;
            offset.y = screenHeight/2 - ((grid_vertical_size - 1)*square_size/2) + square_size*2;

            offset.y -= 50;

            int controller = offset.x;

            for (int j = 0; j < grid_vertical_size; j++)
            {
                for (int i = 0; i < grid_horizontal_size; i++)
                {
                    if (grid[i][j] == EMPTY)
                    {
                        DrawLine(offset.x, offset.y, offset.x + square_size, offset.y, DARKGRAY);
                        DrawLine(offset.x, offset.y, offset.x, offset.y + square_size, DARKGRAY);
                        DrawLine(offset.x + square_size, offset.y, offset.x + square_size, offset.y + square_size, DARKGRAY);
                        DrawLine(offset.x, offset.y + square_size, offset.x + square_size, offset.y + square_size, DARKGRAY);
                        offset.x += square_size;
                    }
                    else if (grid[i][j] == FULL)
                    {
                        DrawRectangle(offset.x, offset.y, square_size, square_size, BLACK);
                        offset.x += square_size;
                    }
                    else if (grid[i][j] == MOVING)
                    {
                        DrawRectangle(offset.x, offset.y, square_size, square_size, RAYWHITE);
                        offset.x += square_size;
                    }
                    else if (grid[i][j] == BLOCK) 
                    {
                        DrawRectangle(offset.x, offset.y, square_size, square_size, DARKGRAY);
                        offset.x += square_size;
                    }
                    else if (grid[i][j] == FADING)
                    {
                        DrawRectangle(offset.x, offset.y, square_size, square_size, fadingColor);
                        offset.x += square_size;
                    }
                }

                offset.x = controller;
                offset.y += square_size;
            }

            offset.x = 500;
            offset.y = 45;

            controller = offset.x;

            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (incomingPiece[i][j] == EMPTY)
                    {
                        DrawLine(offset.x, offset.y, offset.x + square_size, offset.y, DARKGRAY);
                        DrawLine(offset.x, offset.y, offset.x, offset.y + square_size, DARKGRAY);
                        DrawLine(offset.x + square_size, offset.y, offset.x + square_size, offset.y + square_size, DARKGRAY);
                        DrawLine(offset.x, offset.y + square_size, offset.x + square_size, offset.y + square_size, DARKGRAY);
                        offset.x += square_size;
                    }
                    else if (incomingPiece[i][j] == MOVING)
                    {
                    DrawRectangle(offset.x, offset.y, square_size, square_size, RAYWHITE);
                    offset.x += square_size;
                    }
                }
                offset.x = controller;
                offset.y += square_size;
            }
            DrawText("INCOMING: ", offset.x, offset.y - 100, 10, BLUE);
            DrawText(TextFormat("LINES:     %04i", lines), offset.x, offset.y + 20, 10, BLUE);

            if (pause)
            {
                DrawText("GAME PAUSED", screenWidth/2  - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, YELLOW);
            }
        }
        else 
        {
             DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, YELLOW);    
        }
    EndDrawing();
}



void UnloadGame(void)
{

}

void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

static void CheckDetection(bool *detection)
{
    for (int j = grid_vertical_size - 2; j >= 0; j--)
    {
        for (int i = 1; i < grid_horizontal_size - 1; i++)
        {
            if ((grid[i][j] == MOVING) && ((grid[i][j + 1] == FULL) || (grid[i][j + 1] == BLOCK)))
            {
                *detection = true;
            }
        }

    }
}

static void CheckCompletion(bool *lineToDelete)
{
    int calculator = 0;
    bool isEmptyRowFound = false;

    for (int j = grid_vertical_size - 2; j >= 0; j--)
    {
        calculator = 0;
        for (int i = 1; i < grid_horizontal_size - 1; i++)
        {
            if (grid[i][j] == FULL)
            {
                calculator++;
            }
        }
        if (calculator == grid_horizontal_size - 2)
        {
            *lineToDelete = true;
            calculator = 0;

            for (int z = 1; z < grid_horizontal_size - 1; z++)
            {
                grid[z][j] = FADING;
            }
        }
        else 
        {
            if (calculator == 0)
            {
                break;
            }
        }
    }
}

static int DeleteCompleteLines()
{
    int deletedLines = 0;

    for (int j = grid_vertical_size - 2; j > 0; j--)
    {
        bool isFadingRow = true;
        for (int i = 1; i < grid_horizontal_size - 1; i++)
        {
           if (grid[i][j] != FADING)
           {
           isFadingRow = false;
           break;
           }
        }

        if (isFadingRow)
        {
            for (int j2 = j; j2 > 0; j2--)
            {
                for (int i = 1; i < grid_horizontal_size - 1; i++)
                {
                    grid[i][j2] = grid[i][j2 - 1];
                }
            }
            for (int i = 1; i < grid_horizontal_size - 1; i++)
            {
                grid[i][0] = EMPTY;
            }
            deletedLines++;

            j++;
        }
           
    }
    return deletedLines;
}

static void ResolveFallingMovement(bool *detection, bool *pieceActive)
{
      if (*detection)
    {
        for (int j = grid_vertical_size - 2; j >= 0; j--)
        {
            for (int i = 1; i < grid_horizontal_size - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    grid[i][j] = FULL;
                    *detection = false;
                    *pieceActive = false;
                }
            }
        }
    }

    else
    {
        for (int j = grid_vertical_size - 2; j >= 0; j--)
        {
            for (int i = 1; i < grid_horizontal_size - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    grid[i][j+1] = MOVING;
                    grid[i][j] = EMPTY;
                }
            }
        }

        piecePositionY++;
    }
}

static bool ResolveLateralMovement()
{
     bool collision = false;

    if (IsKeyDown(KEY_LEFT))
    {
        for (int j = grid_vertical_size - 2; j >= 0; j--)
        {
            for (int i = 1; i < grid_horizontal_size - 1; i++)
            {
                if (grid[i][j] == MOVING) 
                {
                    if ((i-1 == 0) || (grid[i-1][j] == FULL) || (grid[i-1][j] == BLOCK))
                    {
                        collision = true;
                    }
                }
            }
        }
        if (!collision)
        {
            for (int j = grid_vertical_size - 2; j >= 0; j--)
            {
                for (int i = 1; i < grid_horizontal_size - 1; i++)
                {
                    if (grid[i][j] == MOVING)
                    {
                        grid[i-1][j] = MOVING;
                        grid[i][j] = EMPTY;
                    }
                }
            }
            piecePositionX--;    
        }
    }

    else if (IsKeyDown(KEY_RIGHT))
    {
        for (int j = grid_vertical_size - 2; j >= 0; j--)
        {
            for (int i = 1; i < grid_horizontal_size - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    if ((i+1 == grid_horizontal_size - 1) || (grid[i+1][j] == FULL) || (grid[i+1][j] == BLOCK))
                    {
                        collision = true;
                    }
                }
            }
        }
        if (!collision)
        {
            for (int j = grid_vertical_size - 2; j >= 0; j--)
            {
                for (int i = grid_horizontal_size - 1; i >= 1; i--)
                {
                    if (grid[i][j] == MOVING)
                    {
                        grid[i+1][j] = MOVING;
                        grid[i][j] = EMPTY;
                    }
                }
            }
            piecePositionX++;
        }
    }
    return collision;
}

static bool ResolveTurnMovement()
{
      if (IsKeyDown(KEY_UP))
    {
        gridsquare aux;
        bool checker = false;


        if ((grid[piecePositionX + 3][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY] != EMPTY) &&
            (grid[piecePositionX][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX + 3][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX][piecePositionY + 2] != MOVING)) checker = true;

        if ((grid[piecePositionX + 3][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX + 3][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY + 2] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY + 2] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY + 2] != MOVING)) checker = true;

        
        if (!checker)
        {
            gridsquare temp[4][4];
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    temp[j][3 - i] = piece[i][j];
                }
            }

            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    piece[i][j] = temp[i][j];
                }
            }
            
            /*aux = piece[0][0];
            piece[0][0] = piece[3][0];
            piece[3][0] = piece[3][3];
            piece[3][3] = piece[0][3];
            piece[0][3] = aux;

            aux = piece[1][0];
            piece[1][0] = piece[3][1];
            piece[3][1] = piece[2][3];
            piece[2][3] = piece[0][2];
            piece[0][2] = aux;

            aux = piece[2][0];
            piece[2][0] = piece[3][2];
            piece[3][2] = piece[1][3];
            piece[1][3] = piece[0][1];
            piece[0][2] = aux;

            aux = piece[1][1];
            piece[1][1] = piece[2][1];
            piece[2][1] = piece[2][2];
            piece[1][3] = piece[2][2];
            piece[2][2] = aux; */
        }
        for (int j = grid_vertical_size - 2; j >= 0; j--)
        {
            for (int i = 1; i < grid_horizontal_size - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    grid[i][j] = EMPTY;
                }
            }
        }
        for (int i = piecePositionX; i < piecePositionX + 4; i++)
        {
            for (int j = piecePositionY; j < piecePositionY + 4; j++)
            {
                if (piece[i - piecePositionX][j - piecePositionY] == MOVING)
                {
                    grid[i][j] = MOVING;
                }
            }
        }
        return true;
    }
    return false;
}

