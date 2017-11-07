/// <copyright file="mmpld_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include <Windows.h>
#include <d3d11.h>

#include "trrojan/benchmark.h"
#include "trrojan/enum_parse_helper.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/debugable.h"
#include "trrojan/d3d11/device.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a particle rendering benchmark using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API mmpld_benchmark : public trrojan::benchmark_base,
            public debugable {

    public:

        static const std::string factor_data_set;
        static const std::string factor_viewport_height;
        static const std::string factor_viewport_width;

        /// <summary>
        /// Creates a vector descriptor for the given MMPLD list header.
        /// </summary>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> to_d3d11(
            const mmpld_reader::list_header& header);

        mmpld_benchmark(void);

        virtual ~mmpld_benchmark(void);

        virtual bool can_run(trrojan::environment env,
            trrojan::device device) const noexcept;

        virtual void draw_debug_view(ATL::CComPtr<ID3D11Device> device,
            ATL::CComPtr<ID3D11DeviceContext> deviceContext);

        virtual void on_debug_view_resized(ATL::CComPtr<ID3D11Device> device,
            const unsigned int width, const unsigned int height);

        virtual void on_debug_view_resizing(void);

        virtual void optimise_order(configuration_set& inOutConfs);

        /// <inheritdoc />
        virtual std::vector<std::string> required_factors(void) const;

        virtual size_t run(const configuration_set& configs,
            const on_result_callback& callback);

        virtual trrojan::result run(const configuration& config);

    protected:

    private:

        typedef trrojan::benchmark_base base;
    };

}
}
