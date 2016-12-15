#include <iostream>

#include "trrojan/factor.h"
#include "trrojan/factorator.h"
#include "trrojan/variant.h"


int main(const int argc, const char **argv) {
    trrojan::variant x;

    x = "hugo 12";

    auto xt = x.type();

    auto y = x;

    y = 12;

    auto yt = y.type();

    auto z = std::move(x);


    std::vector<int> values1 = { 1, 2, 3, 4, 5 };
    auto factor1 = trrojan::factor::from_manifestations("factor1", values1);
    std::cout << factor1.name() << " " << factor1.size() << std::endl;
    for (size_t i = 0; i < factor1.size(); ++i) {
        std::cout << factor1[i] << std::endl;
    }
    for (auto it = trrojan::factorator(factor1), end = trrojan::factorator(factor1, true); it != end; ++it) {
        std::cout << *it << std::endl;
    }

    auto factor2 = trrojan::factor::from_manifestations("factor2", values1.begin(), values1.end());
    std::cout << factor2.name() << " " << factor2.size() << std::endl;
    for (size_t i = 0; i < factor2.size(); ++i) {
        std::cout << factor2[i] << std::endl;
    }

    auto factor3 = trrojan::factor::from_manifestations("factor3", std::move(values1));
    std::cout << factor3.name() << " " << factor3.size() << std::endl;
    for (size_t i = 0; i < factor3.size(); ++i) {
        std::cout << factor3[i] << std::endl;
    }

    auto factor4 = trrojan::factor::from_steps("factor4", int(10), int(5), 3);
    std::cout << factor4.name() << " " << factor4.size() << std::endl;
    for (size_t i = 0; i < factor4.size(); ++i) {
        std::cout << factor4[i] << std::endl;
    }

    auto factor5 = trrojan::factor::from_range("factor5", int(0), int(10), 4);
    std::cout << factor5.name() << " " << factor5.size() << std::endl;
    for (size_t i = 0; i < factor5.size(); ++i) {
        std::cout << factor5[i] << std::endl;
    }

    return 0;
}
