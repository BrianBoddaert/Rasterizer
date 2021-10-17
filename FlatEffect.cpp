#include "pch.h"
#include "FlatEffect.h"
#include "Camera.h"

FlatEffect::FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
    :Effect(pDevice, assetFile)
{
    Initialize(pDevice,assetFile);
}

void FlatEffect::Initialize(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    m_pMatViewInverseVariable = m_pEffect->GetVariableByName("gViewInverseMatrix")->AsMatrix();
    m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
    m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
    m_pTechnique = m_pEffect->GetTechniqueByName("FlatTechniqueTransparent");
    m_pNonTransparentTechnique = m_pEffect->GetTechniqueByName("FlatTechniqueNonTransparent");
    m_Transparency = true;

    if (!m_pNonTransparentTechnique->IsValid())
        std::wcout << L"m_pNonTransparentTechnique not valid\n";

    if (!m_pTechnique->IsValid())
        std::wcout << L"m_pTechnique not valid\n";

    if (!m_pMatWorldVariable->IsValid())
        std::wcout << L"m_pMatWorldVariable not valid\n";

    if (!m_pDiffuseMapVariable->IsValid())
        std::wcout << L"m_pDiffuseMapVariable not valid\n";

    if (!m_pMatViewInverseVariable->IsValid())
        std::wcout << L"m_pMatViewInverseVariable not valid\n";

}
FlatEffect::~FlatEffect()
{

    // Flat effect

    if (m_pMatWorldVariable->IsValid())
    {
        m_pMatWorldVariable->Release();
    }

    if (m_pMatViewInverseVariable->IsValid())
    {
        m_pMatViewInverseVariable->Release();
    }

    // Effect base class

    if (m_pCurrentFilteringMethod->IsValid())
    {
        m_pCurrentFilteringMethod->Release();
    }

    if (m_pMatWorldViewProjVariable->IsValid())
    {
        m_pMatWorldViewProjVariable->Release();
    }

    if (m_pTechnique->IsValid())
    {
        m_pTechnique->Release();
    }
    if (m_pNonTransparentTechnique->IsValid())
    {
        m_pNonTransparentTechnique->Release();
    }
    if (m_pEffect->IsValid())
    {
        m_pEffect->Release();
    }


}

void FlatEffect::ToggleTransparency()
{
    m_Transparency = !m_Transparency;
}

ID3DX11EffectTechnique* FlatEffect::GetTechnique() const
{
    if (m_Transparency)
         return m_pTechnique;
    else
        return m_pNonTransparentTechnique;
}

void FlatEffect::SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)
{
    if (m_pDiffuseMapVariable->IsValid())
        m_pDiffuseMapVariable->SetResource(pResourceView);
}

void FlatEffect::Update(float angle)
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
