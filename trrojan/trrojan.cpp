#include <iostream>
#include <numeric>

#include "trrojan/factor.h"
#include "trrojan/factorator.h"
#include "trrojan/variant.h"


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

    std::vector<trrojan::factor> configuration = { fVolumeSizes, fStepSizes, fVolumes };
    std::vector<size_t> offset;
    std::transform(configuration.cbegin(), configuration.cend(),
        std::back_inserter(offset), [](const trrojan::factor& f) {return f.size(); });
    //for (size_t)

    auto cntTests = std::accumulate(configuration.cbegin(), configuration.cend(), 1,
        [](size_t sum, const trrojan::factor& f) { return sum * f.size(); });

    for (size_t i = 0; i < cntTests; ++i) {

    }



    return 0;
}
