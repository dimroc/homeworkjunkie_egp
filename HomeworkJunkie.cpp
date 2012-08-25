#include "dxstdafx.h"
#include ".\homeworkjunkie.h"
#include "globaldefine.h"

CHomeworkJunkie::CHomeworkJunkie(IDirect3DDevice9* pd3dDevice, float screenWidth, float screenHeight) :
        m_background(pd3dDevice, g_cstrBackgroundTex, g_bgWidth, g_bgHeight, g_bgZFar, g_bgTexTile),
        m_splashscreen(pd3dDevice, g_cstrSplashTex, g_splashWidth, g_splashHeight, g_bgZFar, 1),
        m_vUp(0.0f, 1.0f, 0.0f),
        m_vAt(0.0f, 0.0f, 1.0f),
        m_vEye(0.0f, 0.0f, -10.0f),        
        m_movementSpeed(0.02f), m_zSpeed(40.0f),
        halfScreenWidth((int)screenWidth/2), halfScreenHeight((int)screenHeight/2),
        m_deltax(0), m_deltay(0),
        m_cZoomOut(0), m_nextlevel(1), m_levelPW(0), m_bSplash(true),
        m_gameentities(pd3dDevice),
        m_HUD()
{    
    D3DXMatrixIdentity(&m_mIdentity);    

    m_HUD.AddVisionPlane(D3DXVECTOR2(screenWidth, screenHeight), g_cstrVisionTex, 0, pd3dDevice);    
    m_HUD.AddStaticItem(D3DXVECTOR2(-0.57f, 0.32f), 0.1f, 0.2f, g_cstrLogo, pd3dDevice);
    //m_HUD.AddStaticItem(D3DXVECTOR2(0.24f, 0.27f), 0.15f, 0.3f, g_cstrControlsTex, pd3dDevice);

    ShowSplash();
}

CHomeworkJunkie::~CHomeworkJunkie(void)
{    
}

VOID CHomeworkJunkie::ShowSplash()
{
}

VOID CHomeworkJunkie::SetPassword()
{
    UINT start = 0x00000000;
    start |= ('h' << 24);
    start |= ('i' << 16);

    int level = m_nextlevel;
    start += level;
    int index = level % g_nOTPs;

    UINT pw = start ^ g_SBox[index];
    m_levelPW = pw;
}

BOOL CHomeworkJunkie::LoadPassword(UINT pw)
{
    for(int i=0; i< g_nOTPs; i++) 
    {
        UINT start = pw ^ g_SBox[i];
        char c1 = (UCHAR)((start & 0xff000000) >> 24);
        char c2 = (UCHAR)((start & 0x00ff0000) >> 16);
        if(c1 == 'h' && c2 == 'i') {
            m_nextlevel = start & 0x0000ffff;
            m_gameentities.ResetScore();
            StartLevel();            
            return true;
        }                            
    }
    return false;
}

//! Starts Game
VOID CHomeworkJunkie::StartLevel() 
{
    srand((UINT)time(NULL));
    m_gameentities.CleanWorld();
    m_gameentities.PopulateWorld(m_nextlevel);     
    PositionCamera(0.0f, 0.0f, (g_fMinZ - g_fMaxZ)/2); // Reposition camera at center    

    SetPassword();
    m_nextlevel++;
}


BOOL CHomeworkJunkie::OnFrameMove(float elapsedTime)
{
    bool bReturn = false;
    if(!m_bSplash) {
        int result = m_gameentities.OnFrameMove(m_vEye, m_vAt, elapsedTime);
        switch(result)
        {
        case CGameEntities::LEVEL_COMPLETE:
            StartLevel();
            break;
        case CGameEntities::LEVEL_CANCOMPLETE:
            bReturn = true;
            break;
        case CGameEntities::LEVEL_DEATH:
            m_nextlevel--;
            StartLevel();
            break;
        case CGameEntities::LEVEL_GAMEOVER:
            break;
        }
        ZoomEye(elapsedTime);
    }
    return bReturn;
}

HRESULT CHomeworkJunkie::Render(IDirect3DDevice9 *pd3dDevice, const D3DXMATRIX *pmPers, const D3DXMATRIX *pmOrtho)
{
    HRESULT hr;

    if(m_bSplash) {
        //
        // Render things that are in 3D
        //
        V( pd3dDevice->SetTransform(D3DTS_PROJECTION, pmPers) );    
        D3DXMATRIX mView;
        D3DXMatrixLookAtLH(&mView, &m_vEye, &m_vAt, &m_vUp);

        // Set up View. world is always identity for backkground.
        V( pd3dDevice->SetTransform( D3DTS_VIEW, &mView ) );    
        m_splashscreen.Render(pd3dDevice, false);
    }
    else {
        //
        // Render things that are in 3D
        //
        V( pd3dDevice->SetTransform(D3DTS_PROJECTION, pmPers) );    
        D3DXMATRIX mView;
        D3DXMatrixLookAtLH(&mView, &m_vEye, &m_vAt, &m_vUp);

        // Set up View. world is always identity for backkground.
        V( pd3dDevice->SetTransform( D3DTS_VIEW, &mView ) );    
        m_background.Render(pd3dDevice);

        // Render Entities (stick figures, houses, obstacles)
        //
        m_gameentities.Render(pd3dDevice);
        
        // Render 2D things in Orthogonal like HUD/GUI
        //        
        V( pd3dDevice->SetTransform( D3DTS_WORLD, &m_mIdentity ) );
        V( pd3dDevice->SetTransform( D3DTS_VIEW, &m_mIdentity ) );
        V( pd3dDevice->SetTransform(D3DTS_PROJECTION, pmPers) );
        m_HUD.Render(pd3dDevice);
    }

    return S_OK;
}

VOID CHomeworkJunkie::PositionCamera(float nx, float ny, float nz)
{
    m_vEye.x = m_vAt.x = nx;
    m_vEye.y = m_vAt.y = ny;
    m_vEye.z = nz;
    m_vAt.z = nz+1;
}

VOID CHomeworkJunkie::ZoomEye(float elapsedTime)
{   
    // Change Z if zooming
    if(m_cZoomOut == ZOOMING_OUT)
        m_vEye.z -= m_zSpeed * elapsedTime;
    else if(m_cZoomOut == ZOOMING_IN)
        m_vEye.z += m_zSpeed * elapsedTime;

    // Clamp z's
    if(m_vEye.z > g_fMaxZ)
        m_vEye.z = g_fMaxZ;
    else if(m_vEye.z < g_fMinZ)
        m_vEye.z = g_fMinZ;
    m_vAt.z = m_vEye.z+1;
}

VOID CHomeworkJunkie::MouseEvent(bool bLeftMB, int xpos, int ypos, bool bLMB, bool bRMB, int nWheelDelta)
{
    if(!m_bSplash) {
        if(bLMB) // start zoom out
            m_cZoomOut = ZOOMING_OUT;
        else if(bRMB) // start zoom in
            m_cZoomOut = ZOOMING_IN;
        else // stop zoom
            m_cZoomOut = ZOOMING_NO;

        // Set mouse look deltas
        m_deltax = halfScreenWidth - xpos;
        m_deltay = halfScreenHeight - ypos;

        // Have a window of no movement in center of screen
        if(abs(m_deltax) < g_MovementThreshX)
            m_deltax = 0;
        if(abs(m_deltay) < g_MovementThreshY)
            m_deltay = 0;
    }
}

VOID CHomeworkJunkie::KeyboardEvent(UINT nChar, bool bKeyDown)
{   
    if(!m_bSplash) {
        switch(nChar)
        {
        case 'W':
            m_gameentities.PlayerToggleForward(bKeyDown);
            break;
        case 'S':
            m_gameentities.PlayerToggleBackward(bKeyDown);
            break;        
        case 'A':
            m_gameentities.PlayerToggleLeft(bKeyDown);
            break;
        case 'D':
            m_gameentities.PlayerToggleRight(bKeyDown);
            break;
        default:
            break;
        }
    }
    else if(nChar == ' ') {
        StartLevel();
        m_bSplash = false;
    }
}

