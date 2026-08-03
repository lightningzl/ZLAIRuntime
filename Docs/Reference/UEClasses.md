# UE Classes

## 文档职责

本文档只记录当前已实现的 AI Runtime UE 类型、职责、生命周期和依赖。当前范围见 [CurrentMilestone.md](../Current/CurrentMilestone.md)，目标社会模拟类型在实现后才加入本文。

## 当前基线

- 游戏 Runtime Module：`ZL`。
- AI Runtime Plugin：`ZL/Plugins/ZLAIRuntime`。
- Plugin Runtime Module：`ZLAIRuntime`。
- `ZLAIRuntime` 提供 AI Service Client、协议类型和 JSON 转换，不依赖 `ZL` 游戏模块。
- `AZLCharacter`、`AZLPlayerController`、`AZLGameMode` 和 `Variant_*` 属于 UE 模板/玩法示例。
- `ASideScrollingNPC` 等模板 AI 示例不包含 LLM Service 通信能力。
- `ZL/Config/DefaultEngine.ini` 为本机 Service 配置 HTTP No Proxy；命令行参数仍可覆盖。

## Service Client 类型

| 类型 | 职责 | 不负责 |
| --- | --- | --- |
| `UZLAIServiceSubsystem` | 校验并发送 Dialogue 请求，管理 HTTP、超时、关联和单次完成回调 | UI、Prompt、数据库、Gameplay 行为、ToolCall |
| `UZLAIServiceSettings` | 通过 UE Config 提供 Base URL 和外层请求超时 | API Key、模型选择和运行时请求状态 |
| `FZLDialogueRequest` | 表示请求 ID、NPC ID、玩家输入、可选 Context 和可选 Memory 范围 | 保存跨请求正文或访问数据库 |
| `FZLDialogueResponse` | 表示请求关联、NPC ID、纯文本回复和逻辑 Provider | 推断或执行 Gameplay 指令 |
| `FZLServiceError` | 表示错误分类、错误码、消息、请求 ID 和 HTTP 状态 | 暴露底层堆栈、路径或原始 Provider 异常 |

`ZLAIServiceProtocol` 命名空间负责请求序列化、成功响应解析和协议错误解析。字段必须与 [Protocol.md](./Protocol.md) 一致。

## Context 类型

| 类型 | 职责 |
| --- | --- |
| `FZLDialogueNpcContext` | 显示名、场景身份、人格特征、说话风格和目标 |
| `FZLDialogueWorldContext` | UE 已确认的地点、局势和世界事实 |
| `FZLDialogueHistoryMessage` | 一条 `player` 或 `npc` 历史消息 |
| `FZLDialogueContext` | 聚合 NPC、World 和有限历史快照 |

`FZLDialogueRequest::bHasContext` 区分“省略 Context”和“提供完整 Context”。插件不得从 `npc_id`、Actor、World、GameState、UI 或内容资产自动推导快照。

## Memory 类型

| 类型 | 职责 |
| --- | --- |
| `FZLDialogueMemory` | 表示稳定、不透明的 `scope_id` |
| `FZLDialogueRequest::bHasMemory` | 区分“省略 Memory”和“提供完整 Memory 对象” |

Gameplay/UI 显式提供稳定 scope；插件不得读取账号、SaveGame、Actor、World 或平台身份来自动生成。Memory 由 `(scope_id, npc_id)` 隔离，SQLite 路径、行 ID 和查询不进入 UE。

## 依赖方向

```text
ZL Gameplay / UI
  -> Context Snapshot + Optional Memory Scope
  -> ZLAIRuntime public interface
  -> UZLAIServiceSubsystem
  -> ZLAIServiceProtocol
  -> HTTP + JSON
  -> Python Service
```

- Gameplay/UI 构造快照并消费成功或失败结果。
- Subsystem 不持有具体 NPC Actor 的强引用，不直接修改世界状态。
- 插件不依赖具体 Widget、NPC、任务或动画类型。
- UE 不包含 Prompt、Provider SDK、模型配置、API Key 或 SQLite。
- 当前成功回复只包含纯文本，UE 不从文本推断 Gameplay 行为。

## 生命周期与异步约束

- Client 使用 `UGameInstanceSubsystem`，生命周期覆盖关卡切换。
- 每个请求由 UE 生成唯一 `request_id`。
- 请求 Context 和 Memory 在创建 HTTP 前完成边界校验。
- 本地校验失败不创建 HTTP，并且只触发一次失败回调。
- 网络失败、超时、HTTP 错误和解析错误走不同分类。
- 完成回调回到 Game Thread，并在触发前确认上下文仍有效。
- Python Provider 超时小于 UE 外层请求超时。

## 演示入口

- `ZL.AI.DialogueDemo`：无 Context/Memory 回归。
- `ZL.AI.DialogueContextDemo`：人格、世界和历史快照演示。
- `ZL.AI.DialogueMemoryDemo`：连续 Memory、scope/NPC 隔离和重启恢复演示。

演示日志只记录关联元数据、匹配结果和长度，不记录完整输入、Context、scope 或回复正文。

## 验证证据

- 通信与错误处理：[Milestone2Validation.md](../Validation/Milestone2Validation.md)
- Context：[Milestone3Validation.md](../Validation/Milestone3Validation.md)
- Dialogue Memory：[Milestone4Validation.md](../Validation/Milestone4Validation.md)
