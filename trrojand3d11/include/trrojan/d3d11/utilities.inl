// <copyright file="utilities.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::d3d11::set_debug_object_name
 */
template<class T>
void trrojan::d3d11::set_debug_object_name(T *obj, const char *name) {
    if ((obj != nullptr) && (name != nullptr)) {
        auto len = static_cast<UINT>(::strlen(name));

        auto hr = obj->SetPrivateData(WKPDID_D3DDebugObjectName, len, name);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }
}
