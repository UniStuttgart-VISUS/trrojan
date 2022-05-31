// <copyright file="random_sphere_generator.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/random_sphere_generator.h"

#include <random>

#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::random_sphere_generator::create
 */
std::size_t trrojan::random_sphere_generator::create(void *dst,
        const std::size_t cnt_bytes, float& out_max_radius,
        const description &description) {
    //static const create_flags VALID_INPUT_FLAGS // Flags directly copied from user input.
        //= sphere_data_set_base::property_structured_resource;
    const auto stride = get_stride(description.sphere_type);
    const auto retval = description.number * stride;

    if (dst == nullptr) {
        return retval;
    }

    if (cnt_bytes < retval) {
        throw std::invalid_argument("The specified buffer is too small.");
    }

    const auto avg_sphere_size
        = std::abs(description.sphere_size[1] - description.sphere_size[0])
        * 0.5f + (std::min)(description.sphere_size[0],
            description.sphere_size[1]);
    std::uniform_real_distribution<float> pos_dist(0, 1);
    std::uniform_real_distribution<float> rad_dist(description.sphere_size[0],
        description.sphere_size[1]);
    std::mt19937 prng;

    //const auto properties = get_properties(sphere_type);
    //this->_properties |= (flags & VALID_INPUT_FLAGS);
    out_max_radius = std::numeric_limits<float>::lowest();

    prng.seed(description.seed);

    log::instance().write_line(log_level::verbose, "Creating {} random "
        "sphere(s) of type {} on a domain of [{}, {}, {}] with a uniformly "
        "distributed size in [{}, {}]. The random seed is {}.",
        description.number, static_cast<std::uint32_t>(description.sphere_type),
        description.domain_size[0], description.domain_size[1],
        description.domain_size[2], description.sphere_size[0],
        description.sphere_size[1], description.seed);
    for (std::size_t i = 0; i < description.number; ++i) {
        auto p = static_cast<std::uint8_t *>(dst) + (i * stride);
        auto g = static_cast<float>(i) / static_cast<float>(description.number);
        auto cur = reinterpret_cast<float *>(p);

        cur[0] = pos_dist(prng) * description.domain_size[0]
            - 0.5f * description.domain_size[0];
        cur[1] = pos_dist(prng) * description.domain_size[1]
            - 0.5f * description.domain_size[1];
        cur[2] = pos_dist(prng) * description.domain_size[2]
            - 0.5f * description.domain_size[2];
        cur += 3;

#if 0
        cur->x = cur->y = cur->z = g * domainSize[0] - 0.5f * domainSize[0];
#endif

        switch (description.sphere_type) {
            case sphere_type::pos_rad_intensity:
            case sphere_type::pos_rad_rgba32:
            case sphere_type::pos_rad_rgba8:
                *cur = rad_dist(prng);
                if (out_max_radius < *cur) {
                    out_max_radius = *cur;
                }
                ++cur;
                break;

            default:
                out_max_radius = avg_sphere_size;
        }

        switch (description.sphere_type) {
            case sphere_type::pos_intensity:
            case sphere_type::pos_rad_intensity:
                *reinterpret_cast<float *>(cur) = g;
                break;

            case sphere_type::pos_rgba32:
            case sphere_type::pos_rad_rgba32:
                cur[0] = g;
                cur[1] = g;
                cur[2] = g;
                cur[3] = 1.0f;
                break;

            case sphere_type::pos_rgba8:
            case sphere_type::pos_rad_rgba8: {
                auto s = static_cast<std::uint8_t>(g * 255);
                auto d = reinterpret_cast<std::uint8_t *>(cur);
                d[0] = d[1] = d[2] = s;
                d[3] = 255;
            } break;

            default:
                throw std::runtime_error("Unexpected sphere format.");
        }
    } /* end for (std::size_t i = 0; i < cnt_particles; ++i) */

    return retval;
}


/*
 * trrojan::random_sphere_generator::create
 */
std::vector<std::uint8_t> trrojan::random_sphere_generator::create(
        const description& description) {
    std::vector<std::uint8_t> retval(create(nullptr, 0, description));
    create(retval.data(), retval.size(), description);
    return retval;
}


/*
 * trrojan::random_sphere_generator::create
 */
std::vector<std::uint8_t> trrojan::random_sphere_generator::create(
        float& out_max_radius, const description& description) {
    std::vector<std::uint8_t> retval(create(nullptr, 0, description));
    create(retval.data(), retval.size(), out_max_radius, description);
    return retval;
}


/*
 * trrojan::random_sphere_generator::create
 */
std::size_t trrojan::random_sphere_generator::create(void *dst,
        const std::size_t cnt_bytes, const std::string& description,
        const create_flags flags) {
    return create(dst, cnt_bytes, parse_description(description, flags));
}


/*
 * trrojan::random_sphere_generator::create
 */
std::vector<std::uint8_t> trrojan::random_sphere_generator::create(
        const std::string& description, const create_flags flags) {
    return create(parse_description(description));
}


/*
 * trrojan::random_sphere_generator::get_properties
 */
trrojan::random_sphere_generator::properties_type
trrojan::random_sphere_generator::get_properties(const sphere_type type) {
    auto retval = properties_type::none;

    switch (type) {
        case sphere_type::pos_rad_intensity:
        case sphere_type::pos_rad_rgba32:
        case sphere_type::pos_rad_rgba8:
            retval |= properties_type::per_particle_radius;
            break;
    }

    switch (type) {
        case sphere_type::pos_rgba32:
        case sphere_type::pos_rad_rgba32:
            retval |= properties_type::float_colour;
            /* falls through. */
        case sphere_type::pos_rgba8:
        case sphere_type::pos_rad_rgba8:
            retval |= properties_type::per_particle_colour;
            break;
    }

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rad_intensity:
            retval |= properties_type::per_particle_intensity;
            break;
    }

    return retval;
}


/*
 * trrojan::random_sphere_generator::get_stride
 */
std::size_t trrojan::random_sphere_generator::get_stride(
        const sphere_type type) {
    std::size_t retval = 0;

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rgba8:
        case sphere_type::pos_rgba32:
            retval += 3 * sizeof(float);
            break;

        case sphere_type::pos_rad_intensity:
        case sphere_type::pos_rad_rgba8:
        case sphere_type::pos_rad_rgba32:
            retval += 4 * sizeof(float);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rad_intensity:
            retval += 1 * sizeof(float);
            break;

        case sphere_type::pos_rgba8:
        case sphere_type::pos_rad_rgba8:
            retval += 4 * sizeof(byte);
            break;

        case sphere_type::pos_rgba32:
        case sphere_type::pos_rad_rgba32:
            retval += 4 * sizeof(float);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }

    return retval;
}


/*
 * trrojan::random_sphere_generator::parse_description
 */
trrojan::random_sphere_generator::description
trrojan::random_sphere_generator::parse_description(
        const std::string& description, const create_flags flags) {
    static const std::runtime_error PARSE_ERROR("The configuration description "
        "of the random spheres is invalid. The configuration must have the "
        "following format: \"<sphere type> : <number of spheres> : <random "
        "seed or \"-\"> : <domain size> : <sphere size range>\"");
    static const char SEPARATOR = ':';
#define _ADD_SPHERE_TYPE(n) { #n, sphere_type::n }
    static const struct {
        const char *name;
        sphere_type type;
    } SPHERE_TYPES[] = {
        _ADD_SPHERE_TYPE(pos_intensity),
        _ADD_SPHERE_TYPE(pos_rgba32),
        _ADD_SPHERE_TYPE(pos_rgba8),
        _ADD_SPHERE_TYPE(pos_rad_intensity),
        _ADD_SPHERE_TYPE(pos_rad_rgba32),
        _ADD_SPHERE_TYPE(pos_rad_rgba8)
    };
#undef _ADD_SPHERE_TYPE

    random_sphere_generator::description retval;

    /* Parse the type of spheres to generate. */
    auto tok_begin = description.begin();
    auto tok_end = std::find(description.begin(), description.end(), SEPARATOR);
    if (tok_end == description.end()) {
        throw PARSE_ERROR;
    }

    auto token = tolower(trim(std::string(tok_begin, tok_end)));
    for (auto& t : SPHERE_TYPES) {
        if (token == t.name) {
            retval.sphere_type = t.type;
            break;
        }
    }

    if (retval.sphere_type == sphere_type::unspecified) {
        throw std::runtime_error("The type of random spheres to generate is "
            "invalid.");
    }

    if ((flags & create_flags::float_colour) != create_flags::none) {
        // Force 8-bit colours to float on request.
        switch (retval.sphere_type) {
            case sphere_type::pos_rgba8:
                retval.sphere_type = sphere_type::pos_rgba32;
                break;

            case sphere_type::pos_rad_rgba8:
                retval.sphere_type = sphere_type::pos_rad_rgba32;
                break;
        }
    }

    /* Parse the number of spheres to generate. */
    tok_begin = ++tok_end;
    tok_end = std::find(tok_end, description.end(), SEPARATOR);
    if (tok_end == description.end()) {
        throw PARSE_ERROR;
    }

    retval.number = parse<decltype(retval.number)>(std::string(tok_begin,
        tok_end));

    /* Parse the random seed. */
    tok_begin = ++tok_end;
    tok_end = std::find(tok_end, description.end(), SEPARATOR);
    if (tok_end == description.end()) {
        throw PARSE_ERROR;
    }

    try {
        retval.seed = parse<decltype(retval.seed)>(std::string(tok_begin,
            tok_end));
    } catch (...) {
        // Special case: use "real" random seed.
        std::random_device rnd;
        retval.seed = rnd();
    }

    /* Parse the size of the domain. */
    tok_begin = ++tok_end;
    tok_end = std::find(tok_end, description.end(), SEPARATOR);
    if (tok_end == description.end()) {
        throw PARSE_ERROR;
    }

    retval.domain_size = parse<decltype(retval.domain_size)>(
        std::string(tok_begin, tok_end));

    /* Parse the range of possible sphere sizes. */
    tok_begin = ++tok_end;
    tok_end = description.end();
    retval.sphere_size = parse<decltype(retval.sphere_size)>(
        std::string(tok_begin, tok_end));

    return retval;
}
