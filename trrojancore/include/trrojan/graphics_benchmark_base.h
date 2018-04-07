/// <copyright file="graphics_benchmark_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/benchmark.h"


namespace trrojan {

    /* Forward declarations. */
    class camera;

    /// <summary>
    /// Base class for graphics-related benchmarks.
    /// </summary>
    class TRROJANCORE_API graphics_benchmark_base : public benchmark_base {

    public:

        /// <summary>
        /// The type to specify <see cref="factor_manoeuvre_step" /> and
        /// <see cref="factor_manoeuvre_steps" />.
        /// </summary>
        typedef std::uint32_t manoeuvre_step_type;

        /// <summary>
        /// The type to specify <see cref="factor_manoeuvre" />.
        /// </summary>
        typedef std::string manoeuvre_type;

        /// <summary>
        /// Type to specify a 3D point.
        /// </summary>
        typedef std::array<float, 3> point_type;

        /// <summary>
        /// The type to specify <see cref="factor_viewport" />.
        /// </summary>
        typedef std::array<std::uint32_t, 2> viewport_type;

        /// <summary>
        /// The string &quot;manoeuvre&quot; for identifying a string which
        /// specifies a pre-defined camera manoeuvre to be performed during the
        /// test.
        /// </summary>
        static const std::string factor_manoeuvre;

        /// <summary>
        /// The string &quot;manoeuvre_step&quot; for identifying an integral
        /// value identifying the current step in a pre-defined camera
        /// manoeuvre.
        /// </summary>
        static const std::string factor_manoeuvre_step;

        /// <summary>
        /// The string &quot;manoeuvre_steps&quot; for identifying the total
        /// number of steps (samples on the path) in a pre-defined camera
        /// manoeuvre.
        /// </summary>
        static const std::string factor_manoeuvre_steps;

        /// <summary>
        /// The string &quot;viewport&quot; for identifying the desired frame
        /// buffer dimensions.
        /// </summary>
        /// <remarks>
        /// The expected type of the factor is <see cref="viewport_type" />.
        /// </remarks>
        static const std::string factor_viewport;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~graphics_benchmark_base(void) = default;

    protected:

        /// <summary>
        /// Apply the pre-defined manoeuvre defined in
        /// <paramref name="camera" /> on the given
        /// <see cref="trrojan::camera" />.
        /// </summary>
        /// <param name="camera">The camera to apply the manoeuvre to.</param>
        /// <param name="config">The configuration to retrieve the factors of
        /// the manoeuvre from.</param>
        /// <param name="bboxMin">The smallest point of the data bounding box.
        /// </param>
        /// <param name="bboxMax">The largest point of the data bounding box.
        /// </param>
        /// <exception cref="std::bad_cast">If all required factors exist, but
        /// do not have the right type.</exception>
        /// <exception cref="std::runtime_error">If one of the factors is
        /// missing in in <paramref name="config" />.</exception>
        static void apply_manoeuvre(trrojan::camera& camera,
            const configuration& config, const point_type& bboxMin,
            const point_type& bboxMax);

        /// <summary>
        /// Gets the aspect ration of the viewport from the given configuration.
        /// </summary>
        /// <param name="config">The configuration to get the viewport from.
        /// </param>
        /// <returns>The aspect ratio of the viewport.</returns>
        /// <exception cref="std::bad_cast">If the viewport factor exists, but
        /// does not have the right type.</exception>
        /// <exception cref="std::runtime_error">If the viewport factor is not
        /// in <paramref name="config" />.</exception>
        static inline float get_aspect_ratio(const configuration& config) {
            auto vp = graphics_benchmark_base::get_viewport(config);
            auto w = static_cast<float>(vp[0]);
            auto h = static_cast<float>(vp[1]);
            return (w / h);
        }

        /// <summary>
        /// Gets all parameters for a pre-defined camera manoeuvre from the given
        /// configuration.
        /// </summary>
        /// <param name="outmanoeuvre"></param>
        /// <param name="outCurStep"></param>
        /// <param name="outTotalSteps"></param>
        /// <param name="config">The configuration to get the manoeuvre from.
        /// </param>
        /// <exception cref="std::bad_cast">If all required factors exist, but
        /// do not have the right type.</exception>
        /// <exception cref="std::runtime_error">If one of the factors is
        /// missing in in <paramref name="config" />.</exception>
        static void get_manoeuvre(manoeuvre_type& outManoeuvre,
            manoeuvre_step_type& outCurStep, manoeuvre_step_type& outTotalSteps,
            const configuration& config);

        /// <summary>
        /// Gets the viewport from the given configuration.
        /// </summary>
        /// <param name="config">The configuration to get the viewport from.
        /// </param>
        /// <returns>The viewport factor from the configuration.</returns>
        /// <excecption cref="std::bad_cast">If the viewport factor exists, but
        /// does not have the right type.</exception>
        /// <excecption cref="std::runtime_error">If the viewport factor is not
        /// in <paramref name="config" />.</exception>
        static inline viewport_type get_viewport(const configuration& config) {
            return config.get<viewport_type>(factor_viewport);
        }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name">The name of the benchmark, which must be unique
        /// within its plugin.</param>
        inline graphics_benchmark_base(const std::string& name)
            : benchmark_base(name) { }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name">The name of the benchmark, which must be unique
        /// within its plugin.</param>
        /// <param name="default_configs">The default configurations to be
        /// tested.</param>
        inline graphics_benchmark_base(const std::string& name,
                trrojan::configuration_set default_configs)
            : benchmark_base(name, default_configs) { }

        /// <summary>
        /// Adds a 64-step diagonal manoeuvre to the default configuration.
        /// </summary>
        /// <remarks>
        /// The constructor does not do this, because subclasses might not
        /// support, eg if they do not produce images controlled by the default
        /// camera implementation.
        /// </remarks>
        void add_default_manoeuvre(void);
    };
}
