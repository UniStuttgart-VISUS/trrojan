/// <copyright file="executive.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/executive.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdexcept>

#ifndef _WIN32
#include <dlfcn.h>
#endif /* !_WIN32 */

#include "trrojan/io.h"
#include "trrojan/log.h"


/*
 * trrojan::executive::~executive
 */
trrojan::executive::~executive(void) {
    if (this->cur_environment != nullptr) {
        this->cur_environment->on_deactivate();
    }

    for (auto e : this->environments) {
        assert(e.second != nullptr);
        e.second->on_finalise();
    }
    this->environments.clear();
}


/*
 * trrojan::executive::
 */
trrojan::plugin trrojan::executive::find_plugin(const std::string& name) {
    auto it = std::find_if(this->plugins.begin(), this->plugins.end(),
        [&name](const plugin p) { return p->name() == name; });
    if (it != this->plugins.end()) {
        return *it;
    } else {
        log::instance().write(log_level::warning, "The plugin named \"%s\" "
            "does not exist or was not loaded.\n", name.c_str());
        return nullptr;
    }
}


/*
 * trrojan::executive::load_plugins
 */
void trrojan::executive::load_plugins(const cmd_line& cmdLine) {
    try {
        std::vector<std::string> paths;

        log::instance().write(log_level::verbose, "Considering plugins "
            "from the current working directory.\n");
        get_file_system_entries(std::back_inserter(paths), std::string("."),
            false, trrojan::has_extension(plugin_dll::extension));

#ifdef _WIN32
        {
            std::vector<char> mfn(MAX_PATH);
            if (::GetModuleFileName(NULL, mfn.data(),
                    static_cast<DWORD>(mfn.size()))) {
                auto it = std::find(mfn.rbegin(), mfn.rend(),
                    directory_separator_char);
                auto p = std::string(mfn.begin(), it.base());
                log::instance().write(log_level::verbose, "Considering plugins "
                    "from the directory \"%s\" holding the executable.\n",
                    p.c_str());
                get_file_system_entries(std::back_inserter(paths), p,
                    false, trrojan::has_extension(plugin_dll::extension));
            }
        }
#endif /* _WIN32 */

        log::instance().write(log_level::verbose, "Found %u potential "
            "plugin(s).\n", paths.size());

        for (auto& path : paths) {
            try {
                auto dll = plugin_dll::open(path);
                auto ep = dll.find_entry_point();
                if (ep == nullptr) {
                    throw std::runtime_error("Plugin entry point was not "
                        "found.");
                }

                log::instance().write(log_level::verbose, "Found a plugin "
                    "entry point in \"%s\".\n", path.c_str());
                auto plugin = trrojan::plugin(ep());
                if (plugin != nullptr) {
                    log::instance().write(log_level::verbose, "Found plugin "
                        "\"%s\" in \"%s\".\n", plugin->name().c_str(),
                        path.c_str());
                    this->plugins.push_back(std::move(plugin));
                    this->plugin_dlls.push_back(std::move(dll));
                }
            } catch (std::exception& ex) {
                log::instance().write_line(ex);
            }
        }

        log::instance().write(log_level::verbose, "%u plugin(s) have been "
            "loaded. Retrieving execution environments from them ...\n", 
            this->plugins.size());

        for (auto p : this->plugins) {
            std::vector<environment> envs;
            p->create_environments(envs);

            for (auto e : envs) {
                // First, handle potential violations of the contract with the
                // plugin. If the plugin returns invalid stuff, just skip it.
                if (e == nullptr) {
                    log::instance().write(log_level::debug, "The plugin \"%s\" "
                        "returned a nullptr as environment.\n",
                        p->name().c_str());
                    continue;
                }

                auto name = e->name();
                if (this->environments.find(name) != this->environments.end()) {
                    log::instance().write(log_level::debug, "The plugin \"%s\" "
                        "returned the environment \"%s\", which conflicts with "
                        "an already loaded environment. The new environment "
                        "will be ignored.\n", p->name().c_str(), name.c_str());
                    continue;
                }

                // Secon, initialise the plugin and add it to the map.
                try {
                    e->on_initialise(cmdLine);
                    this->environments.insert(std::make_pair(name, e));
                    log::instance().write(log_level::verbose, "The "
                        "environment \"%s\", provided by plugin \"%s\", was "
                        "successfully initialised.\n", name.c_str(),
                        p->name().c_str());

                } catch (std::exception& ex) {
                    log::instance().write_line(ex);
                    log::instance().write(log_level::verbose, "The "
                        "environment \"%s\", provided by plugin \"%s\", failed "
                        "to initialise. The environment will be ignored.\n",
                        name.c_str(), p->name().c_str());
                }
            }
        }

    } catch (std::exception& ex) {
        log::instance().write_line(ex);
    }
}


/*
 * trrojan::executive::trroll
 */
void trrojan::executive::trroll(const std::string& path, output_base& output) {
    typedef trroll_parser::benchmark_configs bcs;
    auto bcss = trroll_parser::parse(path);
    std::vector<benchmark> benchmarks;
    plugin curPlugin;

    // Make sure that the benchmark configurations are grouped.
    std::stable_sort(bcss.begin(), bcss.end(), [](const bcs& l, const bcs& r) {
        return (l.plugin.compare(r.plugin) < 0);
    });
    std::stable_sort(bcss.begin(), bcss.end(), [](const bcs& l, const bcs& r) {
        return (l.benchmark.compare(r.benchmark) < 0);
    });

    for (auto b : bcss) {
        if ((curPlugin == nullptr) || (curPlugin->name() != b.plugin)) {
            curPlugin = this->find_plugin(b.plugin);
            if (curPlugin != nullptr) {
                benchmarks.clear();
                curPlugin->create_benchmarks(benchmarks);

            } else {
                log::instance().write(log_level::warning, "The plugin \"%s\" "
                    "required for the benchmark \"%s\" does not exist or was "
                    "not loaded. The benchmark will be skipped.\n",
                    b.plugin.c_str(), b.benchmark.c_str());
            }
        }

        if (curPlugin != nullptr) {
            auto it = std::find_if(benchmarks.begin(), benchmarks.end(),
                [&b](const benchmark m) { return m->name() == b.benchmark; });
            if (it != benchmarks.end() && (*it != nullptr)) {
                log::instance().write(log_level::information, "Running "
                    "benchmark \"%s\" from plugin \"%s\".\n",
                    b.benchmark.c_str(), b.plugin.c_str());

                // TODO: need to enable env?!
                (**it).run(b.configs, [&output](result&& r) {
                    output << r;
                    return true;
                });

            } else {
                log::instance().write(log_level::warning, "No benchmark named "
                    "\"%s\" was found in plugin \"%s\". The benchmark will be "
                    "skipped.\n", b.benchmark.c_str(), b.plugin.c_str());
            }
        }
    } /* end for (auto b : bcss) */
}


/*
 * trrojan::executive::enable_environment
 */
void trrojan::executive::enable_environment(const std::string& name) {
    if ((this->cur_environment != nullptr)
            && (this->cur_environment->name() != name)) {
        this->cur_environment->on_deactivate();
        this->cur_environment = nullptr;
    }

    if (this->cur_environment == nullptr) {
        auto it = this->environments.find(name);
        if (it != this->environments.end()) {
            this->cur_environment = it->second;
            this->cur_environment->on_activate();

        } else {
            throw std::invalid_argument("Environment does not exist.");
        }
    }
}


/*
 * trrojan::executive::plugin_dll::entry_point_name
 */
const std::string trrojan::executive::plugin_dll::entry_point_name
= "get_trrojan_plugin";


/*
 * trrojan::executive::plugin_dll::extension
 */
const std::string trrojan::executive::plugin_dll::extension
#ifdef _WIN32
= ".dll";
#else /* _WIN32 */
= ".so";
#endif /* _WIN32 */


/*
 * trrojan::executive::plugin_dll::invalid_handle
 */
trrojan::executive::plugin_dll::handle_type
trrojan::executive::plugin_dll::invalid_handle
#ifdef _WIN32
= NULL;
#else /* _WIN32 */
= nullptr;
#endif /* _WIN32 */


/*
 * trrojan::executive::plugin_dll::open
 */
trrojan::executive::plugin_dll trrojan::executive::plugin_dll::open(
        const std::string& path) {
    plugin_dll retval;

#ifdef _WIN32
    auto oldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
    retval.handle = ::LoadLibraryExA(path.c_str(), NULL, 0);
    auto nec = ::GetLastError();
    ::SetErrorMode(oldErrorMode);

    if (retval.handle == plugin_dll::invalid_handle) {
        std::error_code ec(nec, std::system_category());
        throw std::system_error(ec, "Failed to open DLL.");
    }
#else /* _WIN32 */
    retval.handle = ::dlopen(path.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (retval.handle == plugin_dll::invalid_handle) {
        throw std::runtime_error(::dlerror());
    }

#endif /* _WIN32 */

    return std::move(retval);
}


/*
 * trrojan::executive::plugin_dll::~plugin_dll
 */
trrojan::executive::plugin_dll::~plugin_dll(void) {
    try {
        this->close();
    } catch (...) { }
}


/*
 * trrojan::executive::plugin_dll::close
 */
void trrojan::executive::plugin_dll::close(void) {
    if (this->handle != plugin_dll::invalid_handle) {
#ifdef _WIN32
        if (::FreeLibrary(this->handle) != TRUE) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Failed to close DLL.");
        }
#else /* _WIN32 */
        auto ec = ::dlclose(this->handle);
        if (ec != 0) {
            throw std::runtime_error(::dlerror());
        }
#endif /* _WIN32 */

        this->handle = plugin_dll::invalid_handle;
    }
}


/*
 * trrojan::executive::plugin_dll::find
 */
trrojan::executive::plugin_dll::proc_type trrojan::executive::plugin_dll::find(
        const std::string& name) {
#ifdef _WIN32
    return ::GetProcAddress(this->handle, name.c_str());
#else /* _WIN32 */
    return ::dlsym(this->handle, name.c_str());
#endif /* _WIN32 */
}


/*
 * trrojan::executive::plugin_dll::operator =
 */
trrojan::executive::plugin_dll& trrojan::executive::plugin_dll::operator =(
        plugin_dll&& rhs) {
    if (this != std::addressof(rhs)) {
        this->handle = rhs.handle;
        rhs.handle = plugin_dll::invalid_handle;
    }

    return *this;
}
