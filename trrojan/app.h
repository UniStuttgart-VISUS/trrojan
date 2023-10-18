// <copyright file="app.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>

#if defined(TRROJAN_FOR_UWP)
#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/windows.storage.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>

#if (defined(NTDDI_WIN10_RS3) && (NTDDI_VERSION >= NTDDI_WIN10_RS3))
#include <Gamingdeviceinformation.h>
#endif /* (defined(NTDDI_WIN10_RS3) && (NTDDI_VERSION >= NTDDI_WIN10_RS3)) */


/// <summary>
/// The WinRT application object.
/// </summary>
class App : public winrt::implements<App,
        winrt::Windows::ApplicationModel::Core::IFrameworkViewSource,
        winrt::Windows::ApplicationModel::Core::IFrameworkView> {

public:

    typedef winrt::Windows::UI::Core::CoreDispatcher CoreDispatcher;
    typedef winrt::Windows::UI::Core::CoreProcessEventsOption
        CoreProcessEventsOption;
    typedef winrt::Windows::ApplicationModel::Core::CoreApplicationView
        CoreApplicationView;
    typedef winrt::Windows::UI::Core::CoreWindow CoreWindow;
    typedef winrt::Windows::ApplicationModel::Core::IFrameworkView
        IFrameworkView;
    typedef winrt::Windows::UI::Core::PointerEventArgs PointerEventArgs;

    inline IFrameworkView CreateView(void) noexcept {
        return *this;
    }

    inline void Initialize(const CoreApplicationView&) { }

    inline void Load(const winrt::hstring&) { }

    void Run(void);

    inline void SetWindow(const CoreWindow&  window) { }

    inline void Uninitialize(void) { }

    //void OnPointerPressed(const IInspectable& source, PointerEventArgs& args);

    //void OnPointerMoved(const IInspectable& source, PointerEventArgs& args);

    //void AddVisual(float2 const point);

private:

    //CompositionTarget m_target{ nullptr };
    //VisualCollection m_visuals{ nullptr };
    //Visual m_selected{ nullptr };
    //float2 m_offset{};
};
#endif /* defined(TRROJAN_FOR_UWP) */
