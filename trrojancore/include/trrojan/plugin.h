/// <copyright file="plugin.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <string>

#include "trrojan/benchmark.h"
#include "trrojan/environment.h"
#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// This class defines the interface for and a basic implementation of a
    /// plugin.
    /// </summary>
    /// <remarks>
    /// 
    /// </remarks>
    class TRROJANCORE_API plugin_base {

    public:

        /// <summary>
        /// A list of <see cref="trrojan::benchmark" />s.
        /// </summary>
        typedef std::vector<benchmark> benchmark_list;

        /// <summary>
        /// A list of <see cref="trrojan::environment" />s.
        /// </summary>
        typedef std::vector<environment> environment_list;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~plugin_base(void);

        /// <summary>
        /// Creates a instance of each of the benchmarks provided by the plugin.
        /// </summary>
        /// <param name="dst">An <see cref="benchmark_list" /> to append the
        /// benchmarks to.</param>
        /// <returns>The number of benchmarks which have been added to
        /// <paramref name="dst" />.</returns>
        virtual size_t create_benchmarks(benchmark_list& dst) const = 0;

        /// <summary>
        /// Creates an instance of each of the environments provided by the
        /// plugin.
        /// </summary>
        /// <remarks>
        /// The environments returned must be uninitialised, because the
        /// <see cref="trrojan::executive" /> decides when initialisation is to
        /// be performed.
        /// </remarks>
        /// <param name="dst">An <see cref="environment_list" /> to append the
        /// environments to.</param>
        /// <returns>The number of environments which have been added to
        /// <paramref name="dst" />.</returns>
        virtual size_t create_environments(environment_list& dst) const = 0;

        /// <summary>
        /// Answer the name of the plugin.
        /// </summary>
        /// <returns>The name of the plugin</returns>.
        inline const std::string& name(void) const {
            return this->_name;
        }

        /// <summary>
        /// Qualify a benchmark or environment name with the plugin name.
        /// </summary>
        /// <param name="name">The benchmark or environment name.</param>
        /// <returns>The qualified name.</returns>
        inline std::string qualify_name(const std::string& name) const {
            return this->name() + "::" + name;
        }

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name">The name of the plugin</param>
        inline plugin_base(const std::string& name) : _name(name) { }

    private:

        std::string _name;

    };

    /// <summary>
    /// A plugin.
    /// </summary>
    typedef std::shared_ptr<plugin_base> plugin;
}
