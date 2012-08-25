#pragma once

#include "Background.h"
#include "HUD.h"
#include "GameEntities.h"

#define ZOOMING_OUT     0x01
#define ZOOMING_IN      0x10
#define ZOOMING_NO      0x00

//! The game!
class CHomeworkJunkie
{
private:
    CBackground m_background, m_splashscreen; // Background Object
    CHUD m_HUD; // Heads up Display Object  
    CGameEntities m_gameentities;

    D3DXVECTOR3 m_vEye, m_vUp, m_vAt;   // vectors for View Transformation
    D3DXMATRIX m_mIdentity;
    
    int halfScreenWidth, halfScreenHeight; //! cetner of window
    int m_deltax, m_deltay; //! The distance between center of window and mouse position
    float m_movementSpeed, m_zSpeed;    //! mouse move speed and zooming speed

    int m_BoundaryZoomX, m_BoundaryZoomY;   //! Eye looking boundary

    UCHAR m_cZoomOut;   //! Whether user is holding zoomout/in key.

    int m_nextlevel;        //! The next level to play.    
    UINT m_levelPW;

    bool m_bSplash;           //! At Splash Screen.

private:

    VOID ZoomEye(float elapsedTime);  //! Moves eye around based on mouse/keyboard and elapsed time
    VOID PositionCamera(float nx, float ny, float nz);
    VOID StartLevel(); //! Starts the level!
    VOID ShowSplash();

public:
    CHomeworkJunkie(IDirect3DDevice9* pd3dDevice, float screenWidth, float screenHeight);
    ~CHomeworkJunkie(void);   

    BOOL OnFrameMove(float fElapsedTime);
    HRESULT Render(IDirect3DDevice9* pd3dDevice, const D3DXMATRIX *pmPers, const D3DXMATRIX *pmOrtho);

    VOID MouseEvent(bool bLeftMB, int xpos, int ypos, bool bLMB, bool bRMB, int nWheelDelta);
    VOID KeyboardEvent(UINT nChar, bool bKeyDown);

    VOID LoadNextLevel() { StartLevel(); }

    int GetLevel() { return m_nextlevel - 1; } //! Gets the current level number
    float GetScore() { return m_gameentities.GetScore(); } //! Gets the current Score (player scale + amount of carbon eaten!)
    UINT GetPassword() { return m_levelPW; } //! Returns a password based on level
    VOID SetPassword();
    BOOL LoadPassword(UINT pw);
};
