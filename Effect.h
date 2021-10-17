#ifndef Effect_HEADER
#define Effect_HEADER

#include <sstream>


class Effect 
{

public:

	ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	ID3DX11Effect* GetEffect() const;
	virtual void Update(float angle) = 0;
	virtual ~Effect();

	virtual ID3DX11EffectTechnique* GetTechnique() const = 0;

	void SetFilteringMethodToNext();

	virtual void SetDiffuseMap(ID3D11ShaderResourceView* pResourceView) {};
	virtual void SetNormalMap(ID3D11ShaderResourceView* pResourceView) {};
	virtual void SetSpecularMap(ID3D11ShaderResourceView* pResourceView) {};
	virtual void SetGlossinessMap(ID3D11ShaderResourceView* pResourceView) {};
	virtual void ToggleTransparency() {};

protected:
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual void Initialize(ID3D11Device* pDevice, const std::wstring& assetFile);

	ID3DX11Effect* m_pEffect;
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3DX11EffectScalarVariable* m_pCurrentFilteringMethod;

};
#endif
