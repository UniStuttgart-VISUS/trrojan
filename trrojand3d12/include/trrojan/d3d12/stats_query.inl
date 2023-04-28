// <copyright file="stats_query.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

/*
 * trrojan::d3d12::stats_query::evaluate
 */
template<class TIterator>
void trrojan::d3d12::stats_query::evaluate(TIterator oit,
        const size_type buffer) const {
    if (buffer >= this->_cnt_buffers) {
        throw std::invalid_argument("The requested buffer is out of range.");
    }

    void *data;
    D3D12_RANGE range;

    range.Begin = this->result_index(buffer, 0) * sizeof(value_type);
    range.End = this->result_index(buffer, this->_cnt_queries)
        * sizeof(value_type);

    {
        auto hr = this->_result_buffer->Map(0, &range, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    auto results = reinterpret_cast<value_type *>(
        static_cast<std::uint8_t *>(data) + range.Begin);
    for (size_type i = 0; i < this->_cnt_queries; ++i) {
        *oit++ = results[i];
    }

    ::ZeroMemory(&range, sizeof(range));
    this->_result_buffer->Unmap(0, &range);
}
