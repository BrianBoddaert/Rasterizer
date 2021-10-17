 #include "pch.h"
#include "RichEffect.h"
#include "Camera.h"

RichEffect::RichEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
    :Effect(pDevice, assetFile)
{
    Initialize(pDevice, assetFile);
}

void RichEffect::Initialize(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
    m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
    m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
    m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
    m_pTechnique = m_pEffect->GetTechniqueByName("StandardTechnique");

    if (!m_pTechnique->IsValid())
        std::wcout << L"m_pTechnique not valid\n";

    if (!m_pMatWorldVariable->IsValid())
        std::wcout << L"m_pMatWorldVariable not valid\n";

    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"m_pDiffuseMapVariable not valid\n";
    if (!m_pNormalMapVariable->IsValid())
        std::wcout << L"m_pNormalMapVariable not valid\n";
    if (!m_pSpecularMapVariable->IsValid())
        std::wcout << L"m_pSpecularMapVariable not valid\n";
    if (!m_pGlossinessMapVariable->IsValid())
        std::wcout << L"m_pGlossinessMapVariable not valid\n";
    if (!m_pMatWorldViewProjVariable->IsValid())
        std::wcout << L"m_pMatWorldViewProjVariable not valid\n";
    if (!m_pMatViewInverseVariable->IsValid())
        std::wcout << L"m_pMatViewInverseVariable not valid\n";


}

RichEffect::~RichEffect()
{

    if (m_pCurrentFilteringMethod->IsValid())
    {
        m_pCurrentFilteringMethod->Release();
    }

    if (m_pMatWorldViewProjVariable->IsValid())
    {
        m_pMatWorldViewProjVariable->Release();
    }
    if (m_pMatWorldVariable->IsValid())
    {
        m_pMatWorldVariable->Release();
    }

    if (m_pMatViewInverseVariable->IsValid())
    {
        m_pMatViewInverseVariable->Release();
    }

    if (m_pTechnique->IsValid())
    {
        m_pTechnique->Release();
    }

    if (m_pEffect->IsValid())
    {
        m_pEffect->Release();
    }

}

void RichEffect::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)
{
    if (m_pDiffuseMapVariable->IsValid())
        m_pDiffuseMapVariable->SetResource(pResourceView);
}

void RichEffect::SetNormalMap(ID3D11ShaderResourceView* pResourceView)
{
    if (m_pNormalMapVariable->IsValid())
        m_pNormalMapVariable->SetResource(pResourceView);
}
void RichEffect::SetSpecularMap(ID3D11ShaderResourceView* pResourceView)
{
    if (m_pSpecularMapVariable->IsValid())
        m_pSpecularMapVariable->SetResource(pResourceView);
}
void RichEffect::SetGlossinessMap(ID3D11ShaderResourceView* pResourceView)
{
    if (m_pGlossinessMapVariable->IsValid())
        m_pGlossinessMapVariable->SetResource(pResourceView);
}

void RichEffect::Update(float angle)
{
    Camera* camera = Camera::GetInstance();
    Elite::FMatrix4 projectionMatrix = camera->GetLeftHandedProjectionMatrix();
    Elite::FMatrix4 ViewMatrix = camera->GetLeftHandedViewMatrix();

    // WorldViewProjectionMatrix
    Elite::FVector3 worldPos = { 0,0,0 };
    Elite::FMatrix4 translation = Elite::FMatrix4::Identity();
    translation.data[3][0] = worldPos.x;
    translation.data[3][1] = worldPos.y;
    translation.data[3][2] = worldPos.z;

    Elite::FMatrix4 rotation = Elite::MakeRotationY(float(angle * E_TO_RADIANS));

    Elite::FMatrix4 worldMatrix = translation * rotation;

    Elite::FMatrix4 worldViewProjectionMatrix = projectionMatrix * ViewMatrix * worldMatrix;

    m_pMatWorldViewProjVariable->SetMatrix(&worldViewProjectionMatrix[0].x);
    m_pMatWorldVariable->SetMatrix(&worldMatrix[0].x);
    m_pMatViewInverseVariable->SetMatrix(&Inverse(ViewMatrix)[0].x);
}

ID3DX11EffectTechnique* RichEffect::GetTechnique() const
{
    return m_pTechnique;
}