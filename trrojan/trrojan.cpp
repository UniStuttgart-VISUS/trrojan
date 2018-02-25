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


int main(const int argc, const char **argv) {
    const trrojan::cmd_line cmdLine(argv, argv + argc);

    try {
        trrojan::executive exe;
        trrojan::output output;

        exe.load_plugins(cmdLine);

        {
            auto it = trrojan::find_argument("--output", cmdLine.begin(),
                cmdLine.end());
            if (it != cmdLine.end()) {
                output = trrojan::make_output(*it);
                output->open(trrojan::csv_output_params::create(*it));  // TODO: customise
            } else {
                trrojan::log::instance().write_line(trrojan::log_level::warning,
                    "You have not specified an output file. Please do so using "
                    "the --output option.");
                output = std::make_shared<trrojan::console_output>();
                output->open(trrojan::console_output_params::create());
            }
        }

        {
            auto it = trrojan::find_argument("--trroll", cmdLine.begin(),
                cmdLine.end());
            if (it != cmdLine.end()) {
                trrojan::log::instance().write_line(
                    trrojan::log_level::information, "Running benchmarks "
                    "configured in TRROLL script \"%s\"...", *it);
                exe.trroll(*it, *output);
            }
        }

        return 0;

    } catch (std::exception& ex) {
        trrojan::log::instance().write_line(ex);
        return -1;
    }

#if 0
    trrojan::timer t;
    t.start();


    auto fVolumeSizes = trrojan::factor::from_manifestations("VolumeSize", { 256, 512, 1024 });
    auto fStepSizes = trrojan::factor::from_manifestations("StepSize", { 0.75f, 1.0f, 2.0f });
    auto fVolumes = trrojan::factor::from_manifestations("Volume", { std::string("chameleon.raw "), std::string("nova.raw") });

    {
        trrojan::variant var1(std::string("hugo"));
        trrojan::variant var2 = var1;
    }

    trrojan::configuration_set cfgs;
    cfgs.add_factor(fVolumeSizes);
    cfgs.add_factor(fStepSizes);
    cfgs.add_factor(fVolumes);

    cfgs.foreach_configuration([&](const trrojan::configuration& c) {
        std::cout << std::endl << std::endl;
        for (auto& f : c) {
//            std::cout << f.name() << " = " << f.value() << std::endl;
        }
        trrojan::basic_result r1(c, { "hugo", "horst" });
        return true;
    });


    try {
        std::cout << "executive" << std::endl;
        trrojan::executive te;
        te.load_plugins(cmdLine);
        trrojan::csv_output output;
        output.open(trrojan::csv_output_params::create("test.csv"));
        te.trroll("test.trroll", output);
        //te.crowbar();
    } catch (std::exception& ex) {
        std::cout << ex.what() << std::endl;
    }

    {
        std::cout << "smbios" << std::endl;
        auto& sf = trrojan::system_factors::instance();
        std::vector<trrojan::named_variant> sfs;
        sf.get(std::back_inserter(sfs));
        for (auto& s : sfs) {
            std::cout << s << std::endl;
        }
    }


    std::cout << "elapsed: " << t.elapsed_millis() << std::endl;

#endif
    return 0;
}
