/// <copyright file="executive.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "trrojan/environment.h"
#include "trrojan/export.h"
#include "trrojan/plugin.h"


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

    private:

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
        /// Holds all of the plugin descriptors the application has found.
        /// </summary>
        std::vector<plugin> plugins;
    };
}
