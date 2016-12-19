/// <copyright file="plugin.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <string>

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
        /// A list of <see cref="trrojan::environment" />s.
        /// </summary>
        typedef std::vector<environment> environment_list;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~plugin_base(void);

        /// <summary>
        /// Creates an instance of each of the environments provided by the
        /// plugin.
        /// </summary>
        /// <remarks>
        /// The environments returned must be uninitialised, because the
        /// <see cref="trrojan::executive" /> decides when initialisation is to
        /// be performed.
        /// </remarks>
        /// <param name="dst">An <see cref="std::vector" /> to append the
        /// environments to.</param>
        virtual void create_environments(environment_list& dst) const = 0;

        /// <summary>
        /// Answer the name of the plugin.
        /// </summary>
        /// <returns>The name of the plugin</returns>.
        inline const std::string& name(void) const {
            return this->_name;
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
