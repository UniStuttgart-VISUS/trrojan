#include <iostream>
#include <numeric>

#include "trrojan/configuration_set.h"
#include "trrojan/executive.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"


int main(const int argc, const char **argv) {
    trrojan::cmd_line cmdLine(argv, argv + argc);

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

    return 0;
}
