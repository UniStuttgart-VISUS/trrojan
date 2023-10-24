// <copyright file="app.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>
// <author>Christoph Müller</author>

#if defined(TRROJAN_FOR_UWP)
#include "app.h"

#include <atomic>

#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.storage.accesscache.h>
#include <winrt/windows.storage.pickers.h>
#include <winrt/windows.ui.popups.h>

// Does not work:
//#include <WindowsStorageCOM.h>
//#include <pplawait.h>

#include "trrojan/executive.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/on_exit.h"
#include "trrojan/system_factors.h"


/*
 * App::Run
 */
void App::Run(void) {
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::AccessCache;

    auto temp_folder = ApplicationData::Current().TemporaryFolder();
    std::string log_path;
    std::string output_path;
    auto usb_devs = winrt::Windows::Storage::KnownFolders::RemovableDevices();
    std::atomic<State> state(State::PromptTrroll);
    auto window = CoreWindow::GetForCurrentThread();
    auto dispatcher = window.Dispatcher();
    StorageFile trroll_file(nullptr);
    std::string trroll_path;

    // Handlers for copying/moving the benchmark results to a location 
    // accessible by the user.
    const auto on_output_succeeded = [&state](void) {
        EraseBits(state, State::MovingOutput);
    };
    const auto on_output_failed = [&state](const AsyncStatus,
            const winrt::hresult hr) {
        trrojan::log::instance().write_line(trrojan::log_level::error,
            "The file containing the benchmark results could not be moved "
            "to a removable device ({0:x}). All subsequent operations are "
            "aborted.", hr.value);
        EraseBits(state, State::MovingOutput);
        //state = State::Done;
    };
    const auto on_log_succeeded = [&state](void) {
        EraseBits(state, State::MovingLog);
    };
    const auto on_log_failed = [&state](const AsyncStatus,
            const winrt::hresult hr) {
        trrojan::log::instance().write_line(trrojan::log_level::warning,
            "The log file could not be copied to a removable device ({0:x}).",
            hr.value);
        EraseBits(state, State::MovingLog);
    };

    //winrt::Windows::UI::Popups::MessageDialog dlg(L"Giving fabric!");
    //dlg.ShowAsync();

    window.Activate();

    while (state.load(std::memory_order::memory_order_consume) != State::Done) {
        switch (state.load(std::memory_order::memory_order_consume)) {
            case State::PromptTrroll:
                // In idle state, mark the benchmark selection as pending and
                // ask the user to select a trroll script.
                state = State::PromptingTrroll;

                trrojan::pick_file_and_continue({ L".trroll" },
                        [&trroll_file, &trroll_path, &state](StorageFile file) {
                    if (file) {
                        // If the user provided a test script, queue it for
                        // execution from the main thread.
                        trroll_file = file;
                        trroll_path = winrt::to_string(file.Path());
                        state = State::Benchmarking;
                    } else {
                        // If the user did not provide a test script, we exit.
                        trrojan::log::instance().write_line(
                            trrojan::log_level::information,
                            "User chose to end benchmarking by not selecting a "
                            "benchmarking script.");
                        state = State::Done;
                    }
                });
                break;

            case State::Benchmarking: {
                // In the benchmarking state, we run the previously selected
                // troll script. We cannot do that in any other thread, because
                // (i) the UWP render target must periodically call the
                // dispatcher from the main thread and (ii) running it via
                // 'dispatcher' would cause the evenr processing to be queued
                // recursively.
                // Note that we install a scope exit handler to make sure that
                // the state is reset even in case of an exception.
                on_exit([&state](void) { state = State::PromptResult; });
                const auto log_folder = winrt::to_string(temp_folder.Path());
                const auto log_base = trrojan::combine_path(log_folder,
                    GetBaseLogName(trroll_path));

                // Make sure that the log is initialised in local storage.
                log_path = log_base + ".log";
                auto& log = trrojan::log::instance(log_path.c_str());
                log_path = trrojan::get_file_name(log_path);

                trrojan::log::instance().write_line(
                    trrojan::log_level::information,
                    "Running user-selected TRROLL script {}.",
                    trroll_path);
                trrojan::log::instance().write_line(
                    trrojan::log_level::information,
                    "Intermediate output and log files are written to {0}.",
                    log_folder);

                // Build the command line.
                trrojan::cmd_line cmd_line;
                cmd_line.push_back("--output");
                cmd_line.push_back((output_path = log_base + ".csv"));
                output_path = trrojan::get_file_name(output_path);

                // Configure the executive.
                trrojan::executive exe(window);
                exe.load_plugins(cmd_line);

                // Prepare the global parameters for the test run.
                auto output = trrojan::open_output(cmd_line);
                trrojan::cool_down cool_down;
                std::shared_ptr<trrojan::power_collector> power_collector;

                // Run the test.
                exe.trroll(trroll_file, *output, cool_down, power_collector);
                } break;

            case State::PromptResult:
                // Set the two bits for the asynchronous copy operations we are
                // going to start and for the next prompt, too. Once the files
                // are moved, both bits will be erased and only 'PromptTrroll'
                // will remain.
                state = State::PromptTrroll;
#if false
                state = static_cast<State>
                    (static_cast<StateBits>(State::MovingOutput)
                    | static_cast<StateBits>(State::MovingLog)
                    | static_cast<StateBits>(State::PromptTrroll));

                trrojan::log::instance().write_line(
                    trrojan::log_level::information, "Searching for "
                    "removable disk to extract the output files ...");

                trrojan::on_completed(usb_devs.GetFoldersAsync(),
                        [&](IVectorView<StorageFolder> folders) {
                    if (folders.Size() < 1) {
                        trrojan::log::instance().write_line(
                            trrojan::log_level::error, "No output folders for "
                            "the benchmark results were found on a removable "
                            "device. All subsequent operations are aborted.");
                        state = State::Done;

                    } else {
                        auto dst = folders.GetAt(0);
                        auto output = winrt::to_hstring(output_path);
                        auto log = winrt::to_hstring(log_path);

                        trrojan::log::instance().write_line(
                            trrojan::log_level::information, "Delivering the "
                            "results {0} and the log{1} to {2} ...",
                            output_path,
                            log_path,
                            winrt::to_string(dst.Path()));

                        // Move the results to the first folder on the USB
                        // device atomically erase the status bit for that on
                        // completion.
                        trrojan::on_completed(temp_folder.GetFileAsync(output),
                                [dst, &on_output_succeeded, &on_output_failed](
                                StorageFile file) {
                            trrojan::on_completed(
                                file.MoveAsync(dst),
                                on_output_succeeded,
                                on_output_failed);
                        }, on_output_failed);

                        // Copy the log file and erase the bit. Note that we
                        // cannot move the log, because it is still open.
                        trrojan::on_completed(temp_folder.GetFileAsync(log),
                                [dst, &on_log_succeeded, &on_log_failed](
                                StorageFile file) {
                            trrojan::on_completed(
                                file.MoveAsync(dst),
                                on_log_succeeded,
                                on_log_failed);
                        }, on_log_failed);

                    } /* if (folders.Size() < 1) */
                }, on_output_failed);
#endif

#if false
                Pickers::FolderPicker picker;
                // Note: for some weird reason, the *folder* picker does not
                // work without setting a *file* filter.
                picker.FileTypeFilter().Append(L"*");

                trrojan::pick_folder_and_continue(picker,
                        [&temp_folder, &log_path, &output_path, &state](
                        StorageFolder dst) {
                    if (dst) {
                        // Open the application folder, where we have the stored
                        // the output files that are to be copied to 'f'.
                        trrojan::log::instance().write_line(
                            trrojan::log_level::information, "Delivering "
                            "output files {0} and {1} to user-selected folder "
                            "{2}.", output_path, log_path,
                            winrt::to_string(dst.Path()));

                        auto output = winrt::to_hstring(output_path);
                        auto log = winrt::to_hstring(log_path);

                        // Move the results to the specified folder and
                        // atomically erase the status bit for that on
                        // completion.
                        trrojan::on_completed(temp_folder.GetFileAsync(output),
                                [dst, &state](StorageFile file) {
                            try {
                                file.MoveAsync(dst).Completed([&state](
                                        const IAsyncAction, const AsyncStatus) {
                                    EraseBits(state, State::MovingOutput);
                                });
                            } catch (...) {
                                EraseBits(state, State::MovingLog);
                                trrojan::log::instance().write_line(
                                    trrojan::log_level::error, "Moving the "
                                    "output file failed.");
                            }
                        });

                        // Copy the log file and erase the bit. Note that we
                        // cannot move the log, because it is still open.
                        trrojan::on_completed(temp_folder.GetFileAsync(log),
                                [dst, &state](StorageFile file) {
                            try {
                                file.CopyAsync(dst).Completed([&state](
                                        const IAsyncOperation<StorageFile>,
                                        const AsyncStatus) {
                                    EraseBits(state, State::MovingLog);
                                });
                            } catch (...) {
                                EraseBits(state, State::MovingLog);
                                trrojan::log::instance().write_line(
                                    trrojan::log_level::error, "Copying the "
                                    "log file failed.");
                            }
                        });

                    } else {
                        // If the user did not provide a folder, we exit now.
                        state = State::Done;
                    }
                });
#endif
                break;
        } /* switch (state.load(std::memory_order::memory_order_consume)) */

        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
    } /* while (state.load(std::memory_order::memory_order_consume) ...  */

    dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}


//void App::SetWindow(CoreWindow const &window) {
//        //Compositor compositor;
//        //ContainerVisual root = compositor.CreateContainerVisual();
//        //m_target = compositor.CreateTargetForCurrentView();
//        //m_target.Root(root);
//        //m_visuals = root.Children();
//        //
//        //window.PointerPressed({ this, &App::OnPointerPressed });
//        //window.PointerMoved({ this, &App::OnPointerMoved });
//        //
//        //window.PointerReleased([&](auto && ...)
//        //{
//        //    m_selected = nullptr;
//        //});
//}

//void App::OnPointerPressed(IInspectable const &, PointerEventArgs const &args) {
//    float2 const point = args.CurrentPoint().Position();
//
//    for (Visual visual : m_visuals)
//    {
//        float3 const offset = visual.Offset();
//        float2 const size = visual.Size();
//
//        if (point.x >= offset.x &&
//            point.x < offset.x + size.x &&
//            point.y >= offset.y &&
//            point.y < offset.y + size.y)
//        {
//            m_selected = visual;
//            m_offset.x = offset.x - point.x;
//            m_offset.y = offset.y - point.y;
//        }
//    }
//
//    if (m_selected)
//    {
//        m_visuals.Remove(m_selected);
//        m_visuals.InsertAtTop(m_selected);
//    } else
//    {
//        AddVisual(point);
//    }
//}
//
//void OnPointerMoved(IInspectable const &, PointerEventArgs const &args)
//{
//    if (m_selected)
//    {
//        float2 const point = args.CurrentPoint().Position();
//
//        m_selected.Offset(
//            {
//                point.x + m_offset.x,
//                point.y + m_offset.y,
//                0.0f
//            });
//    }
//}
//
//void AddVisual(float2 const point)
//{
//    Compositor compositor = m_visuals.Compositor();
//    SpriteVisual visual = compositor.CreateSpriteVisual();
//
//    static Color colors[] =
//    {
//        { 0xDC, 0x5B, 0x9B, 0xD5 },
//        { 0xDC, 0xED, 0x7D, 0x31 },
//        { 0xDC, 0x70, 0xAD, 0x47 },
//        { 0xDC, 0xFF, 0xC0, 0x00 }
//    };
//
//    static unsigned last = 0;
//    unsigned const next = ++last % _countof(colors);
//    visual.Brush(compositor.CreateColorBrush(colors[next]));
//
//    float const BlockSize = 100.0f;
//
//    visual.Size(
//        {
//            BlockSize,
//            BlockSize
//        });
//
//    visual.Offset(
//        {
//            point.x - BlockSize / 2.0f,
//            point.y - BlockSize / 2.0f,
//            0.0f,
//        });
//
//    m_visuals.InsertAtTop(visual);
//
//    m_selected = visual;
//    m_offset.x = -BlockSize / 2.0f;
//    m_offset.y = -BlockSize / 2.0f;
//}


/*
 * App::EraseBits
 */
void App::EraseBits(std::atomic<State>& state, const State erase) {
    auto expected = state.load();
    auto desired = static_cast<State>(static_cast<StateBits>(expected)
        & ~static_cast<StateBits>(erase));

    while (!state.compare_exchange_weak(expected, desired)) {
        desired = static_cast<State>(static_cast<StateBits>(expected)
            & ~static_cast<StateBits>(erase));
    }
}


/*
 * App::GetBaseLogName
 */
std::string App::GetBaseLogName(const std::string& trroll) {
    const auto& factors = trrojan::system_factors::instance();
    const auto device = factors.gaming_device().get<std::string>();
    const auto now = std::chrono::system_clock::now();
    const auto timestamp = trrojan::to_string<char>(now, true);
    const auto trroll_file = trrojan::get_file_name(trroll, false);
    const auto retval = trroll_file + "_" + device + "_" + timestamp;
    return retval;
}
#endif /* defined(TRROJAN_FOR_UWP) */
