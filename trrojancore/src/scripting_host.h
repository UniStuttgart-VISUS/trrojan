/// <copyright file="scripting_host.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once
#if defined(WITH_CHAKRA)

#include <array>
#include <queue>

#include "ChakraCore.h"


namespace trrojan {

    /// <summary>
    /// The scripting host is responsible for embedding the Chakra Core
    /// scripting engine.
    /// </summary>
    /// <remarks>
    /// <para>The scripting host is a core-internal feature which is not
    /// exported to TRRojan plugins. That is, the plugins cannot extend the
    /// scripting interface itself. They are to be controlled solely via
    /// scripting the test configurations.</para>
    /// <para>The <see cref="scripting_host" /> is modelled after the sample at
    /// https://github.com/Microsoft/Chakra-Samples.</para>
    /// </remarks>
    class scripting_host {

    public:

        /// <summary>
        /// Represents an ES6 task.
        /// </summary>
        class task {

        public:

            task(JsValueRef func, const int delay, JsValueRef thisArg,
                JsValueRef extraArgs, const bool repeat = false);

            ~task(void);

            JsValueRef invoke(void);

        private:

            std::array<JsValueRef, 2> args;
            int cntArgs;
            int delay;
            JsValueRef func;
            bool repeat;
            int time;
        };

        scripting_host(void);

        ~scripting_host(void);

        void run_script();


    private:

        unsigned int currentSourceContext;
        JsRuntimeHandle runtime;
        std::queue<task *> taskQueue;

    };
}

#endif /* defined(WITH_CHAKRA) */
