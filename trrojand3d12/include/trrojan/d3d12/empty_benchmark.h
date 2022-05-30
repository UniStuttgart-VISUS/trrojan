// <copyright file="empty_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d12/benchmark_base.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// An empty benchmark that does nothing but clearing the screen.
    /// </summary>
    class TRROJAND3D12_API empty_benchmark : public benchmark_base {

    public:

        static const char *factor_clear_colour;

        empty_benchmark(void);

        virtual ~empty_benchmark(void);

    protected:

        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) override;

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
