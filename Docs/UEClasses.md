# UE Classes

## 文档范围

记录 AI NPC Runtime 相关 UE C++ 类型、职责和依赖。UE 模板自带的 Gameplay 示例类不在此逐项维护，除非它们被当前 Demo 明确集成。

## 当前基线

- Runtime Module：`ZL`
- 现有 `AZLCharacter`、`AZLPlayerController`、`AZLGameMode` 及 `Variant_*` 类型属于 UE 模板/玩法示例。
- `ASideScrollingNPC` 等现有 AI 示例不包含 LLM Service 通信能力。
- 当前尚未实现 AI Service Client 或协议结构体。

## Milestone 1 计划类型

| 类型 | 状态 | 职责 | 不负责 |
| --- | --- | --- | --- |
| `UZLAIServiceSubsystem` | 计划 | 持有 Service 配置；构造并发送 HTTP 请求；解析响应；通过委托返回成功或失败 | UI、NPC 行为、Prompt、Memory、Tool Call |
| `UZLAIServiceSettings` | 计划 | 通过 UE Config 提供 Base URL 和请求超时 | 运行时请求状态或密钥管理 |
| `FZLDialogueRequest` | 计划 | 表示 `request_id`、`npc_id`、`player_input` | 保存对话历史 |
| `FZLDialogueResponse` | 计划 | 表示 `request_id`、`npc_id`、`reply`、`provider` | 推断或执行 Gameplay 指令 |
| `FZLServiceError` | 计划 | 表示可记录和可展示的错误码、消息及请求 ID | 暴露底层堆栈或内部路径 |

实际实现名称如需调整，必须在同一任务中更新本文件；字段不得偏离 [Protocol.md](./Protocol.md)。

## 依赖方向

```text
Gameplay / UI
    -> UZLAIServiceSubsystem
        -> HTTP + JSON
            -> Python Service
```

- Gameplay/UI 只提交对话请求并消费成功/失败结果。
- 协议结构体可被 Client 使用，但不得依赖具体 UI 或 NPC 类型。
- Subsystem 不持有 NPC Actor 的强引用，不直接修改世界状态。

## 生命周期与异步约束

- Client 使用 `UGameInstanceSubsystem`，生命周期覆盖关卡切换且不依赖特定 Actor。
- 每个请求由客户端生成唯一 `request_id`。
- 回调必须区分网络失败、超时、HTTP 错误和解析错误。
- 回调触发前确认上下文仍有效；不允许悬空 UObject 引用。
- 当前阶段只输出 `reply`，不得从文本中解析 Gameplay 命令。

## 更新规则

新增、删除或改变 AI Runtime 类型职责时，更新类型表、依赖方向和对应验证说明；架构变化同时记录到 [DecisionLog.md](./DecisionLog.md)。
