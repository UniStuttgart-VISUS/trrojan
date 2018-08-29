/// <copyright file="executive.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <iterator>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

#include "trrojan/cool_down.h"
#include "trrojan/environment.h"
#include "trrojan/export.h"
#include "trrojan/image_helper.h"
#include "trrojan/output.h"
#include "trrojan/plugin.h"
#include "trrojan/trroll_parser.h"


namespace trrojan {

    /// <summary>
    /// This is the root class which manages all the plugins and the
    /// environments.
    /// </summary>
    /// <remarks>
    /// 
    /// </remarks>
    class TRROJANCORE_API executive {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline executive(void) { }

        executive(const executive&) = delete;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~executive(void);

        /// <summary>
        /// Search for the plugin with the specified name.
        /// </summary>
        /// <param name="name"></param>
        /// <returns>A pointer to the respective plugin or <c>nullptr</c> if the
        /// plugin was not found.</returns>
        plugin find_plugin(const std::string& name);

        /// <summary>
        /// Returns all benchmarks currently known to the TRRojan.
        /// </summary>
        /// <param name="oit">An output iterator like a back insert
        /// iterator for <see cref="benchmark" />s.</param>
        template<class I> void get_benchmarks(I oit) const;

        /// <summary>
        /// Returns all environments to the given output iterator.
        /// </summary>
        /// <param name="oit">An output iterator like a back insert
        /// iterator.</param>
        template<class I> void get_environments(I oit) const;

        /// <summary>
        /// Answer whether an environment with the given name is available.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        inline bool has_environment(const std::string& name) const {
            return (this->environments.find(name) != this->environments.cend());
        }

        /// <summary>
        /// Runs the given JavaScript to conduct benchmarks wiring the results
        /// to the given <paramref name="output" />.
        /// <s/ummary>
        void javascript(const std::string& path, output_base& output,
            const cool_down& coolDown);

        /// <summary>
        /// Loads all plugins in the current directory, retrieves their
        /// environments and initialises those with the given command line.
        /// </summary>
        /// <param name="cmdLine">The command line arguments passed to the
        /// environments.</param>
        void load_plugins(const cmd_line& cmdLine);

        /// <summary>
        /// Runs the given benchmark using the given configurations.
        /// </summary>
        /// <param name="benchmark">The plugin and bechmark to be run.</param>
        /// <param name="configs">The set of configurations to be tested. This
        /// parameter is passed by value because it will be modified by the
        /// method before actually starting the benchmark. For instance, all
        /// names of <see cref="environment" /> and <see cref="device" />s need
        /// to be replaces with their actual instantiation.</param>
        void run(benchmark_base& benchmark, configuration_set configs,
            output_base& output, const cool_down& coolDown);

        void run(const benchmark& benchmark, const configuration_set& configs,
            output_base& output, const cool_down& coolDown);

        /// <summary>
        /// Runs the benchmarks in the given TRROLL script writing the results
        /// to the given <paramref name="output" />.
        /// </summary>
        void trroll(const std::string& path, output_base& output,
            const cool_down& coolDown);

        executive operator =(const executive&) = delete;

    private:

        /// <summary>
        /// Holds a possible combination of an environment and devices to be
        /// tested.
        /// </summary>
        /// <remarks>
        /// There might be restrictions on which environment is acce
        /// </remarks>
        struct env_dev_set {
            trrojan::environment environment;
            std::vector<trrojan::device> devices;

            inline env_dev_set(void) { }

            inline env_dev_set(trrojan::environment&& e, decltype(devices)&& d)
                : environment(std::move(e)), devices(std::move(d)) { }
        };

        /// <summary>
        /// Abstraction of a plugin DLL.
        /// </summary>
        class plugin_dll {

        public:

            /// <summary>
            /// The type of the plugin entry point retrieving the descriptor.
            /// </summary>
            typedef trrojan::plugin_base *(*entry_point_type)(void);

            /// <summary>
            /// The type of a native DLL handle.
            /// </summary>
#ifdef _WIN32
            typedef HMODULE handle_type;
#else /* _WIN32 */
            typedef void *handle_type;
#endif /* _WIN32 */

            /// <summary>
            /// The type of a native function pointer returned for a resolved
            /// procedure name.
            /// </summary>
#ifdef _WIN32
            typedef FARPROC proc_type;
#else /* _WIN32 */
            typedef void *proc_type;
#endif /* _WIN32 */

            /// <summary>
            /// Open the DLL at the specified location.
            /// </summary>
            static plugin_dll open(const std::string& path);

            /// <summary>
            /// The name of the entry point which provides us with the
            /// <see cref="trrojan::plugin" /> instance.
            /// </summary>
            static const std::string entry_point_name;

            /// <summary>
            /// The extension (including the leading ".") of a DLL.
            /// </summary>
            static const std::string extension;

            /// <summary>
            /// The value of an invalid handle.
            /// </summary>
            static handle_type invalid_handle;

            /// <summary>
            /// Initialises a new instance.
            /// </summary>
            inline plugin_dll(void) : handle(plugin_dll::invalid_handle) { }

            /// <summary>
            /// Move <paramref name="rhs" />.
            /// </summary>
            inline plugin_dll(plugin_dll&& rhs) : handle(rhs.handle) {
                rhs.handle = plugin_dll::invalid_handle;
            }

            /// <summary>
            /// Finalises the instance.
            /// </summary>
            ~plugin_dll(void);

            /// <summary>
            /// Closes the library.
            /// </summary>
            void close(void);

            /// <summary>
            /// Finds the procedure with the specified name.
            /// </summary>
            proc_type find(const std::string& name);

            entry_point_type find_entry_point(void) {
                return reinterpret_cast<entry_point_type>(
                    this->find(plugin_dll::entry_point_name));
            }

            /// <summary>
            /// Move assignment.
            /// </summary>
            plugin_dll& operator =(plugin_dll&& rhs);

        private:

            /// <summary>
            /// The DLL handle if any.
            /// </summary>
            handle_type handle;
        };

        /// <summary>
        /// Enables the environment with the specified name.
        /// </summary>
        /// <remarks>
        /// Any previously enabled environment stored at
        /// <see cref="cur_environment" /> will be properly deactivated by the
        /// method.
        /// </remarks>
        /// <param name="name"></param>
        /// <exception cref="std::invalid_argument>If the specified environment
        /// does not exist.</exception>
        void enable_environment(const std::string& name);

        /// <summary>
        /// Enables the given environment.
        /// </summary>
        /// <remarks>
        /// Any previously enabled environment stored at
        /// <see cref="cur_environment" /> will be properly deactivated by the
        /// method.
        /// </remarks>
        /// <param name="env"></param>
        /// <exception cref="std::invalid_argument>If <paramref name="env" /> is
        /// <c>nullptr</c>.</exception>
        void enable_environment(environment env);

        /// <summary>
        /// Finds the environment designated by the given string variant, or
        /// return the environment if the variant specified an actual
        /// environment pointer.
        /// </summary>
        /// <param name="v"></param>
        /// <returns></returns>
        environment find_environment(const variant& v);

        /// <summary>
        /// Prepare all possible combinations of <see cref="environment" /> and
        /// <see cref="device"> honouring any restrictions make in in
        /// <paramref name="cs" />.
        /// </summary>
        std::vector<env_dev_set> prepare_env_devs(const configuration_set& cs,
            const std::string& factorEnv = environment_base::factor_name,
            const std::string& factorDev = device_base::factor_name);

        /// <summary>
        /// Stores the currently active environment.
        /// </summary>
        environment cur_environment;

        /// <summary>
        /// Holds all execution environments, indexed by their name.
        /// </summary>
        std::map<std::string, environment> environments;

        /// <summary>
        /// Holds the libraries of all plugins that the application has found.
        /// </summary>
        std::vector<plugin_dll> plugin_dlls;

        /// <summary>
        /// Holds all of the plugin descriptors the application has found.
        /// </summary>
        std::vector<plugin> plugins;
    };
}

#include "trrojan/executive.inl"
