/// <copyright file="hlsltypemapping.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#ifdef _MSC_VER
#pragma once
#include <cstdint>

#include <DirectXMath.h>

typedef DirectX::XMFLOAT2 float2;
typedef DirectX::XMFLOAT3 float3;
typedef DirectX::XMFLOAT4 float4;

typedef DirectX::XMINT2 int2;
typedef DirectX::XMINT3 int3;
typedef DirectX::XMINT4 int4;

typedef DirectX::XMFLOAT4X4 float4x4;
typedef std::uint32_t uint;

#define cbuffer /*__declspec(align(16))*/ struct

#define CBUFFER(slot)

#else /* _MSC_VER */

#define CBUFFER(slot) : register(b##slot)

#endif /* _MSC_VER */
