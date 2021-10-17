#pragma once

#include "Effect.h"

class FlatEffect : public Effect
{
public:
	FlatEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	void Update(float angle) override;
	~FlatEffect() override;

	void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView) override;
	ID3DX11EffectTechnique* GetTechnique() const override;
	void ToggleTransparency() override;
protected:
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable;
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
	ID3DX11EffectTechnique* m_pNonTransparentTechnique;
	bool m_Transparency;
	void Initialize(ID3D11Device* pDevice, const std::wstring& assetFile) override;

};

