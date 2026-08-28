# UE Classes

## 文档职责

本文档只记录当前已实现的 AI Runtime UE 类型、职责、生命周期和依赖。当前范围见 [CurrentMilestone.md](../Current/CurrentMilestone.md)，目标社会模拟类型在实现后才加入本文。

## 当前基线

- 游戏 Runtime Module：`ZL`。
- AI Runtime Plugin：`ZL/Plugins/ZLAIRuntime`。
- Plugin Runtime Module：`ZLAIRuntime`。
- Social Runtime Module：`ZLASocialRuntime`。
- `ZLAIRuntime` 提供 AI Service Client、协议类型和 JSON 转换，不依赖 `ZL` 游戏模块。
- `ZLASocialRuntime` 提供社会 Gameplay Tags、Event、Level 1 Agent 和 Profile 基础类型，不依赖 `ZL` 或 HTTP Client 模块。
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

## Social Runtime 基础类型

| 类型 | 职责 |
| --- | --- |
| `FZLSocialEvent` | 表示具有 Event/Root/Parent/Causation ID、Depth、Budget、Confidence、来源/目标、Social 报告端点、位置、强度、通道和生命周期的 UE 权威事件 |
| `FZLSocialAgentProfile` | 表示独立于具体 Actor 的 Agent ID、Level、Faction/Occupation、位置、能力和人格快照；Important NPC 可声明 Report Receiver 与 Faction Authority 能力 |
| `FZLSocialPersonalityTraits` | 表示六个有界人格 Trait，并提供统一 Clamp 行为 |
| `UZLSocialEventArchetype` | 声明可配置事件类型、范围、强度、通道和生命周期的 DataAsset 类型；当前运行时尚未消费该资产 |
| `UZLSocialPersonalityArchetype` | 声明可复用人格 Trait 的 DataAsset 类型；当前运行时尚未接入资产选择或 DataTable 覆盖 |
| `ZLSocialTags` | 定义 Event、Instant State 与 Intent 原生 Gameplay Tags |
| `FZLSocialSpatialIndex` | 维护 Agent 到二维 Cell 的索引，并返回有界半径查询与候选统计 |
| `FZLSocialEventRouter` | 创建受控根事件、校验 Event Chain 硬边界、执行空间查询并按 Root/Agent 去重 |
| `FZLSocialReportConfirmation` | 表示 Gameplay 已确认完成的报告、接收者、因果 ID、确认时间和 Reporter Confidence |
| `FZLSocialPropagation` | 从显式报告确认创建有界 Social 派生 Event，并执行 Depth、Fan-out、Budget、TTL、Importance 与 Reporter/Root 去重 |
| `FZLSocialRelationshipStore` | 维护稀疏有向 Personal Relationship、Reputation 与 Faction Standing，执行来源权重、边界、衰减、Authority/Confidence 校验和 Root 去重 |
| `FZLSocialPerceptionFilter` | 对空间候选执行 Direct/Visual/Auditory/Social、距离衰减、视线、阈值和过期过滤 |
| `FZLSocialInstantState` | 根据 Event、感知强度和 Personality 更新并衰减 Fear、Anger、Curiosity、Alert |
| `FZLSocialShortMemory` | 维护固定容量、按时间可复查的 UE 社会事件环形缓冲区 |
| `FZLSocialLongMemory` | 为 Important NPC 维护有界提升、衰减淘汰和结构化稳定 Top-K 检索，不访问 SQLite 或 Dialogue Memory |
| `FZLSocialAgentState` | 聚合单个 Agent 的 Instant State、Short Memory 与可选 Important NPC Long Memory 更新入口 |
| `FZLSocialRuleDecisionEngine` | 以 Personality、Instant State 和 Event 生成可复现候选分数，并应用硬约束、优先级、冷却、迟滞和稳定平局规则 |
| `FZLSocialSimulation` | 编排 Event Router、Perception、State、Memory 和 Rule Decision，输出不含具体 Actor 的 Intent Command |
| `FZLSocialGameplayAdapter` | `ZL` 私有适配层；显式产生 Punch/Gunshot/Help Event，并把 Intent Command 交给 Gameplay 回调 |
| `FZLSocialAgentDebugSnapshot` | 单 Agent 的只读社会模拟快照，不反向驱动状态且不包含 Dialogue/凭据数据 |
| `ZLSocialDebug` | 格式化安全快照并运行确定性 120 Agent 聚合基准 |

这些类型不包含 Actor、Widget、HTTP、Provider、Python 或 Dialogue Memory 引用。`ZL` 负责把具体 Gameplay 对象转换为稳定 ID 和位置快照。

当前 Social Runtime 接入边界：

- `FZLSocialGameplayAdapter` 只验证 Event 产生、Intent Command 生成和回调交付，尚未连接具体 StateTree、AIController、导航、动画或 Gameplay Ability。
- Gameplay Adapter 当前使用恒定可见的测试 LOS 回调，真实 World Trace 尚未接入。
- `FZLSocialSpatialIndex` 使用二维 Cell 和二维距离，不处理 Z 轴楼层、房间或区域传播。
- Event/Personality DataAsset 类型已经声明，但 Event Router 仍使用 C++ 受控默认值，DataTable 批量覆盖尚未实现。

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
- `ZL.Social.InspectDemo [agent_id]`：输出可复现的最小 Agent 社会状态快照。
- `ZL.Social.Benchmark [agent_count]`：运行确定性 Level 1 场景并输出聚合性能指标。

演示日志只记录关联元数据、匹配结果和长度，不记录完整输入、Context、scope 或回复正文。

## 验证证据

- 通信与错误处理：[Milestone2Validation.md](../Validation/Milestone2Validation.md)
- Context：[Milestone3Validation.md](../Validation/Milestone3Validation.md)
- Dialogue Memory：[Milestone4Validation.md](../Validation/Milestone4Validation.md)
- 确定性社会模拟：[Milestone5Validation.md](../Validation/Milestone5Validation.md)
