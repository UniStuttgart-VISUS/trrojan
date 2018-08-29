/// <copyright file="plugin.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#include "trrojan/plugin.h"

#include "trrojan/opencl/export.h"


namespace trrojan
{
namespace opencl
{

    /// <summary>
    /// Descriptor for the OpenCL plugin.
    /// </summary>
    class TRROJANCL_API plugin : public trrojan::plugin_base
    {

    public:

        typedef trrojan::plugin_base::environment_list environment_list;

        inline plugin(void) : trrojan::plugin_base("opencl") { }

        virtual ~plugin(void);

        virtual size_t create_benchmarks(benchmark_list& dst) const;

        ///
        /// \brief Create one environment for each valid OpenCL platform that is found and
        /// has at least one device available.
        /// \param dst The list of environments.
        /// \return The size of the list of environemnts.
        ///
        virtual size_t create_environments(environment_list& dst) const;
    };

}
}
