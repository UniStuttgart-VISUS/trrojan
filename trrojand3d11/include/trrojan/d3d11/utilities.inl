/// <copyright file="utilities.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


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
