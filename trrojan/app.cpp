// <copyright file="app.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>

#if defined(TRROJAN_FOR_UWP)
#include "app.h"

#include "trrojan/executive.h"
#include "trrojan/io.h"
#include "trrojan/system_factors.h"


/*
 * App::Run
 */
void App::Run(void) {
    trrojan::cmd_line cmd_line;
    trrojan::cool_down cool_down;
    std::shared_ptr<trrojan::power_collector> power_collector;

    auto window = CoreWindow::GetForCurrentThread();
    window.Activate();

    // Generate the file name of the output file.
    const auto storage_folder = trrojan::get_local_storage_folder();
    const auto current_time = trrojan::to_string<char>(std::chrono::system_clock::now(), true);
    const auto device = trrojan::system_factors::instance().gaming_device().get<std::string>();

    // Add the output directory and the log directory to the command line.
    cmd_line.push_back("--output");
    cmd_line.push_back(storage_folder + device + "_" + current_time + ".csv");
    cmd_line.push_back("--log");
    cmd_line.push_back(storage_folder + device + "_" + current_time + ".log");

    // Configure the executive.
    trrojan::executive exe(window);
    exe.load_plugins(cmd_line);

    auto output = trrojan::open_output(cmd_line);

    std::string trroll_path;
    trroll_path = trrojan::join_path(trrojan::executive::executable_directory(),
        "Assets",
        "demo_d11.trroll");
    exe.trroll(trroll_path, *output, cool_down, power_collector);

    //// Copy from local folder to usb pen drive?
    //auto csv_output{ localFolder.GetFileAsync(trrojan::from_utf8(device + "_" + current_time + ".csv")).get() };
    //winrt::Windows::Storage::StorageFolder removableFolder{ winrt::Windows::Storage::KnownFolders::RemovableDevices() };
    //auto folders{ removableFolder.GetFoldersAsync().get() };
    ////Windows::Foundation::Collections::IVectorView<Windows::Storage::StorageFolder> itemsInFolder{
    //// };
    //auto folder_cnt = folders.Size();

    //if (folder_cnt > 0) {
    //    auto folder = folders.GetAt(0);
    //    auto copy_csv_output = csv_output.CopyAsync(folder);
    //}

    auto dispatcher = window.Dispatcher();
    dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
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

#endif /* defined(TRROJAN_FOR_UWP) */
