#include "dxstdafx.h"
#include ".\gameentities.h"

CGameEntities::CGameEntities(IDirect3DDevice9* pd3dDevice) : 
    m_bCameraMoveX(false), m_bCameraMoveY(false), m_lastSpawnTime(0.0f), m_TotalTime(0.0f), m_totalCarbonEaten(0.0f)
{
    //
    // Add sprites to vector
    //
    // Actor
    SSprite *pSprite = new SSprite(pd3dDevice, 6, 10);
    for(int i=0; i<g_nStickFigureFrames; i++)
        pSprite->AddTexture(pd3dDevice, g_cstrStickFigureTex[i]);    
    m_vSprites.push_back(pSprite);

    // Shelters
    pSprite = new SSprite(pd3dDevice, 20, 20);
    pSprite->AddTexture(pd3dDevice, g_cstrStickHouse1Tex);
    m_vSprites.push_back(pSprite);

    //Obstacles
    pSprite = new SSprite(pd3dDevice, 10, 6);
    for(int i=0; i<g_nStickCarFrames; i++)
        pSprite->AddTexture(pd3dDevice, g_cstrStickCarTex[i]);
    m_vSprites.push_back(pSprite);

    // Decorations
    pSprite = new SSprite(pd3dDevice, 20, 20);
    pSprite->AddTexture(pd3dDevice, g_cstrStickDecoTex);
    m_vSprites.push_back(pSprite);

    // Blue Ink Player
    pSprite = new SSprite(pd3dDevice, 6, 10);
    for(int i=0; i<g_nStickFigureFrames; i++)
        pSprite->AddTexture(pd3dDevice, g_cstrPlayerTex[i]);    
    m_vSprites.push_back(pSprite);

    // Highlighter Cursor
    pSprite = new SSprite(pd3dDevice, 6, 6);    
    pSprite->AddTexture(pd3dDevice, g_cstrHighCursor);    
    m_vSprites.push_back(pSprite);
}

CGameEntities::~CGameEntities(void)
{
    for(int i=0; i<(int)m_vSprites.size(); i++)
        SAFE_DELETE(m_vSprites[i]);
}

//Cleans the whole world of everything for a new populate world
VOID CGameEntities::CleanWorld()
{
    m_lEnemies.clear();
    m_lShelters.clear();
    m_lDecorations.clear();
    m_lObstacles.clear();
}

//! Randomly populate board using level as difficulty
VOID CGameEntities::PopulateWorld(int level)
{
    m_level = level;
    m_TotalTime = 0.0f;
    m_lastSpawnTime = 0.0f;

    // Setup Starting Player
    SActor oldPlayer = m_Player;
    m_Player = SActor(g_fStartScale, 0.0f, 0.0f, level);
    m_Player.SpriteID = m_PlayerSpriteID;
    m_Player.bSheltered = true;
    m_Player.bPlayer = true;
    m_Player.movement = oldPlayer.movement;
    m_Player.UpdateWorldMatrix();
    
    // Spawn Random Houses
    SpawnHouses(level); // Also spawns starting home at 0,0.

    // Spawn Random Decoration
    SpawnDecorations(level);

    // Spawn Random Enemies
    SpawnEnemies(level);    
}

VOID CGameEntities::UpdateCamera(D3DXVECTOR3 &vEye, D3DXVECTOR3 &vAt, float speed, UCHAR movement, float elapsedTime)
{
    float diffx = fabs(m_Player.x - vEye.x);
    float diffy = fabs(m_Player.y - vEye.y);

    float zoomFactor = abs(vEye.z - g_fMaxZ)/abs(g_fMinZ);

    if( (movement & PLAYER_FORWARD && movement & PLAYER_RIGHT) ||
        (movement & PLAYER_FORWARD && movement & PLAYER_LEFT) ||
        (movement & PLAYER_BACK && movement & PLAYER_RIGHT) ||
        (movement & PLAYER_BACK && movement & PLAYER_LEFT) )
        speed *= OneOverRadical2;

    float step = (speed + zoomFactor * g_ZoomSpeed * g_fZReducer) * elapsedTime; // move faster when zoomed out
    
    if(diffx > g_MovementThreshX)
        m_bCameraMoveX = true;
    else if(diffx < g_MovementThreshX - g_MovementBuffer)
        m_bCameraMoveX = false;

    if(m_bCameraMoveX)
    {
        if(m_Player.x > vEye.x)
            m_CameraStepX = step;
        else
            m_CameraStepX = -step;

        vEye.x += m_CameraStepX;
        vAt = vEye;
        vAt.z = 10.0f;
    }
    if(diffy > g_MovementThreshY)
        m_bCameraMoveY = true;
    else if(diffy < g_MovementThreshY - g_MovementBuffer)
        m_bCameraMoveY = false;

    if(m_bCameraMoveY)
    {
        if(m_Player.y > vEye.y)
            m_CameraStepY = step;
        else
            m_CameraStepY = -step;

        vEye.y += m_CameraStepY;
        vAt = vEye;
        vAt.z = 10.0f;
    }
}

INT CGameEntities::OnFrameMove(D3DXVECTOR3 &vEye, D3DXVECTOR3 &vAt, float elapsedTime)
{
    m_TotalTime += elapsedTime;
    m_Player.OnFrameMove(elapsedTime);

    if(CheckPlayerCollision()) // Player Died!
        return LEVEL_DEATH;
    if(m_lEnemies.size() <= 0)
        return LEVEL_COMPLETE;

    float maxScale = -0.1f;
    list<SActor>::iterator itAct;
    for(itAct = m_lEnemies.begin(); itAct != m_lEnemies.end(); itAct++) {
        itAct->PerformAI(elapsedTime, m_Player.bSheltered, m_Player.x, m_Player.y, m_Player.radius, m_level - 1);
        itAct->OnFrameMove(elapsedTime);
        maxScale = (maxScale < itAct->fScale) ? itAct->fScale : maxScale;
    }

    SpawnBulldozer(elapsedTime);    // Spawns a Bulldozer if time has elapsed past g_Bulld..

    bool bDeleteObstacle = false;
    float speedIncr = 0.0f;
    list<SObstacle>::iterator itObs, itDelete;
    for(itObs = m_lObstacles.begin(); itObs != m_lObstacles.end(); itObs++) {
        itObs->OnFrameMove(elapsedTime, m_Player, m_lShelters, speedIncr);
        if(itObs->bDone) {
            itDelete = itObs;
            bDeleteObstacle = true;
        }
    }
    if(bDeleteObstacle) // stupid stl implementation segfaults when removing in loop above. and NO its not me.
        m_lObstacles.erase(itDelete);

    list<SShelter>::iterator itShe;
    for(itShe = m_lShelters.begin(); itShe != m_lShelters.end(); itShe++)
        itShe->OnFrameMove(elapsedTime);
    
    UpdateCamera(vEye, vAt, m_Player.speed + speedIncr, m_Player.movement, elapsedTime); // Update Camera/Eye based on player position

    if(m_Player.fScale > maxScale + g_fStartScale * 2)
        return LEVEL_CANCOMPLETE;
    return 0;
}
HRESULT CGameEntities::Render(IDirect3DDevice9* pd3dDevice)
{
    // Batch render by entity (batches by VB)
    DrawDecorations(pd3dDevice);
    DrawShelters(pd3dDevice);
    DrawObstacles(pd3dDevice);
    DrawEnemies(pd3dDevice);

    DrawPlayer(pd3dDevice);

    return S_OK;
}

////////////////////////////// Private CGameEntities Functions ////////////////

BOOL CGameEntities::CheckPlayerCollision()
{       
    // if player ever gets off board he dies!
    if(!WithinWidth(m_Player.x) || !WithinHeight(m_Player.y)) {
        m_totalCarbonEaten = 0.0f;        
        return true; // true is dead
    }
    list<SDecoration>::iterator itDeco;    
    for(itDeco = m_lDecorations.begin(); itDeco != m_lDecorations.end(); itDeco++)
    {   // check for eating of trees
        if(IsCollision(m_Player.x, m_Player.y, m_Player.radius, itDeco->x, itDeco->y, itDeco->radius)) {
            m_Player.fScale += itDeco->fScale * g_TreeEatMultiplier;
            m_Player.speed = g_fDefSpeed + g_fSpeedIncr*(m_Player.fScale - g_fStartScale)/g_fStartScale*m_level; // make speed increase with scale
            m_Player.radius = m_Player.fScale * g_PlayerRadius;
            m_lDecorations.erase(itDeco);
            m_Player.UpdateWorldMatrix();
            m_Player.nFrame = 3;            
            break;
        }
    }
    m_Player.bSheltered = false;
    list<SShelter>::iterator itHouse;
    for(itHouse = m_lShelters.begin(); itHouse != m_lShelters.end(); itHouse++)
    {   // check if sheltered
        if(IsCollision(m_Player.x, m_Player.y, m_Player.radius, itHouse->x, itHouse->y, itHouse->radius)) {
            if(m_Player.fScale - 0.15f <= itHouse->fScale) { // Only get sheltered if smaller than house
                m_Player.bSheltered = true;
                itHouse->lastVisit = m_TotalTime;
                break;
            }
        }
    }
    if(!m_Player.bSheltered) 
    {  // check if sheltered by bulldozer
        list<SObstacle>::iterator itObs;
        for(itObs = m_lObstacles.begin(); itObs != m_lObstacles.end(); itObs++)
        {
            if(IsCollision(m_Player.x, m_Player.y, m_Player.radius, itObs->x, itObs->y, itObs->radius*1.2f)) {
                m_Player.bSheltered = true;
                break;
            }
        }
    }

    if(m_Player.bSheltered)
        return false;
    else    // check for eating!
    {
        list<SActor>::iterator itAct;
        for(itAct = m_lEnemies.begin(); itAct != m_lEnemies.end(); itAct++) 
        {               
            if(IsCollision(m_Player.x, m_Player.y, m_Player.radius, itAct->x, itAct->y, itAct->radius*0.9f))
            { // return true if player scale is less(gets eaten)!! otherwise increase scale if player eats.
                if(m_Player.fScale < itAct->fScale * 0.9f) {
                    itAct->fScale += m_Player.fScale * g_EatMultiplier;
                    itAct->radius = itAct->fScale * g_PlayerRadius;
                    itAct->speed = g_fDefSpeed + g_fSpeedIncr*(itAct->fScale - g_fStartScale)/g_fStartScale; // make speed increase with scale
                    itAct->nFrame = 3;

                    m_totalCarbonEaten = 0.0f;
                    return true;
                }
                else {
                    m_totalCarbonEaten += itAct->fScale;
                    m_Player.fScale += itAct->fScale * g_EatMultiplier;
                    m_Player.radius = m_Player.fScale * g_PlayerRadius;
                    m_Player.speed = g_fDefSpeed + g_fSpeedIncr*(m_Player.fScale - g_fStartScale)/g_fStartScale*m_level; // make speed increase with scale
                    m_lEnemies.erase(itAct);
                    m_Player.UpdateWorldMatrix();
                    m_Player.nFrame = 3;
                    return false;
                }
            }            
        }
        return false;
    }
}

VOID CGameEntities::DrawEnemies(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;
    PlaceSpriteVBOnDevice(pd3dDevice, m_ActorSpriteID);
    list<SActor>::iterator it;
    for(it = m_lEnemies.begin(); it != m_lEnemies.end(); it++) {        
        PlaceSpriteInfoOnDevice(pd3dDevice, it->mWorld, it->SpriteID, it->nFrame);
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }
}

VOID CGameEntities::DrawObstacles(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;
    PlaceSpriteVBOnDevice(pd3dDevice, m_ObstacleSpriteID);
    V( pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE) );
    list<SObstacle>::iterator it;
    for(it = m_lObstacles.begin(); it != m_lObstacles.end(); it++) {
        PlaceSpriteInfoOnDevice(pd3dDevice, it->mWorld, it->SpriteID, it->nFrame);
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }
    V( pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW) );
}

VOID CGameEntities::DrawDecorations(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;
    PlaceSpriteVBOnDevice(pd3dDevice, m_DecorationSpriteID);
    list<SDecoration>::iterator it;
    for(it = m_lDecorations.begin(); it != m_lDecorations.end(); it++) {        
        PlaceSpriteInfoOnDevice(pd3dDevice, it->mWorld, it->SpriteID, it->nFrame);
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }
}

VOID CGameEntities::DrawShelters(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;
    PlaceSpriteVBOnDevice(pd3dDevice, m_ShelterSpriteID);
    list<SShelter>::iterator it;
    for(it = m_lShelters.begin(); it != m_lShelters.end(); it++) {        
        PlaceSpriteInfoOnDevice(pd3dDevice, it->mWorld, it->SpriteID, it->nFrame);
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }
}
VOID CGameEntities::DrawPlayer(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;
    //Draw HighLighter
    D3DXMATRIX mtxTrans, mtxScale;
    D3DXMatrixTranslation(&mtxTrans, m_Player.x, m_Player.y+m_Player.fScale*10 + 4.0f, g_zDist);
    D3DXMatrixScaling(&mtxScale, g_CursorSizeMult, g_CursorSizeMult, 0);
    mtxScale *= mtxTrans;
        
    PlaceSpriteVBOnDevice(pd3dDevice, m_HighlighterCurID);
    PlaceSpriteInfoOnDevice(pd3dDevice, mtxScale, m_HighlighterCurID, 0);
    V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );

    // Draw Player
    PlaceSpriteVBOnDevice(pd3dDevice, m_Player.SpriteID);
    PlaceSpriteInfoOnDevice(pd3dDevice, m_Player.mWorld, m_Player.SpriteID, m_Player.nFrame);
    V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
}

VOID CGameEntities::PlaceSpriteVBOnDevice(IDirect3DDevice9* pd3dDevice, int index)
{
    HRESULT hr;
    if(index >= (int)m_vSprites.size())
        index = (int)m_vSprites.size() - 1;

    V( pd3dDevice->SetStreamSource(0, m_vSprites[index]->m_pVB, 0, sizeof(BoxVertex)) );
}

VOID CGameEntities::PlaceSpriteInfoOnDevice(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX &mWorld, int SpriteIndex, int nFrame)
{
    HRESULT hr;
    V( pd3dDevice->SetTransform( D3DTS_WORLD, &mWorld) );

    if(SpriteIndex >= (int)m_vSprites.size())
        SpriteIndex = (int)m_vSprites.size() - 1;    
    if(nFrame >= (int)m_vSprites[SpriteIndex]->nFrames)
        nFrame = (int)m_vSprites[SpriteIndex]->nFrames - 1;

    LPDIRECT3DTEXTURE9 pTex = m_vSprites[SpriteIndex]->m_vpTex[nFrame];    
    V( pd3dDevice->SetTexture(0, pTex) );
}

//// Spawn Functions ////
BOOL CGameEntities::IsOnBoard(float nx, float ny, float nr) {
    return (WithinWidth(nx - nr) && WithinWidth(nx + nr) && WithinHeight(ny - nr) && WithinHeight(ny + nr));
}

VOID CGameEntities::SpawnHouses(int level)
{
    float maxWidth = g_bgWidth / 2;
    float maxHeight = g_bgHeight / 2;
    int totalWidth = (int)g_bgWidth;
    int totalHeight = (int)g_bgHeight;

    // Setup Starting House    
    m_lShelters.push_back(SShelter(g_fStartScale*2, 0, 0));
        
    bool bCurtesyNeighborDone = false;
    int NeighborCount = 0;
    
    for(int i=0; i<g_HousesStartLevel; i++) 
    {
        int count = 0; // used to check if spawning of houses failed!! It can if they are all huge and no room on BG!
        bool bAcceptable = false;
        while(!bAcceptable) 
        {
            if(bCurtesyNeighborDone == false) {
                count++;
                float posx = (rand() % totalWidth) - maxWidth;
                float posy = (rand() % totalHeight) - maxHeight;
                float scale = ( ((float)(rand() % g_MaxScale))/10.0f + g_fStartScale*2.0f) ;

                SShelter newHouse(scale, posx, posy);

                if(IsOnBoard(posx, posy, newHouse.radius) && fabs(posx) < g_CurtesyRadius && fabs(posy) < g_CurtesyRadius)
                {   // make sure position does not collide with existing house
                    bool bCollision = false;
                    list<SShelter>::iterator it;
                    for(it = m_lShelters.begin(); it != m_lShelters.end(); it++) {
                        if(IsCollision(newHouse.x, newHouse.y, newHouse.radius, it->x, it->y, it->radius))
                        {
                            bCollision = true;
                            break;
                        }
                    }
                    if(!bCollision) {                        
                        m_lShelters.push_back(newHouse);
                        bAcceptable = true;
                        NeighborCount++;
                        if(NeighborCount > 3)
                            bCurtesyNeighborDone = true;
                    }
                    else if(count > g_nSpawnRetries) {                    
                        m_lShelters.clear();
                        // Setup Starting House    
                        m_lShelters.push_back(SShelter(g_fStartScale, 0, 0));
                        i = 0;
                        count = 0;
                    }
                }// end of if IsOnBoard         
            }
            

            else {
                count++;
                float posx = (rand() % totalWidth) - maxWidth;
                float posy = (rand() % totalHeight) - maxHeight;
                float scale = ( ((float)(rand() % g_MaxScale))/10.0f + g_fStartScale*3.0f) ;

                SShelter newHouse(scale, posx, posy);

                if(IsOnBoard(posx, posy, newHouse.radius))
                {   // make sure position does not collide with existing house
                    bool bCollision = false;
                    list<SShelter>::iterator it;
                    for(it = m_lShelters.begin(); it != m_lShelters.end(); it++) {
                        if(IsCollision(newHouse.x, newHouse.y, newHouse.radius, it->x, it->y, it->radius))
                        {
                            bCollision = true;
                            break;
                        }
                    }
                    if(!bCollision) {
                        m_lShelters.push_back(newHouse);
                        bAcceptable = true;
                    }
                    else if(count > g_nSpawnRetries) {                    
                        m_lShelters.clear();
                        // Setup Starting House    
                        m_lShelters.push_back(SShelter(g_fStartScale, 0, 0));
                        i = 0;
                        count = 0;
                    }
                } // end of if IsOnBoard         
            }
        }
    }
}

VOID CGameEntities::SpawnDecorations(int level)
{
    float maxWidth = g_bgWidth / 2;
    float maxHeight = g_bgHeight / 2;
    int totalWidth = (int)g_bgWidth;
    int totalHeight = (int)g_bgHeight;
    
    int totalRespawnCount = 0;
    for(int i=0; i<g_DecorationsStartLevel; i++) {
        int count = 0;
        bool bAcceptable = false;
        while(!bAcceptable) 
        {
            float posx = (rand() % totalWidth) - maxWidth;
            float posy = (rand() % totalHeight) - maxHeight;
            float scale = ((float)(rand() % (g_MaxScale - 3))/10 + g_fStartScale*3.5f);
            SDecoration deco(scale, posx, posy);
            
            if(IsOnBoard(posx, posy, deco.radius) && fabs(posx) > g_TreeRadius && fabs(posy) > g_TreeRadius)
            { 
                bool bCollision = false;
                list<SShelter>::iterator it;
                for(it = m_lShelters.begin(); it != m_lShelters.end(); it++) {
                    if(IsCollision(deco.x, deco.y, deco.radius, it->x, it->y, it->radius))
                    {
                        bCollision = true;
                        break;
                    }
                }
                if(!bCollision) {
                    m_lDecorations.push_back(deco);
                    bAcceptable = true;
                }
                else if(count > g_nSpawnRetries) {                    
                    m_lDecorations.clear();
                    i = 0;
                    count = 0;
                    if(totalRespawnCount > 10) {
                        SpawnHouses(level);
                        totalRespawnCount = 0;
                    }
                    else    totalRespawnCount++;
                }
            }            
        }
    }
}

VOID CGameEntities::SpawnEnemies(int level)
{
    float maxWidth = g_bgWidth / 2;
    float maxHeight = g_bgHeight / 2;
    int totalWidth = (int)g_bgWidth;
    int totalHeight = (int)g_bgHeight;    
    int levelAdjust = level*2 - 1;    
    int levelHeightAdjust = min(levelAdjust, g_MaxEnemyAdjust);

    for(int i=0; i<g_EnemiesStartLevel + min(levelAdjust, 15)*g_EnemiesLevelMult; i++) {
        bool bAcceptable = false;
        while(!bAcceptable) 
        {
            float posx = (rand() % totalWidth) - maxWidth;
            float posy = (rand() % totalHeight) - maxHeight;
            // TODO: Pick Scale better to ensure player can actually win and you have nice sizes
            float scale1 = ((float)(rand() % (g_MaxScale + levelHeightAdjust))/10 + g_fStartScale);
            float scale2 = ((float)(rand() % (g_MaxScale + levelHeightAdjust))/10 + g_fStartScale);            
            float scale = (scale1<scale2) ? scale1 : scale2;

            SActor act(scale, posx, posy, level);
            float curtesy = max(1.0f, g_CurtesyRadius - float(level)/2.0f);
            if(IsOnBoard(posx, posy, act.radius) && fabs(posx) > curtesy && fabs(posy) > curtesy) 
            {   //Make Sure you spawn a certain curtesy distance from center
                m_lEnemies.push_back(act);
                bAcceptable = true;
           }
        }
    }
}

VOID CGameEntities::SpawnBulldozer(float elapsedTime)
{   // Check if time to spawn a bulldozer!
    m_lastSpawnTime += elapsedTime;    
    if(m_lastSpawnTime > g_BulldozerIntervals - ((m_level-1)*g_BulldozerLevelDiff))
    {   // Spawn bulldozer at last visited house that's not home!
        list<SShelter>::iterator itTarget;        
        bool bFound1 = false;
        double lastVisit = 0.0f;

        list<SShelter>::iterator itHouse = m_lShelters.begin();
        itHouse++; // never target home. Home is first entry always
        for( ; itHouse != m_lShelters.end(); itHouse++)
        {
            if(itHouse->bBulldozed == false && lastVisit < itHouse->lastVisit) {
                lastVisit = itHouse->lastVisit;
                bFound1 = true;                
                itTarget = itHouse;
            }
        }
        if(bFound1)
        {   // Spawn the obstacle only if there isnt another bulldozer coming in opposite direction
            bool bCollision = false;
            list<SObstacle>::iterator itBull;
            for(itBull = m_lObstacles.begin(); itBull != m_lObstacles.end(); itBull++)
            {
                if(itBull->y - itBull->radius < itTarget->y && itBull->y + itBull->radius > itTarget->y) {
                    bCollision = true;
                    break;
                }
            }
            if(!bCollision) {
                itTarget->bBulldozed = true;
                SObstacle obs(itTarget);            
                m_lObstacles.push_back(obs);
            }
        }
        m_lastSpawnTime = 0.0f;
    }       
}

////////////////////////////// Player Functions ///////////////////////////////

VOID CGameEntities::PlayerToggleForward(bool bStart) {
    if(bStart)
        m_Player.movement |= PLAYER_FORWARD;
    else
        m_Player.movement ^= PLAYER_FORWARD;
}

VOID CGameEntities::PlayerToggleBackward(bool bStart) {
    if(bStart)
        m_Player.movement |= PLAYER_BACK;
    else
        m_Player.movement ^= PLAYER_BACK;
}

VOID CGameEntities::PlayerToggleLeft(bool bStart) {
    if(bStart)
        m_Player.movement |= PLAYER_LEFT;
    else
        m_Player.movement ^= PLAYER_LEFT;
}

VOID CGameEntities::PlayerToggleRight(bool bStart) {
    if(bStart)
        m_Player.movement |= PLAYER_RIGHT;
    else
        m_Player.movement ^= PLAYER_RIGHT;
}

////////////////////////////// SSPrite Functions ////////////////////////////
CGameEntities::SSprite::SSprite(IDirect3DDevice9* pd3dDevice, float width, float height) : 
    m_pVB(NULL), nFrames(0), animSpeed(1.0f)
{
    HRESULT hr;
    // Make new VB
    int size  = 4*sizeof(BoxVertex);
    V( pd3dDevice->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, BoxVertex::FVF, D3DPOOL_MANAGED, &m_pVB, NULL) );

    // max tex coordinates
    float maxu = 1, maxv = 1;    

    // Lock the buffer to gain access to the vertices 
    BoxVertex* pVertices;
    V( m_pVB->Lock(0, size, (VOID**)&pVertices, 0 ) );

    pVertices[0] = BoxVertex(D3DXVECTOR3(-width/2, -height/2, 0), 0, maxv);
    pVertices[1] = BoxVertex(D3DXVECTOR3(-width/2, height/2, 0), 0, 0);    
    pVertices[2] = BoxVertex(D3DXVECTOR3(width/2, -height/2, 0), maxu, maxv);    
    pVertices[3] = BoxVertex(D3DXVECTOR3(width/2, height/2, 0), maxu, 0);

    V( m_pVB->Unlock() );
}

CGameEntities::SSprite::~SSprite()
{
    SAFE_RELEASE(m_pVB);
    for(int i=0; i<(int)m_vpTex.size(); i++)
        SAFE_RELEASE(m_vpTex[i]);
}

VOID CGameEntities::SSprite::AddTexture(IDirect3DDevice9* pd3dDevice, LPCWSTR texFileName)
{
    HRESULT hr;
    // Create texture.
    LPDIRECT3DTEXTURE9 pTex;
    V(D3DXCreateTextureFromFile(pd3dDevice, texFileName, &pTex) );

    m_vpTex.push_back(pTex);
    nFrames++;
}

////////////////////////////// CEntities Functions ////////////////////////////

CGameEntities::IEntity::IEntity() : 
    timeElapsed(0.0f), speed(g_fDefSpeed)
{
    D3DXMatrixIdentity(&mWorld);
}

CGameEntities::IEntity::~IEntity()
{
}

VOID CGameEntities::IEntity::OnFrameMove(float elapsedTime)
{
}

VOID CGameEntities::IEntity::UpdateWorldMatrix(bool bFlipHorizontal)
{
    D3DXMATRIX mtxScale, mtxTranslate;
    D3DXMatrixScaling(&mtxScale, fScale, fScale, 0);
    if(bFlipHorizontal) {
        D3DXMATRIX mtxFlip;
        D3DXMatrixScaling(&mtxFlip, -1, 1, 1);        
        mtxScale *= mtxFlip;
    }
    D3DXMatrixTranslation(&mtxTranslate, x, y, g_zDist);
    mWorld = mtxScale * mtxTranslate;
}

////////////////// Actor ////////////////
CGameEntities::SActor::SActor() : 
    movement(CGameEntities::PLAYER_STOP), bSheltered(false), bPlayer(false), lastTime(0)
{
    SpriteID = m_ActorSpriteID;
    nFrame = 1;
    fScale = g_fStartScale;
    x = 0; y = 0;
    speed = g_fDefSpeed; // make speed increase with scale
    radius = fScale * g_PlayerRadius;    
    UpdateWorldMatrix();
}

CGameEntities::SActor::SActor(float scale, float _x, float _y, int level) : 
    movement(CGameEntities::PLAYER_STOP), bSheltered(false), bPlayer(false), lastTime(0)
{
    SpriteID = m_ActorSpriteID;
    nFrame = 1;
    fScale = scale;
    x = _x;
    y = _y;
    speed = g_fDefSpeed + g_fSpeedIncr*(scale - g_fStartScale)/g_fStartScale; // make speed increase with scale
    radius = fScale * g_PlayerRadius;
    UpdateWorldMatrix();
}

CGameEntities::SActor::~SActor()
{
}

VOID CGameEntities::SActor::OnFrameMove(float elapsedTime)
{
    using CGameEntities;

    timeElapsed += elapsedTime;
    float movementStep = speed * elapsedTime;
    bool bAnimate = true;    

    if(movement == PLAYER_STOP) {
        if(timeElapsed > g_fAnimSpeed) {
            nFrame = 1;
            timeElapsed = 0;
        }
        bAnimate = false;
    }
    else {
        float movey = 0.0f, movex = 0.0f;
        if(movement & PLAYER_FORWARD)        movey += movementStep;
        if(movement & PLAYER_BACK)           movey -= movementStep;
        if(movement & PLAYER_LEFT)           movex -= movementStep;
        if(movement & PLAYER_RIGHT)          movex += movementStep;
        
        if(movex > 0.0f && !CGameEntities::WithinWidth(x+movex+radius))     movex = 0.0f;
        else if(movex < 0.0f && !CGameEntities::WithinWidth(x+movex-radius))     movex = 0.0f;

        if(movey > 0.0f && !CGameEntities::WithinHeight(y+movey+radius))     movey = 0.0f;
        else if(movey < 0.0f && !CGameEntities::WithinHeight(y+movey-radius))     movey = 0.0f;        

        if(fabs(movey) > 0.0f && fabs(movex) > 0.0f) {
            movey *= OneOverRadical2;
            movex *= OneOverRadical2;
        }
        x += movex;
        y += movey;

        UpdateWorldMatrix();
    }
    if(bAnimate) {
        if(timeElapsed > g_fAnimSpeed) {
            nFrame = (nFrame == 2) ? 0 : 2;
            timeElapsed = 0;
        }
    }
}

BOOL CGameEntities::SActor::WithinSightRadius(float _x, float _y, float rad, int levelMult)
{
    float diffx = x - _x;
    float diffy = y - _y;
    float magnitude = sqrt((diffx*diffx + diffy*diffy));
    if(magnitude == 0) // same point!
        return true; 
    float normalx = diffx / magnitude;
    float normaly = diffy / magnitude;

    float stepmag = radius + rad + g_SightRadius + levelMult*g_SightRadIncr;
    float stepx = normalx * stepmag;
    float stepy = normaly * stepmag;

    float tx = (stepx == 0) ? 0 : diffx / stepx;
    float ty = (stepy == 0) ? 0 : diffy / stepy;

    return (tx <= 1 && ty <= 1);
}

VOID CGameEntities::SActor::PerformAI(float elapsedTime, bool bSheltered, float playerx, float playery, float playerrad, int level)
{
    using CGameEntities;
    lastTime += elapsedTime;
    if(!bSheltered && WithinSightRadius(playerx, playery, playerrad, level))
    {  // Check if player is not sheltered and in Enemy sight radius
        if(x < playerx) movement = PLAYER_RIGHT;
        else if(x > playerx) movement = PLAYER_LEFT;
        if(y < playery) movement |= PLAYER_FORWARD;
        else if(y > playery) movement |= PLAYER_BACK;
    }
    else // stroll around at some speed
    {        
        if(lastTime > g_AIRefreshTime)
        {   // perform new random move
            movement = (UCHAR)(rand() % 16);
            lastTime = 0.0f;
        }
    }
}

////////////////// Shelter ////////////////
CGameEntities::SShelter::SShelter() : lastVisit(0.0f), bBulldozed(false)
{
    SpriteID = m_ShelterSpriteID;
    nFrame = 0;
    fScale = g_fStartScale;    
    radius = fScale * g_ShelterRadius;
    x = 0; y = 0;
    UpdateWorldMatrix();
}

CGameEntities::SShelter::SShelter(float scale, float _x, float _y) : lastVisit(0.0f), bBulldozed(false)
{
    SpriteID = m_ShelterSpriteID;
    nFrame = 0;
    fScale = scale;
    x = _x;
    y = _y;
    radius = scale * g_ShelterRadius;
    UpdateWorldMatrix();
}

CGameEntities::SShelter::~SShelter()
{
}

////////////////// Obstacle ////////////////

CGameEntities::SObstacle::SObstacle(list<SShelter>::iterator it, bool _bConstructing) :
    lastTime(0.0f), bDone(false), itTarget(it)
{
    SpriteID = m_ObstacleSpriteID;
    nFrame = 0;
    bConstructing = _bConstructing;

    fScale = it->fScale * 2;
    radius = fScale * g_ObstacleRadius;
    speed = g_fDefSpeed * 3;
    
    if(it->x < 0) {
        bLeftToRight = true;
        x = -g_bgWidth/2 - 20.0f;
    }
    else {
        bLeftToRight = false;
        x = g_bgWidth/2 + 20.0f;
    }
    y = it->y;
    tx = it->x; ty = it->y;

    UpdateWorldMatrix(!bLeftToRight);
}

CGameEntities::SObstacle::~SObstacle()
{
}
VOID CGameEntities::SObstacle::OnFrameMove(float elapsedTime, SActor &player, list<SShelter> &shelters, float &speedIncr)
{
    lastTime += elapsedTime;
    float movementStep = speed * elapsedTime;

    // Move bulldozer
    if(bLeftToRight) {
        x += movementStep;
        UpdateWorldMatrix();
    }
    else {
        x -= movementStep;
        UpdateWorldMatrix(true);
    }

    // Move player if next to bulldozer
    if(IsCollision(player.x, player.y, player.radius, x, y, radius) && fScale*4 > player.fScale)
    {
        if(bLeftToRight)
            player.x += movementStep;
        else if(!bLeftToRight)
            player.x -= movementStep;
        else 
            speedIncr = 0;
        if(player.movement & CGameEntities::PLAYER_LEFT)
            player.x += player.speed*elapsedTime;
        if(player.movement & CGameEntities::PLAYER_RIGHT)
            player.x -= player.speed*elapsedTime;
        speedIncr = speed - player.speed;
        player.UpdateWorldMatrix();        
    }

    // move house being bulldozed
    if(IsCollision(itTarget->x, itTarget->y, itTarget->radius, x, y, radius))
    {
        if(bLeftToRight)
            itTarget->x += movementStep;
        else if(!bLeftToRight)
            itTarget->x -= movementStep;
        itTarget->UpdateWorldMatrix();
    }

    if(bLeftToRight && x > g_bgWidth/2 + 20.0f) {
        bDone = true; // off board so mark for deletion
        shelters.erase(itTarget);
    }
    else if(!bLeftToRight && x < -g_bgWidth/2 - 20.0f) {
        bDone = true;
        shelters.erase(itTarget);
    }

    if(lastTime > g_fAnimSpeed) {
        nFrame++;
        if(nFrame >= g_nStickCarFrames)
            nFrame = 0;
        lastTime = 0.0f;
    }
}

////////////////// Decorations ////////////////
CGameEntities::SDecoration::SDecoration(float scale, float _x, float _y)
{
    SpriteID = m_DecorationSpriteID;
    nFrame = 0;    
    fScale = scale;   
    radius = scale * g_ShelterRadius;
    x = _x; y = _y;
    UpdateWorldMatrix();
}
CGameEntities::SDecoration::~SDecoration()
{
}

////////////////////////// COLLISION FUNCTION ///////////////////////

//! Radiuses better be positive
BOOL IsCollision(float nx, float ny, float nr, float ox, float oy, float or)
{ 
    float diffx = nx - ox;
    float diffy = ny - oy;
    float magnitude = sqrt((diffx*diffx + diffy*diffy));
    if(magnitude == 0) // same point!
        return true; 
    float normalx = diffx / magnitude;
    float normaly = diffy / magnitude;

    float stepmag = nr + or;
    float stepx = normalx * stepmag;
    float stepy = normaly * stepmag;

    float tx = (stepx == 0) ? 0 : diffx / stepx;
    float ty = (stepy == 0) ? 0 : diffy / stepy;

    return (tx <= 1 && ty <= 1);
}
