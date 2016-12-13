#include "..\..\trrojancore\include\trrojan\variant.h"

int main(const int argc, const char **argv) {
    trrojan::variant x;

    x = "hugo 12";

    auto xt = x.type();

    auto y = x;

    y = 12;

    auto yt = y.type();

    auto z = std::move(x);

    return 0;
}
