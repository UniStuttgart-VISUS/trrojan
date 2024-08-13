// <copyright file="app.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>

#if defined(TRROJAN_FOR_UWP)
#include <functional>

#include <Windows.h>

#include <winrt/windows.applicationmodel.core.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.storage.h>


/// <summary>
/// The WinRT application object.
/// </summary>
class App : public winrt::implements<App,
        winrt::Windows::ApplicationModel::Core::IFrameworkViewSource,
        winrt::Windows::ApplicationModel::Core::IFrameworkView> {

public:

    template<class R> using IAsyncOperation = winrt::Windows::Foundation::IAsyncOperation<R>;
    template<class T> using IVectorView = winrt::Windows::Foundation::Collections::IVectorView<T>;
    using AsyncStatus = winrt::Windows::Foundation::AsyncStatus;
    using CoreApplication = winrt::Windows::ApplicationModel::Core::CoreApplication;
    using CoreDispatcher = winrt::Windows::UI::Core::CoreDispatcher;
    using CoreProcessEventsOption = winrt::Windows::UI::Core::CoreProcessEventsOption;
    using CoreApplicationView = winrt::Windows::ApplicationModel::Core::CoreApplicationView;
    using CoreWindow = winrt::Windows::UI::Core::CoreWindow;
    using IFrameworkView =winrt::Windows::ApplicationModel::Core::IFrameworkView;
    using PointerEventArgs =winrt::Windows::UI::Core::PointerEventArgs;
    using StorageFile = winrt::Windows::Storage::StorageFile;
    using StorageFolder = winrt::Windows::Storage::StorageFolder;

    inline IFrameworkView CreateView(void) noexcept {
        return *this;
    }

    inline void Initialize(const CoreApplicationView&) { }

    inline void Load(const winrt::hstring&) { }

    void Run(void);

    void SetWindow(const CoreWindow& window);

    inline void Uninitialize(void) { }

    //void OnPointerPressed(const IInspectable& source, PointerEventArgs& args);

    //void OnPointerMoved(const IInspectable& source, PointerEventArgs& args);

    //void AddVisual(float2 const point);

private:

    /// <summary>
    /// The state of the main thread.
    /// </summary>
    enum class State {

        /// <summary>
        /// The user selected no trroll file, which indicates that the
        /// application should exit.
        /// </summary>
        Done = 0x0000,

        /// <summary>
        /// The initial state, which requests the application to show a file
        /// picker asking for the trroll script to execute.
        /// </summary>
        PromptTrroll = 0x0001,

        /// <summary>
        /// The file picker for the troll script is currently displayed, but has
        /// not yet been confirmed or cancelled by the user.
        /// </summary>
        PromptingTrroll = 0x0002,

        /// <summary>
        /// The trroll script has been set and should be executed. Requesting
        /// the script to run and actually running the script is the same
        /// thread, because the benchmark will block the STA thread and
        /// periodically execute the dispatcher.
        /// </summary>
        Benchmarking = 0x0004,

        /// <summary>
        /// THe benchmark has completed and the user should be prompted for the
        /// location to copy the results to.
        /// </summary>
        PromptResult = 0x0008,

        /// <summary>
        /// The the application is copying the output file.
        /// </summary>
        MovingOutput = 0x0010,

        /// <summary>
        /// The the application is copying the log file.
        /// </summary>
        MovingLog = 0x0020,
    };

    /// <summary>
    /// Atomically all bits set in <paramref name="erase" /> from
    /// <paramref name="state" />.
    /// </summary>
    static void EraseBits(std::atomic<State>& state, const State erase);

    /// <summary>
    /// Gets the name of the output file that should be created for the given test
    /// case without the file extension.
    /// </summary>
    static std::string GetBaseName(const std::string& trroll);

    /// <summary>
    /// Gets the underlying integral value of the given
    /// <paramref name="state" />.
    /// </summary>
    static inline constexpr std::underlying_type<State>::type GetUnderlying(
            const State state) {
        return static_cast<std::underlying_type<State>::type>(state);
    }

    /// <summary>
    /// The name of the log file.
    /// </summary>
    static const std::string LogFile;

    /// <summary>
    /// Copy the output and log file to the first folder on a removable device
    /// the application can find.
    /// </summary>
    HANDLE CopyResultsToUsbAsync(const std::string& outFile,
        const std::string& logFile);

    /// <summary>
    /// Prompts the user for the TRROLL script to be executed.
    /// </summary>
    void PromptTrroll(void);

    /// <summary>
    /// Runs the specified TRROLL file.
    /// </summary>
    void RunTrroll(StorageFile file, const bool prompt_next = true);

    /// <summary>
    /// Runs the specified TRROLL files one after another.
    /// </summary>
    void RunTrrolls(IVectorView<StorageFile> files);

    //State _state;

    winrt::agile_ref<CoreDispatcher> _dispatcher;
    winrt::agile_ref<CoreWindow> _window;
    winrt::agile_ref<StorageFolder> _workFolder;


    //CompositionTarget m_target{ nullptr };
    //VisualCollection m_visuals{ nullptr };
    //Visual m_selected{ nullptr };
    //float2 m_offset{};
};
#endif /* defined(TRROJAN_FOR_UWP) */
