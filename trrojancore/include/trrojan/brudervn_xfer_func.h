// <copyright file="brudervn_xfer_func.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <string>
#include <vector>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Load Valentin's super-fancy transfer function format.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    std::vector<std::uint8_t> TRROJANCORE_API load_brudervn_xfer_func(
        const std::string& path);

} /* namespace trrojan */
