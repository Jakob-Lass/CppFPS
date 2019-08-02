#include <iostream>
#include <math.h>
#include <chrono>
using namespace std;
#include <tchar.h>

#include <Windows.h>
#include <vector>
#include <algorithm>

int nScreenWidth = 240;
int nScreenHeight = 100;

float fPlayerX = 10.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

float fFOV = 3.141592/4.0;
float fDepth = 16.0f; // view distance

int nMapHeight = 16;
int nMapWidth = 16;

char cCeiling = ' ';

char cFloor = '.';

char cWallShade(float fDepth, float fDistanceToWall)
{
    char returnChar = ' ';
    if      (fDistanceToWall <=fDepth/4.0){           returnChar = '#';}//0x2588;} // Very close
    else if (fDistanceToWall < fDepth/3.0){           returnChar = 'X';}//0x2593;} 
    else if (fDistanceToWall < fDepth/2.0){           returnChar = 'x';}//0x2592;} 
    else if (fDistanceToWall < fDepth/1.5){           returnChar = '=';}//0x2591;} // Very far
    else if (fDistanceToWall < fDepth/1.0){           returnChar = '~';}//0x2591;} // Very far
    return returnChar;
}

char cFloorShade(int y, int nScreenHeight)
{
    float b = 1.0f  - (((float)y - (float)nScreenHeight/2.0f ) / ((float)nScreenHeight/2.0f));

    char returnChar = ' ';
    if      (b <=0.15){           returnChar = '#';}//0x2588;} // Very close
    else if (b < 0.25){           returnChar = 'x';}//0x2593;} 
    else if (b < 0.50){           returnChar = '*';}//0x2593;} 
    else if (b < 0.75){           returnChar = '.';}//0x2592;} 
    else if (b < 0.90){           returnChar = '-';}//0x2592;} 
    
    return returnChar;
}


float fRotationSpeed = 0.8f; // Speed with which the player can rotate
float fWalkSpeed = 5.0f;

bool bCollission(wstring map, int nMapWidth, float fPlayerX, float fPlayerY)
{
    if(map[(int)fPlayerY*nMapWidth+(int)fPlayerX] == '#')
    {
        return true;
    }
    else
    {
        return false;
    }
    
}

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
    
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    chrono::duration<float> elapsedTime;
    float fElapsedTime = 0.0f;
    while(1) // game loop
    {
        // calculate game loop duration
        tp2 = chrono::system_clock::now();
        elapsedTime = tp2-tp1;
        tp1 = tp2;
        fElapsedTime = elapsedTime.count();


        // add controls
        // Handle rotation

        if (GetAsyncKeyState((unsigned short)'A') && 0x8000)
        {
            fPlayerA -= fRotationSpeed * fElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'D') && 0x8000)
        {
            fPlayerA += fRotationSpeed * fElapsedTime;
        }
        if (GetAsyncKeyState((unsigned short)'W') && 0x8000)
        {
            fPlayerX += sinf(fPlayerA)* fWalkSpeed * fElapsedTime;
            fPlayerY += cosf(fPlayerA)* fWalkSpeed * fElapsedTime;
            if (bCollission(map,nMapWidth,fPlayerX,fPlayerY))
            {
                fPlayerX -= sinf(fPlayerA)* fWalkSpeed * fElapsedTime;
                fPlayerY -= cosf(fPlayerA)* fWalkSpeed * fElapsedTime;
            }
        }
        if (GetAsyncKeyState((unsigned short)'S') && 0x8000)
        {
            fPlayerX -= sinf(fPlayerA)* fWalkSpeed * fElapsedTime;
            fPlayerY -= cosf(fPlayerA)* fWalkSpeed * fElapsedTime;
            if (bCollission(map,nMapWidth,fPlayerX,fPlayerY))
            {
                fPlayerX += sinf(fPlayerA)* fWalkSpeed * fElapsedTime;
                fPlayerY += cosf(fPlayerA)* fWalkSpeed * fElapsedTime;
            }
        }

        for(int x = 0; x < nScreenWidth; x++)
        {
            // Make a ray tracing for each column along the screen and find distance to closest wall
            float fRayAngle = (fPlayerA-fFOV/2.0)+((float)x/nScreenWidth)*fFOV;

            float fDistanceToWall = 0;
            bool bHitWall = false;
            bool bBoundary = false;

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
                        vector<pair<float, float>> p; // distance to coners, and dot product
                        for(int ty = 0; ty < 2; ty++)
                        {
                            for(int tx = 0; tx < 2; tx++)
                            {
                                float vy = (float)nTestY + ty - fPlayerY;
                                float vx = (float)nTestX + tx - fPlayerX;
                                float d = sqrt(vy*vy+vx*vx);
                                float dot = (fEyeX*vx/d)+(fEyeY*vy/d);
                                p.push_back(make_pair(d,dot));
                            }
                        }
                        sort(p.begin(), p.end(), [](const pair<float,float> &left, const pair<float,float> &right){return left.first < right.first;} );

                        float fBound = 0.002;
                        if(acos(p.at(0).second) < fBound) {bBoundary = true;}
                        if(acos(p.at(1).second) < fBound) {bBoundary = true;}
                        if(acos(p.at(2).second) < fBound) {bBoundary = true;}
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
                        screen[y*nScreenWidth+x] = cWallShade(fDepth,fDistanceToWall);

                        if (bBoundary)
                        {
                            screen[y*nScreenWidth+x] = ' ';
                        }
                    }
                    else
                    {
                        {
                            screen[y*nScreenWidth+x] = cFloorShade(y, nScreenHeight);
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