#pragma once

#include "GlobalDefine.h"
#include "mp3player.h"

using std::list;
using std::vector;

class CGameEntities
{
private:
    static const UCHAR PLAYER_STOP           =   0x00;    
    static const UCHAR PLAYER_FORWARD        =   0x01;
    static const UCHAR PLAYER_BACK           =   0x02;    
    static const UCHAR PLAYER_LEFT           =   0x04;    
    static const UCHAR PLAYER_RIGHT          =   0x08;

    static const int m_ActorSpriteID         =  0;
    static const int m_ShelterSpriteID       =  1;
    static const int m_ObstacleSpriteID      =  2;
    static const int m_DecorationSpriteID    =  3;
    static const int m_PlayerSpriteID        =  4;
    static const int m_HighlighterCurID      =  5;

    /////////////////////////// STRUCTS /////////////////////////////
    struct SSprite {
        IDirect3DVertexBuffer9* m_pVB;
        vector<LPDIRECT3DTEXTURE9> m_vpTex;
        int nFrames;
        float animSpeed; //! animation speed

        SSprite(IDirect3DDevice9* pd3dDevice, float width, float height); // height and width of billboard
        ~SSprite();

        VOID AddTexture(IDirect3DDevice9* pd3dDevice, LPCWSTR texFileName);
    };

    //! Interface containing all Entity attributes. Inherited by other elements.
    struct IEntity { // polymorphism is unecessary
        int SpriteID; //! Which Group of Textures this Entity uses
        int nFrame; //! Which frame in the Texture Group it's in (sprites).
        float fScale; //! scale of entity!
        float x, y; //! position
        float radius; //! for collision detection
        float timeElapsed;
        float speed; //! movement speed
        D3DXMATRIX mWorld; //! Every Entity has it's own world mtx based on it's 2D position.    

        IEntity();
        ~IEntity();

        virtual VOID OnFrameMove(float elapsedTime);
        VOID UpdateWorldMatrix(bool bFlipHorizontal = false);
    };
    struct SDecoration : public IEntity {        
        SDecoration(float scale, float _x, float _y);
        ~SDecoration();
    };
    struct SActor : public IEntity { 
        UCHAR movement;
        bool bSheltered, bPlayer;
        float width, height;
        float lastTime;

        SActor();
        SActor(float scale, float x, float y, int level);
        ~SActor();
        virtual VOID OnFrameMove(float elapsedTime);
        VOID PerformAI(float elapsedTime, bool bSheltered, float playerx, float playery, float playerrad, int level);
        BOOL WithinSightRadius(float playerx, float playery, float playerrad, int level);
    };
    struct SShelter : public IEntity {
        double lastVisit;
        bool bBulldozed;
        SShelter();
        SShelter(float scale, float x, float y);
        ~SShelter();
    };
    struct SObstacle : public IEntity {
        float tx, ty;
        bool bConstructing, bLeftToRight, bDone;
        float lastTime;
        list<SShelter>::iterator itTarget;

        SObstacle(list<SShelter>::iterator itTarget, bool _bConstructing = false);        
        ~SObstacle();
        virtual VOID OnFrameMove(float elapsedTime, SActor &player, list<SShelter> &shelters, float &speedIncr);
    };

    bool m_bCameraMoveX, m_bCameraMoveY;
    float m_CameraStepX, m_CameraStepY;
    float m_lastSpawnTime;

    int m_level;
    float m_totalCarbonEaten; //! how many pencil ppl he's eaten since last death

    double m_TotalTime;

    vector<SSprite*> m_vSprites;

    list<SDecoration> m_lDecorations;
    list<SActor> m_lEnemies;
    list<SShelter> m_lShelters;
    list<SObstacle> m_lObstacles;

    SActor m_Player;
private:
    VOID DrawDecorations(IDirect3DDevice9* pd3dDevice);
    VOID DrawShelters(IDirect3DDevice9* pd3dDevice);
    VOID DrawObstacles(IDirect3DDevice9* pd3dDevice);
    VOID DrawEnemies(IDirect3DDevice9* pd3dDevice);
    VOID DrawPlayer(IDirect3DDevice9* pd3dDevice);

    VOID PlaceSpriteVBOnDevice(IDirect3DDevice9* pd3dDevice, int index);
    VOID PlaceSpriteInfoOnDevice(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX &mWorld, int SpriteIndex, int nFrame);

    VOID SpawnHouses(int level);
    VOID SpawnDecorations(int level);
    VOID SpawnEnemies(int level);
    VOID SpawnBulldozer(float elapsedTime);

    BOOL CheckPlayerCollision(); //! Checks if players eat trees(decorations) or eat/get eaten    
    BOOL IsOnBoard(float nx, float ny, float nr);

    VOID UpdateCamera(D3DXVECTOR3 &vEye, D3DXVECTOR3 &vAt, float speed, UCHAR movement, float elapsedTime);
    
    static BOOL WithinWidth(float x) { 
        static float halfWidth = g_bgWidth/2;
        return abs(x) < halfWidth; }

    static BOOL WithinHeight(float y) {
        static float halfHeight = g_bgHeight/2;
        return abs(y) < halfHeight; }


public:
    static const int LEVEL_COMPLETE     = 1;
    static const int LEVEL_DEATH        = 2;
    static const int LEVEL_GAMEOVER     = 3;
    static const int LEVEL_CANCOMPLETE  = 4;

    CGameEntities(IDirect3DDevice9* pd3dDevice);
    ~CGameEntities(void);

    VOID PopulateWorld(int level);
    VOID CleanWorld();    

    INT OnFrameMove(D3DXVECTOR3 &vEye, D3DXVECTOR3 &vAt, float elapsedTime);
    HRESULT Render(IDirect3DDevice9* pd3dDevice);

    float GetScore() { return (m_totalCarbonEaten)*g_ScoreDisplayMult; }
    VOID ResetScore() { m_totalCarbonEaten = 0.0f; }

    VOID PlayerToggleForward(bool bKeyDown);
    VOID PlayerToggleBackward(bool bKeyDown);
    VOID PlayerToggleLeft(bool bKeyDown);
    VOID PlayerToggleRight(bool bKeyDown);
};

BOOL IsCollision(float nx, float ny, float nr, float ox, float oy, float or);
