// <copyright file="trrojan.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>
// <author>Michael Becher</author>

#include <iostream>
#include <memory>
#include <numeric>

#if defined(TRROJAN_FOR_UWP)
#include <Windows.h>
#include <winrt/windows.applicationmodel.core.h>
#endif /* defined(TRROJAN_FOR_UWP) */

#include "trrojan/cmd_line.h"
#include "trrojan/console_output.h"
#include "trrojan/executive.h"
#include "trrojan/log.h"
#include "trrojan/power_collector.h"
#include "trrojan/power_state_scope.h"

#include "app.h"


#if defined(TRROJAN_FOR_UWP)
/// <summary>
/// Entry point of the TRRojan UWP app.
/// </summary>
/// <param name="hInst"></param>
/// <param name="hPrev"></param>
/// <param name="cmdLine"></param>
/// <param name="show"></param>
/// <returns></returns>
int __stdcall wWinMain(HINSTANCE hInst, HINSTANCE hPrev, PWSTR cmdLine,
        int show) {
    using namespace winrt;
    Windows::ApplicationModel::Core::CoreApplication::Run(make<App>());
    return 0;
}

#else /* defined(TRROJAN_FOR_UWP) */
/// <summary>
/// Entry point of the TRRojan application.
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns>Zero in case of success, -1 in case of an uncaught exception.
/// </returns>
int main(const int argc, const char **argv) {
    const trrojan::cmd_line cmdLine(argv, argv + argc);
    std::shared_ptr<trrojan::power_collector> power_collector;
    std::unique_ptr<trrojan::power_state_scope> power_state_scope;

    try {
        /* Configure the log, which must be the very first step. */
        {
            auto it = trrojan::find_argument("--log", cmdLine.begin(),
                cmdLine.end());
            if (it != cmdLine.end()) {
                // Initialise the singleton with a file sink. If this is not
                // done, the default initialisation with a console sink is done
                // lazily.
                auto& l = trrojan::log::instance(it->c_str());
            }
        }

        /* Print the copyright notice. */
        if (!trrojan::contains_switch("--nologo", cmdLine.begin(),
                cmdLine.end())) {
            std::cout << "TRRojan" << std::endl;
            std::cout << "Copyright (C) 2016 - 2023 Visualisierungsinstitut "
                "der Universität Stuttgart."
                << std::endl << std::endl;
            std::cout << "The way you're meant to be trrolled." 
                << std::endl << std::endl;
        }

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
        {
            auto it = trrojan::find_argument("--power", cmdLine.begin(),
                cmdLine.end());
            if (it != cmdLine.end()) {
                // Create and start a power collector on request.
                power_collector = std::make_shared<trrojan::power_collector>();
                power_collector->start(*it, std::chrono::milliseconds(5));
            }
        }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

        /* Configure the output target for the results. */
        auto output = trrojan::open_output(cmdLine);

        /* Determine cool-down behaviour. */
        trrojan::cool_down coolDown;
        {
            auto it = trrojan::find_argument("--cool-down-frequency",
                cmdLine.begin(), cmdLine.end());
            if (it != cmdLine.end()) {
                typedef decltype(coolDown.frequency) v_t;
                coolDown.frequency = v_t(trrojan::parse<v_t::rep>(it->c_str()));
            }
        }
        {
            auto it = trrojan::find_argument("--cool-down-duration",
                cmdLine.begin(), cmdLine.end());
            if (it != cmdLine.end()) {
                typedef decltype(coolDown.duration) v_t;
                coolDown.duration = v_t(trrojan::parse<v_t::rep>(it->c_str()));
            }
        }

        /* Configure GPU boost behaviour. */
        {
            if (trrojan::contains_switch("--stable-power-state",
                    cmdLine.begin(), cmdLine.end())) {
                power_state_scope.reset(new trrojan::power_state_scope());
            }
        }

        /* Configure skipping. */
        std::size_t continue_at = 0;
        {
            auto it = trrojan::find_argument("--continue-at",
                cmdLine.begin(), cmdLine.end());
            if (it != cmdLine.end()) {
                continue_at = trrojan::parse<std::size_t>(it->c_str());
            }
        }

        /* Configure the executive. */
        trrojan::executive exe;
        exe.load_plugins(cmdLine);

        /* Run TRROLL script if any. */
        {
            auto it = trrojan::find_argument("--trroll", cmdLine.begin(),
                cmdLine.end());
            if (it != cmdLine.end()) {
                trrojan::log::instance().write_line(
                    trrojan::log_level::information, "Running benchmarks "
                    "configured in TRROLL script \"{}\" ...", *it);
                exe.trroll(*it, *output, coolDown, continue_at,
                    power_collector);
            }
        }

        return 0;

    } catch (std::exception& ex) {
        trrojan::log::instance().write_line(ex);
        return -1;
    }
}
#endif /* defined(TRROJAN_FOR_UWP) */
