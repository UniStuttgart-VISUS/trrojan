﻿// <copyright file="device.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


#if defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d12::device::device
 */
template<class TGenerator>
trrojan::d3d12::device::device(const winrt::com_ptr<IDXGIFactory4>& dxgiFactory,
            TGenerator&& d3dDevice)
        : _d3d_device(std::forward<TGenerator>(d3dDevice)),
            _dxgi_factory(dxgiFactory),
            _next_fence(0) {
    // First of all, make sure that the generator is working and obtain the
    // metadata for the device.
    this->set_desc_from_device();

    // Now, delete the device again such that it is not active unless a
    // benchmark actually needs it.
    this->_d3d_device.reset(nullptr);
    assert(this->_command_queue == nullptr);
    assert(this->_fence == nullptr);
}
#endif /* defined(TRROJAN_FOR_UWP) */
