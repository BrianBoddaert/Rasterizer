#include "pch.h"
#include "Mesh.h"
#include "FlatEffect.h"
#include "Texture.h"
#include "ObjParser.h"

Mesh::Mesh(ID3D11Device* pDevice, Effect* effect, const std::vector<DirectX_Vertex_Input>& verticesWorldSpace, const std::vector<int>& indices, Texture* diffuseTexture, Texture* normalTexture, Texture* specularTexture,Texture* glossTexture)
{
    m_pEffect = effect;
    m_pDiffuseTexture = diffuseTexture;
    m_pNormalTexture = normalTexture;
    m_pSpecularTexture = specularTexture;
    m_pGlossinessTexture = glossTexture;
    m_pCullModeIndex = 0;


    Load(pDevice, verticesWorldSpace, indices);

    if (diffuseTexture)
    m_pEffect->SetDiffuseMap(m_pDiffuseTexture->GetTextureResourceView());

    if (normalTexture)
    m_pEffect->SetNormalMap(m_pNormalTexture->GetTextureResourceView());

    if (specularTexture)
    m_pEffect->SetSpecularMap(m_pSpecularTexture->GetTextureResourceView());

    if (glossTexture)
    m_pEffect->SetGlossinessMap(m_pGlossinessTexture->GetTextureResourceView());
}

void Mesh::ToggleTransparency()
{
    m_pEffect->ToggleTransparency();
}
Mesh::~Mesh()
{

    if (m_pVertexBuffer)
    {
    m_pVertexBuffer->Release();
    }
    if (m_pIndexBuffer)
    {
        m_pIndexBuffer->Release();
    }

    if (m_pVertexLayout)
    {
        m_pVertexLayout->Release();
    }

    if (m_pEffect)
    {
        delete m_pEffect;
        m_pEffect = nullptr;
    }

    if (m_pDiffuseTexture)
    {
        delete m_pDiffuseTexture;
        m_pDiffuseTexture = nullptr;
    }
    if (m_pNormalTexture)
    {
        delete m_pNormalTexture;
        m_pNormalTexture = nullptr;
    }
    if (m_pSpecularTexture)
    {
        delete m_pSpecularTexture;
        m_pSpecularTexture = nullptr;
    }
    if (m_pGlossinessTexture)
    {
        delete m_pGlossinessTexture;
        m_pGlossinessTexture = nullptr;
    }




}
void Mesh::SetFilteringMethodToNext()
{
    m_pEffect->SetFilteringMethodToNext();
}

void Mesh::Render(ID3D11DeviceContext* pDeviceContext, float angle)
{

    // Update Data
    m_pEffect->Update(angle);

    UINT stride = sizeof(DirectX_Vertex_Input);
    UINT offset = 0;
    pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

    //Set index buffer
    pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Set the input layout
    pDeviceContext->IASetInputLayout(m_pVertexLayout);

    //Set primitive topology
    pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //Render a triangle
    D3DX11_TECHNIQUE_DESC techDesc;
    m_pEffect->GetTechnique()->GetDesc(&techDesc);

    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        m_pEffect->GetTechnique()->GetPassByIndex(0)->Apply(0, pDeviceContext);
        pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
    }
    
}

void Mesh::SetCullModeToNext()
{
    m_pCullModeIndex == 2 ? m_pCullModeIndex = 0 : m_pCullModeIndex++;
}
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

