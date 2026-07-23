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

## 当前类型

| 类型 | 状态 | 职责 | 不负责 |
| --- | --- | --- | --- |
| `UZLAIServiceSubsystem` | 已实现 | 生成请求 ID；从设置读取地址与超时；构造并发送 HTTP 请求；分类、记录并返回成功或失败 | UI、NPC 行为、Prompt、Memory、Tool Call、持久配置 |
| `UZLAIServiceSettings` | 已实现 | 通过 UE Config 提供 Base URL 和请求超时 | 运行时请求状态或密钥管理 |
| `FZLDialogueRequest` | 已实现 | 表示 `request_id`、`npc_id`、`player_input` | 保存对话历史 |
| `FZLDialogueResponse` | 已实现 | 表示 `request_id`、`npc_id`、`reply`、`provider` | 推断或执行 Gameplay 指令 |
| `FZLServiceError` | 已实现 | 表示错误分类、错误码、消息、请求 ID 和 HTTP 状态码 | 暴露底层堆栈或内部路径 |

`ZLAIServiceProtocol` 命名空间公开请求序列化、成功响应解析和协议错误解析函数。Gameplay/UI 调用方只提交 NPC ID 和玩家输入；Base URL 与请求超时由 `UZLAIServiceSettings` 从 Game Config 读取。

实际实现名称如需调整，必须在同一任务中更新本文件；字段不得偏离 [Protocol.md](./Protocol.md)。

## 当前演进约束

真实 LLM 接入不计划新增 UE Runtime 类型，也不把 OpenAI SDK、API Key、模型配置或 Prompt 引入 UE。现有公开类型按以下方式继续使用：

| 类型 | 计划 |
| --- | --- |
| `UZLAIServiceSubsystem` | 保持请求入口和异步委托；已验证新增 Provider HTTP 错误路径和单次完成回调 |
| `UZLAIServiceSettings` | UE 外层请求超时为 30 秒，明确大于 Python Provider 默认 20 秒；不增加模型或密钥设置 |
| `FZLDialogueRequest` | 字段保持不变，不加入 Provider、模型、人格、历史或世界状态 |
| `FZLDialogueResponse` | `Provider` 继续使用字符串，接受 `stub`、`openai` 和未来未知标识，不加入模型名 |
| `FZLServiceError` | `Code` 继续使用字符串，保留新增 Provider 错误码；`Category` 仍归类为 HTTP，不新增供应商专用枚举 |

UE 当前工作的重点是协议兼容和真实链路验证，而不是扩展 Gameplay 能力。控制台命令 `ZL.AI.DialogueDemo` 继续作为最小演示入口；正式 UMG、NPC 人格、多轮对话和行为执行不在当前范围。

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
- 游戏模块通过 `ZL.AI.DialogueDemo <npc_id> <player_input>` 控制台命令提供最小演示入口；命令只依赖插件公开接口。

## 生命周期与异步约束

- Client 使用 `UGameInstanceSubsystem`，生命周期覆盖关卡切换且不依赖特定 Actor。
- 每个请求由客户端生成唯一 `request_id`。
- 回调必须区分网络失败、超时、HTTP 错误和解析错误。
- 回调触发前确认上下文仍有效；不允许悬空 UObject 引用。
- 当前阶段只输出 `reply`，不得从文本中解析 Gameplay 命令。

## 已有实现验证基线

现有 AI Runtime 类型已通过 UE 编译、协议/失败处理自动化测试和 Stub Service 端到端演示验证。Milestone 2 进一步验证了字符串 Provider 前向兼容、`429`/`502`/`503`/`504` 错误保留，以及成功和失败回调各自恰好完成一次。可复查证据见 [Milestone2Validation.md](./Validation/Milestone2Validation.md)，历史基线见 [Milestone1Validation.md](./Validation/Milestone1Validation.md)。
