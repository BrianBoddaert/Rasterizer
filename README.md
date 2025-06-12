## Software rasterizer (30 FPS)

![SRAS_Animated](https://github.com/user-attachments/assets/d161462f-88b6-4fbd-97b2-cbd7c41fd246)


## DirectX rasterizer (10000+ FPS)

![DX_Animated-ezgif com-optimize](https://github.com/user-attachments/assets/c040bb8c-b5fb-4f4e-aafb-13beddfa3094)

The software rasterizer was made solely in C++ and runs on the CPU where as my DirectX Rasterizer is made in C++ with HLSL and runs on the GPU.

My rasterizer includes the following features:

- .obj File loader that removes duplicate vertices
- Triangle mesh rasterizer
- Directional lighting
- Transparency
- Front- and back culling
- Fustrum culling
- Dynamic camera

For my directX rasterizer I also implemented phong shading and applied normal maps, glossiness maps and specular maps. I made both the trianglestrip method- as well as the trianglelist method for primitive topology and implemented both point- linear and anisotropic filtering for texture filtering. Which are interchangeable by the press of a button.

## Vertex Transformation

Transforms the vertices from 3D World space to 2D clipping space, and then put through the perspective divide into normalized device coordinates (NDC). Afterwards frustum culling is applied to only render what is necessary.

```
// Transforms all the vertices from 3D world space to 2D view space
void Elite::Renderer::SoftwareRasterizer_VertexTransformationFunction(const std::vector<SoftwareRasterizer_Vertex_Input>& originalVertices, std::vector<Vertex_Output>& transformedVertices,
	const FMatrix4& cameraToWorld, uint32_t width, uint32_t height, float fovAngle) const
{
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

		// From model space, to world space, view space, projection space
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
			output.position.x = ((output.position.x + 1) / 2) * width;  // From ortographic back into perspective
			output.position.y = ((1 - output.position.y) / 2) * height; // From ortographic back into perspective
		}

		transformedVertices.push_back(output);
	}
}
```

## Attribute Interpolation

Performs depth attribute interpolation using barycentric coordinates. Also stores the correct data in the depth buffer.

```
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
view rawDepthInterpolation.cpp hosted with ❤ by GitHub
DirectX Rasterizer Code
Pixel Shader: combined normal map, specular map, glossiness map and diffuse texture in HLSL.

// Pixel Shader
float4 StandardPixelShader(VS_OUTPUT input) : SV_TARGET
{
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverseMatrix[3].xyz);
	float3 tangentWorld = NormalMapTangentSpace(input);

	float specularIntensity = saturate(dot(viewDirection, reflect(gLightDirection, tangentWorld)));
	specularIntensity = pow(specularIntensity, GlossShading(input).r * gShininess);
	
	// Diffuse + Phong + Ambient
	return float4(((DiffuseShading(input) + SpecularShading(input) * specularIntensity) / gPI) * gLightColor * gLightIntensity * CalculateObservedArea(tangentWorld),1);
}
```

## Load Mesh
Called for every mesh to link the vertex- and index buffer to the device and adding our textures to the vertices.

```
void Mesh::Load(ID3D11Device* pDevice, const std::vector<DirectX_Vertex_Input>& vertices, const std::vector<int>& indices)
{
    //Create Vertex Layout 
    
    HRESULT result = S_OK;
    static const uint32_t numElements{ 5 };
    D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

    vertexDesc[0].SemanticName = "POSITION";
    vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDesc[0].AlignedByteOffset = 0;
    vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    vertexDesc[1].SemanticName = "COLOR";
    vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDesc[1].AlignedByteOffset = 12;
    vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    vertexDesc[2].SemanticName = "TEXCOORD";
    vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
    vertexDesc[2].AlignedByteOffset = 24;
    vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    vertexDesc[3].SemanticName = "NORMAL";
    vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDesc[3].AlignedByteOffset = 32;
    vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    vertexDesc[4].SemanticName = "TANGENT";
    vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    vertexDesc[4].AlignedByteOffset = 44;
    vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    // Create the input layout
    D3DX11_PASS_DESC passDesc;
    m_pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
    result = pDevice->CreateInputLayout(
        vertexDesc,
        numElements,
        passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize,
        &m_pVertexLayout);

    if (FAILED(result))
        return;

    // Create vertex buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(DirectX_Vertex_Input) * (uint32_t)vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA initData = { 0 };
    initData.pSysMem = vertices.data();
    result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
    if (FAILED(result))
        return;

    // Create index buffer
    m_AmountIndices = (uint32_t)indices.size();
    bd.Usage = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;
    initData.pSysMem = indices.data();
    result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
    if (FAILED(result))
        return;
}
```
