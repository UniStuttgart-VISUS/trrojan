// <copyright file="gpu_timer.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


///*
// * trrojan::d3d12::gpu_timer::end_frame
// */
//template<class TContainer>
//typename std::enable_if<std::is_same<typename TContainer::value_type,
//    trrojan::d3d12::gpu_timer::millis_type>::value>::type
//trrojan::d3d12::gpu_timer::end_frame(std::back_insert_iterator<TContainer> oit,
//        ID3D12GraphicsCommandList *cmd_list) {
//    const auto buffer = this->end_frame(cmd_list);
//}


/*
 * trrojan::d3d12::gpu_timer::evaluate
 */
template<class TIterator>
void trrojan::d3d12::gpu_timer::evaluate(TIterator oit,
        const size_type buffer) const {
    if (buffer >= this->_cnt_buffers) {
        throw std::invalid_argument("The requested buffer is out of range.");
    }

    const auto off = std::min(query_location::start, query_location::end);
    void *data;
    D3D12_RANGE range;

    range.Begin = this->result_index(buffer, 0, off)
        * sizeof(value_type);
    range.End = this->result_index(buffer, this->_cnt_queries, off)
        * sizeof(value_type);

    {
        auto hr = this->_result_buffer->Map(0, &range, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    auto timestamps = reinterpret_cast<value_type *>(data);
    for (size_type i = 0; i < this->size(); ++i) {
        *oit++ = timestamps[2 * i + static_cast<size_type>(query_location::end)]
            - timestamps[2 * i + static_cast<size_type>(query_location::start)];
    }

    ::ZeroMemory(&range, sizeof(range));
    this->_result_buffer->Unmap(0, &range);
}
