#include "AMesh.h"
#include "GraphicsEngine.h"
#include "Utils.h"

void AMesh::SetVS(GraphicsEngine* _gp, std::string _path)
{
	_gp->CreateInputLayer(
		this->pip.inputLayout,
		const_cast<D3D11_INPUT_ELEMENT_DESC*>(ModelVertexType::inputLayerDECS),
		7,
		this->pip.vertexShader,
		Utils::ToWString(_path)
	);
	this->pip.vertexStructSize = sizeof(ModelVertexType);
	int a = 1;
}

void AMesh::SetPS(GraphicsEngine* _gp, std::string _path)
{
	_gp->CreatePixelShader(this->pip.pixelShader, Utils::ToWString(_path));
}
