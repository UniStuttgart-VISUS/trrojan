/// <copyright file="scripting_host.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#if defined(WITH_CHAKRA)
#include "ChakraCore.h"
#endif /* defined(WITH_CHAKRA) */


namespace trrojan {

    /* Forward declarations. */
    class benchmark_base;
    class device_base;
    class environment_base;
    class executive;

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

        /// <summary>
        /// Runs the given JavaScript code.
        /// </summary>
        /// <remarks>
        /// The scripting host projects the given
        /// <see cref="trrojan::executive" /> into the JavaScript runtime.
        /// Therefore, the caller must make sure that the object exists until
        /// this method returns and that the object is not modified while the
        /// script is runninng.
        /// </remarks>
        /// <param name="exe"></param>
        /// <param name="code"></param>
        /// <exception cref="trrojan::scripting_exception"></exception> 
        void run_code(trrojan::executive& exe, const char_type *code);

        inline void run_code(trrojan::executive& exe,
                const std::basic_string<char_type>& code) {
            this->run_code(exe, code.c_str());
        }

        void run_script(trrojan::executive& exe, const std::string& path);

    private:

#if defined(WITH_CHAKRA)
        typedef std::vector<std::shared_ptr<trrojan::benchmark_base>> bench_list;
        typedef std::vector<std::shared_ptr<trrojan::environment_base>> env_list;

        static JsValueRef call(JsValueRef object, const char_type *name,
            JsValueRef *args, const unsigned short cntArgs);

        static bool get_bool(JsValueRef value);

        static void *get_external_data(JsValueRef value);

        template<class T> static inline T *get_external_data(JsValueRef value) {
            return static_cast<T *>(scripting_host::get_external_data(value));
        }

        static int get_int(JsValueRef value);

        static std::vector<char> get_string(JsValueRef value);

        static JsValueType get_type(JsValueRef value);

        static JsValueRef global(void);

        static JsValueRef CHAKRA_CALLBACK on_configuration_add(JsValueRef callee,
            bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_configuration_ctor(JsValueRef callee,
            bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static void CHAKRA_CALLBACK on_configuration_dtor(void *data);

        static JsValueRef CHAKRA_CALLBACK on_environment_devices(
            JsValueRef callee, bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_trrojan_benchmarks(
            JsValueRef callee, bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_trrojan_environments(
            JsValueRef callee, bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_trrojan_log(JsValueRef callee,
            bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_trrojan_run(JsValueRef callee,
            bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef project_array(const size_t size);

        static JsValueRef project_class(const char_type *name,
            const JsNativeFunction ctor,
            const std::map<std::wstring, JsNativeFunction>& methods);

        static JsValueRef project_method(JsValueRef object,
            const char_type *name, JsNativeFunction callback,
            void *callbackState = nullptr);

        static JsValueRef project_object(void);

        static JsValueRef project_object(const char_type *name);

        static JsValueRef project_object(benchmark_base *benchmark);

        static JsValueRef project_object(device_base *env);

        static JsValueRef project_object(environment_base *env);

        static void project_property(JsValueRef object, const char_type *name,
            JsValueRef property);

        static JsValueRef project_value(const bool value);

        static JsValueRef project_value(const int value);

        static JsValueRef project_value(const double value);

        static JsValueRef project_value(const char *value);

        static JsValueRef undefined(void);

        static void set_indexed_property(JsValueRef property,
            const size_t index, JsValueRef value);

        static JsValueRef unproject_property(JsValueRef object,
            const char_type *name);

        static JsValueRef configPrototype;
        unsigned int currentSourceContext;
        JsRuntimeHandle runtime;
        //std::queue<task *> taskQueue;
#endif /* defined(WITH_CHAKRA) */

    };
}
