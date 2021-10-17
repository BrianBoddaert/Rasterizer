#include "pch.h"
#include "Texture.h"
#include <SDL_image.h>
#include "Effect.h"

Texture::Texture(const std::string& path,ID3D11Device* pDevice)
	:m_pTextureResourceView{}
{
	m_pSurface = IMG_Load(path.c_str());
	m_pSurfacePixels = (uint32_t*)m_pSurface->pixels;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT> (m_pSurface->h * m_pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pTexture);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;
	if (m_pTexture)
	hr = pDevice->CreateShaderResourceView(m_pTexture, &SRVDesc, &m_pTextureResourceView);

}

Elite::RGBColor Texture::Sample(const Elite::FVector2& uv)
{
	SDL_LockSurface(m_pSurface);
	int width = int(uv.x * m_pSurface->w);
	int height = int(uv.y * m_pSurface->h);
	Uint8 r{};
	Uint8 g{};
	Uint8 b{};
	SDL_GetRGB(m_pSurfacePixels[Uint32(width + (height * m_pSurface->w))], m_pSurface->format, &r, &g, &b);

	return Elite::RGBColor(r / 255.f, g / 255.f, b / 255.f);
}

Texture::~Texture()
{
	if (m_pTextureResourceView)
	{
		m_pTextureResourceView->Release();
	}

	if (m_pTexture)
	{
		m_pTexture->Release();
	}
	SDL_FreeSurface(m_pSurface);
	m_pSurfacePixels = nullptr;
}

ID3D11ShaderResourceView* Texture::GetTextureResourceView() const
{
	return m_pTextureResourceView;
}