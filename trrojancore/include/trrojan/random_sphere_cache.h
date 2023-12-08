// <copyright file="random_sphere_cache.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <unordered_map>

#include "trrojan/random_sphere_generator.h"
#include "trrojan/temp_file.h"
#include "trrojan/with_user_data.h"


namespace trrojan {

    /// <summary>
    /// Manages a cache of temporary files holding random spheres, which will be
    /// released once the instance of the cache is deleted.
    /// </summary>
    /// <typeparam name="TKey">The key for caching the spheres.</typeparam>
    /// <typeparam name="TUserData">The type of the user data attached to the
    /// file, which can, for instance, be used to cache additional metadata.
    /// </typeparam>
    template<class TKey, class TUserData>
    class random_sphere_cache final {

    public:

        /// <summary>
        /// The key for caching the spheres.
        /// </summary>
        typedef TKey key_type;

        /// <summary>
        /// The cached type, which is the path to the file.
        /// </summary>
        struct value_type final : public with_user_data<TUserData> {
            temp_file file;
        };

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        random_sphere_cache(void) = default;

        random_sphere_cache(const random_sphere_cache&) = delete;

        /// <summary>
        /// Deletes all cached files.
        /// </summary>
        inline void clear(void) {
            this->_files.clear();
        }

        /// <summary>
        /// Searches the given key in the cache.
        /// </summary>
        /// <param name="key"></param>
        /// <returns>If found, a pointer to the temporary file, which is valid
        /// as long as the cache is not cleared or deleted. <c>nullptr</c>
        /// otherwise.</returns>
        value_type *get(const key_type& key);

        /// <summary>
        /// Caches the given <paramref name="path" /> as temporary file to be
        /// deleted once the cache is cleared.
        /// </summary>
        /// <param name="key"></param>
        /// <param name="path"></param>
        /// <returns></returns>
        value_type *put(const key_type& key, const std::string& path);

        random_sphere_cache& operator =(const random_sphere_cache&) = delete;

    private:

        std::unordered_map<key_type, std::unique_ptr<value_type>> _files;
    };

} /* namespace trrojan */

#include "trrojan/random_sphere_cache.inl"
