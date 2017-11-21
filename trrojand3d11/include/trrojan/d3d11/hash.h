/// <copyright file="hash.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <atlbase.h>
#include <d3d11.h>


namespace trrojan {
namespace d3d11 {

    struct device_hash {
        size_t operator ()(const ATL::CComPtr<ID3D11Device> &d) const {
            return reinterpret_cast<size_t>(d.p);
        }
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
