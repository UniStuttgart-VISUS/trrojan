/// <copyright file="device.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph M�ller</author>

#include "trrojan/opencl/device.h"
#include "trrojan/log.h"

#include <sstream>

/*
 * trrojan::opencl::device::device
 */
trrojan::opencl::device::device()
    : _vendor(VENDOR_UNKNOWN)
    , _type(TYPE_ALL)
{
}


/*
 * trrojan::opencl::device::device
 */
trrojan::opencl::device::device(cl::Device d)
    : _cl_device(d)
{
    _name = d.getInfo<CL_DEVICE_NAME>();

//    std::string vendor = d.getInfo<CL_DEVICE_VENDOR>();
//    if (vendor.find("Intel") != std::string::npos)
//        _vendor = VENDOR_AMD;
//    else if (vendor.find("AMD") != std::string::npos
//             || vendor.find("Advanced Micro Devices") != std::string::npos)
//        _vendor = VENDOR_INTEL;
//    else if (vendor.find("NVIDIA") != std::string::npos)
//        _vendor = VENDOR_NVIDIA;
//    else
//        _vendor = VENDOR_UNKNOWN;

//    if (type.find("GPU") != std::string::npos)
//        _type = TYPE_GPU;
//    else if (type.find("CPU") != std::string::npos)
//        _type = TYPE_CPU;
//    else if (type.find("ACCELERATOR") != std::string::npos)
//        _type = TYPE_ACCELERATOR;
//    else
//        _type = TYPE_OTHER;

    set_device_props(_cl_device);
    create_id(_vendor, _cl_device);
}


/*
 * trrojan::opencl::device::~device
 */
trrojan::opencl::device::~device(void) { }

void trrojan::opencl::device::set_cl_device(cl::Device dev)
{
    _name = dev.getInfo<CL_DEVICE_NAME>();
    //_unique_id = std::to_string(dev.getInfo<CL_DEVICE_VENDOR_ID>()); // TODO set unique ID
    _cl_device = dev;

    set_device_props(_cl_device);
    create_id(_vendor, _cl_device);
}

trrojan::opencl::vendor trrojan::opencl::device::get_vendor() const
{
    return this->_vendor;
}

trrojan::opencl::hardware_type trrojan::opencl::device::get_type() const
{
    return this->_type;
}

void trrojan::opencl::device::create_id(vendor vendor, cl::Device dev)
{
    // get a unique device ID from device specifying ID and PCIe slot ID.
    switch (vendor)
    {
    case VENDOR_AMD:
    {
        // use cl_amd_device_topology extension from <CL/cl_ext.h>
        cl_device_topology_amd topology;
        cl_int status = clGetDeviceInfo(dev(),
                                        0x4037, //CL_DEVICE_TOPOLOGY_AMD,
                                        sizeof(cl_device_topology_amd),
                                        &topology,
                                        NULL);
        if(status != CL_SUCCESS)
        {
            throw cl::Error(status);
        }
        if (topology.raw.type == 1) //CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD = 1
        {
            _unique_id = static_cast<unsigned int>(topology.pcie.bus);
            _unique_id |= static_cast<unsigned int>(topology.pcie.device);
            _unique_id |= static_cast<unsigned int>(topology.pcie.function);
//            std::cout << "INFO: Topology: " << "PCI[ B#" << (int)topology.pcie.bus
//                << ", D#" << (int)topology.pcie.device << ", F#"
//                << (int)topology.pcie.function << " ]" << std::endl;
        }
        break;
    }
    case VENDOR_NVIDIA:
    {
        // use cl_nv_device_attribute_query extension from <CL/cl_ext.h>
        cl_int bus_id;
        cl_int slot_id;
        cl_int status = clGetDeviceInfo(dev(),
                                        0x4008, //CL_DEVICE_PCI_BUS_ID_NV,
                                        sizeof(cl_int),
                                        &bus_id,
                                        NULL);
        if(status != CL_SUCCESS)
        {
            throw cl::Error(status);
        }

        status = clGetDeviceInfo(dev(),
                                 0x4009, //CL_DEVICE_PCI_SLOT_ID_NV,
                                 sizeof(cl_int),
                                 &slot_id,
                                 NULL);
        if(status != CL_SUCCESS)
        {
            throw cl::Error(status);
        }
        _unique_id = static_cast<unsigned int>(bus_id);      // devce is (slot << 3)
        _unique_id |= static_cast<unsigned int>(slot_id);    // function is (slot & 7)
        break;
    }
//    case VENDOR_INTEL:
//    {
//    }
    default:
    {
        _unique_id = *((int*)(&dev()));
        break;
    }
    }

    std::ostringstream os;
    os << "OpenCL device topology (PCI bus & device): " << _unique_id;
    log::instance().write_line(log_level::information, os.str().c_str());
}

/**
 * trrojan::opencl::device::set_device_props
 */
void trrojan::opencl::device::set_device_props(cl::Device dev)
{
    std::string vendor = dev.getInfo<CL_DEVICE_VENDOR>();
    _vendor = util::get_vendor_from_string(vendor);
    _type = static_cast<hardware_type>(dev.getInfo<CL_DEVICE_TYPE>());
}
