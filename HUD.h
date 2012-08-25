#pragma once

using std::vector;
using std::set;

class CHUD
{
private:
    class CStaticHUDItem {
    private:
        static const UINT FVF = D3DFVF_XYZ | D3DFVF_TEX1;

        D3DXVECTOR2 pos;
        float height, width;        
    public:
        IDirect3DVertexBuffer9* m_pVB;
        LPDIRECT3DTEXTURE9 m_pTex;

        CStaticHUDItem(D3DXVECTOR2 bottomleftPos, float height, float width, LPCWSTR texFileName, IDirect3DDevice9* pd3dDevice);
        ~CStaticHUDItem() { SAFE_RELEASE(m_pVB); SAFE_RELEASE(m_pTex); }
    };

    class CVisionPlane {
    private:
        static const UINT FVF = D3DFVF_XYZ | D3DFVF_TEX0;
        D3DXVECTOR2 pos;        
        int zOrder;
    public:
        IDirect3DVertexBuffer9* m_pVB;
        LPDIRECT3DTEXTURE9 m_pTex;

        CVisionPlane(D3DXVECTOR2 screenDim, LPCWSTR texFileName, int zOrder, IDirect3DDevice9* pd3dDevice);
        ~CVisionPlane() { SAFE_RELEASE(m_pVB); SAFE_RELEASE(m_pTex);  }
        int GetZOrder() const { return zOrder; }
    };
    class VisionPlanePointerFunctor {
    public:
        bool operator()(const CVisionPlane* lhs, const CVisionPlane* rhs ) const {
            return lhs->GetZOrder() < rhs->GetZOrder();
        }
    };
    vector<CStaticHUDItem*> m_vStaticItems;
    set<CVisionPlane*, VisionPlanePointerFunctor> m_sVisionPlane;

public:
    CHUD(void);    
    ~CHUD(void);

    //! Adds a static Item to HUD
    VOID AddStaticItem(D3DXVECTOR2 botleftPos, float height, float width, LPCWSTR texFileName, IDirect3DDevice9* pd3dDevice);

    //! Adds a vision plane immediately in front of the camera in the order zOrder with other Vision Planes
    BOOL AddVisionPlane(D3DXVECTOR2 centerPos, LPCWSTR texFileName, int zOrder, IDirect3DDevice9* pd3dDevice);
    HRESULT Render(IDirect3DDevice9* pd3dDevice);
};
