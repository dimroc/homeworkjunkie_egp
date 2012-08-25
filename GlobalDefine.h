#pragma once

struct BoxVertex {
    static const UINT FVF = D3DFVF_XYZ | D3DFVF_TEX1;

    D3DXVECTOR3 pos;    
    float u,v;

    BoxVertex() {}
    BoxVertex(D3DXVECTOR3 vec, float _u, float _v) :
        pos(vec), u(_u), v(_v)
    {} 
};

///////////////////////////// PASSWORD CONSTANTS ////////////////////

static const int g_nOTPs                  = 10;
static const UINT g_SBox[10]               = { 0x046b779f,
                                              0x04cd99ea,
                                              0x0dee4586,
                                              0x02deaf33,
                                              0x0cb565f5,
                                              0x1255fef2,
                                              0x09e3456a,
                                              0x0ff2f3ea,
                                              0x0da4693a,
                                              0x1329afff
                                             };

/////////////////////////// GAME CONSTANTS /////////////////////////////

static const int g_HousesStartLevel       =   30;
static const int g_DecorationsStartLevel  =   20;
static const int g_EnemiesStartLevel      =   20;
static const float g_EnemiesLevelMult     =   10.0f;

static const float g_EatMultiplier      =   0.1f;
static const float g_TreeEatMultiplier  =   0.35f;

static const int g_MaxScale             =   8;
static const int g_MaxEnemyAdjust       =   5;

static const float g_CurtesyRadius      =   8.0f;
static const float g_TreeRadius         =   8.0f;
static const float g_PlayerRadius       =   3.5f;
static const float g_ShelterRadius      =   10.0f;
static const float g_ObstacleRadius     =   4.0f;
static const float g_SightRadius        =   6.0f;
static const float g_SightRadIncr       =   0.2f;

static const float g_BulldozerIntervals =   10.0f;
static const float g_BulldozerLevelDiff =   2.0f;

static const float g_AIRefreshTime      =   2.0f;

static const int g_nSpawnRetries        =   20;

static const float g_ScoreDisplayMult   =   100.0f;

static const float g_CursorSizeMult     =   0.4f;

//////////////////////////// BACKGROUND STUFF /////////////////////////////

static const float g_bgWidth            =   120.0f;
static const float g_bgHeight           =   90.0f;
static const float g_bgZFar             =   10.0f;
static const float g_bgTexTile          =   1.0f;

static const float g_splashWidth        =   20.0f;
static const float g_splashHeight       =   15.0f;

//////////////////////////// MOVEMENT CONSTANTS //////////////////////////

static const float g_MovementThreshX      =   4.0f; // no movement window
static const float g_MovementThreshY      =   3.5f;
static const float g_MovementBuffer     =   0.1f; //! Camera buffer zone
static const float g_ZoomSpeed          =   10.0f; //! Camera Zoom speed

static const float g_fStartScale        =   0.05f; // starting scale for player
static const float g_fDefSpeed          =   3.5f; // default speed

static const float g_fSpeedIncr         =   0.02f; // Speed increase factor as a result of scale

static const float g_fAnimSpeed         =   0.3f; // sprite animation speed threshold

static const float g_zDist              =   g_bgZFar - 0.1f;

static const float g_fZReducer          =   0.005f;

static const float g_fMaxZ              =   -5.0f;
static const float g_fMinZ              =   -g_bgWidth;


/////////////////////////// TEXTURE FILENAMES /////////////////////////////

static const int g_nStickFigureFrames   =   4;
static LPCWSTR g_cstrStickFigureTex[4]  =   { L"Media/stick_figure_right.bmp",
                                              L"Media/stick_figure_still.bmp",
                                              L"Media/stick_figure_left.bmp",
                                              L"Media/stick_figure_eat.bmp" };

static LPCWSTR g_cstrPlayerTex[4]       =   { L"Media/stick_figure_right_blue.bmp",
                                              L"Media/stick_figure_still_blue.bmp",
                                              L"Media/stick_figure_left_blue.bmp",
                                              L"Media/stick_figure_eat_blue.bmp" };

static LPCWSTR g_cstrStickHouse1Tex     =   L"Media/stick_house1.bmp";

static const int g_nStickCarFrames      =   3;
static LPCWSTR g_cstrStickCarTex[3]     =   { L"Media/stick_bulldozer1.bmp",
                                              L"Media/stick_bulldozer2.bmp",
                                              L"Media/stick_bulldozer3.bmp" };

static LPCWSTR g_cstrStickDecoTex       =   L"Media/stick_deco_blue.bmp";

static LPCWSTR g_cstrSplashTex          =   L"Media/splash_inkoppression.png";
static LPCWSTR g_cstrBackgroundTex      =   L"Media/bg_homeworkjunkie.png";
static LPCWSTR g_cstrVisionTex          =   L"Media/impaired_vision2.bmp";
static LPCWSTR g_cstrLogo               =   L"Media/homeworkjunkie_logo2.bmp";

static LPCWSTR g_cstrHighCursor         =   L"Media/highlight_cursor.bmp";

/////////////////////////// THEME SONG ////////////////////////////////////

static LPCWSTR g_cstrThemeSong          =   L"Media/lexxus.mp3";

/////////////////////////// SOUND FX ////////////////////////////////////

static LPCWSTR g_cstrCrunchWav          =   L"Media/crunch.wav";
static LPCWSTR g_cstrGraaWav            =   L"Media/graa.wav";
static LPCWSTR g_cstrBullWav            =   L"Media/bulling.wav";

/////////////////////////// MATH STUFF ////////////////////////////////////

static const float OneOverRadical2      =   1/1.414214f;


