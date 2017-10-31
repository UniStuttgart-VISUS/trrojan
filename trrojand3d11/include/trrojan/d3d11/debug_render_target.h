/// <copyright file="debug_render_target.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d11/render_target_base.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements an on-screen render target for debugging purposes.
    /// </summary>
    class TRROJAND3D11_API debug_render_target : public render_target_base {

    public:

        inline debug_render_target(void) : render_target_base(nullptr) { }

        virtual ~debug_render_target(void);

        virtual void resize(const unsigned int width,
            const unsigned int height);

    };
}
}
