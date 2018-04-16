/// <copyright file="scripting_host.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "scripting_host.h"

#include <climits>
#include <cstring>
#include <ctime>
#include <sstream>
#include <stdexcept>

#include "trrojan/benchmark.h"
#include "trrojan/configuration.h"
#include "trrojan/executive.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/scripting_exception.h"
#include "trrojan/text.h"
#include "trrojan/variant.h"

#if 0
void CALLBACK PromiseContinuationCallback(JsValueRef task, void *callbackState) {
    // Save promises in taskQueue.
    JsValueRef global;
    JsGetGlobalObject(&global);
    queue<Task*> * q = (queue<Task*> *)callbackState;
    q->push(new Task(task, 0, global, JS_INVALID_REFERENCE));
}
#endif


#if 0
/*
 * trrojan::scripting_host::task::task
 */
trrojan::scripting_host::task::task(JsValueRef func, const int delay,
        JsValueRef thisArg, JsValueRef extraArgs, const bool repeat)
    : args({ thisArg, extraArgs }), cntArgs(1), delay(delay), func(func),
        repeat(repeat), time(0) {
    this->time = ::clock() / static_cast<double>(CLOCKS_PER_SEC / 1000);

    ::JsAddRef(this->func, nullptr);
    ::JsAddRef(this->args[0], nullptr);

    if (extraArgs != JS_INVALID_REFERENCE) {
        ::JsAddRef(this->args[1], nullptr);
        this->cntArgs = 2;
    }
}


/*
 * trrojan::scripting_host::task::~task
 */
trrojan::scripting_host::task::~task(void) {
    ::JsRelease(this->func, nullptr);
    ::JsRelease(this->args[0], nullptr);

    if (this->args[1] != JS_INVALID_REFERENCE) {
        ::JsRelease(this->args[1], nullptr);
    }
}


/*
 * trrojan::scripting_host::task::invoke
 */
JsValueRef trrojan::scripting_host::task::invoke(void) {
    JsValueRef retval = JS_INVALID_REFERENCE;
    ::JsCallFunction(this->func, this->args.data(), this->cntArgs, &retval);
    return retval;
}
#endif



/*
 * trrojan::scripting_host::scripting_host
 */
trrojan::scripting_host::scripting_host(void)
#if defined(WITH_CHAKRA)
        : currentSourceContext(0), runtime(JS_INVALID_RUNTIME_HANDLE) {
    JsContextRef context;

    /* Create the runtime and a single execution context. */
    {
        auto r = ::JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create JavaScript "
                "runtime.");
        }
    }

    {
        auto r = ::JsCreateContext(runtime, &context);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create JavaScript "
                "execution context.");
        }
    }

    {
        auto r =::JsSetCurrentContext(context);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to set JavaScript execution "
                "context.");
        }
    }

    /* Set up ES6 promise. */
    //if (JsSetPromiseContinuationCallback(PromiseContinuationCallback, &taskQueue) != JsNoError)
    //throw "failed to set PromiseContinuationCallback.";

    /* Project immutable native objects and methods into JavaScript. */
    {
        auto logLevel = scripting_host::project_object(L"LogLevel");

        {
            auto constant = scripting_host::project_value(
                static_cast<int>(log_level::debug));
            scripting_host::project_property(logLevel, L"DEBUG", constant);
        }

        {
            auto constant = scripting_host::project_value(
                static_cast<int>(log_level::verbose));
            scripting_host::project_property(logLevel, L"VERBOSE", constant);
        }

        {
            auto constant = scripting_host::project_value(
                static_cast<int>(log_level::information));
            scripting_host::project_property(logLevel, L"INFORMATION",
                constant);
        }

        {
            auto constant = scripting_host::project_value(
                static_cast<int>(log_level::warning));
            scripting_host::project_property(logLevel, L"WARNING", constant);
        }

        {
            auto constant = scripting_host::project_value(
                static_cast<int>(log_level::error));
            scripting_host::project_property(logLevel, L"ERROR", constant);
        }

        if (scripting_host::configPrototype == JS_INVALID_REFERENCE) {
            std::map<std::wstring, JsNativeFunction> methods;
            methods[L"add"] =  scripting_host::on_configuration_add;
            scripting_host::configPrototype = scripting_host::project_class(
                L"Configuration", scripting_host::on_configuration_ctor,
                methods);
        }
    }
#else /* defined(WITH_CHAKRA) */
{
#endif /* defined(WITH_CHAKRA) */
}


/*
 * trrojan::scripting_host::~scripting_host
 */
trrojan::scripting_host::~scripting_host(void) {
#if defined(WITH_CHAKRA)
    if (this->runtime) {
        ::JsDisposeRuntime(this->runtime);
    }
#endif /* defined(WITH_CHAKRA) */
}


/*
 * trrojan::scripting_host::run_code
 */
void trrojan::scripting_host::run_code(trrojan::executive& exe,
        const char_type *code) {
#if defined(WITH_CHAKRA)
    bench_list benchmarks;
    env_list environments;
    JsValueRef result;

    exe.get_benchmarks(std::back_inserter(benchmarks));
    exe.get_environments(std::back_inserter(environments));

    auto executive = scripting_host::project_object(L"trrojan");

    scripting_host::project_method(executive, L"benchmarks",
        scripting_host::on_trrojan_benchmarks, &benchmarks);
    scripting_host::project_method(executive, L"environments",
        scripting_host::on_trrojan_environments, &environments);
    scripting_host::project_method(executive, L"log",
        scripting_host::on_trrojan_log, nullptr);

    auto r = ::JsRunScript(code, this->currentSourceContext++, L"", &result);
    ::JsCollectGarbage(this->runtime);

    if (r != JsNoError) {
        JsValueRef exception;
        JsPropertyIdRef msgName;
        JsValueRef msgValue;

        {
            auto r = ::JsGetAndClearException(&exception);
            if (r != JsNoError) {
                throw scripting_exception(r, "Failed to retrieve JavaScript "
                    "exception.");
            }
        }

        {
            auto r = ::JsGetPropertyIdFromName(L"message", &msgName);
            if (r != JsNoError) {
                throw scripting_exception(r, "Failed to retrieve error message "
                    "ID.");
            }
        }

        {
            auto r = ::JsGetProperty(exception, msgName, &msgValue);
            if (r != JsNoError) {
                throw scripting_exception(r, "Failed to retrieve error "
                    "message.");
            }
        }

        auto message = scripting_host::get_string(msgValue);
        throw scripting_exception(r, message.data());
    }

#else  /* defined(WITH_CHAKRA) */
    throw std::logic_error("Cannot run JavaScript, because this version of "
        "TRRojan does not support scripting.");

#endif /* defined(WITH_CHAKRA) */
#if 0
    try {
        JsValueRef result;
        JsValueRef promiseResult;


        // Run the script.
        if (JsRunScript(script.c_str(), currentSourceContext++, L"", &result) != JsNoError) {
            // Get error message
            JsValueRef exception;
            if (JsGetAndClearException(&exception) != JsNoError)
                return L"failed to get and clear exception";

            JsPropertyIdRef messageName;
            if (JsGetPropertyIdFromName(L"message", &messageName) != JsNoError)
                return L"failed to get error message id";

            JsValueRef messageValue;
            if (JsGetProperty(exception, messageName, &messageValue))
                return L"failed to get error message";

            const wchar_t *message;
            size_t length;
            if (JsStringToPointer(messageValue, &message, &length) != JsNoError)
                return L"failed to convert error message";

            return message;
        }

        // Execute tasks stored in taskQueue
        while (!taskQueue.empty()) {
            Task* task = taskQueue.front();
            taskQueue.pop();
            int currentTime = clock() / (double) (CLOCKS_PER_SEC / 1000);
            if (currentTime - task->_time > task->_delay) {
                task->invoke();
                if (task->_repeat) {
                    task->_time = currentTime;
                    taskQueue.push(task);
                }
                else {
                    delete task;
                }
            }
            else {
                taskQueue.push(task);
            }
        }

        // Convert the return value to wstring.
        JsValueRef stringResult;
        const wchar_t *returnValue;
        size_t stringLength;
        if (JsConvertValueToString(result, &stringResult) != JsNoError)
            return L"failed to convert value to string.";
        if (JsStringToPointer(stringResult, &returnValue, &stringLength) != JsNoError)
            return L"failed to convert return value.";
        return returnValue;
    } catch (...) {
        return L"chakrahost: fatal error: internal error.\n";
    }
#endif
}


/*
 * trrojan::scripting_host::run_script
 */
void trrojan::scripting_host::run_script(trrojan::executive& exe,
        const std::string& path) {
    auto code = read_text_file(path);
    this->run_code(exe, from_utf8(code));
}


#if defined(WITH_CHAKRA)
/*
 * trrojan::scripting_host::call
 */
JsValueRef trrojan::scripting_host::call(JsValueRef object,
        const char_type *name, JsValueRef *args, const unsigned short cntArgs) {
    assert(object != JS_INVALID_REFERENCE);
    assert(name != nullptr);
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto func = scripting_host::unproject_property(object, name);

    auto r = ::JsCallFunction(func, args, cntArgs, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to call JavaScript function.");
    }

    return retval;
}


/*
 *  trrojan::scripting_host::get_bool
 */
bool trrojan::scripting_host::get_bool(JsValueRef value) {
    assert(value != JS_INVALID_REFERENCE);
    bool retval = false;
    JsValueRef boolean = JS_INVALID_REFERENCE;

    // Make sure that the object is a Boolean value in JavaScript.
    ::JsConvertValueToBoolean(value, &boolean);

    // Retrieve the number as integer.
    ::JsBooleanToBool(boolean, &retval);

    return retval;
}


/*
 * trrojan::scripting_host::get_external_data
 */
void *trrojan::scripting_host::get_external_data(JsValueRef value) {
    assert(value != JS_INVALID_REFERENCE);
    void *retval = nullptr;

    auto r = ::JsGetExternalData(value, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to retrieve external data.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::get_int
 */
int trrojan::scripting_host::get_int(JsValueRef value) {
    assert(value != JS_INVALID_REFERENCE);
    int retval = 0;
    JsValueRef num = JS_INVALID_REFERENCE;

    // Make sure that the object is an int in JavaScript.
    ::JsConvertValueToNumber(value, &num);

    // Retrieve the number as integer.
    ::JsNumberToInt(num, &retval);

    return retval;
}


/*
 * trrojan::scripting_host::get_string
 */
std::vector<char> trrojan::scripting_host::get_string(JsValueRef value) {
    assert(value != JS_INVALID_REFERENCE);
    std::vector<char> retval;
    size_t length = 0;
    JsValueRef str = JS_INVALID_REFERENCE;

    // Convert the value to a string in JavaScript.
    ::JsConvertValueToString(value, &str);

    // Determine the required buffer length.
    ::JsCopyString(str, nullptr, 0, &length);
    retval.resize(length + 1);

    // Retrieve the string.
    ::JsCopyString(str, retval.data(), retval.size(), nullptr);

    return retval;
}


/*
 * trrojan::scripting_host::get_type
 */
JsValueType trrojan::scripting_host::get_type(JsValueRef value) {
    JsValueType retval;

    auto r = ::JsGetValueType(value, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to determine value type.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::global
 */
JsValueRef trrojan::scripting_host::global(void) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsGetGlobalObject(&retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to retrieve JavaScript's global "
            "object.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::on_configuration_add
 */
JsValueRef trrojan::scripting_host::on_configuration_add(JsValueRef callee,
        bool isConstruct, JsValueRef *arguments, unsigned short cntArguments,
        void * callbackState) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    if (cntArguments != 3) {
        throw std::runtime_error("Configuration::add must have two arguments.");
    }

    auto config = scripting_host::get_external_data<configuration>(arguments[0]);
    auto factor = scripting_host::get_string(arguments[1]);
    auto type = scripting_host::get_type(arguments[2]);

    log::instance().write_line(log_level::debug, "Adding factor \"%s\" from "
        "JavaScript ...", factor.data());

    switch (type) {
        case JsValueType::JsBoolean:
            config->add(factor.data(), scripting_host::get_bool(arguments[2]));
            break;

        case JsValueType::JsNumber:
            // TODO
            break;

        //case JsValueType::JsObject:
        //    break;

        case JsValueType::JsTypedArray:
            // TODO
            //::JsGetTypedArrayInfo()
            break;

        case JsValueType::JsString:
        default: {
            std::string value(scripting_host::get_string(arguments[2]).data());
            config->add(factor.data(), value);
            } break;
    }

    return retval;
}


/*
 * trrojan::scripting_host::on_configuration_ctor
 */
JsValueRef trrojan::scripting_host::on_configuration_ctor(JsValueRef callee,
        bool isConstruct, JsValueRef *arguments, unsigned short cntArguments,
        void *callbackState) {
    assert(isConstruct);
    JsValueRef retval = JS_INVALID_REFERENCE;

    log::instance().write_line(log_level::debug, "Creating configuration from "
        "JavaScript ...");
    {
        auto r = ::JsCreateExternalObject(new trrojan::configuration(),
            scripting_host::on_configuration_dtor, &retval);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create configuration "
                "object.");
        }
    }

    {
        auto r = ::JsSetPrototype(retval, scripting_host::configPrototype);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to assign configuration "
                "prototye.");
        }
    }

    return retval;
}


/*
 * trrojan::scripting_host::on_configuration_dtor
 */
void trrojan::scripting_host::on_configuration_dtor(void *data) {
    log::instance().write_line(log_level::debug, "Releasing configuration from "
        "JavaScript ...");
    auto config = static_cast<trrojan::configuration *>(data);
    delete config;
}


/*
 * trrojan::scripting_host::on_environment_devices
 */
JsValueRef trrojan::scripting_host::on_environment_devices(JsValueRef callee,
        bool isConstruct, JsValueRef *arguments, unsigned short cntArguments,
        void *callbackState) {
    assert(!isConstruct);
    assert(arguments != nullptr);
    assert(callbackState != nullptr);
    auto env = static_cast<trrojan::environment_base *>(callbackState);
    std::vector<device> devices;
    env->get_devices(devices);
    auto retval = scripting_host::project_array(devices.size());

    for (size_t i = 0; i < devices.size(); ++i) {
        auto e = scripting_host::project_object(devices[i].get());
        scripting_host::set_indexed_property(retval, i, e);
    }

    return retval;
}


/*
 * trrojan::scripting_host::on_trrojan_benchmarks
 */
JsValueRef trrojan::scripting_host::on_trrojan_benchmarks(JsValueRef callee,
        bool isConstruct, JsValueRef *arguments, unsigned short cntArguments,
        void *callbackState) {
    assert(!isConstruct);
    assert(arguments != nullptr);
    assert(callbackState != nullptr);
    auto elements = static_cast<bench_list *>(callbackState);
    auto retval = scripting_host::project_array(elements->size());

    for (size_t i = 0; i < elements->size(); ++i) {
        auto b = scripting_host::project_object((*elements)[i].get());
        scripting_host::set_indexed_property(retval, i, b);
    }

    return retval;
}


/*
 * trrojan::scripting_host::on_trrojan_environments
 */
JsValueRef CALLBACK trrojan::scripting_host::on_trrojan_environments(
        JsValueRef callee, bool isConstruct, JsValueRef *arguments,
        unsigned short cntArguments, void *callbackState) {
    assert(!isConstruct);
    assert(arguments != nullptr);
    assert(callbackState != nullptr);
    auto elements = static_cast<env_list *>(callbackState);
    auto retval = scripting_host::project_array(elements->size());

    for (size_t i = 0; i < elements->size(); ++i) {
        auto e = scripting_host::project_object((*elements)[i].get());
        scripting_host::set_indexed_property(retval, i, e);
    }

    return retval;
}


/*
 * trrojan::scripting_host::on_executive_log
 */
JsValueRef CALLBACK trrojan::scripting_host::on_trrojan_log(
        JsValueRef callee, bool isConstruct, JsValueRef *arguments,
        unsigned short cntArguments, void *callbackState) {
    assert(!isConstruct);
    assert(arguments != nullptr);
    // Note: The first argument is the object reference itself.
    auto level = log_level::information;    // The requested log level.
    std::stringstream msg;                  // Composes the final message.
    unsigned short offset = 1;              // Offset of first output parameter.

    if (cntArguments > 2) {
        // If there are more than two parameters in the function call, interpret
        // the first one as the log level.
        auto l = scripting_host::get_int(arguments[offset]);
        level = static_cast<log_level>(l);
        ++offset;
    }

    for (unsigned short i = offset; i < cntArguments; ++i) {
        if (i > offset) {
            msg << " ";
        }

        auto value = scripting_host::get_string(arguments[i]);
        msg << value.data();
    }

    log::instance().write_line(level, msg.str());

    return JS_INVALID_REFERENCE;
}


/*
 * trrojan::scripting_host::on_trrojan_run
 */
JsValueRef trrojan::scripting_host::on_trrojan_run(JsValueRef callee,
        bool isConstruct, JsValueRef *arguments, unsigned short cntArguments,
        void *callbackState) {
    JsValueRef retval = JS_INVALID_REFERENCE;
    // TODO
    return retval;
}


/*
 * trrojan::scripting_host::project_array
 */
JsValueRef trrojan::scripting_host::project_array(const size_t size) {
    assert(size <= UINT_MAX);
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsCreateArray(static_cast<unsigned int>(size), &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to create JavaScript array.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::project_class
 */
JsValueRef trrojan::scripting_host::project_class(const char_type *name,
        const JsNativeFunction ctor,
        const std::map<std::wstring, JsNativeFunction>& methods) {
    assert(name != nullptr);

    // Create ctor at global scope.
    auto clazz = scripting_host::project_method(scripting_host::global(), name,
        ctor);

    // Create the prototype object.
    auto retval = scripting_host::project_object();

    // Add methods to prototype.
    for (auto& m : methods) {
        scripting_host::project_method(retval, m.first.c_str(), m.second);
    }

    scripting_host::project_property(clazz, L"prototype", retval);

    return retval;
}


/*
 * trrojan::scripting_host::project_method
 */
JsValueRef trrojan::scripting_host::project_method(JsValueRef object,
        const char_type *name, JsNativeFunction callback, void *callbackState) {
    JsValueRef function = JS_INVALID_REFERENCE;

    {
        auto r = ::JsCreateFunction(callback, callbackState, &function);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create JavaScript "
                "function.");
        }
    }

    scripting_host::project_property(object, name, function);

    return function;
}


/*
 * trrojan::scripting_host::project_object
 */
JsValueRef trrojan::scripting_host::project_object(void) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    {
        auto r = ::JsCreateObject(&retval);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create JavaScript object.");
        }
    }

    return retval;
}


/*
 * trrojan::scripting_host::project_object
 */
JsValueRef trrojan::scripting_host::project_object(const char_type *name) {
    assert(name != nullptr);
    auto retval = scripting_host::project_object();
    scripting_host::project_property(scripting_host::global(), name, retval);
    return retval;
}


/*
 * trrojan::scripting_host::project_object
 */
JsValueRef trrojan::scripting_host::project_object(
        trrojan::benchmark_base *benchmark) {
    assert(benchmark != nullptr);
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsCreateExternalObject(&benchmark, nullptr, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to project benchmark to "
            "JavaScript.");
    }

    auto b = scripting_host::project_value(benchmark->name().c_str());
    scripting_host::project_property(retval, L"name", b);

    return retval;
}


/*
 * trrojan::scripting_host::project_object
 */
JsValueRef trrojan::scripting_host::project_object(device_base *dev) {
    if (dev != nullptr) {
        auto retval = scripting_host::project_object();
        scripting_host::project_property(retval, L"name",
            scripting_host::project_value(dev->name().c_str()));
        return retval;

    } else {
        // We represent the empty environment by "undefined" in JS.
        return scripting_host::undefined();
    }
}

/*
 * trrojan::scripting_host::project_object
 */
JsValueRef trrojan::scripting_host::project_object(environment_base *env) {
    if (env != nullptr) {
        auto retval = scripting_host::project_object();
        scripting_host::project_property(retval, L"name", 
            scripting_host::project_value(env->name().c_str()));
        scripting_host::project_method(retval, L"devices",
            scripting_host::on_environment_devices, env);
        return retval;

    } else {
        // We represent the empty environment by "undefined" in JS.
        return scripting_host::undefined();
    }
}


/*
 * trrojan::scripting_host::project_property
 */
void trrojan::scripting_host::project_property(JsValueRef object,
        const char_type *name, JsValueRef property) {
    assert(object != JS_INVALID_REFERENCE);
    assert(name != nullptr);
    assert(property != JS_INVALID_REFERENCE);
    JsPropertyIdRef id;

    {
        auto r = ::JsGetPropertyIdFromName(name, &id);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to retrieve JavaScript "
                "property ID from a name.");
        }
    }

    {
        auto r = ::JsSetProperty(object, id, property, true);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to set JavaScript property.");
        }
    }
}


/*
 * trrojan::scripting_host::project_value
 */
JsValueRef trrojan::scripting_host::project_value(const bool value) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsBoolToBoolean(value, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to create JavaScript Boolean "
            "value.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::project_value
 */
JsValueRef trrojan::scripting_host::project_value(const int value) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsIntToNumber(value, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to create JavaScript number.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::project_value
 */
JsValueRef trrojan::scripting_host::project_value(const double value) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsDoubleToNumber(value, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to create JavaScript number.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::project_value
 */
JsValueRef trrojan::scripting_host::project_value(const char *value) {
    auto len = (value == nullptr) ? 0 : ::strlen(value);

    if (len > 0) {
        JsValueRef retval = JS_INVALID_REFERENCE;

        auto r = ::JsCreateString(value, len, &retval);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to set JavaScript string.");
        }

        return retval;

    } else {
        return scripting_host::undefined();
    }
}


/*
 * trrojan::scripting_host::undefined
 */
JsValueRef trrojan::scripting_host::undefined(void) {
    JsValueRef retval = JS_INVALID_REFERENCE;

    auto r = ::JsGetUndefinedValue(&retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to retrieve JavaScript's "
            "undefined value.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::set_indexed_property
 */
void trrojan::scripting_host::set_indexed_property(JsValueRef property,
        const size_t index, JsValueRef value) {
    assert(index <= INT_MAX);
    auto i = scripting_host::project_value(static_cast<int>(index));
    auto r = ::JsSetIndexedProperty(property, i, value);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to set indexed property.");
    }
}


/*
 * trrojan::scripting_host::unproject_property
 */
JsValueRef trrojan::scripting_host::unproject_property(JsValueRef object,
        const char_type *name) {
    assert(object != JS_INVALID_REFERENCE);
    assert(name != nullptr);
    JsValueRef retval = JS_INVALID_REFERENCE;
    JsPropertyIdRef id;

    {
        auto r = ::JsGetPropertyIdFromName(name, &id);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to retrieve JavaScript "
                "property ID from a name.");
        }
    }

    {
        auto r = ::JsGetProperty(object, id, &retval);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to retrieve JavaScript "
                "property by its ID.");
        }
    }

    return retval;
}


/*
 * trrojan::scripting_host::configPrototype
 */
JsValueRef trrojan::scripting_host::configPrototype = JS_INVALID_REFERENCE;
#endif /* defined(WITH_CHAKRA) */
