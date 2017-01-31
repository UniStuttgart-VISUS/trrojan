/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include <string>
#include <memory>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Base class for devices to be tested.
    /// </summary>
    class TRROJANCORE_API device_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device_base(void);

        // TODO: define the interface.

        /// <summary>
        /// Gets the name of the device.
        /// </summary>
        /// <returns>The name of the device (type).</returns>
        inline const std::string& name(void) const {
            return this->_name;
        }

        /// <summary>
        /// Gets the unique id of the device.
        /// </summary>
        /// <remarks>
        /// The unique identifier is implementation-defined and might
        /// only be unique on the current machine and using the current
        /// executable.
        /// </remarks>
        /// <returns>The unique id of the device.</returns>
        inline const unsigned int& unique_id(void) const {
            return this->_unique_id;
        }

    protected:

        /// <summary>
        /// The name (i.e. 'type') of the device.
        /// </summary>
        std::string _name;

        /// <summary>
        /// The unique id, specifying this unique device.
        /// </summary>
        unsigned int _unique_id;
    };

    /// <summary>
    /// A device.
    /// </summary>
    typedef std::shared_ptr<device_base> device;
}
