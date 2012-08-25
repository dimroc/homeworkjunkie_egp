#pragma once

class CBackground
{
private:
    IDirect3DVertexBuffer9* m_pVB;
    LPDIRECT3DTEXTURE9 m_pTex;
public:
    CBackground(IDirect3DDevice9* pd3dDevice, LPCWSTR texFileName, float width, float height, float zfar, float textTile);
    ~CBackground();

    HRESULT Render(IDirect3DDevice9* pd3dDevice, bool bMipmap = true);
};
