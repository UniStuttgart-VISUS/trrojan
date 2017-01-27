/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/opencl/export.h"

//#include "trrojan/opencl/environment.h"

#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__) || defined(__MACOSX)
    #include "OpenCL/cl.hpp"
#else
    #include <CL/cl.hpp>
#endif

#include <map>

namespace trrojan {
namespace opencl {

    /// <summary>
    /// The hardware vendor enum.
    /// </summary>
    enum vendor
    {
        VENDOR_UNKNOWN  = (1 << 0),
        VENDOR_AMD      = (1 << 1),
        VENDOR_INTEL    = (1 << 2),
        VENDOR_NVIDIA   = (1 << 3),
        VENDOR_ANY      = (1 << 4),
    };

    /// <summary>
    /// The hardware type enum.
    /// </summary>
    enum hardware_type
    {
        TYPE_DEFAULT        = (1 << 0),     // CL_DEVICE_TYPE_DEFAULT
        TYPE_CPU            = (1 << 1),     // CL_DEVICE_TYPE_CPU
        TYPE_GPU            = (1 << 2),     // CL_DEVICE_TYPE_GPU
        TYPE_ACCELERATOR    = (1 << 3),     // CL_DEVICE_TYPE_ACCELERATOR
        TYPE_CUSTOM         = (1 << 4),     // CL_DEVICE_TYPE_CUSTOM
        TYPE_ALL            = 0xFFFFFFFF,   // CL_DEVICE_TYPE_ALL
    };

    /// <summary>
    /// Helper struct for map initialization.
    /// </summary>
    /// <see href="http://stackoverflow.com/questions/207976/how-to-easily-map-c-enums-to-strings" />
    template<typename T> struct map_init_helper
    {
        T& data;
        map_init_helper(T& d) : data(d) {}

        /// <summary>
        /// Returning *this from operator() allows the chaining of operator(),
        /// like operator<< on std::ostream s.
        /// </summary>
        map_init_helper& operator() (typename T::key_type const& key,
                                     typename T::mapped_type const& value)
        {
            data[key] = value;
            return *this;
        }
    };

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

        /// <summary>
        /// Returns a <see cref="trrojan::map_init_helper" />.
        /// </summary>
        template<typename T> map_init_helper<T> map_init(T& item)
        {
            return map_init_helper<T>(item);
        }

        /// <summary>
        /// Map of vendor names.
        /// </summary>
        std::map<vendor, const char*> _vendor_names;

        /// <summary>
        /// Map of type names.
        /// </summary>
        std::map<hardware_type, const char*> _type_names;
    };

}
}
