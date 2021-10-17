#include "pch.h"

//Project includes
#include "RichEffect.h"
#include "FlatEffect.h"
#include "ERenderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Camera.h"
#include "ObjParser.h"

Elite::Renderer::Renderer(SDL_Window* pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
{
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pCamera = Camera::GetInstance(m_Width, m_Height, { 0, 0,25 }, { 0, 0, 1 });

	//Initialize DirectX pipeline
	InitializeDirectX();
	InitializeSoftwareRasterizer(pWindow);

	m_IsInitialized = true;
	std::cout << "DirectX is ready\n";

	m_pDiffuseTexture = new Texture("Resources/vehicle_diffuse.png", m_pDevice);
	m_pNormalMap = new Texture("Resources/vehicle_normal.png", m_pDevice);
	m_pSpecularMap = new Texture("Resources/vehicle_specular.png", m_pDevice);
	m_pGlossinessMap = new Texture("Resources/vehicle_gloss.png", m_pDevice);

	ObjParser::GetInstance()->Parse("Resources/fireFX.obj", m_SoftwareRasterizer_VertexBuffer, m_DirectX_VertexBuffer, m_IndexBuffer);

	m_pMeshes.push_back(new Mesh(m_pDevice, new FlatEffect(m_pDevice, L"Resources/PosCol3D.fx"), m_DirectX_VertexBuffer, m_IndexBuffer, new Texture("Resources/fireFX_diffuse.png", m_pDevice)));

	m_SoftwareRasterizer_VertexBuffer.clear();
	m_DirectX_VertexBuffer.clear();
	m_IndexBuffer.clear();

	ObjParser::GetInstance()->Parse("Resources/vehicle.obj", m_SoftwareRasterizer_VertexBuffer, m_DirectX_VertexBuffer, m_IndexBuffer);

	m_pMeshes.push_back(new Mesh(m_pDevice, new RichEffect(m_pDevice, L"Resources/PosCol3D.fx"), m_DirectX_VertexBuffer, m_IndexBuffer, m_pDiffuseTexture, m_pNormalMap, m_pSpecularMap, m_pGlossinessMap));

	
}

#pragma region DirectX
HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create Device and Device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, 0, 0, D3D11_SDK_VERSION, &m_pDevice, &featureLevel, &m_pDeviceContext);

	if (FAILED(result))
	{
		return result;
	}

	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));

	if (FAILED(result))
	{
		return result;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferDesc.Width = m_Width;
	swapChainDesc.BufferDesc.Height = m_Height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMInfo{};
	SDL_VERSION(&sysWMInfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
	swapChainDesc.OutputWindow = sysWMInfo.info.win.window;
	//Create SwapChain and hook it into the handle of te SDL window
	m_pSwapChain = {};
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

	if (FAILED(result))
	{
		return result;
	}

	//Create the Depth/Stencil Buffer and View
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;
	//Create the resource view for our Depth/Stencil Buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	//Create the actual resource and the 'matching' resource view
	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);

	if (FAILED(result))
	{
		return result;
	}

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);

	if (FAILED(result))
	{
		return result;
	}

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));

	if (FAILED(result))
	{
		return result;
	}

	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);

	if (FAILED(result))
	{
		return result;
	}

	//Bind the Views to the Output Merger Stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the Viewport
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}

Elite::Renderer::~Renderer()
{

	for (size_t i = 0; i < m_pMeshes.size(); i++)
	{
		delete m_pMeshes[i];
	}
	//Render Target View
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
	}

	//Render Target Buffer
	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
	}
	//Depth Stencil View
	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
	}

	//Depth Stencil Buffer
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
	}
	//Swap Chain
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
	}
	//DXGIFactory
	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
	}

	//Device Context
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
	}

	//Device
	if (m_pDevice)
	{

		m_pDevice->Release();
	}



}

void Elite::Renderer::DirectX_Render(float angle)
{
	if (!m_IsInitialized)
		return;

	// Clear buffers
	RGBColor clearColor = RGBColor(0.f, 0.f, 0.3f);
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Render
	// . . . 
	for (size_t i = 0; i < m_pMeshes.size(); i++)
	{
		m_pMeshes[i]->Render(m_pDeviceContext, angle);
	}

	// Present
	m_pSwapChain->Present(0, 0);
}


void Elite::Renderer::SetFilteringMethodToNext()
{
	for (size_t i = 0; i < m_pMeshes.size(); i++)
	{
		m_pMeshes[i]->SetFilteringMethodToNext();
	}
}

void Elite::Renderer::ToggleTransparency()
{
	for (size_t i = 0; i < m_pMeshes.size(); i++)
	{
		m_pMeshes[i]->ToggleTransparency();
	}
}
void Elite::Renderer::SetCullModeToNext()
{
	for (size_t i = 0; i < m_pMeshes.size(); i++)
	{
		m_pMeshes[i]->SetCullModeToNext();
	}
}

#pragma endregion DirectX

#pragma region SoftwareRasterizer
void Elite::Renderer::InitializeSoftwareRasterizer(SDL_Window* pWindow)
{
	m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	for (uint32_t r = 0; r < m_Height; ++r)
	{
		for (uint32_t c = 0; c < m_Width; ++c)
		{
			m_DepthBuffer.push_back( float{ FLT_MAX });
		}
	}

	m_ZFar = 100;
	m_ZNear = 0.1f;
	m_Angle = 0.0f;

	m_PrimitiveTechnology = PrimitiveTechnology::TriangleList;

}

void Elite::Renderer::SoftwareRasterizer_Render(float angle)
{

	m_Angle = -angle;

	SDL_LockSurface(m_pBackBuffer);

	std::vector<Vertex_Output> transformedVertices;

	SoftwareRasterizer_VertexTransformationFunction(m_SoftwareRasterizer_VertexBuffer, transformedVertices,
		m_pCamera->GetRightHandedViewMatrix(), m_Width, m_Height, m_pCamera->GetFOV());

#pragma region Reset
	SoftwareRasterizer_FillScreen({ 50,50,50 });

	//Reset depthbuffer;
	for (uint32_t r = 0; r < m_Height; ++r)
	{
		for (uint32_t c = 0; c < m_Width; ++c)
		{
			uint32_t index = c + r * m_Width;
			m_DepthBuffer[index] = FLT_MAX;
		}
	}
#pragma endregion //Color the background and reset the depthbuffer



	size_t increment = m_PrimitiveTechnology == PrimitiveTechnology::TriangleStrip ? 1 : 3;
	size_t limit = m_PrimitiveTechnology == PrimitiveTechnology::TriangleStrip ? 2 : 0;

	for (size_t i = 0; i + limit < m_IndexBuffer.size(); i += increment)
	{
		Vertex_Output transformedTriangle[3];
		transformedTriangle[0] = (transformedVertices[m_IndexBuffer[i]]);
		transformedTriangle[1] = (transformedVertices[m_IndexBuffer[i + 1]]);
		transformedTriangle[2] = (transformedVertices[m_IndexBuffer[i + 2]]);

		if (!transformedTriangle[0].isValid || !transformedTriangle[1].isValid || !transformedTriangle[2].isValid)
			continue;

		if (m_PrimitiveTechnology == PrimitiveTechnology::TriangleStrip &&
			!SoftwareRasterizer_IsRightHanded(transformedTriangle))
		{
			Vertex_Output temp = transformedTriangle[1];
			transformedTriangle[1] = transformedTriangle[2];
			transformedTriangle[2] = temp;
		}

		if (transformedTriangle[0].position == transformedTriangle[1].position ||
			transformedTriangle[0].position == transformedTriangle[2].position ||
			transformedTriangle[1].position == transformedTriangle[2].position) continue;

		IPoint2 min;
		IPoint2 max;

		SoftwareRasterizer_CreateBoundingBoxes(max, min, transformedTriangle);

		for (uint32_t r = static_cast<uint32_t>(min.y); r < static_cast<uint32_t>(max.y); ++r)
		{
			for (uint32_t c = static_cast<uint32_t>(min.x); c < static_cast<uint32_t>(max.x); ++c)
			{

				if (SoftwareRasterizer_Hit(FPoint2(float(c), float(r)), transformedTriangle))
				{
					float pixeldepth = 0;

					InterpolatedData interpolatedData = {};

					if (SoftwareRasterizer_BaryCentricCoordinates(IPoint2(c, r), transformedTriangle, pixeldepth, interpolatedData))
					{
						uint32_t index = c + r * m_Width;
						m_DepthBuffer[index] = pixeldepth;

						if (!m_VisualizeDepthBuffer) interpolatedData.color = SoftwareRasterizer_PixelShading(interpolatedData);

						m_pBackBufferPixels[index] = SDL_MapRGB(m_pBackBuffer->format,
							Uint8(interpolatedData.color.r * 255),
							Uint8(interpolatedData.color.g * 255),
							Uint8(interpolatedData.color.b * 255));
					}

				}

			}
		}

	}

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}
// Creates bounding boxes depending on the furthest vertices.
void Elite::Renderer::SoftwareRasterizer_CreateBoundingBoxes(IPoint2& max, IPoint2& min, const Vertex_Output transformedTriangle[3])
{
	min.x = (int)std::floor(std::min(transformedTriangle[0].position.x, transformedTriangle[1].position.x));
	min.x = (int)std::floor(std::min(float(min.x), transformedTriangle[2].position.x));

	min.y = (int)std::floor(std::min(transformedTriangle[0].position.y, transformedTriangle[1].position.y));
	min.y = (int)std::floor(std::min(float(min.y), transformedTriangle[2].position.y));

	max.x = (int)std::ceil(std::max(transformedTriangle[0].position.x, transformedTriangle[1].position.x));
	max.x = (int)std::ceil(std::max(float(max.x), transformedTriangle[2].position.x));

	max.y = (int)std::ceil(std::max(transformedTriangle[0].position.y, transformedTriangle[1].position.y));
	max.y = (int)std::ceil(std::max(float(max.y), transformedTriangle[2].position.y));

	max.x = Elite::Clamp(max.x, 0, int(m_Width - 1));
	max.y = Elite::Clamp(max.y, 0, int(m_Height - 1));
	min.x = Elite::Clamp(min.x, 0, int(m_Width - 1));
	min.y = Elite::Clamp(min.y, 0, int(m_Height - 1));
}

// Fills screen with a color (used for background)
void Elite::Renderer::SoftwareRasterizer_FillScreen(const Elite::FVector3& rgb)
{
	for (uint32_t r = 0; r < m_Height; ++r)
	{
		for (uint32_t c = 0; c < m_Width; ++c)
		{
			m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				Uint8(rgb.r),
				Uint8(rgb.g),
				Uint8(rgb.b));
		}
	}
}

// Transforms all the vertices from 3D world space to 2D view space
void Elite::Renderer::SoftwareRasterizer_VertexTransformationFunction(const std::vector<SoftwareRasterizer_Vertex_Input>& originalVertices, std::vector<Vertex_Output>& transformedVertices,
	const FMatrix4& cameraToWorld, uint32_t width, uint32_t height, float fovAngle) const
{
	// Perspective divide
	Elite::FMatrix4 projectionMatrix = m_pCamera->GetRightHandedProjectionMatrix();

	Elite::FMatrix4 ViewMatrix = Inverse(cameraToWorld);

	for (size_t i = 0; i < originalVertices.size(); i++)
	{

		Vertex_Output output;
		output.color = originalVertices[i].color;
		output.normal = FVector3(originalVertices[i].normal.x, originalVertices[i].normal.y, originalVertices[i].normal.z);
		output.position = FPoint3(originalVertices[i].position.x, originalVertices[i].position.y, originalVertices[i].position.z);
		output.tangent = FVector3(originalVertices[i].tangent.x, originalVertices[i].tangent.y, originalVertices[i].tangent.z);
		output.uv = FPoint2(originalVertices[i].uv.x, originalVertices[i].uv.y);
		// WorldViewProjectionMatrix
		Elite::FVector3 worldPos = { 0,0,0 };
		Elite::FMatrix4 translation = FMatrix4::Identity();
		translation.data[3][0] = worldPos.x;
		translation.data[3][1] = worldPos.y;
		translation.data[3][2] = worldPos.z;

		Elite::FMatrix4 rotation = Elite::MakeRotationY(float(m_Angle * E_TO_RADIANS));

		Elite::FMatrix4 worldMatrix = translation * rotation;

		//setting normals to the world pos
		Elite::FVector4 normalVector = worldMatrix * Elite::FVector4(output.normal);
		output.normal = Elite::FVector3(normalVector.x, normalVector.y, normalVector.z);

		// setting viewdirection to the world pos
		FVector3 viewDirection = FVector3(worldMatrix * FVector4(FPoint4(originalVertices[i].position))) - FVector3(FPoint3(m_pCamera->GetPosition()));
		Normalize(viewDirection);
		output.viewDirection = viewDirection;

		//setting positions to the world pos
		Elite::FMatrix4  WorldViewProjectionMatrix = projectionMatrix * ViewMatrix * worldMatrix;

		output.position = WorldViewProjectionMatrix * FPoint4(originalVertices[i].position);
		// Perspective divide
		output.position.x /= output.position.w;
		output.position.y /= output.position.w;
		output.position.z /= output.position.w;

		// Frustum culling
		if (output.position.x < -1 || output.position.x > 1 ||
			output.position.y < -1 || output.position.y > 1 ||
			output.position.z < 0 || output.position.z > 1)
		{
			output.isValid = false;
		}
		else
		{
			output.position.x = ((output.position.x + 1) / 2) * width;
			output.position.y = ((1 - output.position.y) / 2) * height;
		}

		transformedVertices.push_back(output);


	}
}

bool Elite::Renderer::SoftwareRasterizer_SaveBackbufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "BackbufferRender.bmp");
}

//Checks if a triangle and point are colliding
bool Elite::Renderer::SoftwareRasterizer_Hit(const Elite::FPoint2& p, const Vertex_Output vertices[3]) const
{
	Elite::FVector2 vertex0 = Elite::FVector2(vertices[0].position.x, vertices[0].position.y);
	Elite::FVector2 vertex1 = Elite::FVector2(vertices[1].position.x, vertices[1].position.y);
	Elite::FVector2 vertex2 = Elite::FVector2(vertices[2].position.x, vertices[2].position.y);
	Elite::FVector2 point = Elite::FVector2(p);

	Elite::FVector2 edge1 = vertex1 - vertex0;
	Elite::FVector2 pointToSide1 = point - vertex0;
	Elite::FVector2 edge2 = vertex2 - vertex1;
	Elite::FVector2 pointToSide2 = point - vertex1;
	Elite::FVector2 edge3 = vertex0 - vertex2;
	Elite::FVector2 pointToSide3 = point - vertex2;

	if (Cross(pointToSide1, edge1) >= 0 && Cross(pointToSide2, edge2) >= 0 && Cross(pointToSide3, edge3) >= 0)
	{
		return true;
	}

	return false;

}

//Checks if a triangle is counterclockwise.
bool Elite::Renderer::SoftwareRasterizer_IsRightHanded(const Vertex_Output transformedTriangle[3])
{
	if (((transformedTriangle[1].position.x - transformedTriangle[0].position.x) * (transformedTriangle[1].position.y + transformedTriangle[0].position.y) +
		(transformedTriangle[2].position.x - transformedTriangle[1].position.x) * (transformedTriangle[2].position.y + transformedTriangle[1].position.y) +
		(transformedTriangle[0].position.x - transformedTriangle[2].position.x) * (transformedTriangle[0].position.y + transformedTriangle[2].position.y)
		) > 0)
		return true;

	return false;
}

// Interpolates a value according to the barycentric coordinate system.
//(Many calculations are done in a different function so they aren't unnecessarily repeated.
Elite::FVector3 Elite::Renderer::SoftwareRasterizer_Interpolate(const Elite::FVector3& valueP1, const Elite::FVector3& valueP2, const Elite::FVector3& valueP3
	, const Elite::FPoint3& uninterpolatedZComponents, const Elite::FPoint3& baryCentricCoordinates) const
{
	float wInterpolated = 1 / ((1 / uninterpolatedZComponents[0]) * baryCentricCoordinates[0] + (1 / uninterpolatedZComponents[1]) * baryCentricCoordinates[1] + (1 / uninterpolatedZComponents[2]) * baryCentricCoordinates[2]);

	Elite::FVector3 value = (valueP1 / uninterpolatedZComponents[0]) * baryCentricCoordinates[0] + (valueP2 / uninterpolatedZComponents[1]) * baryCentricCoordinates[1] + (valueP3 / uninterpolatedZComponents[2]) * baryCentricCoordinates[2];
	value *= wInterpolated;
	return value;
}


// Calculates the barycentric coordinates and calls the interpolation. Also transforms the normal into tangent space.
bool Elite::Renderer::SoftwareRasterizer_BaryCentricCoordinates(const Elite::IPoint2& p, const Vertex_Output vertices[3], float& depthBuffer, InterpolatedData& interpolatedData) const
{
#pragma region CalculateBarycentricCoordinates
	Elite::RGBColor color = {};

	Elite::FVector2 vertex0 = Elite::FVector2(vertices[0].position.x, vertices[0].position.y);
	Elite::FVector2 vertex1 = Elite::FVector2(vertices[1].position.x, vertices[1].position.y);
	Elite::FVector2 vertex2 = Elite::FVector2(vertices[2].position.x, vertices[2].position.y);
	Elite::FVector2 point = Elite::FVector2(p);

	float divider = Cross(vertex0 - vertex2, vertex0 - vertex1);
	float w0 = Cross(point - vertex1, vertex2 - vertex1) / divider;
	float w1 = Cross(point - vertex2, vertex0 - vertex2) / divider;
	float w2 = Cross(point - vertex0, vertex1 - vertex0) / divider;

#pragma endregion

#pragma region DepthBuffer

	float zBuffer = 1 / (1 / (vertices[0].position.z) * w0 + 1 / (vertices[1].position.z) * w1 + 1 / (vertices[2].position.z) * w2);

	depthBuffer = zBuffer;

	uint32_t pWidth = p.x + p.y * m_Width;

	if (depthBuffer >= m_DepthBuffer[pWidth]) return false;


	if (m_VisualizeDepthBuffer)
	{
		zBuffer = Remap(zBuffer, 0.985f, 1.f);

		color = Elite::RGBColor{ zBuffer,zBuffer ,zBuffer };
		return true;
	}

#pragma endregion

#pragma region Interpolation

	// --Interpolate UV--
	FVector2 uv = FVector2(SoftwareRasterizer_Interpolate(FVector2(vertices[0].uv), FVector2(vertices[1].uv), FVector2(vertices[2].uv),
		FPoint3(vertices[0].position.w, vertices[1].position.w, vertices[2].position.w),
		FPoint3(w0, w1, w2)));
	uv = Elite::FVector2(uv.x, 1 - uv.y); // Flip Y
	if ((uv.y < 0 || uv.x < 0 || uv.y > 1 || uv.x > 1)) return false; // This is of course never supposed to happen
	color = m_pDiffuseTexture->Sample(uv);

	// --Interpolate normal--
	interpolatedData.normal = SoftwareRasterizer_Interpolate(vertices[0].normal, vertices[1].normal, vertices[2].normal,
		FPoint3(vertices[0].position.w, vertices[1].position.w, vertices[2].position.w),
		FPoint3(w0, w1, w2));
	Normalize(interpolatedData.normal);
	Elite::RGBColor sampledNormalRGB = m_pNormalMap->Sample(uv);
	FVector3 sampledNormal = { sampledNormalRGB.r,sampledNormalRGB.g,sampledNormalRGB.b };
	// Shift it from 0-1 range to -1 to 1 range
	sampledNormal *= 2;
	sampledNormal.r -= 1.0f;
	sampledNormal.g -= 1.0f;
	sampledNormal.b -= 1.0f;

	// --Interpolate viewdirection--
	Elite::FVector3 viewDirection = SoftwareRasterizer_Interpolate(vertices[0].viewDirection, vertices[1].viewDirection, vertices[2].viewDirection,
		FPoint3(vertices[0].position.w, vertices[1].position.w, vertices[2].position.w),
		FPoint3(w0, w1, w2));

	// --Interpolate tangent--
	FVector3 tangent = SoftwareRasterizer_Interpolate(vertices[0].tangent, vertices[1].tangent, vertices[2].tangent,
		FPoint3(vertices[0].position.w, vertices[1].position.w, vertices[2].position.w),
		FPoint3(w0, w1, w2));
	Normalize(tangent);
#pragma endregion

#pragma region NormalToTangentSpace

	// Calculate binormal and
	FVector3 binormal = Cross(tangent, interpolatedData.normal);
	binormal = -binormal; // Flip binormal
	Normalize(binormal);
	FMatrix3 tangentSpaceAxis = FMatrix3(tangent, binormal, interpolatedData.normal);

	sampledNormal = tangentSpaceAxis * sampledNormal;
#pragma endregion

	// Set variables to return
	interpolatedData.normal = sampledNormal;
	interpolatedData.uv = uv;
	interpolatedData.viewDirection = viewDirection;
	interpolatedData.color = color;

	return true;
}

void Elite::Renderer::SoftwareRasterizer_ToggleVisualizeDepthBuffer()
{
	m_VisualizeDepthBuffer = !m_VisualizeDepthBuffer;
}

// Set the final color of the pixel dependant on the specular- , gloss- and previously interpolated normal map
Elite::RGBColor Elite::Renderer::SoftwareRasterizer_PixelShading(InterpolatedData& interpolatedData)
{
	FVector3 lightDirection = { .577f,-.577f,-.577f };
	RGBColor lightColor = { 1,1,1 };
	float intensity = 7;

	float pi = 3.14159265359f;

	float observedArea = (Dot(-interpolatedData.normal, lightDirection));
	observedArea = Clamp(observedArea, 0.f, 1.f);
	RGBColor finalColor;

	float multiplier = 25.f;
	Elite::RGBColor gloss = m_pGlossinessMap->Sample(interpolatedData.uv);
	float specularIntensity = Clamp(Dot(interpolatedData.viewDirection, Reflect(lightDirection, interpolatedData.normal)), 0.f, 1.f);
	specularIntensity = std::pow(specularIntensity, gloss.r * multiplier);

	Elite::RGBColor specular = m_pSpecularMap->Sample(interpolatedData.uv);
	RGBColor specularColor = specular * specularIntensity;

	finalColor = RGBColor(((interpolatedData.color + specularColor) / pi ) * lightColor * intensity * observedArea);
	finalColor.MaxToOne();
	return finalColor;
}

#pragma endregion SoftwareRasterizer