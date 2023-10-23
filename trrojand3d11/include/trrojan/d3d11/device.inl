// <copyright file="device.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


#if defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d11::device::device
 */
template<class TGenerator>
trrojan::d3d11::device::device(TGenerator&& generator)
        : d3dContext(std::bind(&device::make_context, this)),
        d3dDevice(std::forward<TGenerator>(generator)) {
    // First of all, make sure that the generator is working and obtain the
    // metadata for the device.
    this->set_desc();

    // Now, delete the device again such that it is not active unless a
    // benchmark actually needs it.
    this->d3dDevice.reset(nullptr);
}
#endif /* defined(TRROJAN_FOR_UWP) */
