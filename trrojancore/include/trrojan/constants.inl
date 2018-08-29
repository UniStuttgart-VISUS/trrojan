/// <copyright file="constants.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>


#define __TRROJANCODE_DEF_CONST(name, value)                                   \
template<class T>                                                              \
const typename trrojan::constants<T>::type trrojan::constants<T>::name         \
= static_cast<typename trrojan::constants<T>::type>(value)

__TRROJANCODE_DEF_CONST(bits_per_byte, CHAR_BIT);
__TRROJANCODE_DEF_CONST(bytes_per_gigabyte, 1024 * 1024 * 1024);
__TRROJANCODE_DEF_CONST(bytes_per_kilobyte, 1024);
__TRROJANCODE_DEF_CONST(bytes_per_megabyte, 1024 * 1024);
__TRROJANCODE_DEF_CONST(millis_per_second, 1000);
__TRROJANCODE_DEF_CONST(pi, 3.14159265358979323846);
__TRROJANCODE_DEF_CONST(seconds_per_hour, 60 * 60);
__TRROJANCODE_DEF_CONST(seconds_per_minute, 60);

#undef __TRROJANCODE_DEF_CONST
