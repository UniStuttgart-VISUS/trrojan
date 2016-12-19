#include <iostream>
#include <numeric>

#include "trrojan/configuration_set.h"


int main(const int argc, const char **argv) {
    //std::vector<int> values1 = { 1, 2, 3, 4, 5 };
    //auto factor1 = trrojan::factor::from_manifestations("factor1", values1);
    //std::cout << factor1.name() << " " << factor1.size() << std::endl;
    //for (size_t i = 0; i < factor1.size(); ++i) {
    //    std::cout << factor1[i] << std::endl;
    //}
    //for (auto it = trrojan::factorator(factor1), end = trrojan::factorator(factor1, true); it != end; ++it) {
    //    std::cout << *it << std::endl;
    //}

    //auto factor2 = trrojan::factor::from_manifestations("factor2", values1.begin(), values1.end());
    //std::cout << factor2.name() << " " << factor2.size() << std::endl;
    //for (size_t i = 0; i < factor2.size(); ++i) {
    //    std::cout << factor2[i] << std::endl;
    //}

    //auto factor3 = trrojan::factor::from_manifestations("factor3", std::move(values1));
    //std::cout << factor3.name() << " " << factor3.size() << std::endl;
    //for (size_t i = 0; i < factor3.size(); ++i) {
    //    std::cout << factor3[i] << std::endl;
    //}

    //auto factor4 = trrojan::factor::from_steps("factor4", int(10), int(5), 3);
    //std::cout << factor4.name() << " " << factor4.size() << std::endl;
    //for (size_t i = 0; i < factor4.size(); ++i) {
    //    std::cout << factor4[i] << std::endl;
    //}

    //auto factor5 = trrojan::factor::from_range("factor5", int(0), int(10), 4);
    //std::cout << factor5.name() << " " << factor5.size() << std::endl;
    //for (size_t i = 0; i < factor5.size(); ++i) {
    //    std::cout << factor5[i] << std::endl;
    //}

    auto fVolumeSizes = trrojan::factor::from_manifestations("VolumeSize", { 256, 512, 1024 });
    auto fStepSizes = trrojan::factor::from_manifestations("StepSize", { 0.75f, 1.0f, 2.0f });
    auto fVolumes = trrojan::factor::from_manifestations("Volume", { std::string("chameleon.raw "), std::string("nova.raw") });

    trrojan::configuration_set cfgs;
    cfgs.add_factor(fVolumeSizes);
    cfgs.add_factor(fStepSizes);
    cfgs.add_factor(fVolumes);

    cfgs.foreach_configuration([&](const trrojan::configuration_set::configuration& c) {
        std::cout << std::endl << std::endl;
        for (auto& f : c) {
            std::cout << f.first << " = " << f.second << std::endl;
        }
        return true;
    });


    return 0;
}