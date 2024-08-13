// <copyright file="sphere_rendering_configuration.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <string>
#include <vector>

#include "trrojan/configuration.h"

#include "trrojan/d3d12/utilities.h"


/* Forward declarations. */
struct SphereConstants;
struct TessellationConstants;
struct ViewConstants;


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Composable container for processing the factors of the sphere rendering.
    /// </summary>
    class TRROJAND3D12_API sphere_rendering_configuration final {

    public:

        /// <summary>
        /// Type for edge tessellation factors.
        /// </summary>
        typedef std::array<float, 4> edge_tess_factor_type;

        /// <summary>
        /// Type identifying a frame index.
        /// <s/ummary>
        typedef std::uint32_t frame_type;

        /// <summary>
        /// Type for inside tessellation factors.
        /// </summary>
        typedef std::array<float, 2> inside_tess_factor_type;

        /// <summary>
        /// An identifier for a single shader variant.
        /// </summary>
        /// <remarks>
        /// The shader ID is a bitmask that captures the properties of the
        /// requested rendering technique along with the particularities of the
        /// data set.
        /// </remarks>
        typedef std::uint64_t shader_id_type;

        /// <summary>
        /// Gets the names of all sphere rendering methods.
        /// </summary>
        static std::vector<std::string> get_methods(void);

        /// <summary>
        /// Gets the identifier for the given rendering method.
        /// </summary>
        /// <remarks>
        /// This method only retrieves the base code of the technique with the
        /// specified name. The code returned does not contain any flags
        /// modifying the behaviour, which can be specified in the
        /// configuration, nor does it contain any data-dependent features.
        /// </remarks>
        /// <param name="method">The name of the rendering method to get the ID
        /// for.</param>
        /// <returns>The ID of the requested shader, or 0 if no such shader was
        /// found.</returns>
        static shader_id_type get_shader_id(const std::string& method);

        static const char *factor_adapt_tess_maximum;
        static const char *factor_adapt_tess_minimum;
        static const char *factor_adapt_tess_scale;
        static const char *factor_conservative_depth;
        static const char *factor_data_set;
        static const char *factor_edge_tess_factor;
        static const char *factor_fit_bounding_box;
        static const char *factor_force_float_colour;
        static const char *factor_frame;
        static const char *factor_gpu_counter_iterations;
        static const char *factor_hemi_tess_scale;
        static const char *factor_inside_tess_factor;
        static const char *factor_method;
        static const char *factor_min_prewarms;
        static const char *factor_min_wall_time;
        static const char *factor_poly_corners;
        static const char *factor_vs_raygen;
        static const char *factor_vs_xfer_function;

        /// <summary>
        /// Initialises a new instance by extracting the properties from a
        /// configuration.
        /// </summary>
        explicit sphere_rendering_configuration(const configuration& config);

        /// <summary>
        /// Answer the maximum allowable tessellation factor when using adaptive
        /// tessellation.
        /// </summary>
        inline float adapt_tess_maximum(void) const noexcept {
            return this->_adapt_tess_maximum;
        }

        /// <summary>
        /// Answer the minimum allowable tessellation factor when using adaptive
        /// tessellation.
        /// </summary>
        inline float adapt_tess_minimum(void) const noexcept {
            return this->_adapt_tess_minimum;
        }

        /// <summary>
        /// Answer the scaling factor that yields the tessellation factor when
        /// applied to the distance when using adaptive tessellation.
        /// </summary>
        inline float adapt_tess_scale(void) const noexcept {
            return this->_adapt_tess_scale;
        }

        /// <summary>
        /// Answer whether conservative depth output is enabled.
        /// </summary>
        inline bool conservative_depth(void) const noexcept {
            return this->_conservative_depth;
        }

        /// <summary>
        /// Answer the path or definition of the data set.
        /// </summary>
        inline std::string data_set(void) const noexcept {
            return this->_data_set;
        }

        /// <summary>
        /// Answer the edge tessellation factor when using constant
        /// tessellation.
        /// </summary>
        inline edge_tess_factor_type edge_tess_factor(void) const noexcept {
            return this->_edge_tess_factor;
        }

        /// <summary>
        /// Answer whether the bounding box of MMPLD data sets should be
        /// recomputed on load instead of relying on what is in the file.
        /// </summary>
        inline bool fit_bounding_box(void) const noexcept {
            return this->_fit_bounding_box;
        }

        /// <summary>
        /// Answer whether all colour data should be forced to floating-point
        /// data even if the input used integral numbers.
        /// </summary>
        inline bool force_float_colour(void) const noexcept {
            return this->_force_float_colour;
        }

        /// <summary>
        /// Answer the number of the frame to be loaded.
        /// </summary>
        inline frame_type frame(void) const noexcept {
            return this->_frame;
        }

        /// <summary>
        /// Populates the tessellation constants from the configuration.
        /// </summary>
        void get_tessellation_constants(TessellationConstants& dst) const;

        /// <summary>
        /// Answer how many iterations should be rendered for measuring the
        /// timings using GPU-based queries.
        /// </summary>
        inline unsigned int gpu_counter_iterations(void) const noexcept {
            return this->_gpu_counter_iterations;
        }

        /// <summary>
        /// Asnwer the scaling factor for tessellation when rendering
        /// hemispheres instead of full ones.
        /// </summary>
        inline float hemi_tess_scale(void) const noexcept {
            return this->_hemi_tess_scale;
        }

        /// <summary>
        /// Answer the inside tessellation factor when using tessellation.
        /// </summary>
        inline inside_tess_factor_type inside_tess_factor(void) const noexcept {
            return this->_inside_tess_factor;
        }

        /// <summary>
        /// Answer the name of the rendering method.
        /// </summary>
        inline const std::string& method(void) const noexcept {
            return this->_method;
        }

        /// <summary>
        /// Answer the number of iterations to render for prewarming without
        /// measuring.
        /// </summary>
        inline unsigned int min_prewarms(void) const noexcept {
            return this->_min_prewarms;
        }

        /// <summary>
        /// Answer the minimum number of milliseconds that need to be rendered
        /// for measuring the frame rate using the wall-time clock.
        /// </summary>
        inline unsigned int min_wall_time(void) const noexcept {
            return this->_min_wall_time;
        }

        /// <summary>
        /// Answer the number of corners of the polygon sprite.
        /// </summary>
        inline unsigned int poly_corners(void) const noexcept {
            return this->_poly_corners;
        }

        /// <summary>
        /// Compute the shader ID for this configuration.
        /// </summary>
        /// <remarks>
        /// The method only retrieves the method code and method-dependent
        /// features, but not any data-dependent features. These must be added
        /// later. Note that some flags in the returned code are set
        /// speculatively and might need to be erased if they are not relevant
        /// for the data being visualised.
        /// </remarks>
        shader_id_type shader_id(void) const;

        /// <summary>
        /// Answer whether the rays should be computed in the vertex shader and
        /// interpolated instead of using per-pixel computation.
        /// </summary>
        inline bool vs_raygen(void) const noexcept {
            return this->_vs_raygen;
        }

        /// <summary>
        /// Answer whether the transfer function should be applied in the vertex
        /// shader rather than in the pixel shader.
        /// </summary>
        inline bool vs_xfer_function(void) const noexcept {
            return this->_vs_xfer_function;
        }

    private:

        float _adapt_tess_maximum;
        float _adapt_tess_minimum;
        float _adapt_tess_scale;
        bool _conservative_depth;
        std::string _data_set;
        edge_tess_factor_type _edge_tess_factor;
        bool _fit_bounding_box;
        bool _force_float_colour;
        frame_type _frame;
        unsigned int _gpu_counter_iterations;
        float _hemi_tess_scale;
        inside_tess_factor_type _inside_tess_factor;
        std::string _method;
        unsigned int _min_prewarms;
        unsigned int _min_wall_time;
        unsigned int _poly_corners;
        bool _vs_raygen;
        bool _vs_xfer_function;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
