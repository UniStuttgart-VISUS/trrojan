/// <copyright file="gpu_timer.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

/*
 * include\the\graphics\directx\d3d11_performance_counter.inl
 *
 * Copyright (C) 2015 TheLib Team (http://www.thelib.org/license)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of TheLib, TheLib Team, nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THELIB TEAM AS IS AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THELIB TEAM BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */



/*
 * trrojan::d3d11::gpu_timer<L>::to_milliseconds
 */
template<size_t L> typename
trrojan::d3d11::gpu_timer<L>::millis_type
trrojan::d3d11::gpu_timer<L>::to_milliseconds(
        const difference_type value, const value_type frequency) {
    auto v = static_cast<millis_type>(value);
    auto s = trrojan::constants<millis_type>::millis_per_second;
    auto f = static_cast<millis_type>(frequency);
    return ((v * s) / f);
}


/*
 * trrojan::d3d11::gpu_timer<L>::to_milliseconds
 */
template<size_t L> typename
trrojan::d3d11::gpu_timer<L>::millis_type
trrojan::d3d11::gpu_timer<L>::to_milliseconds(
        const value_type value, const value_type frequency) {
    auto v = static_cast<millis_type>(value);
    auto s = trrojan::constants<millis_type>::millis_per_second;
    auto f = static_cast<millis_type>(frequency);
    return ((v * s) / f);
}


/*
 * trrojan::d3d11::gpu_timer<L>::infinite
 */
template<size_t L> const typename trrojan::d3d11::gpu_timer<L>::size_type
trrojan::d3d11::gpu_timer<L>::infinite
    = static_cast<typename trrojan::d3d11::gpu_timer<L>::size_type>(
    (std::numeric_limits<gpu_timer::value_type>::max)());


/*
 * trrojan::d3d11::gpu_timer<L>::~gpu_timer
 */
template<size_t L>
trrojan::d3d11::gpu_timer<L>::~gpu_timer(void) { }


/*
 * trrojan::d3d11::gpu_timer<L>::end_frame
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::end_frame(void) {
    assert(this->immediateContext != nullptr);
    assert(this->active_query().disjoint_query != nullptr);
    this->immediateContext->End(this->active_query().disjoint_query);
    this->idxActiveQuery = ++this->idxActiveQuery % L;
}


/*
 * trrojan::d3d11::gpu_timer<L>::end_frame
 */
template<size_t L>
template<class C>
typename std::enable_if<std::is_same<
    typename C::value_type,
    typename trrojan::d3d11::gpu_timer<L>::millis_type>::value,
    bool>::type
trrojan::d3d11::gpu_timer<L>::end_frame(
        std::back_insert_iterator<C> oit, const bool isLenient) {
    bool isDisjoint = false;
    value_type frequency = 0;
    bool retval = false;

    /* Issue end of frame. */
    this->end_frame();

    /* Read back performance data if possible. */
    if (this->can_evaluate()) {
        this->evaluate_frame(isDisjoint, frequency);
        if (!isDisjoint) {
            for (size_type i = 0; i < this->size(); ++i) {
                try {
                    *oit++ = gpu_timer::to_milliseconds(this->evaluate(i),
                        frequency);
                } catch (...) {
                    if (isLenient) {
                        *oit++ = static_cast<millis_type>(-1);
                    } else {
                        throw;
                    }
                }
                retval = true;
            }
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::gpu_timer<L>::end
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::end(
        const size_type query) {
    assert(this->immediateContext != nullptr);
    assert(this->active_query().timestamp_end_query.size() > query);
    assert(this->active_query().timestamp_end_query[query] != nullptr);
    this->immediateContext->End(this->active_query().timestamp_end_query[query]);
}


/*
 * trrojan::d3d11::gpu_timer<L>::evaluate
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::evaluate(
        value_type& outStart, value_type& outEnd, const size_type query) {
    if (!this->can_evaluate()) {
        throw std::runtime_error("You need to measure more frames before "
            "evaluating the GPU timer.");
    }

    while (!this->try_evaluate(outStart, outEnd, query)) {
        ::SwitchToThread();
    }
}


/*
 * trrojan::d3d11::gpu_timer<L>::evaluate
 */
template<size_t L> typename
trrojan::d3d11::gpu_timer<L>::difference_type
trrojan::d3d11::gpu_timer<L>::evaluate(
        const size_type query) {
    if (!this->can_evaluate()) {
        throw std::runtime_error("You need to measure more frames before "
            "evaluating the GPU timer.");
    }

    value_type start, end;
    while (!this->try_evaluate(start, end, query)) {
        ::SwitchToThread();
    }

    return static_cast<difference_type>(end)
        -static_cast<difference_type>(start);
}


/*
 * trrojan::d3d11::gpu_timer<L>::evaluate_frame
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::evaluate_frame(
        bool& outIsDisjoint, value_type& outFrequency,
        value_type timeout) {
    if (!this->can_evaluate()) {
        throw std::runtime_error("You need to measure more frames before "
            "evaluating the GPU timer.");
    }

    if (timeout != gpu_timer::infinite) {
        timeout += static_cast<value_type>(timer::to_millis(timer::now()));
    }

    while (!this->try_evaluate_frame(outIsDisjoint, outFrequency)) {
        if (timer::to_millis(timer::now()) > timeout) {
            throw std::runtime_error("Evaluation of GPU frame counters "
                "took too long.");
        } else {
            ::SwitchToThread();
        }
    }
}


/*
 * trrojan::d3d11::gpu_timer<L>::initialise
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::initialise(
        ID3D11Device *device, const size_type cntPerFrameQueries) {
    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device must be valid.");
    }
    if (this->device != nullptr) {
        throw std::runtime_error("The gpu_timer has already been initialised "
            "before.");
    }

    this->device = device;
    this->device->GetImmediateContext(&this->immediateContext);

    /* Create the timestamp queries. */
    this->resize(cntPerFrameQueries);
}


/*
 * trrojan::d3d11::gpu_timer<L>::resize
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::resize(
        const size_type cntPerFrameQueries) {
    for (auto& q : this->queries) {
        q.disjoint_query = nullptr;  // Mark as invalid.
        this->assert_queries(q.timestamp_end_query, cntPerFrameQueries);
        this->assert_queries(q.timestamp_start_query, cntPerFrameQueries);
    }

    this->idxActiveQuery = 0;
}


/*
 * trrojan::d3d11::gpu_timer<L>::start_frame
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::start_frame(void) {
    assert(this->immediateContext != nullptr);

    auto& q = this->active_query();

    if (q.disjoint_query == nullptr) {
        D3D11_QUERY_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;

        auto hr = this->device->CreateQuery(&desc, &q.disjoint_query);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    this->immediateContext->Begin(q.disjoint_query);
}


/*
 * trrojan::d3d11::gpu_timer<L>::start
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::start(
        const size_type query) {
    assert(this->immediateContext != nullptr);
    assert(this->active_query().timestamp_start_query.size() > query);
    assert(this->active_query().timestamp_start_query[query] != nullptr);
    this->immediateContext->End(this->active_query().timestamp_start_query[
        query]);
}


/*
 * trrojan::d3d11::gpu_timer<L>::try_evaluate
 */
template<size_t L>
bool trrojan::d3d11::gpu_timer<L>::try_evaluate(
        value_type& outStart, value_type& outEnd, const size_type query) {
    assert(this->immediateContext != nullptr);
    assert(this->passive_query().timestamp_start_query.size() > query);
    assert(this->passive_query().timestamp_end_query.size() > query);
    assert(this->passive_query().timestamp_start_query[query] != nullptr);
    assert(this->passive_query().timestamp_end_query[query] != nullptr);

    HRESULT hr = S_OK;
    auto& q = this->passive_query();

    /* Check whether we have enough frames to do the query. */
    if (q.disjoint_query == nullptr) {
        return false;
    }

    /* Try to retrieve the start point. */
    hr = this->immediateContext->GetData(q.timestamp_start_query[query],
        &outStart, sizeof(outStart), 0);
    switch (hr) {
        case S_OK:
            // Continue.
            break;

        case S_FALSE:
            return false;

        default:
            throw ATL::CAtlException(hr);
    }

    /* Try to retrieve the end point. */
    hr = this->immediateContext->GetData(q.timestamp_end_query[query],
        &outEnd, sizeof(outStart), 0);
    switch (hr) {
        case S_OK:
            // Continue.
            break;

        case S_FALSE:
            return false;

        default:
            throw ATL::CAtlException(hr);
    }

    return true;
}


/*
 * trrojan::d3d11::gpu_timer<L>::try_evaluate_frame
 */
template<size_t L>
bool trrojan::d3d11::gpu_timer<L>::try_evaluate_frame(
        bool& outIsDisjoint, value_type& outFrequency) {
    assert(this->immediateContext != nullptr);
    D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;

    auto& q = this->passive_query();

    /* Check whether we have enough frames to do the query. */
    if (q.disjoint_query == nullptr) {
        return false;
    }

    /* Do the query. */
    HRESULT hr = this->immediateContext->GetData(q.disjoint_query,
        &disjointData, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0);
    switch (hr) {
        case S_OK:
            outIsDisjoint = (disjointData.Disjoint != 0);
            outFrequency = disjointData.Frequency;
            return true;

        case S_FALSE:
            return false;

        default:
            throw ATL::CAtlException(hr);
    }
}


/*
 * trrojan::d3d11::gpu_timer<L>::assert_queries
 */
template<size_t L>
void trrojan::d3d11::gpu_timer<L>::assert_queries(
        query_list_type& list, const size_type size) {
    assert(this->device != nullptr);
    D3D11_QUERY_DESC desc;
    HRESULT hr = S_OK;

    ::ZeroMemory(&desc, sizeof(desc));
    desc.Query = D3D11_QUERY_TIMESTAMP;

    list.resize(size);
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (*it == nullptr) {
            hr = this->device->CreateQuery(&desc, &(*it));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    }
}
