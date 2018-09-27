// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef __asyncqueue_h__
#define __asyncqueue_h__

#include <stdint.h>

/// <summary>
/// A token returned when registering a callback to identify the registration. This token
/// is later used to unregister the callback.
/// </summary>
typedef uint64_t registration_token_t;

/// <summary>
/// An async_queue_t contains async work. An async queue has two sides:  a worker side and
/// a completion side.  Each side can have different rules for how queued calls
/// are dispatched.
/// </summary>
typedef struct async_queue_t* async_queue_handle_t;

/// <summary>
/// Describes how async callbacks are processed.
/// </summary>
typedef enum AsyncQueueDispatchMode
{
    /// <summary>
    /// Async callbacks are invoked manually by DispatchAsyncQueue
    /// </summary>
    AsyncQueueDispatchMode_Manual,

    /// <summary>
    /// Async callbacks are queued to the thread that created the queue
    /// and will be processed when the thread is alertable.
    /// </summary>
    AsyncQueueDispatchMode_FixedThread,

    /// <summary>
    /// Async callbacks are queued to the system thread pool and will
    /// be processed in order by the thread pool across multiple thread
    /// pool threads.
    /// </summary>
    AsyncQueueDispatchMode_ThreadPool,
    
    /// <summary>
    /// Async callbacks are queued to the system thread pool and
    /// will be processed one at a time.
    /// </summary>
    AsyncQueueDispatchMode_SerializedThreadPool,
    
    /// <summary>
    /// Async callbacks are not queued at all but are dispatched
    /// immediately by the thread that submits them.
    /// </summary>
    AsyncQueueDispatchMode_Immediate
    
} AsyncQueueDispatchMode;

/// <summary>
/// Describes the tye of async callback.
/// </summary>
typedef enum AsyncQueueCallbackType
{
    /// <summary>
    /// Async work callbacks
    /// </summary>
    AsyncQueueCallbackType_Work,

    /// <summary>
    /// Completion callbacks after work is done
    /// </summary>
    AsyncQueueCallbackType_Completion
} AsyncQueueCallbackType;

/// <summary>
/// A callback that is invoked by the async queue.
/// </summary>
/// <param name='context'>A context pointer that was passed during SubmitAsyncCallback.</param>
/// <seealso cref='SubmitAsyncCallback' />
typedef void CALLBACK AsyncQueueCallback(_In_opt_ void* context);

/// <summary>
/// A callback that is invoked by the async queue whenever an item is submitted for execution.
/// </summary>
/// <param name='context'>A context pointer that was passed during RegisterAsyncQueueCallbackSubmitted.</param>
/// <param name='queue'>The async queue where the callback was submitted.</param>
/// <param name='type'>The type of callback.</param>
/// <seealso cref='RegisterAsyncQueueCallbackSubmitted' />
typedef void CALLBACK AsyncQueueCallbackSubmitted(_In_opt_ void* context, _In_ async_queue_handle_t queue, _In_ AsyncQueueCallbackType type);

/// <summary>
/// A callback that is invoked by the async queue when RemoveAsyncQueueCallbacks is called.
/// Ths callback is used to selectively remove items from the callback queue.
/// </summary>
/// <param name='predcateContext'>A context pointer that was passed to RemoveAsyncQueueCallbacks.</param>
/// <param name='callbackContext'>The context poiter that was passed to SubmitAsyncCallback.</param>
/// <seealso cref='RemoveAsyncQueueCallbacks' />
typedef bool CALLBACK AsyncQueueRemovePredicate(_In_opt_ void* predicateContext, _In_opt_ void* callbackContext);

/// <summary>
/// Creates an Async Queue, which can be used to queue
/// different async calls together.  Async Queues are
/// reference counted objects.  Release the reference
/// by calling CloseAsyncQueue.
/// </summary>
/// <param name='workDispatchMode'>The dispatch mode for the "work" side of the queue.</param>
/// <param name='completionDispatchMode'>The dispatch mode for the "completion" side of the queue.</param>
/// <param name='queue'>The newly created queue.</param>
STDAPI CreateAsyncQueue(
    _In_ AsyncQueueDispatchMode workDispatchMode,
    _In_ AsyncQueueDispatchMode completionDispatchMode,
    _Out_ async_queue_handle_t* queue);

/// <summary>
/// Duplicates the async_queue_handle_t object.  Use CloseAsyncQueue to close it.
/// </summary>
/// <param name='queueHandle'>The queue to reference.</param>
/// <param name='duplicatedHandle'>The duplicated queue handle.</param>
STDAPI DuplicateAsyncQueueHandle(
    _In_ async_queue_handle_t queueHandle,
    _Out_ async_queue_handle_t* duplicatedHandle);

/// <summary>
/// Processes items in the async queue of the given type. If an item
/// is processed this will return true. If there are no items to process
/// this returns false.  You can pass a timeout, which will cause
/// DispatchAsyncQueue to wait for something to arrive in the queue.
/// </summary>
/// <param name='queue'>The queue to dispatch work on.</param>
/// <param name='type'>The type of work to dispatch.</param>
/// <param name='timeoutInMs'>The number of ms to wait for work to arrive before returning false.</param>
STDAPI_(bool) DispatchAsyncQueue(
    _In_ async_queue_handle_t queue,
    _In_ AsyncQueueCallbackType type,
    _In_ uint32_t timeoutInMs);

/// <summary>
/// Returns TRUE if there is no outstanding work in this
/// queue for the given callback type.
/// </summary>
/// <param name='queue'>The queue to check.</param>
/// <param name='type'>The callback type in the queue to check.</param>
STDAPI_(bool) IsAsyncQueueEmpty(
    _In_ async_queue_handle_t queue,
    _In_ AsyncQueueCallbackType type);

/// <summary>
/// Closes the async queue.  A queue can only be closed if it
/// is not in use by an async api or is empty.  If not true, the queue
/// will be marked for closure and closed when it can. 
/// </summary>
/// <param name='queue'>The queue to close.</param>
STDAPI_(void) CloseAsyncQueue(
    _In_ async_queue_handle_t queue);

/// <summary>
/// Submits a callback to the queue for the given callback.  The callback will be added
/// to the queue immediately if delayMs is zero.  If non-zero, the callback will be added
/// after delayMs milliseconds.
/// type.
/// </summary>
/// <param name='queue'>The queue to submit the callback to.</param>
/// <param name='type'>The type of callback.</param>
/// <param name='delayMs'>The number of milliseconds to delay before submitting the callback to the queue.</param>
/// <param name='callbackContext'>An optional context pointer that will be passed to the callback.</param>
/// <param name='callback'>A pointer to the callback function.</param>
STDAPI SubmitAsyncCallback(
    _In_ async_queue_handle_t queue,
    _In_ AsyncQueueCallbackType type,
    _In_ uint32_t delayMs,
    _In_opt_ void* callbackContext,
    _In_ AsyncQueueCallback* callback);

/// <summary>
/// Walks all callbacks in the queue of the given type searching
/// for callback pointers that match searchCallback, or all callbacks
/// if searchCallback is null. For each one, the predicate is invoked
/// with the callback's context.  If the predicate wishes to remove
/// the callback, it should return true.
/// This should be called before an object is deleted
/// to ensure there are no orphan callbacks in the async queue
/// that could later call back into the deleted object.
/// </summary>
/// <param name='queue'>The queue to remove callbacks from.</param>
/// <param name='type'>The callback type to remove.</param>
/// <param name='searchCallback'>The callback function pointer to search for.</param>
/// <param name='predicateContext'>An optional context pointer to pass to the predicate.</param>
/// <param name='removePredicate'>A callback that will be invoked to decide if a callback should be removed.</param>
STDAPI_(void) RemoveAsyncQueueCallbacks(
    _In_ async_queue_handle_t queue,
    _In_ AsyncQueueCallbackType type,
    _In_ AsyncQueueCallback* searchCallback,
    _In_opt_ void* predicateContext,
    _In_ AsyncQueueRemovePredicate* removePredicate);

/// <summary>
/// Registers a callback that will be invoked whenever a callback
/// is submitted to this queue.
/// </summary>
/// <param name='queue'>The queue to register the submit callback to.</param>
/// <param name='context'>An optional context pointer to be passed to the submit callback.</param>
/// <param name='callback'>A callback that will be invoked when a new callback is submitted to the queue.</param>
/// <param name='token'>A token used in a later call to UnregisterAsyncCallbackSubmitted to remove the callback.</param>
STDAPI RegisterAsyncQueueCallbackSubmitted(
    _In_ async_queue_handle_t queue,
    _In_opt_ void* context,
    _In_ AsyncQueueCallbackSubmitted* callback,
    _Out_ registration_token_t* token);

/// <summary>
/// Unregisters a previously registered callback.
/// </summary>
/// <param name='queue'>The queue to remove the submit callback from.</param>
/// <param name='token'>The token returned from RegisterAsyncQueueCallbackSubmitted.</param>
STDAPI_(void) UnregisterAsyncQueueCallbackSubmitted(
    _In_ async_queue_handle_t queue,
    _In_ registration_token_t token);

#endif // #ifndef __asyncqueue_h__