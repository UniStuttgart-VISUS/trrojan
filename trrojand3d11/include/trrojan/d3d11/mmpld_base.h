/// <copyright file="mmpld_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include <Windows.h>
#include <d3d11.h>

#include "trrojan/mmpld_reader.h"

#include "trrojan/d3d11/export.h"
#include "trrojan/d3d11/device.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API mmpld_base {

    public:

        /// <summary>
        /// Creates a vertex input layout descriptor for the given MMPLD list
        /// header.
        /// </summary>
        /// <param name="header">The header of the particle list to get the
        /// vertex format for.</param>
        /// <returns>The vertex descriptor.</returns>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> get_mmpld_input(
            const mmpld_reader::list_header& header);

        virtual ~mmpld_base(void) = default;

    protected:

        mmpld_base(void) = default;

    };

}
}
