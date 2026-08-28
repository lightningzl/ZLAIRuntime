# Architecture

## 文档职责

本文档只描述当前已经接受并实现的系统模块、职责边界和依赖方向。目标社会模拟设计见 [SocialSimulationDesign.md](./SocialSimulationDesign.md)；尚未进入当前实现的目标不得写成现有架构。

## 总体结构

```text
Player / UI
    |
    v
UE5 Runtime
  - ZL Gameplay/UI
    - Context Snapshot Source
    - Social Event Producer + Intent Adapter
  - ZLAIRuntime Plugin
    - AI Service Client
    - Protocol Types
    - ZLASocialRuntime Module
      - Social Gameplay Tags
      - Event Chain/Agent/Profile Types
      - Event Router + 2D Spatial Index
      - Perception Filter
      - Explicit Report Confirmation + Bounded Propagation
      - Instant/Long-Term State + Sparse Relationship/Faction
      - Bounded Short/Long Social Memory + Structured Retrieval
      - Relationship-Aware Deterministic Rule Decision
      - Sanitized Debug Snapshot + Aggregate Metrics
    |
    | HTTP + versioned JSON
    v
Python AI Service
  - FastAPI Route
  - Dialogue Service
  - Memory Service
      `- SQLite Repository
  - Context Builder
  - Dialogue Provider Interface
      |- Kimi Provider
      `- Stub Provider
```

## 模块边界

### UE5 Runtime

`ZL` Gameplay/UI 只通过 `ZLAIRuntime` 插件公开接口提交请求和消费结果。

- Gameplay/UI 负责从自身已知状态构造当前请求的 NPC 人格、世界状态和有限历史快照，并显式决定是否提供稳定的 Memory 范围。
- 插件负责请求校验与发起、端到端 HTTP 管理、协议解析和结果交付。
- 插件不从 Actor、World、GameState、SaveGame、账号、UI 或内容资产自动抓取上下文或 Memory 标识。
- UE 不负责 Prompt、模型 SDK、Provider 选择、密钥、Memory 存储检索或 Python 生成编排。
- 插件不得依赖 `ZL` 游戏模块、具体 UI 或 NPC Actor。
- 未来 Gameplay Tool 的最终校验和执行权仍属于 UE；当前实现只消费纯文本 Dialogue 回复。

`ZLASocialRuntime` 是同一插件内与 HTTP Client 隔离的 Runtime Module：

- 公开社会 Event、Level 1 Agent、Personality Profile 和 DataAsset 基础类型；当前只声明 Archetype 类型，运行时 Event Preset 仍由 Router 的受控默认值提供，尚未接入资产加载或 DataTable 覆盖。
- 通过原生 Gameplay Tags 定义 Event、Instant State 和 Intent 的稳定扩展点。
- 只依赖 UE Runtime 基础模块与 `GameplayTags`，不依赖 `ZL`、`ZLAIRuntime`、HTTP、Python、Provider、Widget 或具体 NPC Actor。
- `ZL` 可以依赖并适配其公开接口；`ZLASocialRuntime` 不反向依赖游戏模块。
- Agent 仅在注册、注销或跨 Cell 移动时更新二维网格；Event Router 只枚举半径覆盖的 Cell，再做精确二维距离过滤。
- Event 包含 Root/Parent、Depth、Budget、Causation、Confidence 和 Social 来源基础字段；根事件由 Router 初始化并受深度、预算和生命周期硬上限约束。
- Router 为 Punch、Gunshot、Help 提供受控默认参数，拒绝非法/过期事件，并按 `(root_event_id, agent_id)` 确定性去重。
- `FZLSocialPropagation` 只在 Gameplay 显式确认报告完成后创建 Social 派生 Event；它限制深度、单节点 Fan-out、Root Budget、TTL、低重要度和 Reporter/Root 重复，并确定性衰减 Confidence。规则产生 `Intent.Report` 不调用该入口。
- Relationship Store 只在相关交互发生时创建有向 `(observer_id, subject_id)` 边，保存有界 Trust、Affinity、Fear、Familiarity 与 Reputation；Faction Standing 只有具备 Authority Capability 的 Important NPC 经显式确认且 Confidence 达标后才更新，并按 Root/Faction 去重。
- Perception Filter 在空间候选上执行 Direct/Visual/Auditory/Social 能力、距离衰减、视线、阈值和过期检查；Direct Target 与指定 Social Receiver 不受普通半径和遮挡过滤。
- 感知结果按 Event 强度与 Personality 更新 Fear、Anger、Curiosity、Alert；状态值有界并按时间衰减。
- Level 1 Short Social Memory 使用固定容量环形缓冲区，保存 UE 权威事件摘要，不持久化且不与 Python Dialogue Memory 同步。
- Important NPC 默认使用 Short 16、Long 8 的有界 Social Memory；Long Memory 只提升高 Importance 或显式 Anchored 事件，按衰减后重要度稳定淘汰，并通过事件、人物、Faction、地点和时间过滤执行有界 Top-K 检索。
- Rule Decision 对 Ignore、Observe、Investigate、Flee、Report、Assist、Confront 计算可复现候选分数；Relationship、Faction Standing、相关 Long Memory、Occupation 和来源 Confidence 以稳定 Reason Code/Score Contribution 参与评分，能力与重复报告硬约束先于评分，高优先级极端事件可越过普通冷却，平分按 Tag 稳定排序。
- `FZLSocialSimulation` 编排模块内纵向链路并输出纯数据 Intent Command；`ZL` 的 Gameplay Adapter 显式产生受控 Event、确认报告完成、处理 Social 派生 Event、确认 Authority Assessment 并完成 Intent 回调交付，不反向泄露具体 Actor 到 Runtime 模块。确定性无界面场景注册 5 个 Important NPC 并验证完整数据闭环；具体 StateTree、AIController 或 Gameplay Intent 执行器尚未实现。
- `ZL.Social.InspectDemo` 输出选定 Agent 的人格、即时状态、最近事件、短期记忆数量、候选分数与 Intent；`ZL.Social.Benchmark` 输出注册数、空间候选、感知数、规则次数和处理耗时，不输出 Dialogue、scope、Prompt 或凭据。

### Python AI Service

Python Service 负责 AI 推理编排，不直接访问或修改 UE 世界。

- Route 负责 HTTP 输入输出适配。
- Dialogue Service 负责协调可选 Memory 读取、Context Builder、单次 Provider 调用和成功后的 Memory 写入。
- Memory Service 负责范围隔离、检索预算、历史合并和幂等轮次语义，不执行 SQL。
- SQLite Repository 负责 Schema、索引、连接、查询和事务，不参与 Prompt 或 Provider 编排。
- Context Builder 负责将固定系统约束、NPC 人格、世界状态、持久化历史、有限客户端历史和当前输入组装为供应商无关的生成上下文。
- Provider 接口隔离供应商 SDK 与上游数据格式。
- Provider 实现负责把内部生成上下文映射为供应商请求，并完成供应商异常分类。
- Service 不负责动画、移动、任务、战斗等 Gameplay 行为。
- Memory Service 只持久化显式启用范围中的已完成对话轮次；Route、Context Builder 和 Provider 不直接访问数据库。
- Service、Memory Service、Context Builder 和 Provider 都不读取 UE 世界。

### Protocol

[Protocol.md](../Reference/Protocol.md) 是 UE 与 Python 的唯一共享边界。两端不得依赖对方的内部类型、SDK 或目录结构。

## 依赖方向

```text
ZL Gameplay/UI Context Snapshot -> ZLAIRuntime Plugin -> HTTP/JSON Protocol

FastAPI Route -> Dialogue Service
                     |-> Memory Service -> SQLite Repository
                     |-> Context Builder
                     `-> Dialogue Provider Interface -> Provider SDK
```

- Gameplay 层不处理 HTTP、JSON 或 Provider 细节。
- Route 不直接调用具体 Provider SDK。
- Dialogue Service 协调 Schema、Memory Service、Context Builder 和 Provider，不依赖 FastAPI HTTP 类型、SQL 或具体 SDK。
- Memory Service 依赖抽象 Repository 能力和内部对话类型，不依赖 FastAPI、Provider SDK 或 UE 类型。
- SQLite Repository 不依赖 Route、Context Builder 或 Provider。
- Context Builder 不依赖 FastAPI、Settings、网络、数据库或具体 Provider，并且只有它能把协议上下文转换为内部生成上下文。
- Provider 实现不得构造 UE/Python 协议响应。
- Provider 接口不得接收协议 Schema 或 UE 类型。

## 后续扩展边界

- **Memory Retrieval 扩展**：向量检索、摘要、事实抽取和相关性评分必须继续位于 Memory Service 边界内，不得扩散到 Route、UE Client 或 Provider。
- **Tool Planner**：生成结构化 Tool Call。
- **UE Tool Registry/Executor**：白名单校验、参数校验和 Gameplay 执行；最终执行权始终在 UE。

当前 Memory 实现使用 SQLite 结构化对话轮次和最近历史检索。Context Builder 接收 Memory Service 已准备的内部历史，但不承担存储、查询、事务、摘要或向量检索职责。

当前允许范围由 [CurrentMilestone.md](../Current/CurrentMilestone.md) 定义；具体模块和类型见 [PythonModules.md](../Reference/PythonModules.md) 与 [UEClasses.md](../Reference/UEClasses.md)，SQLite 的物理结构和事务设计见 [DatabaseDesign.md](../Reference/DatabaseDesign.md)。
