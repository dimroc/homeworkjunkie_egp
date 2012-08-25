#include "dxstdafx.h"
#include ".\hud.h"
#include "GlobalDefine.h"

CHUD::CHUD(void)
{
}

CHUD::~CHUD(void)
{
    for(int i=0; i<(int)m_vStaticItems.size(); i++)
        SAFE_DELETE(m_vStaticItems[i]);

    set<CVisionPlane*, VisionPlanePointerFunctor>::iterator it;
    for(it = m_sVisionPlane.begin(); it != m_sVisionPlane.end(); it++)
        SAFE_DELETE(*it);
}

VOID CHUD::AddStaticItem(D3DXVECTOR2 botleftPos, float height, float width, LPCWSTR texFileName, IDirect3DDevice9* pd3dDevice)
{
    CStaticHUDItem* pHUDitem;
    pHUDitem = new CStaticHUDItem(botleftPos, height, width, texFileName, pd3dDevice);

    m_vStaticItems.push_back(pHUDitem);
}

BOOL CHUD::AddVisionPlane(D3DXVECTOR2 centerPos, LPCWSTR texFileName, int zOrder, IDirect3DDevice9* pd3dDevice)
{
    CVisionPlane* pVisionPlane;
    pVisionPlane = new CVisionPlane(centerPos, texFileName, zOrder, pd3dDevice);

    return m_sVisionPlane.insert(pVisionPlane).second;
}

HRESULT CHUD::Render(IDirect3DDevice9* pd3dDevice)
{
    HRESULT hr;

    V( pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) ); 
    // take alpha from alpha channel
    V( pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE) );
    V( pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1) );

    V( pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA) );
    V( pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA) );
    
    V( pd3dDevice->SetFVF(BoxVertex::FVF) );

    //
    // Render VisionPlanes first!
    //
    set<CVisionPlane*, VisionPlanePointerFunctor>::iterator it;
    for(it = m_sVisionPlane.begin(); it != m_sVisionPlane.end(); it++) {
        CVisionPlane* pvision = *it;
        V( pd3dDevice->SetTexture(0, pvision->m_pTex) );
        V( pd3dDevice->SetStreamSource(0, pvision->m_pVB, 0, sizeof(BoxVertex)) );
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }

    // Render STatic Items
    for(int i=0; i<(int)m_vStaticItems.size(); i++) {
        CStaticHUDItem* pitem = m_vStaticItems[i];
        V( pd3dDevice->SetTexture(0, pitem->m_pTex) );
        V( pd3dDevice->SetStreamSource(0, pitem->m_pVB, 0, sizeof(BoxVertex)) );
        V( pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2) );
    }
    return S_OK;
}


////////////////////////////// CStaticHUDItem Functions //////////////////////////
CHUD::CStaticHUDItem::CStaticHUDItem(D3DXVECTOR2 botleftPos, float _height, float _width, LPCWSTR texFileName, IDirect3DDevice9* pd3dDevice) : 
    m_pVB(NULL), m_pTex(NULL), height(_height), width(_width), pos(botleftPos)
{
    HRESULT hr;    
    // Create texture.
    V(D3DXCreateTextureFromFile(pd3dDevice, texFileName, &m_pTex) );
    // Create background polygon.

    int size  = 4*sizeof(BoxVertex);
    V( pd3dDevice->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, BoxVertex::FVF, D3DPOOL_MANAGED, &m_pVB, NULL) );

    // max tex coordinates
    float maxu = 1, maxv = 1;
    float zFar = 1;

    // Lock the buffer to gain access to the vertices 
    BoxVertex* pVertices;
    V( m_pVB->Lock(0, size, (VOID**)&pVertices, 0 ) );

    pVertices[0] = BoxVertex(D3DXVECTOR3(pos.x, pos.y, zFar), 0, maxv);
    pVertices[1] = BoxVertex(D3DXVECTOR3(pos.x, pos.y+height, zFar), 0, 0);    
    pVertices[2] = BoxVertex(D3DXVECTOR3(pos.x+width, pos.y, zFar), maxu, maxv);    
    pVertices[3] = BoxVertex(D3DXVECTOR3(pos.x+width, pos.y+height, zFar), maxu, 0);   

    V( m_pVB->Unlock() );
}

////////////////////////////// CVisionPlane Functions ///////////////////////////
CHUD::CVisionPlane::CVisionPlane(D3DXVECTOR2 screenDim, LPCWSTR texFileName, int _zOrder, IDirect3DDevice9* pd3dDevice) : 
    m_pVB(NULL), m_pTex(NULL), pos(screenDim), zOrder(_zOrder)
{
    HRESULT hr;    
    // Create texture.
    V(D3DXCreateTextureFromFile(pd3dDevice, texFileName, &m_pTex) );    

    int size  = 4*sizeof(BoxVertex);
    V( pd3dDevice->CreateVertexBuffer( size, D3DUSAGE_WRITEONLY, BoxVertex::FVF, D3DPOOL_MANAGED, &m_pVB, NULL) );

    // max tex coordinates
    float w = 0.7f, h = 0.5f;
    float maxu = 1, maxv = 1;
    float zFar = 1;

    // Lock the buffer to gain access to the vertices 
    BoxVertex* pVertices;
    V( m_pVB->Lock(0, size, (VOID**)&pVertices, 0 ) );

    pVertices[0] = BoxVertex(D3DXVECTOR3(-w, -h, zFar), 0, maxv);
    pVertices[1] = BoxVertex(D3DXVECTOR3(-w, h, zFar), 0, 0);    
    pVertices[2] = BoxVertex(D3DXVECTOR3(w, -h, zFar), maxu, maxv);    
    pVertices[3] = BoxVertex(D3DXVECTOR3(w, h, zFar), maxu, 0);

    V( m_pVB->Unlock() );
}
