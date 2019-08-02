#include <iostream>
#include <math.h>
using namespace std;


#include <Windows.h>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

float fFOV = 3.141592/4.0;
float fDepth = 16.0f; // view distance

int nMapHeight = 16;
int nMapWidth = 16;

char cCeiling = ' ';
char cWall = '#';
char cFloor = '.';

int main()
{
    char *screen  = new char[nScreenWidth*nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);

    DWORD dwBytesWritten = 0;

    // generate map

    wstring map;
    
    map += L"################";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"#..............#";
    map += L"################";
    

    while(1) // game loop
    {
        // add controls
        // Handle rotation

        if (GetAsyncKeyState((unsigned short)'A') && 0x8000)
        {
            fPlayerA -= (0.1f);
        }
        if (GetAsyncKeyState((unsigned short)'D') && 0x8000)
        {
            fPlayerA += (0.1f);
        }

        for(int x = 0; x < nScreenWidth; x++)
        {
            // Make a ray tracing for each column along the screen and find distance to closest wall
            float fRayAngle = (fPlayerA-fFOV/2.0)+((float)x/nScreenWidth)*fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false;

            float fEyeX = sinf(fRayAngle); // unit vector along view
            float fEyeY = cosf(fRayAngle);

            while(!bHitWall && fDistanceToWall < fDepth)
            {
                fDistanceToWall += 0.1f; 

                // calculate ray position
                int nTestX = (int)(fPlayerX+fEyeX*fDistanceToWall);
                int nTestY = (int)(fPlayerY+fEyeY*fDistanceToWall);

                if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) // outside map
                {
                    bHitWall = true;
                    fDistanceToWall = fDepth;
                }
                else
                {
                    if (map[nTestX+nTestY*nMapWidth] == '#')
                    {
                        bHitWall = true;
                    }
                }
                int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float) fDistanceToWall);
                int nFloor = nScreenHeight - nCeiling;

                for (int y=0; y<nScreenHeight; y++)
                {
                    if (y<=nCeiling) // it is part of the ceiling
                    {
                        screen[y*nScreenWidth+x] = cCeiling;
                    }
                    else if (y>nCeiling && y <=  nFloor) // We are wall
                    {
                        screen[y*nScreenWidth+x] = cWall;
                    }
                    else
                    {
                        {
                            screen[y*nScreenWidth+x] = cFloor;
                        }
                    }
                    
                }
            }
        }
        screen[nScreenWidth*nScreenHeight -1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, {0, 0}, &dwBytesWritten);
        
    }

    

    return 0;
}