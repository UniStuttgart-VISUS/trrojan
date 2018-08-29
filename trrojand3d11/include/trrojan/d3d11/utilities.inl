/// <copyright file="utilities.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>


/*
 * trrojan::d3d11::set_debug_object_name
 */
template<class T>
void trrojan::d3d11::set_debug_object_name(T *obj, const char *name) {
    if ((obj != nullptr) && (name != nullptr)) {
        auto len = static_cast<UINT>(::strlen(name));

        auto hr = obj->SetPrivateData(WKPDID_D3DDebugObjectName, len, name);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}
