/// <copyright file="executive.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

#include "trrojan/csv_output.h"
#include "trrojan/environment.h"
#include "trrojan/excel_output.h"
#include "trrojan/export.h"
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

        inline executive(void) { }

        executive(const executive&) = delete;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~executive(void);

        /// <summary>
        /// Answer whether an environment with the given name is available.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        inline bool has_environment(const std::string& name) const {
            return (this->environments.find(name) != this->environments.cend());
        }

        void load_plugins(void);

        executive operator =(const executive&) = delete;

        // TODO: remove this
        void crowbar() {
            try {
                trroll_parser::parse("test.trroll");
            } catch (std::exception& ex) {
                std::cerr << ex.what() << std::endl;
            }

            configuration_set ec;
            plugin_base::benchmark_list bs;
            plugin_base::environment_list es;

            for (auto p : this->plugins)
            {
                p->create_benchmarks(bs);
                p->create_environments(es);
            }

            // iterate environments
            for (auto e : es)
            {
                ec.replace_factor(factor::from_manifestations<environment>("environment", e));
                std::vector<device> dst;
                e->get_devices(dst);

                for (auto d : dst)
                {
                    ec.replace_factor(factor::from_manifestations<device>("device", d));
                    if (e->name().find("Intel") != std::string::npos)
                    {
                        ec.replace_factor(factor::from_manifestations("device_type", 1 << 1));
                    }
                    std::cout << std::endl << "=== " << d->name() << " ===" << std::endl;

                    for (auto b : bs)
                    {
                        // skip stream bech for testing volume raycast bench
                        if (b->name() != "stream")
                        {
                            auto fn = b->name();
#ifdef _WIN32f
                            fn += std::string(".xslx");
                            excel_output writer;
                            writer.open(excel_output_params::create(fn, true));
#else
                            fn += std::string(".csv");
                            csv_output writer;
                            writer.open(csv_output_params::create(fn));
#endif
                            b->run(ec, [&writer](result&& r) {
                                static_cast<output_base&>(writer) << r;
                                return true;
                            });
                        }
                    }
                }

            }


//            for (auto b : bs) {
//                std::cout << "=== " << b->name() << " ===" << std::endl;
//                auto fn = b->name();
//#ifdef _WIN32
//                fn += std::string(".xslx");
//                excel_output writer;
//                writer.open(excel_output_params::create(fn, true));
//#else
//                fn += std::string(".csv");
//                csv_output writer;
//                writer.open(csv_output_params::create(fn));
//#endif
//                b->run(ec, [&writer](result&& r) {
//                    static_cast<output_base&>(writer) << r;
//                    return true;
//                });

//                writer.close();
//            }
        }

    private:

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
        /// <param name="name"></param>
        void enable_environment(const std::string& name);

        /// <summary>
        /// Stores the currently active environment.
        /// </summary>
        environment cur_environment;

        /// <summary>
        /// Holds all execution environments, indexed by their name.
        /// </summary>
        std::unordered_map<std::string, environment> environments;

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
