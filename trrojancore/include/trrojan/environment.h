/// <copyright file="environment.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// This class defines the interface for and some default implementation of
    /// an execution environment.
    /// </summary>
    /// <remarks>
    /// Execution environments provide the basic setup for some tests, eg by
    /// setting up OpenCL or other APIs.
    /// </remarks>
    class TRROJANCORE_API environment_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment_base(void);

        /// <summary>
        /// Gets the name of the execution environment.
        /// </summary>
        /// <returns>The name of the execution environment.</returns>
        inline const std::string& name(void) const {
            return this->_name;
        }

        /// <summary>
        /// The <see cref="trrojan::executive" /> calls this method before the
        /// environment is actually used to perform some benchmarks.
        /// </summary>
        /// <remarks>
        /// <para>The default implementation does nothing.</para>
        /// <para>This method should be used to allocate (graphics) resources.
        /// Failure should be indicated by throwing an exception.</para>
        /// </remarks>
        virtual void on_activate(void);

        /// <summary>
        /// The <see cref="trrojan::executive" /> calls this method to undo
        /// effects from <see cref="on_activate" />.
        /// </summary>
        /// <remarks>
        /// <para>The default implementation does nothing.</para>
        /// <para>This method should not throw!</para>
        /// </remarks>
        virtual void on_deactivate(void) throw();

        /// <summary>
        /// The <see cref="trrojan::executive" /> calls this method to undo
        /// effects from <see cref="on_initialise" />.
        /// </summary>
        /// <remarks>
        /// <para>The default implementation does nothing.</para>
        /// <para>The environment will not be used any more after this method
        /// has been called. All resources should be freed in the call.</para>
        /// <para>This method should not throw!</para>
        /// </remarks>
        virtual void on_finalise(void) throw();

        /// <summary>
        /// The <see cref="trrojan::executive" /> calls this method for one-time
        /// initialisation before the environment is actually used.
        /// </summary>
        /// <remarks>
        /// <para>The default implementation does nothing.</para>
        /// <para>The environment should use this method to perform
        /// initialisation tasks which might fail. Failure should be indicated
        /// by throwing an exception. If possible, (graphics) resources should
        /// be allocated on activation to prevent unused stuff being allocated
        /// while other environments being used.</para>
        /// </remarks>
        /// <param name="cmdLine"> The command line of the application
        /// invocation.</param>
        virtual void on_initialise(const std::vector<std::string>& cmdLine);

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name">The name of the environment</param>
        inline environment_base(const std::string& name) : _name(name) { }

    private:

        std::string _name;
    };

    /// <summary>
    /// An execution environment.
    /// </summary>
    typedef std::shared_ptr<environment_base> environment;
}
