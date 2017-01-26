/// <copyright file="image_helper.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::save_image
 */
template<class T>
void trrojan::save_image(const std::string& fileName, const T *data,
        const size_t width, const size_t height, const size_t channels) {
#if defined(TRROJAN_WITH_CIMG)
    std::array<int, 3> dim = { width, height, 1 };
    cimg_write(fileName, data, dim, channels);

#elif defined(_WIN32)
    auto hr = ::CoInitialize(nullptr);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to initialise COM.");
    }

    try {
        auto format = detail::wic_format_traits<T>::id(channels);
        trrojan::wic_save(trrojan::get_wic_factory(), data, width, height,
            0, format, fileName, ::GUID_NULL);
        ::CoUninitialize();
    } catch (...) {
        ::CoUninitialize();
        throw;
    }

#else
    throw std::logic_error("No implementation is available for saving images.");
#endif
}
