#define UNICODE

#include "graphics.h"
#include "common.h"
#include "config.h"
#include "sider.h"
#include "utf8.h"

#include "d3d11.h"
//#include "d3dcompiler.h"
#include "FW1FontWrapper.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#define SAFE_RELEASE(x) if (x) { x->Release(); x = NULL; }
#define DBG(n) if (_config->_debug & n)

extern config_t*_config;
extern dx11_t DX11;
extern ID3D11BlendState* g_pBlendState;
extern ID3D11InputLayout* g_pTexInputLayout;
extern ID3D11VertexShader* g_pTexVertexShader;
extern ID3D11PixelShader* g_pTexPixelShader;
extern ID3D11SamplerState *g_pSamplerLinear;
extern bool _in_on_frame;


static bool load_image(const char *image_path, ID3D11Resource **ppTexture, ID3D11ShaderResourceView **ppTextureView)
{
    HRESULT hr;
    wchar_t *ws = Utf8::utf8ToUnicode(image_path);
    if (memcmp(".dds", image_path+strlen(image_path)-4, 4)==0) {
        hr = DirectX::CreateDDSTextureFromFile(DX11.Device, ws, ppTexture, ppTextureView);
    }
    else {
        // try other supported formats
        hr = DirectX::CreateWICTextureFromFile(DX11.Device, ws, ppTexture, ppTextureView);
    }
    Utf8::free(ws);
    if (SUCCEEDED(hr)) {
        DBG(2<<17) logu_("Loaded 2D texture: {%s}\n", image_path);
    }
    else {
        logu_("PROBLEM: Cannot load texture from: {%s}\n", image_path);
        return false;
    }
    return true;
}

static bool get_image_dimensions(ID3D11Resource *pTexture, int *width, int *height)
{
    D3D11_RESOURCE_DIMENSION resType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    pTexture->GetType( &resType );

    switch( resType ) {
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                ID3D11Texture2D* tex = (ID3D11Texture2D*)pTexture;
                D3D11_TEXTURE2D_DESC desc;
                tex->GetDesc(&desc);

                // This is a 2D texture. Check values of desc here
                DBG(2>>17) logu_("get_image_dimenstions: texture Width: %d\n", desc.Width);
                DBG(2>>17) logu_("get_image_dimenstions: texture Height: %d\n", desc.Height);
                DBG(2>>17) logu_("get_image_dimenstions: texture MipLevels: %d\n", desc.MipLevels);
                DBG(2>>17) logu_("get_image_dimenstions: texture ArraySize: %d\n", desc.ArraySize);
                DBG(2>>17) logu_("get_image_dimenstions: texture Format: %d\n", desc.Format);
                *width = desc.Width;
                *height = desc.Height;
                return true;
            }
        default:
            logu_("PROBLEM: Not a 2D texture\n");
    }
    return false;
}

int gfx_image(lua_State *L) {
    if (!_in_on_frame) {
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "image error: cannot be called outside of display_frame handler");
        return lua_error(L);
    }

    lua_pop(L, lua_gettop(L));
    return 0;
}

int gfx_sprite(lua_State *L) {
    if (!_in_on_frame) {
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "sprite error: cannot be called outside of display_frame handler");
        return lua_error(L);
    }

    int x = 0;
    if (lua_isnumber(L, 2)) {
        x = luaL_checkint(L, 2);
    }

    int y = 0;
    if (lua_isnumber(L, 3)) {
        y = luaL_checkint(L, 3);
    }

    float image_alpha = 1.0f;
    if (lua_isnumber(L, 4)) {
        image_alpha = lua_tonumber(L, 4);
    }

    int width = 0;
    if (lua_isnumber(L, 5)) {
        width = luaL_checkint(L, 5);
    }

    int height = 0;
    if (lua_isnumber(L, 6)) {
        height = luaL_checkint(L, 6);
    }

    ID3D11Resource *pTexture(NULL);
    ID3D11ShaderResourceView *pTextureView(NULL);

    if (lua_isstring(L, 1)) {
        // filename
        const char *s = luaL_checkstring(L, 1);
        if (!s) {
            lua_pop(L, lua_gettop(L));
            lua_pushstring(L, "sprite error: no filename provided");
            return lua_error(L);
        }
        // try to load the image
        if (!load_image(s, &pTexture, &pTextureView)) {
            lua_pop(L, lua_gettop(L));
            lua_pushfstring(L, "sprite error: unable to load image from %s", s);
            return lua_error(L);
        }

        // if either width or height is not specified - get them from the image
        if (width == 0 || height == 0) {
            int image_width, image_height;
            if (!get_image_dimensions(pTexture, &image_width, &image_height)) {
                lua_pop(L, lua_gettop(L));
                lua_pushstring(L, "sprite error: unable to determine image dimensions");
                return lua_error(L);
            }

            if (width == 0 && height == 0) {
                width = image_width;
                height = image_height;
            } else if (width == 0) {
                width = int(height * (float(image_width) / float(image_height)));
            } else if (height == 0) {
                height = int(width * (float(image_height) / float(image_width)));
            }
        }
    }

    lua_pop(L, lua_gettop(L));

    // draw the image
    HRESULT hr;

    // define and set the constant buffers
    // constant buffer
    TexConstants constants;
    constants.maxAlpha = image_alpha;

    ID3D11Buffer* pConstantBuffer(NULL);
    D3D11_BUFFER_DESC bd = { 0 };
    bd.ByteWidth = 16;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = &constants;

    if (FAILED(DX11.Device->CreateBuffer(&bd, &initData, &pConstantBuffer))) {
        logu_("DX11.Device->CreateBuffer failed for constant buffer\n");
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "directx error: failed to create constants buffer");
        return lua_error(L);
    }

    const TexturedVertex texVertices[] =
    {
        { -1.f, 1.f, 0.3f, 1.f, 0.f, 0.f, 0.f, 0.f },
        { 1.f, 1.f, 0.3f, 1.f, 1.f, 0.f, 0.f, 0.f },
        { 1.f, -1.f, 0.3f, 1.f, 1.f, 1.f, 0.f, 0.f },
        { 1.f, -1.f, 0.3f, 1.f, 1.f, 1.f, 0.f, 0.f },
        { -1.f, -1.f, 0.3f, 1.f, 0.f, 1.f, 0.f, 0.f },
        { -1.f, 1.f, 0.3f, 1.f, 0.f, 0.f, 0.f, 0.f },
    };

    ID3D11Buffer* pTexVertexBuffer(NULL);
    {
        D3D11_BUFFER_DESC bd;
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = sizeof(texVertices);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = texVertices;
        hr = DX11.Device->CreateBuffer(&bd, &initData, &pTexVertexBuffer);
        if (FAILED(hr)) {
            SAFE_RELEASE(pConstantBuffer);
            logu_("DX11.Device->CreateBuffer failed (for image)\n");
            lua_pop(L, lua_gettop(L));
            lua_pushstring(L, "directx error: failed to create vertex buffer");
            return lua_error(L);
        }
    }

    // Create the render target view
    ID3D11RenderTargetView* pRenderTargetView(NULL);
    ID3D11Texture2D* pRenderTargetTexture;
    hr = DX11.SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pRenderTargetTexture);
    if (FAILED(hr)) {
        logu_("DX11.SwapChain->GetBuffer failed\n");
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "directx error: failed to get buffer for render target texture");
        return lua_error(L);
    }
    hr = DX11.Device->CreateRenderTargetView(pRenderTargetTexture, NULL, &pRenderTargetView);
    if (FAILED(hr)) {
        pRenderTargetTexture->Release();
        logu_("DX11.Device->CreateRenderTargetView failed\n");
        lua_pop(L, lua_gettop(L));
        lua_pushstring(L, "directx error: failed to create render target view");
        return lua_error(L);
    }
    pRenderTargetTexture->Release();

    RECT rc = {0, 0, (LONG)DX11.Width, (LONG)DX11.Height};
    float top = 0;

    DX11.Context->IASetInputLayout(g_pTexInputLayout);

    UINT stride = sizeof(TexturedVertex);
    UINT offset = 0;
    DX11.Context->IASetVertexBuffers(0, 1, &pTexVertexBuffer, &stride, &offset);
    DX11.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DX11.Context->VSSetShader(g_pTexVertexShader, NULL, 0);
    DX11.Context->PSSetShader(g_pTexPixelShader, NULL, 0);
    DX11.Context->PSSetShaderResources( 0, 1, &pTextureView );
    DX11.Context->PSSetSamplers( 0, 1, &g_pSamplerLinear );
    DX11.Context->PSSetConstantBuffers( 0, 1, &pConstantBuffer );

    D3D11_VIEWPORT vp;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = x;
    vp.TopLeftY = y;
    vp.Width = width;
    vp.Height = height;
    DX11.Context->RSSetViewports(1, &vp);
    DX11.Context->OMSetRenderTargets(1, &pRenderTargetView, NULL);

    DX11.Context->OMSetBlendState(g_pBlendState, NULL, 0xffffffff);
    DX11.Context->Draw(6, 0); //6 vertices start at 0

    SAFE_RELEASE(pTexture);
    SAFE_RELEASE(pTextureView);
    SAFE_RELEASE(pTexVertexBuffer);
    SAFE_RELEASE(pRenderTargetView);
    SAFE_RELEASE(pConstantBuffer);
    return 0;
}
