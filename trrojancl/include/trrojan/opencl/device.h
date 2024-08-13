/// <copyright file="device.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/opencl/export.h"
#include "trrojan/opencl/util.h"

namespace trrojan
{
namespace opencl
{

    /// <summary>
    /// Representation of an OpenCL device.
    /// </summary>
    class TRROJANCL_API device : public trrojan::device_base
    {

    public:

        typedef std::shared_ptr<device> pointer;

        /// <summary>
        /// Default constructor.
        /// <\summary>
        device(void);

        /// <summary>
        /// Construct the device from an OpenCL device object <paramref name="dev">.
        /// <\summary>
        /// <param name="dev">The OpenCL device to be set.</param>
        device(cl::Device d);

        /// <summary>
        /// Destructor.
        /// <\summary>
        virtual ~device(void);

        /// <summary>
        /// Set this device to the OpenCL device <paramref name="dev">.
        /// <\summary>
        /// <param name="dev">The OpenCL device to be set.</param>
        void set_cl_device(cl::Device dev);

        /// <summary>
        /// Get the reference to the OpenCL device object.
        /// <\summary>
        inline const cl::Device &get() const
        {
            return _cl_device;
        }

        /// <summary>
        /// Get the device vendor. (AMD, Intel, NVIDIA, other)
        /// <\summary>
        trrojan::opencl::vendor get_vendor() const;

        /// <summary>
        /// Get the OpenCL device type. (CPU, GPU, ACCELLERATOR, OTHER)
        /// <\summary>
        trrojan::opencl::hardware_type get_type() const;

    private:

        /// <summary>
        /// Set device properties, i.e. the device vendor and the device type that are
        /// queried with the OpenCL API. Default values are 'vendor_unknown' and 'type_other'.
        /// </summary>
        /// <param name="dev">The OpenCL device object</param>
        void set_device_props(cl::Device dev);

        /// <summary>
        /// Create a unique device id from PCIe bus id, device id and PCIe function.
        /// <see href="https://anteru.net/blog/2014/08/01/2483/" />
        /// </summary>
        /// <param name="vendor">Device vendor, to call specific API extensions.</param>
        /// <param name="dev">OpenCL device object whose ID is to be generated.</param>
        void create_id(vendor vendor, cl::Device dev);

        /// <summary>
        /// The OpenCL device.
        /// </summary>
        cl::Device _cl_device;

        /// <summary>
        /// The OpenCL device vendor.
        /// </summary>
        trrojan::opencl::vendor _vendor;

        /// <summary>
        /// The OpenCL device type.
        /// </summary>
        trrojan::opencl::hardware_type _type;
    };

}
}
