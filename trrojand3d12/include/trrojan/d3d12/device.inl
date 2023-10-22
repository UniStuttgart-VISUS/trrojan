// <copyright file="device.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::d3d12::device::device
 */
template<class TGenerator>
trrojan::d3d12::device::device(const ATL::CComPtr<IDXGIFactory4>& dxgiFactory,
            TGenerator&& d3dDevice)
        : _d3d_device(std::forward<TGenerator>(d3dDevice)),
            _dxgi_factory(dxgiFactory),
            _next_fence(0) {
    // First of all, make sure that the generator is working and obtain the
    // metadata for the device.
    this->set_desc();

    // Now, delete the device again such that it is not active unless a
    // benchmark actually needs it.
    this->_d3d_device.reset(nullptr);
    assert(this->_command_queue == nullptr);
    assert(this->_fence == nullptr);
}
