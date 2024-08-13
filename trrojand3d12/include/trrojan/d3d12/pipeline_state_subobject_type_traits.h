// <copyright file="pipeline_state_subobject_type_traits.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <d3d12.h>

#include "trrojan/d3d12/d3dx12.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Enables resolving information about a
    /// <see cref="D3D12_PIPELINE_STATE_SUBOBJECT_TYPE" /> at compile time.
    /// </summary>
    template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type>
    struct pipeline_state_subobject_type_traits { };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_FLAGS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_VS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_GS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_STREAM_OUTPUT type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_HS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_DS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_PS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_CS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS;
    };

    //template<> struct pipeline_state_subobject_type_traits<
    //        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS> {
    //    typedef CD3DX12_PIPELINE_STATE_STREAM_AS type;
    //    static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
    //        = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS;
    //};

    //template<> struct pipeline_state_subobject_type_traits<
    //        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS> {
    //    typedef CD3DX12_PIPELINE_STATE_STREAM_MS type;
    //    static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
    //        = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS;
    //};

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1 type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL1;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_CACHED_PSO type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CACHED_PSO;
    };

    template<> struct pipeline_state_subobject_type_traits<
            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING> {
        typedef CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING type;
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value
            = D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
