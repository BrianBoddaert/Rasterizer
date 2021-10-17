#pragma once
#include "Effect.h"

class RichEffect : public Effect
{
public:
	RichEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	void Update(float angle) override;
	~RichEffect() override;

	void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView)   override;
	void SetNormalMap(ID3D11ShaderResourceView* pResourceView)    override;
	void SetSpecularMap(ID3D11ShaderResourceView* pResourceView)  override;
	void SetGlossinessMap(ID3D11ShaderResourceView* pResourceView)override;

	ID3DX11EffectTechnique* GetTechnique() const override;

protected:
	ID3DX11EffectMatrixVariable* m_pMatViewInverseVariable;
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
protected:
	void Initialize(ID3D11Device* pDevice, const std::wstring& assetFile) override;
};

