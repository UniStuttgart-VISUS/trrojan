// <copyright file="bench_render_target.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2012 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/bench_render_target.h"

#include <cassert>
#include <sstream>

#include "trrojan/d3d12/utilities.h"


/*
 * trrojan::d3d12::bench_render_target::bench_render_target
 */
trrojan::d3d12::bench_render_target::bench_render_target(
        const trrojan::device& device) : base(device) {
    assert(this->device() != nullptr);
}


/*
 * trrojan::d3d12::bench_render_target::resize
 */
void trrojan::d3d12::bench_render_target::resize(const unsigned int width,
        const unsigned int height) {
    std::vector<ATL::CComPtr<ID3D12Resource>> buffers(this->pipeline_depth());

    for (std::size_t i = 0; i < buffers.size(); ++i) {
        buffers[i] = create_render_target(this->device(), width, height,
            DXGI_FORMAT_R8G8B8A8_UNORM);
    }

    this->set_buffers(buffers);
}
