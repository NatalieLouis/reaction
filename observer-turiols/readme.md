 事件类型细分

    定义DEMStateEvent枚举，明确区分不同状态变化（如DeadToActive）。
    观察者通过getInterestedEvents()方法声明自己关注的事件类型（如WebSocketObserver只关注DeadToActive）。
    被观察者维护event_observers_映射表，仅通知对特定事件感兴趣的观察者，避免无效通知。

2. 异步通知与线程池

    实现ThreadPool类，管理工作线程并支持带优先级的任务队列。
    被观察者在notifyObservers()中，将观察者的handleEvent方法封装为任务提交到线程池，实现异步执行。
    耗时操作（如ImgAppObserver的 3 秒休眠）在独立线程中执行，不会阻塞被观察者或其他观察者。

3. 观察者优先级

    观察者构造时指定优先级（1 为最高，10 为最低）。
    被观察者通知前，按优先级对观察者排序，确保高优先级任务（如LoggingObserver的日志记录）优先执行。
    线程池使用优先队列存储任务，保证高优先级任务先被执行。

4. 自动重连机制

    实现RetryHandler模板类，支持对任意操作的重试（可配置最大重试次数和间隔）。
    WebSocketObserver中，对可能失败的连接操作使用RetryHandler，失败后自动重试（最多 3 次，间隔 1 秒）。
    重试过程中捕获异常并打印日志，最终失败时明确提示。


状态变化时，LoggingObserver（优先级 1）最先执行，确保日志记录不丢失。
WebSocketObserver（优先级 2）的连接操作失败后自动重试，最终成功。
ImgAppObserver（优先级 3）的耗时操作在后台执行，不阻塞其他流程。
观察者只处理自己关注的事件（如WebSocketObserver不响应ActiveToIdle事件）。


事件过滤机制：允许观察者基于事件内容（如特定 DEM 实例 ID）进一步过滤事件。
重试策略可配置：支持指数退避重试（间隔随次数增加）或随机间隔，避免网络拥塞。
任务超时控制：为异步任务设置超时时间，防止无限阻塞。
监控指标收集：记录每个观察者的处理耗时、成功率，用于性能分析。
分布式支持：通过消息队列（如 RabbitMQ）实现跨服务的观察者模式。
