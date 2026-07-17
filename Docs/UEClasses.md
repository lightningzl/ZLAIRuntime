# UE Classes

## 文档范围

记录 AI NPC Runtime 相关 UE C++ 类型、职责和依赖。UE 模板自带的 Gameplay 示例类不在此逐项维护，除非它们被当前 Demo 明确集成。

## 当前基线

- 游戏 Runtime Module：`ZL`
- AI Runtime Plugin：`ZL/Plugins/ZLAIRuntime`
- Plugin Runtime Module：`ZLAIRuntime`
- 现有 `AZLCharacter`、`AZLPlayerController`、`AZLGameMode` 及 `Variant_*` 类型属于 UE 模板/玩法示例。
- `ASideScrollingNPC` 等现有 AI 示例不包含 LLM Service 通信能力。
- `ZLAIRuntime` 插件提供 AI Service Client、协议类型和 JSON 转换能力，不依赖 `ZL` 游戏模块。

## Milestone 1 类型

| 类型 | 状态 | 职责 | 不负责 |
| --- | --- | --- | --- |
| `UZLAIServiceSubsystem` | M1-05 已实现 | 生成请求 ID；从设置读取地址与超时；构造并发送 HTTP 请求；分类、记录并返回成功或失败 | UI、NPC 行为、Prompt、Memory、Tool Call、持久配置 |
| `UZLAIServiceSettings` | M1-05 已实现 | 通过 UE Config 提供 Base URL 和请求超时 | 运行时请求状态或密钥管理 |
| `FZLDialogueRequest` | M1-04 已实现 | 表示 `request_id`、`npc_id`、`player_input` | 保存对话历史 |
| `FZLDialogueResponse` | M1-04 已实现 | 表示 `request_id`、`npc_id`、`reply`、`provider` | 推断或执行 Gameplay 指令 |
| `FZLServiceError` | M1-05 已实现 | 表示错误分类、错误码、消息、请求 ID 和 HTTP 状态码 | 暴露底层堆栈或内部路径 |

`ZLAIServiceProtocol` 命名空间公开请求序列化、成功响应解析和协议错误解析函数。Gameplay/UI 调用方只提交 NPC ID 和玩家输入；Base URL 与请求超时由 `UZLAIServiceSettings` 从 Game Config 读取。

实际实现名称如需调整，必须在同一任务中更新本文件；字段不得偏离 [Protocol.md](./Protocol.md)。

## 依赖方向

```text
Gameplay / UI
    -> ZLAIRuntime Plugin
        -> UZLAIServiceSubsystem
            -> ZLAIServiceProtocol
                -> HTTP + JSON
                    -> Python Service
```

- Gameplay/UI 只提交对话请求并消费成功/失败结果。
- 协议结构体可被 Client 使用，但不得依赖具体 UI 或 NPC 类型。
- Subsystem 不持有 NPC Actor 的强引用，不直接修改世界状态。
- 游戏模块通过 `ZL.AI.DialogueDemo <npc_id> <player_input>` 控制台命令提供 Milestone 1 演示入口；命令只依赖插件公开接口。

## 生命周期与异步约束

- Client 使用 `UGameInstanceSubsystem`，生命周期覆盖关卡切换且不依赖特定 Actor。
- 每个请求由客户端生成唯一 `request_id`。
- 回调必须区分网络失败、超时、HTTP 错误和解析错误。
- 回调触发前确认上下文仍有效；不允许悬空 UObject 引用。
- 当前阶段只输出 `reply`，不得从文本中解析 Gameplay 命令。

## M1-04 验证

- UE 5.8 `ZLEditor Win64 Development` 使用 MSVC `14.44.35228` 编译通过。
- UE 5.8 自动化测试 `ZLAIRuntime.Protocol` 共 3 项，覆盖请求序列化、成功响应解析和错误响应解析，全部通过。
- UE 5.8 联调测试 `ZLAIRuntime.Integration.ServiceClientCallbacks` 在本地 Python Stub Service 上通过，覆盖真实 HTTP `200` 成功响应、`400 invalid_request`、唯一请求 ID、字段透传和 Game Thread 成功/失败回调。

## M1-05 验证

- `DefaultGame.ini` 提供默认 `ServiceBaseUrl=http://127.0.0.1:8000` 和 10 秒请求超时，可在 Project Settings 的 `ZL AI Service` 中覆盖。
- `EZLServiceErrorCategory` 区分客户端、网络、超时、HTTP 和解析失败；日志包含 `request_id`、分类、错误码与 HTTP 状态。
- UE 5.8 `ZLEditor Win64 Development` 编译通过；完整 `ZLAIRuntime` 自动化测试共 6 项全部通过，覆盖配置、失败分类、协议与真实 Service 联调。

## M1-06 验证

- `ZL.AI.DialogueDemo` 接收 NPC ID 和玩家输入，通过 `UZLAIServiceSubsystem` 异步发起请求。
- 成功回调将 `reply` 输出为绿色屏幕消息和 `LogZL` Display 日志；失败回调将错误码与消息输出为红色屏幕消息和 Warning 日志。
- UE 5.8 Game 模式已分别在 Stub Service 启动和停止状态下执行命令，确认成功回复和失败信息均可见且游戏正常退出。

## 更新规则

新增、删除或改变 AI Runtime 类型职责时，更新类型表、依赖方向和对应验证说明；架构变化同时记录到 [DecisionLog.md](./DecisionLog.md)。

## Milestone 1 最终验证

- UE 5.8 `ZLEditor Win64 Development` 编译成功，完整 `ZLAIRuntime` 自动化测试共 6 项全部通过。
- 单次 Game 会话连续发起 10 次请求，记录 10 条请求、10 条成功回复和 0 条失败；游戏按退出码 0 结束且无遗留进程。
- `ZLAIRuntime` Runtime Plugin 的源码与 Build.cs 未引入数据库、向量库、真实 LLM 或 Tool Use 运行时依赖。
