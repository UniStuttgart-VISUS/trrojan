/// <copyright file="trrojan.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include <iostream>
#include <numeric>

#include "trrojan/cmd_line.h"
#include "trrojan/console_output.h"
#include "trrojan/executive.h"
#include "trrojan/log.h"


/// <summary>
/// Entry point of the TRRojan application.
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns>Zero in case of success, -1 in case of an uncaught exception.
/// </returns>
int main(const int argc, const char **argv) {
    const trrojan::cmd_line cmdLine(argv, argv + argc);

    try {
        /* Configure the log, which must be the very first step. */
        {
            auto it = trrojan::find_argument("--log", cmdLine.begin(),
                cmdLine.end());
            if (it != cmdLine.end()) {
                // Initialise the singleton with a file sink. If this is not
                // done, the default initialisation with a console sink is done
                // lazily.
                auto l = trrojan::log::instance(it->c_str());
            }
        }

        /* Configure the output target for the results. */
        auto output = trrojan::open_output(cmdLine);

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
                    "configured in TRROLL script \"%s\" ...", *it);
                exe.trroll(*it, *output);
            }
        }

        return 0;

    } catch (std::exception& ex) {
        trrojan::log::instance().write_line(ex);
        return -1;
    }
}
