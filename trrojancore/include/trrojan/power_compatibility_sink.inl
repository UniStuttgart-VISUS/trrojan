// <copyright file="power_compatibility_sink.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for more information.
// </copyright>
// <author>Christoph Müller</author>



/*
 * trrojan::power_compatibility_sink::write_samples
 */
template<class TIterator>
void trrojan::power_compatibility_sink::write_samples(
    _In_ const TIterator begin,
    _In_ const TIterator end,
    _In_ const visus::pwrowg::sensor_description *sensors) {
    assert(sensors != nullptr);
    const auto power_uid = this->_power_uid.load(std::memory_order_acquire);

    //"sensor";"timestamp";"valid";"voltage";"current";"power";"power_uid"
    //"Tinkerforge/localhost:4223/Ufm";13301178422235;1;-3.40282e+38;-3.40282e+38;inf;1
    //"Tinkerforge/localhost:4223/UgC";13301178422235;1;-3.40282e+38;-3.40282e+38;44.698;1
    //"Tinkerforge/localhost:4223/UeW";13301178422235;1;-3.40282e+38;-3.40282e+38;inf;1
    //"ADL/ASIC/AMD Radeon PRO W6800/0";13301185622222;1;-3.40282e+38;-3.40282e+38;96;1

    if (this->_first) {
        this->_first = false;
        this->_stream
            << "\"sensor\"" << this->_delimiter
            << "\"timestamp\"" << this->_delimiter
            << "\"valid\"" << this->_delimiter
            << "\"voltage\"" << this->_delimiter
            << "\"current\"" << this->_delimiter
            << "\"power\"" << this->_delimiter
            << "\"power_uid\"" << std::endl;
    }

    for (auto it = begin; it != end; ++it) {
        auto& d = sensors[it->source];

        this->_stream
            << "\"" << to_utf8(d.id()) << "\"" << this->_delimiter
            << it->timestamp.value() << this->_delimiter
            << 1 << this->_delimiter
            << this->_delimiter
            << this->_delimiter
            << it->reading.floating_point << this->_delimiter
            << power_uid << std::endl;
    }

}
