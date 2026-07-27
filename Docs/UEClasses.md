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
| `UZLAIServiceSubsystem` | M3-06 已调整 | 生成请求 ID；提供无上下文和完整上下文重载；在创建 HTTP 前校验；共享构造、发送、超时和单次完成逻辑 | UI、NPC 行为、Prompt、Memory、Tool Call、持久配置 |
| `UZLAIServiceSettings` | 已实现 | 通过 UE Config 提供 Base URL 和请求超时 | 运行时请求状态或密钥管理 |
| `FZLDialogueRequest` | M3-02 已调整 | 表示 `request_id`、`npc_id`、`player_input` 和可选瞬时上下文 | 保存跨请求对话状态 |
| `FZLDialogueResponse` | 已实现 | 表示 `request_id`、`npc_id`、`reply`、`provider` | 推断或执行 Gameplay 指令 |
| `FZLServiceError` | 已实现 | 表示错误分类、错误码、消息、请求 ID 和 HTTP 状态码 | 暴露底层堆栈或内部路径 |

`ZLAIServiceProtocol` 命名空间公开请求序列化、成功响应解析和协议错误解析函数。Gameplay/UI 调用方只提交 NPC ID 和玩家输入；Base URL 与请求超时由 `UZLAIServiceSettings` 从 Game Config 读取。

实际实现名称如需调整，必须在同一任务中更新本文件；字段不得偏离 [Protocol.md](./Protocol.md)。

## Milestone 3 上下文类型

| 类型 | 状态 | 职责 | 不负责 |
| --- | --- | --- | --- |
| `FZLDialogueNpcContext` | M3-02 已实现 | 表示显示名、场景身份、人格特征、说话风格和当前目标 | 从 `npc_id` 或 Actor 自动推导设定 |
| `FZLDialogueWorldContext` | M3-02 已实现 | 表示地点、局势和 UE 已确认的事实 | 持有或查询 `UWorld`、`GameState`、任务系统 |
| `FZLDialogueHistoryMessage` | M3-02 已实现 | 表示一条 `player`/`npc` 历史消息 | 表示 system、tool 或供应商专用消息 |
| `FZLDialogueContext` | M3-02 已实现 | 聚合 NPC、世界和按序有限历史快照 | 持久化会话、Memory 或自动收集 Gameplay 状态 |
| `FZLDialogueRequest` | M3-02 已调整 | 通过 `bHasContext` 区分省略与完整 `FZLDialogueContext` | 保存跨请求状态 |

协议中的 `context` 可选，但存在时必须完整。UE 结构应能明确区分“未提供上下文”和“提供完整上下文”，不得用空对象替代缺失字段。

## 当前实现约束

Milestone 3 只扩展供应商无关的协议类型和请求入口，不把 Kimi/OpenAI 兼容 SDK、API Key、模型配置或 Prompt 引入 UE。现有公开类型按以下方式继续使用：

| 类型 | 实现 |
| --- | --- |
| `UZLAIServiceSubsystem` | 已保留现有 `NpcId + PlayerInput` 入口，并增加接受完整上下文的重载；共享 HTTP、超时和单次完成逻辑 |
| `UZLAIServiceSettings` | UE 外层请求超时为 30 秒，明确大于 Python Provider 默认 20 秒；不增加模型或密钥设置 |
| `FZLDialogueRequest` | 可选携带完整上下文；不加入 Provider、模型、密钥、Memory ID 或 Tool 定义 |
| `FZLDialogueResponse` | `Provider` 继续使用字符串，接受 `stub`、`kimi` 和未来未知标识，不加入模型名 |
| `FZLServiceError` | `Code` 继续使用字符串，保留新增 Provider 错误码；`Category` 仍归类为 HTTP，不新增供应商专用枚举 |

Gameplay/UI 负责显式构造上下文快照，插件不得依赖具体 NPC Actor、关卡、UI、DataTable 或内容资产。控制台演示只使用固定脱敏示例验证链路；正式 UMG、内容创作工具、持久化会话和行为执行不在当前范围。

## 依赖方向

```text
Gameplay / UI Context Snapshot
    -> ZLAIRuntime Plugin
        -> UZLAIServiceSubsystem
            -> ZLAIServiceProtocol
                -> HTTP + JSON
                    -> Python Service
```

- Gameplay/UI 构造并提交当前对话快照，消费成功/失败结果。
- 协议结构体可被 Client 使用，但不得依赖具体 UI 或 NPC 类型。
- Subsystem 不持有 NPC Actor 的强引用，不直接修改世界状态。
- 旧 `ZL.AI.DialogueDemo <npc_id> <player_input>` 入口继续作为无上下文回归；上下文演示入口只依赖插件公开类型。
- `ZL.AI.DialogueContextDemo <persona|world|history> <npc_id> <player_input>` 使用固定脱敏快照验证人格、世界和历史三类上下文；日志只记录关联元数据和长度，不记录完整输入、上下文或回复。

## 生命周期与异步约束

- Client 使用 `UGameInstanceSubsystem`，生命周期覆盖关卡切换且不依赖特定 Actor。
- 每个请求由客户端生成唯一 `request_id`。
- 回调必须区分网络失败、超时、HTTP 错误和解析错误。
- 上下文在发送前完成协议边界校验；校验失败不得创建 HTTP 请求，并只触发一次 Client 失败回调。
- 回调触发前确认上下文仍有效；不允许悬空 UObject 引用。
- 回复仍只包含 `reply`，不得从文本中解析 Gameplay 命令。

## 已有实现验证基线

现有 AI Runtime 类型已通过 UE 编译、协议/失败处理自动化测试和真实 Kimi 端到端演示验证。Milestone 2 进一步验证了字符串 Provider 前向兼容、`429`/`502`/`503`/`504` 错误保留，以及成功和失败回调各自恰好完成一次；证据见 [Milestone2Validation.md](./Validation/Milestone2Validation.md)。

Milestone 3 上下文结构、协议边界校验和 JSON 序列化已在 M3-02 实现。M3-06 已完成兼容请求重载、上下文 Game 演示和本地 HTTP 集成；`ZLEditor` 编译、完整 `ZLAIRuntime` 自动化 8/8、旧/新入口与非法上下文单次完成均已验证。证据见 [Milestone3Validation.md](./Validation/Milestone3Validation.md)。
