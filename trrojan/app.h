// <copyright file="app.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>

#if defined(TRROJAN_FOR_UWP)
#include <windows.h>

#include <winrt/windows.applicationmodel.core.h>
#include <winrt/windows.storage.h>


/// <summary>
/// The WinRT application object.
/// </summary>
class App : public winrt::implements<App,
        winrt::Windows::ApplicationModel::Core::IFrameworkViewSource,
        winrt::Windows::ApplicationModel::Core::IFrameworkView> {

public:

    template<class R> using IAsyncOperation = winrt::Windows::Foundation::IAsyncOperation<R>;
    using AsyncStatus = winrt::Windows::Foundation::AsyncStatus;
    using CoreDispatcher = winrt::Windows::UI::Core::CoreDispatcher;
    using CoreProcessEventsOption = winrt::Windows::UI::Core::CoreProcessEventsOption;
    using CoreApplicationView = winrt::Windows::ApplicationModel::Core::CoreApplicationView;
    using CoreWindow = winrt::Windows::UI::Core::CoreWindow;
    using IFrameworkView =winrt::Windows::ApplicationModel::Core::IFrameworkView;
    using PointerEventArgs =winrt::Windows::UI::Core::PointerEventArgs;
    using StorageFile = winrt::Windows::Storage::StorageFile;

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

    enum class State {
        Done = 0x0000,
        Idle = 0x0001,
        Benchmarking = 0x0002
    };

    static std::string GetBaseLogPath(const std::string& trroll);

    //CompositionTarget m_target{ nullptr };
    //VisualCollection m_visuals{ nullptr };
    //Visual m_selected{ nullptr };
    //float2 m_offset{};
};
#endif /* defined(TRROJAN_FOR_UWP) */
