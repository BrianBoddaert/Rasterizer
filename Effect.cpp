#include "pch.h"
#include "Effect.h"
#include "Camera.h"

Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    Initialize(pDevice,assetFile);
}
void Effect::Initialize(ID3D11Device* pDevice, const std::wstring& assetFile) 
{
    m_pEffect = LoadEffect(pDevice, assetFile);

    m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
    m_pCurrentFilteringMethod = m_pEffect->GetVariableByName("gCurrentFilteringMethod")->AsScalar();
    m_pCurrentFilteringMethod->SetInt(0);
}
Effect::~Effect() {}

ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
    HRESULT result = S_OK;
    ID3D10Blob* pErrorBlob = nullptr;
    ID3DX11Effect* pEffect;

    DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined (_DEBUG )
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    result = D3DX11CompileEffectFromFile(assetFile.c_str(),
        nullptr,
        nullptr,
        shaderFlags,
        0,
        pDevice,
        &pEffect,
        &pErrorBlob);
    if (FAILED(result))
    {
        if (pErrorBlob != nullptr)
        {
            char* pErrors = (char*)pErrorBlob->GetBufferPointer();

            std::wstringstream ss;
            for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
                ss << pErrors[i];

            OutputDebugStringW(ss.str().c_str());
            pErrorBlob->Release();
            pErrorBlob = nullptr;

            std::wcout << ss.str() << std::endl;
        }
        else
        {
            std::wstringstream ss;
            ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
            std::wcout << ss.str() << std::endl;
            return nullptr;
        }
    }
    

    return pEffect;
}

ID3DX11Effect* Effect::GetEffect() const
{
    return m_pEffect;
}

void Effect::SetFilteringMethodToNext()
{
    int currentFilteringMethod;
    Effect::m_pCurrentFilteringMethod->GetInt(&currentFilteringMethod);

    currentFilteringMethod == 2 ? currentFilteringMethod = 0 : currentFilteringMethod++;

    Effect::m_pCurrentFilteringMethod->SetInt(currentFilteringMethod);

    std::cout << "Filtering method set to: ";

    switch (currentFilteringMethod)
    {
        case 0:  std::cout << "Point" << std::endl; break;
        case 1:  std::cout << "Linear" << std::endl; break;
        case 2:  std::cout << "Anistropic" << std::endl; break;
    }

}