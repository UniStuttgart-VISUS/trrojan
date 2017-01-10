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
}
