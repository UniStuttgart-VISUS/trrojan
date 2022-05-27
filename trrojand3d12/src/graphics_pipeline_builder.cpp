// <copyright file="graphics_pipeline_builder.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/graphics_pipeline_builder.h"


/*
 * trrojan::d3d12::graphics_pipeline_builder::build
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::graphics_pipeline_builder::build(ID3D12Device *device) {
    if (device == nullptr) {
        throw std::invalid_argument("The device to build the pipeline state "
            "for must be a valid pointer.");
    }

    ATL::CComPtr<ID3D12PipelineState> retval;

    auto hr = device->CreateGraphicsPipelineState(&this->_desc,
        IID_PPV_ARGS(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}
