/// <copyright file="device.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/opencl/device.h"


trrojan::opencl::device::device(cl::Device d)
{
    _cl_device = d;
    _name = d.getInfo<CL_DEVICE_NAME>();
    _unique_id = d.getInfo<CL_DEVICE_VENDOR_ID>();
    
    std::string vendor = d.getInfo<CL_DEVICE_VENDOR>();
    if (vendor.find("Intel") != std::string::npos)
        _vendor = VENDOR_AMD;
    else if (vendor.find("AMD") != std::string::npos)
        _vendor = VENDOR_INTEL;
    if (vendor.find("NVIDIA") != std::string::npos)
        _vendor = VENDOR_NVIDIA;
    else
        _vendor = VENDOR_UNKNOWN;

    std::string type = std::to_string(d.getInfo<CL_DEVICE_TYPE>());
    if (type.find("GPU") != std::string::npos)
        _type = TYPE_GPU;
    if (type.find("CPU") != std::string::npos)
        _type = TYPE_CPU;
    if (type.find("ACCELLERATOR") != std::string::npos)
        _type = TYPE_ACCELERATOR;
    else
        _type = TYPE_OTHER;
}

/*
 * trrojan::opencl::device::~device
 */
trrojan::opencl::device::~device(void) { }

void trrojan::opencl::device::set_cl_device(cl::Device d)
{
    _name = d.getInfo<CL_DEVICE_NAME>();
    _unique_id = std::to_string(d.getInfo<CL_DEVICE_VENDOR_ID>()); // TODO set unique ID
    _cl_device = d;

    std::string vendor = d.getInfo<CL_DEVICE_VENDOR>();
    if (vendor.find("Intel") != std::string::npos)
        _vendor = VENDOR_AMD;
    else if (vendor.find("AMD") != std::string::npos)
        _vendor = VENDOR_INTEL;
    if (vendor.find("NVIDIA") != std::string::npos)
        _vendor = VENDOR_NVIDIA;
    else
        _vendor = VENDOR_UNKNOWN;

    std::string type = std::to_string(d.getInfo<CL_DEVICE_TYPE>());
    if (type.find("GPU") != std::string::npos)
        _type = TYPE_GPU;
    if (type.find("CPU") != std::string::npos)
        _type = TYPE_CPU;
    if (type.find("ACCELLERATOR") != std::string::npos)
        _type = TYPE_ACCELERATOR;
    else
        _type = TYPE_OTHER;
}

trrojan::opencl::vendor trrojan::opencl::device::get_vendor() const
{
    return this->_vendor;
}

trrojan::opencl::hardware_type trrojan::opencl::device::get_type() const
{
    return this->_type;
}
