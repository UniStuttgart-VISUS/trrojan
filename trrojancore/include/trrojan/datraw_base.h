// <copyright file="datraw_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "datraw.h"

#include "trrojan/camera.h"
#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// A base class that provices handling of datraw files, eg for use with
    /// volume rendering benchmarks.
    /// </summary>
    class TRROJANCORE_API datraw_base {

    public:

        /// <summary>
        /// The type used to specify frames of a dat/raw file.
        /// </summary>
        typedef datraw::raw_reader<char>::time_step_type frame_type;

        /// <summary>
        /// The dat volume information type used in the TRRojan.
        /// </summary>
        typedef datraw::info<char> info_type;

        /// <summary>
        /// The reader for raw files used in the TRRojan.
        /// </summary>
        typedef datraw::raw_reader<char> reader_type;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~datraw_base(void) = default;

    protected:

        /// <summary>
        /// Compute a step size for a bounding box scaled to [0, 1] such that
        /// each texel on the longest edge of the data set is sampled once.
        /// </summary>
        /// <param name="info">The volume info to compute the step size for.
        /// </param>
        /// <returns>The suggested base step size for the data set.</returns>
        static float calc_base_step_size(const info_type& info);

        /// <summary>
        /// Computes the &quot;physical size&quot; of the data set, which is
        /// determined by the resolution of the volume and the distance between
        /// the slices. The result is the desired size of the bounding box.
        /// </summary>
        static std::array<float, 3> calc_physical_size(const info_type& info);

        /// <summary>
        /// Compute a non-uniform scaling factor for the ray based on the slice
        /// distances of the volume, which is basically the reciprocal of the
        /// distances.
        /// </summary>
        static std::array<float, 3> calc_ray_scale(const info_type& info);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline datraw_base(void) = default;

        /// <summary>
        /// Computes the base step size for the currently loaded volume.
        /// </summary>
        inline float calc_base_step_size(void) const {
            return datraw_base::calc_base_step_size(this->_volume_info);
        }

        /// <summary>
        /// Computes the bounding box of edge points of
        /// <see cref="volume_info" /> being centred in the world.
        /// </summary>
        void calc_bounding_box(glm::vec3& outStart, glm::vec3& outEnd) const;

        /// <summary>
        /// Make a suggestion for near and far clipping plane for
        /// <see cref="volume_info" /> shown by the given
        /// <see cref="camera" />.
        /// </summary>
        /// <remarks>
        /// The camera must be fully configured for the final view before this
        /// method can be called. Otherwise, the results might be unexpected.
        /// </remarks>
        std::pair<float, float> calc_clipping_planes(const camera& cam) const;

        /// <summary>
        /// Computes the desired size of the bounding box for the volume
        /// described in <see cref="volume_info" />.
        /// </summary>
        inline std::array<float, 3> calc_physical_volume_size(void) const {
            return datraw_base::calc_physical_size(this->_volume_info);
        }

        /// <summary>
        /// Computes the non-uniform scaling factor of the ray for the slice
        /// distances of the volume descriibed in <see cref="volume_info" />.
        /// </summary>
        inline std::array<float, 3> calc_ray_scale(void) const {
            return datraw_base::calc_ray_scale(this->_volume_info);
        }

        /// <summary>
        /// Gets the number of voxels in the currently loaded data set.
        /// </summary>
        inline std::array<std::uint32_t, 3> get_volume_resolution(void) const {
            return std::array<std::uint32_t, 3> {
                this->_volume_info.resolution()[0],
                this->_volume_info.resolution()[1],
                this->_volume_info.resolution()[2]
            };
        }

        /// <summary>
        /// Stores the dat file currently being processed.
        /// </summary>
        info_type _volume_info;
    };
}
