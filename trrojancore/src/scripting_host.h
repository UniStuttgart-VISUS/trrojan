/// <copyright file="scripting_host.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <queue>
#include <vector>

#if defined(WITH_CHAKRA)
#include "ChakraCore.h"
#endif /* defined(WITH_CHAKRA) */


namespace trrojan {

    /// <summary>
    /// The scripting host is responsible for embedding the Chakra Core
    /// scripting engine.
    /// </summary>
    /// <remarks>
    /// <para>The scripting host is a core-internal feature which is not
    /// exported to TRRojan plugins. That is, the plugins cannot extend the
    /// scripting interface itself. They are to be controlled solely via
    /// scripting the test configurations. Note that the global namespace
    /// of TRRojan is used rather than a detail namespace because future
    /// version of TRRojan might expose scripting functionality to the
    /// plugins.</para>
    /// <para>The <see cref="scripting_host" /> is modelled after the sample at
    /// https://github.com/Microsoft/Chakra-Samples.</para>
    /// <para>The <see cref="scripting_host" /> projects the following
    /// functionality of the TRRojan into JavaScript:
    /// log_level.debug, log_level.verbose, log_level.information,
    /// log_level.warning, log_level.error
    /// trrojan.log(level, ...)
    /// </para>
    /// </remarks>
    class scripting_host {

    public:

        typedef wchar_t char_type;

        /// <summary>
        /// Represents an ES6 task.
        /// </summary>
        //class task {

        //public:

        //    task(JsValueRef func, const int delay, JsValueRef thisArg,
        //        JsValueRef extraArgs, const bool repeat = false);

        //    ~task(void);

        //    JsValueRef invoke(void);

        //private:

        //    std::array<JsValueRef, 2> args;
        //    int cntArgs;
        //    int delay;
        //    JsValueRef func;
        //    bool repeat;
        //    int time;
        //};

        scripting_host(void);

        ~scripting_host(void);

        void run_code(const char_type *code);


    private:

#if defined(WITH_CHAKRA)
        static int get_int(JsValueRef value);

        static std::vector<char> get_string(JsValueRef value);

        static JsValueRef CALLBACK on_trrojan_log(JsValueRef callee,
            bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static void project_method(JsValueRef object, const char_type *name,
            JsNativeFunction callback, void *callbackState);

        static JsValueRef project_number(const int number);

        static JsValueRef project_object(const char_type *name);

        static void project_property(JsValueRef object, const char_type *name,
            JsValueRef property);

        static JsValueRef unproject_property(JsValueRef object,
            const char_type *name);

        unsigned int currentSourceContext;
        JsRuntimeHandle runtime;
        //std::queue<task *> taskQueue;
#endif /* defined(WITH_CHAKRA) */

    };
}
