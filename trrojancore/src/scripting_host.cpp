/// <copyright file="scripting_host.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#if defined(WITH_CHAKRA)
#include "scripting_host.h"

#include <ctime>
#include <stdexcept>


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



/*
 * trrojan::scripting_host::scripting_host
 */
trrojan::scripting_host::scripting_host(void) : currentSourceContext(0),
        runtime(nullptr) { 
    if (::JsCreateRuntime(JsRuntimeAttributeNone, nullptr, &runtime)
            != JsNoError) {
        throw std::runtime_error("Failed to create JavaScript runtime.");
    }

    JsContextRef context;
    if (::JsCreateContext(runtime, &context) != JsNoError) {
        throw std::runtime_error("Failed to create JavaScript execution context.");
    }

    if (::JsSetCurrentContext(context) != JsNoError) {
        throw std::runtime_error("Failed to set JavaScript execution context.");
    }

    // Set up ES6 Promise 
    //if (JsSetPromiseContinuationCallback(PromiseContinuationCallback, &taskQueue) != JsNoError)
    //throw "failed to set PromiseContinuationCallback.";
}


/*
 * trrojan::scripting_host::~scripting_host
 */
trrojan::scripting_host::~scripting_host(void) {
    if (this->runtime) {
        ::JsDisposeRuntime(this->runtime);
    }
}


/*
 * trrojan::scripting_host::run_script
 */
void trrojan::scripting_host::run_script() {
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



#endif /* defined(WITH_CHAKRA) */

