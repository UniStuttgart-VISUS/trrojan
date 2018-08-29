/// <copyright file="scripting_host.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <queue>
#include <stdexcept>
#include <vector>

#if defined(WITH_CHAKRA)
#include "ChakraCore.h"
#endif /* defined(WITH_CHAKRA) */

#include "trrojan/cool_down.h"
#include "trrojan/output.h"


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
    /// <list type="bullet">
    /// <item>
    ///     <term>LogLevel</term>
    ///     <description>A global object holding the possible log levels that
    /// can be passed to <c>trrojan.log</c>.</description>
    /// </item>
    /// <item>
    ///     <term>LogLevel.DEBUG</term>
    ///     <description>A constant value identifying the debugging log
    /// level.</description>
    /// </item>
    /// <item>
    ///     <term>LogLevel.VERBOSE</term>
    ///     <description>A constant value identifying the verbose log
    /// level.</description>
    /// </item>
    /// <item>
    ///     <term>LogLevel.INFORMATION</term>
    ///     <description>A constant value identifying the information log
    /// level.</description>
    /// </item>
    /// <item>
    ///     <term>LogLevel.WARNING</term>
    ///     <description>A constant value identifying the warning log
    /// level.</description>
    /// </item>
    /// <item>
    ///     <term>LogLevel.ERROR</term>
    ///     <description>A constant value identifying the error log
    /// level.</description>
    /// </item>
    /// <item>
    ///     <term>trrojan</term>
    ///     <description>The global handle providing access to the TRRojan.
    /// </description>
    /// </item>
    /// <item>
    ///     <term>trrojan.log(level, msg)</term>
    ///     <description>Writes a message to the logging facility of the
    /// TRRojan.</description>
    /// </item>
    /// <item>
    ///     <term>trrojan.benchmarks()</term>
    ///     <description>Provides a list of all loaded benchmarsk.</description>
    /// </item>
    /// <item>
    ///     <term>trrojan.environments()</term>
    ///     <description>Provides a list of all available benchmarking
    /// environments.</description>
    /// </item>
    /// </list>
    /// </para>
    /// </remarks>
    class scripting_host {

    public:

#if defined(_WIN32)
        typedef wchar_t char_type;
#else /* defined(_WIN32) */
        typedef char char_type;
#endif /* defined(_WIN32) */

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

        scripting_host(trrojan::output_base& output, const cool_down& coolDown);

        scripting_host(const scripting_host&) = delete;

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

        scripting_host& operator =(const scripting_host&) = delete;

    private:

#if defined(WITH_CHAKRA)
        typedef std::vector<std::shared_ptr<trrojan::benchmark_base>> bench_list;
        typedef std::vector<std::shared_ptr<trrojan::environment_base>> env_list;
        typedef std::map<std::basic_string<char_type>, JsNativeFunction>
            method_map_type;

        static JsValueRef call(JsValueRef object, const char_type *name,
            JsValueRef *args, const unsigned short cntArgs);

        template<class T> static bool fits_range(const double value);

        template<class T> static inline bool fits_range(
                const std::vector<double>& values) {
            return std::all_of(values.begin(), values.end(),
                [](double v) { return scripting_host::fits_range<T>(v); });
        }

        static size_t get_array_length(JsValueRef value);

        static bool get_bool(JsValueRef value);

        static double get_double(JsValueRef value);

        static void *get_ext_data(JsValueRef value);

        template<class T> static inline T *get_ext_data(JsValueRef value) {
            return static_cast<T *>(scripting_host::get_ext_data(value));
        }

        static JsValueRef get_indexed_property(JsValueRef property,
            const size_t index);

        static std::vector<JsValueRef> get_indexed_property(
            JsValueRef property);

        static int get_int(JsValueRef value);

        static std::vector<char> get_string(JsValueRef value);

        static JsValueType get_type(JsValueRef value);

        static JsValueRef global(void);

        static JsValueRef CHAKRA_CALLBACK on_configuration_set_ctor(
            JsValueRef callee, bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static void CHAKRA_CALLBACK on_configuration_set_dtor(void *data);

        static JsValueRef CHAKRA_CALLBACK on_configuration_set_set(
            JsValueRef callee, bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_environment_devices(
            JsValueRef callee, bool isConstruct, JsValueRef *arguments,
            unsigned short cntArguments, void *callbackState);

        static JsValueRef CHAKRA_CALLBACK on_trrojan_benchmark(
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
            const method_map_type& methods,
            void *ctorCallbackState = nullptr);

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

        static trrojan::variant to_variant(JsValueRef value);

        template<class T> static trrojan::variant to_variant(
            const std::vector<double>& value);

        static std::vector<trrojan::variant> to_variant_list(JsValueRef value,
            const bool isRecursion = false);

        static void unproject_exception(const JsErrorCode state);

        static JsValueRef unproject_property(JsValueRef object,
            const char_type *name);

        JsValueRef configSetPrototype;
        const cool_down& coolDown;
        unsigned int currentSourceContext;
        trrojan::executive *executive;
        trrojan::output_base& output;
        JsRuntimeHandle runtime;
        //std::queue<task *> taskQueue;
#endif /* defined(WITH_CHAKRA) */

    };
}

#include "scripting_host.inl"
