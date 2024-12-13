#pragma once

#include <d3d11.h>
#include <assert.h>
#include <vector>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <string>
#include <dxgi.h>
#include <tchar.h>
#include <DirectXPackedVector.h>
#include <map>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <Assimp/Importer.hpp>
#include <Assimp/scene.h>
#include <Assimp/postprocess.h>
#include <Assimp/config.h>
#include <Assimp/cimport.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

// #pragma comment(lib,"d3d11.lib")
// #pragma comment(lib,"d3dcompiler.lib")

#ifdef DEBUG
#include "SimpleMath.h"
// #pragma comment(lib, "Assimp/assimp-vc143-mtd.lib")
#else // DEBUG
#include "SimpleMath.h"
// #pragma comment(lib, "Assimp/assimp-vc143-mt.lib")
#endif

using int8 = __int8;
using int16 = __int16;
using int32 = __int32;
using int64 = __int64;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

using Color = DirectX::XMFLOAT4;

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;
using Matrix = DirectX::SimpleMath::Matrix;
using Quaternion = DirectX::SimpleMath::Quaternion;

// MeshID / Material ID
using InstanceID = std::pair<uint64, uint64>;