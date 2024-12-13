#pragma once
#include "pch.h"

class AMaterial
{
public:
	std::string name;
	ComPtr<ID3D11ShaderResourceView> diffusMap;
	ComPtr<ID3D11ShaderResourceView> specularMap;
	ComPtr<ID3D11ShaderResourceView> normalMap;

	Color ambient = Color(0.f, 0.f, 0.f, 1.f);
	Color diffuse = Color(1.f, 1.f, 1.f, 1.f);
	Color specular = Color(0.f, 0.f, 0.f, 1.f);
	Color emissive = Color(0.f, 0.f, 0.f, 1.f);
};

