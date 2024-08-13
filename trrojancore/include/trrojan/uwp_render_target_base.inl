// <copyright file="uwp_render_target_base.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>
// <author>Michael Becher</author>


#if defined(TRROJAN_FOR_UWP)
/*
 * trrojan::uwp_render_target_base<TBaseClass>::set_window
 */
template<class TBaseClass>
void trrojan::uwp_render_target_base<TBaseClass>::set_window(
        const winrt::agile_ref<core_window>& window) {
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Graphics::Display;
    using namespace winrt::Windows::UI::Core;

    auto wnd = window.get();
    if (!wnd) {
        throw std::invalid_argument("A valid CoreWindow must be provided.");
    }

    wnd.Dispatcher().RunAsync(CoreDispatcherPriority::Normal,
            [this, window](void) {
        this->_window = window;

        const auto di = DisplayInformation::GetForCurrentView();
        const auto bounds = window.get().Bounds();
        this->_logical_size = size_type(bounds.Width, bounds.Height);
        this->_native_orientation = di.NativeOrientation();
        this->_current_orientation = di.CurrentOrientation();
        this->_dpi = di.LogicalDpi();

        // Calculate the necessary swap chain and render target size in pixels.
        this->_output_size.Width = dips_to_pixels(this->_logical_size.Width);
        this->_output_size.Height = dips_to_pixels(this->_logical_size.Height);

        // Prevent zero size DirectX content from being created.
        this->_output_size.Width = (std::max)(this->_output_size.Width, 1.0f);
        this->_output_size.Height = (std::max)(this->_output_size.Height, 1.0f);

        this->resize(this->_output_size.Width, this->_output_size.Height);
    }).get();
}


/*
 * trrojan::uwp_render_target_base<TBaseClass>::uwp_render_target_base
 */
template<class TBaseClass>
template<class... TArgs>
trrojan::uwp_render_target_base<TBaseClass>::uwp_render_target_base(
        TArgs&&... args)
    : TBaseClass(std::forward<TArgs>(args)...), _dpi(96.0f) { }


/*
 * trrojan::uwp_render_target_base<TBaseClass>::try_resize_view
 */
template<class TBaseClass>
bool trrojan::uwp_render_target_base<TBaseClass>::try_resize_view(
        const unsigned int width,
        const unsigned int height) {
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::UI::ViewManagement;

    auto wnd = this->_window.get();
    auto retval = (wnd != nullptr);

    if (retval) {
        auto size = size_type(pixels_to_dips(width), pixels_to_dips(height));

        // This was suggested by Bing AI ...
        if (winrt::impl::is_sta_thread()) {
            auto view = ApplicationView::GetForCurrentView();
            retval = view.TryResizeView(size);
        } else {
            wnd.Dispatcher().RunAsync(CoreDispatcherPriority::Normal,
                    [this, &size, &retval](void) {
                auto view = ApplicationView::GetForCurrentView();
                retval = view.TryResizeView(size);
            }).get();
        }
    }

    return retval;
}
#endif /* defined(TRROJAN_FOR_UWP) */
