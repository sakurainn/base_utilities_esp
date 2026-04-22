#pragma once

#include "../deps/MyTinySTL/functional"
#include "../deps/MyTinySTL/string"
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief 基于FreeRTOS Task的线程封装
 * 支持:
 *  - 启动/停止线程
 *  - 设置栈大小、优先级、CPU核心
 *  - 两种使用方式：回调函数或继承重写run()
 *  - 线程休眠辅助
 *
 * 使用示例 - 回调方式:
 * @code
 *   Thread thread("my_thread", 4096, tskIDLE_PRIORITY + 1, 0);
 *   thread.start([](){
 *       while(!Thread::should_stop()) {
 *           printf("thread running\n");
 *           Thread::sleep(1000);
 *       }
 *   });
 *   // ...
 *   thread.stop();
 * @endcode
 *
 * 使用示例 - 继承方式:
 * @code
 *   class MyThread : public Thread {
 *   protected:
 *       void run() override {
 *           while(!should_stop()) {
 *               printf("thread running\n");
 *               sleep(1000);
 *           }
 *       }
 *   };
 *
 *   MyThread thread;
 *   thread.start("my_thread", 4096, tskIDLE_PRIORITY + 1);
 * @endcode
 */
class Thread {
public:
    using RunCallback = std::function<void()>;

    /**
     * @brief 构造函数 (回调方式)
     * @param name 线程名称
     * @param stack_size 栈大小 (字节)
     * @param priority 优先级
     * @param core CPU核心 (0-1, -1表示不指定)
     */
    explicit Thread(const char* name = "thread",
                    uint32_t stack_size = 4096,
                    UBaseType_t priority = tskIDLE_PRIORITY + 1,
                    BaseType_t core = tskNO_AFFINITY)
        : m_task(nullptr)
        , m_should_stop(false)
        , m_name(name)
        , m_stack_size(stack_size)
        , m_priority(priority)
        , m_core(core)
        , m_callback(nullptr)
    {}

    /**
     * @brief 析构函数，自动停止线程
     */
    virtual ~Thread() {
        stop();
    }

    // 不允许拷贝
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    // 不允许移动 (因为TaskHandle包含this指针)
    Thread(Thread&&) = delete;
    Thread& operator=(Thread&&) = delete;

    /**
     * @brief 启动线程 (使用构造函数中设置的参数)
     * @param callback 线程执行回调
     * @return true 启动成功
     */
    bool start(RunCallback callback) {
        if (m_task != nullptr) {
            stop();
        }

        m_should_stop = false;
        m_callback = std::move(callback);

        BaseType_t ret;
        if (m_core == tskNO_AFFINITY) {
            ret = xTaskCreate(task_entry, m_name, m_stack_size, this, m_priority, &m_task);
        } else {
            ret = xTaskCreatePinnedToCore(task_entry, m_name, m_stack_size, this, m_priority, &m_task, m_core);
        }

        return ret == pdPASS;
    }

    /**
     * @brief 启动线程 (继承方式，重写run())
     * @return true 启动成功
     */
    bool start() {
        return start(nullptr);
    }

    /**
     * @brief 启动线程，重新指定参数
     * @param name 线程名称
     * @param stack_size 栈大小
     * @param priority 优先级
     * @param core CPU核心
     * @param callback 回调函数
     * @return true 启动成功
     */
    bool start(const char* name, uint32_t stack_size,
               UBaseType_t priority, BaseType_t core,
               RunCallback callback = nullptr) {
        m_name = name;
        m_stack_size = stack_size;
        m_priority = priority;
        m_core = core;
        return start(std::move(callback));
    }

    /**
     * @brief 停止线程
     * @param timeout_ms 等待线程退出超时，0表示不等待
     * @return true 停止成功
     */
    bool stop(uint32_t timeout_ms = 0) {
        if (m_task == nullptr) {
            return false;
        }

        // 设置停止标志
        m_should_stop = true;

        // 如果需要等待，简单等待一下让线程自己退出
        if (timeout_ms > 0) {
            uint32_t start = xTaskGetTickCount();
            while (eTaskGetState(m_task) != eDeleted &&
                   (xTaskGetTickCount() - start) < pdMS_TO_TICKS(timeout_ms)) {
                vTaskDelay(1);
            }
        }

        // 删除任务
        vTaskDelete(m_task);
        m_task = nullptr;
        return true;
    }

    /**
     * @brief 检查线程是否正在运行
     * @return true 正在运行
     */
    bool is_running() const {
        return m_task != nullptr && !m_should_stop;
    }

    /**
     * @brief 获取是否应该停止
     * @note 线程函数中应该定期检查这个标志
     */
    bool should_stop() const {
        return m_should_stop;
    }

    /**
     * @brief 获取任务句柄
     * @return TaskHandle_t
     */
    TaskHandle_t task_handle() {
        return m_task;
    }

    /**
     * @brief 获取线程名称
     * @return const char*
     */
    const char* name() const {
        return m_name.c_str();
    }

    /**
     * @brief 线程静态休眠
     * @param ms 休眠毫秒
     */
    static void sleep(uint32_t ms) {
        vTaskDelay(pdMS_TO_TICKS(ms));
    }

    /**
     * @brief 获取当前堆栈剩余空间
     * @return 剩余字节数
     */
    UBaseType_t get_free_stack() const {
        if (m_task == nullptr) {
            return 0;
        }
        return uxTaskGetStackHighWaterMark(m_task);
    }

    /**
     * @brief 获取当前任务优先级
     * @return 优先级
     */
    UBaseType_t get_priority() const {
        if (m_task == nullptr) {
            return m_priority;
        }
        return uxTaskPriorityGet(m_task);
    }

    /**
     * @brief 设置优先级
     * @param priority 新优先级
     */
    void set_priority(UBaseType_t priority) {
        if (m_task != nullptr) {
            vTaskPrioritySet(m_task, priority);
        }
        m_priority = priority;
    }

protected:
    /**
     * @brief 线程入口函数，继承方式时需要重写
     * @note 默认会调用回调函数，如果没有回调就什么都不做
     */
    virtual void run() {
        if (m_callback) {
            m_callback();
        }
    }

private:
    static void task_entry(void* arg) {
        Thread* self = static_cast<Thread*>(arg);
        if (self != nullptr) {
            self->run();
        }

        // 运行结束后清空任务句柄
        self->m_task = nullptr;
        vTaskDelete(nullptr);
    }

    TaskHandle_t m_task;
    bool m_should_stop;
    std::string m_name;
    uint32_t m_stack_size;
    UBaseType_t m_priority;
    BaseType_t m_core;
    RunCallback m_callback;
};

/**
 * @brief 获取当前线程的任务句柄
 * @return TaskHandle_t
 */
inline TaskHandle_t get_current_task() {
    return xTaskGetCurrentTaskHandle();
}

/**
 * @brief 获取当前系统空闲堆大小
 * @return 空闲字节数
 */
inline size_t get_free_heap() {
    return xPortGetFreeHeapSize();
}
