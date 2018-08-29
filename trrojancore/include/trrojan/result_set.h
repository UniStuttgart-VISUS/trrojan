/// <copyright file="configuration.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph M�ller</author>

#pragma once

#include <vector>

#include "trrojan/result.h"


namespace trrojan {

    /// <summary>
    /// The results obtained from running one configuration.
    /// </summary>
    typedef std::vector<result> result_set;

}
