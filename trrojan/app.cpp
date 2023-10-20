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
#include <winrt/windows.storage.pickers.h>

// Does not work:
//#include <WindowsStorageCOM.h>
//#include <pplawait.h>

#include "trrojan/executive.h"
#include "trrojan/io.h"
#include "trrojan/on_exit.h"
#include "trrojan/system_factors.h"


/*
 * App::Run
 */
void App::Run(void) {
    std::atomic<State> state(State::Idle);
    auto window = CoreWindow::GetForCurrentThread();
    auto dispatcher = window.Dispatcher();
    StorageFile trroll_file(nullptr);
    std::string trroll_path;

    window.Activate();

    while (state.load(std::memory_order::memory_order_consume) != State::Done) {
        switch (state.load(std::memory_order::memory_order_consume)) {
            case State::Idle:
                // In idle state, mark the benchmark selection as pending and
                // ask the user to select a trroll script.
                state = State::Waiting;

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
                on_exit([&state](void) { state = State::Idle; });
                const auto log_base = GetBaseLogPath(trroll_path);

                // Build the command line.
                trrojan::cmd_line cmd_line;
                cmd_line.push_back("--output");
                cmd_line.push_back(log_base + ".csv");
                cmd_line.push_back("--log");
                cmd_line.push_back(log_base + ".log");

                // Configure the executive.
                trrojan::executive exe(window);
                exe.load_plugins(cmd_line);

                // Prepare the global parameters for the test run.
                auto output = trrojan::open_output(cmd_line);
                trrojan::cool_down cool_down;
                std::shared_ptr<trrojan::power_collector> power_collector;

                // Run the test.
                exe.trroll(trroll_file, *output, cool_down, power_collector);

                // TODO: Prompt user to copy output and log.
#if 0
                auto csv_output{ localFolder.GetFileAsync(trrojan::from_utf8(device + "_" + current_time + ".csv")).get() };
                winrt::Windows::Storage::StorageFolder removableFolder{ winrt::Windows::Storage::KnownFolders::RemovableDevices() };
                auto folders{ removableFolder.GetFoldersAsync().get() };
                //Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFolder> itemsInFolder{
                // };
                auto folder_cnt = folders.Size();

                if (folder_cnt > 0) {
                    auto folder = folders.GetAt(0);
                    auto copy_csv_output = csv_output.CopyAsync(folder);
                }
#endif
                } break;
        }

        dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
    }
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
 * App::GetBaseLogPath
 */
std::string App::GetBaseLogPath(const std::string& trroll) {
    const auto& factors = trrojan::system_factors::instance();
    const auto device = factors.gaming_device().get<std::string>();
    const auto now = std::chrono::system_clock::now();
    const auto storage_folder = trrojan::get_local_storage_folder();
    const auto timestamp = trrojan::to_string<char>(now, true);
    const auto trroll_file = trrojan::get_file_name(trroll, false);
    const auto base = trroll_file + "_" + device + "_" + timestamp;
    return trrojan::combine_path(storage_folder, base);
}

#endif /* defined(TRROJAN_FOR_UWP) */
