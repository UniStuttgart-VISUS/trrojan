// <copyright file="random_sphere_cache.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::random_sphere_cache<TKey, TUserData>::get
 */
template<class TKey, class TUserData>
typename trrojan::random_sphere_cache<TKey, TUserData>::value_type *
trrojan::random_sphere_cache<TKey, TUserData>::get(const key_type& key) {
    auto it = this->_files.find(key);
    return (it != this->_files.end()) ? it->second.get() : nullptr;
}


/*
 * trrojan::random_sphere_cache<TKey, TUserData>::put
 */
template<class TKey, class TUserData>
typename trrojan::random_sphere_cache<TKey, TUserData>::value_type *
trrojan::random_sphere_cache<TKey, TUserData>::put(const key_type& key,
        const std::string& path) {
    auto& retval = this->_files[key];
    retval.reset(new value_type());
    retval->file = temp_file::from_path(path);
    return retval.get();
}
