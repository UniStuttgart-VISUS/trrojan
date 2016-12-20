/// <copyright file="executive.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/executive.h"

#include <iostream>
#include <stdexcept>

#ifndef _WIN32
#include <dlfcn.h>
#endif /* !_WIN32 */

#include "trrojan/log.h"


/*
 * trrojan::executive::~executive
 */
trrojan::executive::~executive(void) {
    if (this->cur_environment != nullptr) {
        this->cur_environment->on_deactivate();
    }

    for (auto e : this->environments) {
        e.second->on_finalise();
    }
}


/*
 * trrojan::executive::load_plugins
 */
void trrojan::executive::load_plugins(void) {
    try {
        log::instance().write(log_level::debug, "%s: remove debugging code", "TODO");
        auto p = plugin_dll::open("trrojancl.dll"); // TODO: Enumerate!!!!
        auto e = p.find_entry_point();
        auto xx = plugin(e());
        std::vector<environment> ee;
        xx->create_environments(ee);
        std::cout << "remove test code for " << xx->name() << std::endl;
    } catch (std::exception ex) {
        log::instance().write(ex);
    }
}


/*
 * trrojan::executive::enable_environment
 */
void trrojan::executive::enable_environment(const std::string& name) {
    if (this->cur_environment != nullptr) {
        this->cur_environment->on_deactivate();
    }
    this->cur_environment = this->environments[name];
    this->cur_environment->on_activate();
}


/*
 * trrojan::executive::plugin_dll::entry_point_name
 */
const std::string trrojan::executive::plugin_dll::entry_point_name
= "get_trrojan_plugin";


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
