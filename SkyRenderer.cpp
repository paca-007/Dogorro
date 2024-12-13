#include "SkyRenderer.h"
#include "GraphicsEngine.h"
#include "FbxData.h"
#include "FbxMeshData.h"

void CreateSphere(int LatLines, int LongLines)
{
	//NumVertices = LatLines * LongLines;
	//NumFaces = (LatLines - 1) * (LongLines - 1) * 2;

	//float sphereYaw = 0.0f;
	//float spherePitch = 0.0f;

	//std::vector vertices(NumVertices);

	//D3DXVECTOR3 currVertPos = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	//for (DWORD i = 0; i < LatLines; ++i)
	//{
	//	sphereYaw = i * (3.14 / LatLines);
	//	for (DWORD j = 0; j < LongLines; ++j)
	//	{
	//		spherePitch = j * (3.14 / LongLines);
	//		D3DXMatrixRotationYawPitchRoll(&rotationMatrix, sphereYaw, spherePitch, 0);
	//		D3DXVec3TransformCoord(&currVertPos, &DefaultForward, &rotationMatrix);
	//		D3DXVec3Normalize(&currVertPos, &currVertPos);
	//		vertices[i * LongLines + j].pos = currVertPos;
	//	}
	//}

	//D3D10_BUFFER_DESC bd;
	//bd.Usage = D3D10_USAGE_IMMUTABLE;
	//bd.ByteWidth = sizeof(Vertex) * NumVertices;
	//bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	//bd.CPUAccessFlags = 0;
	//bd.MiscFlags = 0;
	//D3D10_SUBRESOURCE_DATA InitData;
	//InitData.pSysMem = &vertices[0];
	//d3dDevice->CreateBuffer(&bd, &InitData, &VertexBuffer);

	//std::vector indices(NumFaces * 3);

	//int k = 0;
	//for (DWORD i = 0; i < LatLines - 1; ++i)
	//{
	//	for (DWORD j = 0; j < LongLines - 1; ++j)
	//	{
	//		indices[k] = i * LongLines + j;
	//		indices[k + 1] = i * LongLines + j + 1;
	//		indices[k + 2] = (i + 1) * LongLines + j;

	//		indices[k + 3] = (i + 1) * LongLines + j;
	//		indices[k + 4] = i * LongLines + j + 1;
	//		indices[k + 5] = (i + 1) * LongLines + j + 1;

	//		k += 6; // next quad
	//	}
	//}

	//D3D10_BUFFER_DESC ibd;
	//ibd.Usage = D3D10_USAGE_IMMUTABLE;
	//ibd.ByteWidth = sizeof(DWORD) * NumFaces * 3;
	//ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	//ibd.CPUAccessFlags = 0;
	//ibd.MiscFlags = 0;
	//D3D10_SUBRESOURCE_DATA iinitData;
	//iinitData.pSysMem = &indices[0];
	//d3dDevice->CreateBuffer(&ibd, &iinitData, &IndexBuffer);

	//UINT stride = sizeof(Vertex);
	//UINT offset = 0;
	//d3dDevice->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	//d3dDevice->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

}
