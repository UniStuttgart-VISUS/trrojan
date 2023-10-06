// <copyright file="volume_rendering_configuration.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <string>
#include <vector>

#include "trrojan/configuration.h"
#include "trrojan/datraw_base.h"

#include "trrojan/d3d12/utilities.h"


/* Forward declarations. */
struct SphereConstants;
struct TessellationConstants;
struct ViewConstants;


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Composable container for processing the factors of the volume rendering
    /// benchmarks.
    /// </summary>
    class TRROJAND3D12_API volume_rendering_configuration final {

    public:

        /// <summary>
        /// Type identifying a frame index.
        /// <s/ummary>
        typedef datraw_base::frame_type frame_type;

        /// <summary>
        /// The type to express the step size of the volume renderer.
        /// </summary>
        typedef float step_size_type;

        ///// <summary>
        ///// Gets the names of all sphere rendering methods.
        ///// </summary>
        //static std::vector<std::string> get_methods(void);

        ///// <summary>
        ///// Gets the identifier for the given rendering method.
        ///// </summary>
        ///// <remarks>
        ///// This method only retrieves the base code of the technique with the
        ///// specified name. The code returned does not contain any flags
        ///// modifying the behaviour, which can be specified in the
        ///// configuration, nor does it contain any data-dependent features.
        ///// </remarks>
        ///// <param name="method">The name of the rendering method to get the ID
        ///// for.</param>
        ///// <returns>The ID of the requested shader, or 0 if no such shader was
        ///// found.</returns>
        //static shader_id_type get_shader_id(const std::string& method);

        static const char *factor_data_set;
        static const char *factor_ert_threshold;
        static const char *factor_frame;
        static const char *factor_fovy_deg;
        static const char *factor_gpu_counter_iterations;
        static const char *factor_max_steps;
        static const char *factor_min_prewarms;
        static const char *factor_min_wall_time;
        static const char *factor_step_size;
        static const char *factor_xfer_func;

        /// <summary>
        /// Initialises a new instance by extracting the properties from a
        /// configuration.
        /// </summary>
        explicit volume_rendering_configuration(const configuration& config);

        /// <summary>
        /// Answer the path to the datraw data set.
        /// </summary>
        inline const std::string& data_set(void) const noexcept {
            return this->_data_set;
        }

        /// <summary>
        /// Gets the opacity that needs to be accumulated to terminate the ray.
        /// </summary>
        inline float ert_threshold(void) const noexcept {
            return this->_ert_threshold;
        }

        /// <summary>
        /// Gets the zero-based frame from the datraw file that should be
        /// rendered.
        /// </summary>
        inline frame_type frame(void) const noexcept {
            return this->_frame;
        }

        /// <summary>
        /// Gets the field of view along the y-axis in degrees.
        /// </summary>
        inline float fovy_deg(void) const noexcept {
            return this->_fovy_deg;
        }

        /// <summary>
        /// Answer how many iterations should be rendered for measuring the
        /// timings using GPU-based queries.
        /// </summary>
        inline unsigned int gpu_counter_iterations(void) const noexcept {
            return this->_gpu_counter_iterations;
        }

        /// <summary>
        /// Gets the maximum number of steps the ray should advance before being
        /// terminated regardless of whether sufficient opacity has been
        /// accumulated.
        /// </summary>
        inline unsigned int max_steps(void) const noexcept {
            return this->_max_steps;
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
        /// Answer the step size to advance the ray in each iteration.
        /// </summary>
        inline step_size_type step_size(void) const noexcept {
            return this->_step_size;
        }

        /// <summary>
        /// Gets the path to the transfer function.
        /// </summary>
        inline const std::string &xfer_func(void) const noexcept {
            return this->_xfer_func;
        }

    private:

        std::string _data_set;
        float _ert_threshold;
        frame_type _frame;
        float _fovy_deg;
        unsigned int _gpu_counter_iterations;
        unsigned int _max_steps;
        unsigned int _min_prewarms;
        unsigned int _min_wall_time;
        step_size_type _step_size;
        std::string _xfer_func;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
