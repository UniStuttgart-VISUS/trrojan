/// <copyright file="scripting_host.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "scripting_host.h"

#include <ctime>
#include <sstream>
#include <stdexcept>

#include "trrojan/executive.h"
#include "trrojan/log.h"
#include "trrojan/scripting_exception.h"

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
trrojan::scripting_host::scripting_host(void) : currentSourceContext(0),
        runtime(nullptr) {
#if defined(WITH_CHAKRA)
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

    /* Project native objects and methods into JavaScript. */
    {
        auto logLevel = scripting_host::project_object(L"log_level");

        {
            auto constant = scripting_host::project_number(
                static_cast<int>(log_level::debug));
            scripting_host::project_property(logLevel, L"debug", constant);
        }

        {
            auto constant = scripting_host::project_number(
                static_cast<int>(log_level::verbose));
            scripting_host::project_property(logLevel, L"verbose", constant);
        }

        {
            auto constant = scripting_host::project_number(
                static_cast<int>(log_level::information));
            scripting_host::project_property(logLevel, L"information",
                constant);
        }

        {
            auto constant = scripting_host::project_number(
                static_cast<int>(log_level::warning));
            scripting_host::project_property(logLevel, L"warning", constant);
        }

        {
            auto constant = scripting_host::project_number(
                static_cast<int>(log_level::error));
            scripting_host::project_property(logLevel, L"error", constant);
        }


        auto executive = scripting_host::project_object(L"trrojan");

        scripting_host::project_method(executive, L"log",
            scripting_host::on_trrojan_log, nullptr);
    }
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
void trrojan::scripting_host::run_code(const char_type *code) {
#if defined(WITH_CHAKRA)
    JsValueRef result;

    auto r = ::JsRunScript(code, this->currentSourceContext++, L"", &result);
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


#if defined(WITH_CHAKRA)
/*
 * trrojan::scripting_host::get_int
 */
int trrojan::scripting_host::get_int(JsValueRef value) {
    assert(value != JS_INVALID_REFERENCE);
    int retval = 0;
    JsValueRef num;

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
    JsValueRef str;

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
 * trrojan::scripting_host::on_executive_log
 */
JsValueRef CALLBACK trrojan::scripting_host::on_trrojan_log(
        JsValueRef callee, bool isConstruct, JsValueRef *arguments,
        unsigned short cntArguments, void *callbackState) {
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
 * trrojan::scripting_host::project_method
 */
void trrojan::scripting_host::project_method(JsValueRef object,
        const char_type *name, JsNativeFunction callback, void *callbackState) {
    JsValueRef function;

    {
        auto r = ::JsCreateFunction(callback, callbackState, &function);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create JavaScript "
                "function.");
        }
    }

    scripting_host::project_property(object, name, function);
}


/*
 * trrojan::scripting_host::project_number
 */
JsValueRef trrojan::scripting_host::project_number(const int number) {
    JsValueRef retval;

    auto r = ::JsIntToNumber(number, &retval);
    if (r != JsNoError) {
        throw scripting_exception(r, "Failed to create JavaScript number.");
    }

    return retval;
}


/*
 * trrojan::scripting_host::project_object
 */
JsValueRef trrojan::scripting_host::project_object(const char_type *name) {
    assert(name != nullptr);
    JsValueRef global;
    JsValueRef retval;

    {
        auto r = ::JsCreateObject(&retval);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to create JavaScript object.");
        }
    }
    
    {
        auto r = ::JsGetGlobalObject(&global);
        if (r != JsNoError) {
            throw scripting_exception(r, "Failed to retrieve JavaScript's "
                "global object.");
        }
    }

    scripting_host::project_property(global, name, retval);

    return retval;
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
 * trrojan::scripting_host::unproject_property
 */
JsValueRef trrojan::scripting_host::unproject_property(JsValueRef object,
        const char_type *name) {
    assert(object != JS_INVALID_REFERENCE);
    assert(name != nullptr);
    JsValueRef retval;
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
#endif /* defined(WITH_CHAKRA) */
