// <copyright file="uwp_render_target_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>
// <author>Michael Becher</author>

#pragma once

#if defined(TRROJAN_FOR_UWP)
#include <algorithm>
#include <stdexcept>

#include <winrt/base.h>
#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>
#include <winrt/windows.ui.viewmanagement.h>

#include <Windows.h>


namespace trrojan {

    /// <summary>
    /// The base class for render targets that use a UWP window.
    /// </summary>
    /// <remarks>
    /// This class provides some baseline functionality to handle the
    /// association between UWP windows and rendering resources.
    /// </remarks>
    /// <typeparam name="TBaseClass">The base render target, e.g. for D3D11 or
    /// for D3D12, for use in the CRTP.</typeparam>
    template<class TBaseClass>
    class uwp_render_target_base : public TBaseClass {

    public:

        using core_window = winrt::Windows::UI::Core::CoreWindow;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~uwp_render_target_base(void) = default;

        /// <summary>
        /// Connect the render target with an existing UWP window.
        /// </summary>
        /// <param name="window"></param>
        void set_window(const winrt::agile_ref<core_window>& window);

    protected:

        using orientation_type = winrt::Windows::Graphics::Display::DisplayOrientations;
        using size_type = winrt::Windows::Foundation::Size;

        static constexpr float dips_per_inch = 96.0f;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        template<class... TArgs> uwp_render_target_base(TArgs&&... args);

        inline constexpr int dips_to_pixels(const float dips) {
            auto retval = ::floorf(dips * this->_dpi / dips_per_inch + 0.5f);
            return static_cast<int>(retval);
        };

        inline constexpr float pixels_to_dips(const int pixels) {
            return static_cast<float>(pixels) * dips_per_inch / this->_dpi;
        }

        bool try_resize_view(const unsigned int width,
            const unsigned int height);

        orientation_type _current_orientation;
        float _dpi;
        size_type _logical_size;
        orientation_type _native_orientation;
        size_type _output_size;
        winrt::agile_ref<core_window> _window;
    };

} /* namespace trrojan */

#include "trrojan/uwp_render_target_base.inl"
#endif /* defined(TRROJAN_FOR_UWP) */
