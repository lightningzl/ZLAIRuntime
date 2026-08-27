# Milestone 5：确定性社会模拟基础

## 状态

- 状态：`已完成`
- 完成日期：2026-08-03
- 验收证据：[Milestone5Validation.md](../Validation/Milestone5Validation.md)

本文档是 Milestone 5 完成时的范围定稿，只用于历史追溯，不定义当前允许实施的范围。

## 目标

在不修改现有 UE/Python Dialogue 协议、不引入 ToolCall 或新的 LLM 决策路径的前提下，建立纯 UE、事件驱动、可测试的社会模拟纵向切片：

```text
Gameplay Event
  -> Spatial Query
  -> Perception Filter
  -> Instant State
  -> Short Social Memory
  -> Personality Rule Decision
  -> UE Intent Delivery
```

本阶段证明了 100+ 普通 NPC 可以对同一事件产生不同、确定性且有性能预算的反应。现有 Dialogue、Context 和 SQLite Memory 作为回归基线，不承担社会模拟状态存储。

## 完成范围

### 模块与数据

- 新增独立 UE Runtime Module `ZLASocialRuntime`，不把社会模拟职责塞入现有 HTTP Client。
- `ZL` Gameplay 通过公开接口产生 Event，并通过回调消费纯数据 Gameplay Intent。
- 使用 Gameplay Tags 表达 Event、Instant State 和 Intent 类型。
- 声明 Event/Personality DataAsset Archetype 类型和 DataTable 扩展边界；本阶段运行时仍使用受控默认值，未接入资产加载或 DataTable 覆盖流程。
- 社会模拟模块不依赖 Python SDK、Prompt、HTTP Schema、具体 Widget 或具体 NPC Actor 实现。

### Event 与传播

- 支持 Punch、Gunshot、Help 三个受控事件；为 Talk/Shout/Steal/Kill 预留 Tag 和数据扩展点。
- Event 包含唯一 ID、类型、来源、目标、位置、半径、严重度、Noise、感知通道、时间和过期信息。
- 使用事件触发的二维空间索引查询，不每帧扫描全部 NPC。
- 支持 Direct、Visual、Auditory 三种感知通道。
- 对同一 Event/NPC 执行确定性去重和稳定排序。
- 本阶段只处理直接感知，不生成 NPC→NPC 二级社会传播。

### NPC 与规则

- Level 1 NPC 不调用 LLM。
- Personality 使用 Brave、FearSensitivity、Curiosity、Justice、Aggression、Social 六个 `[0,1]` Trait。
- Instant State 包含 Fear、Anger、Curiosity、Alert，并具有有界更新与衰减。
- Short Social Memory 使用固定容量，不进行持久化、摘要或语义检索。
- Rule/Utility System 产生 Ignore、Observe、Flee、Report、Assist、Confront 中的合法 Intent。
- 能力硬约束、事件优先级、行为冷却和迟滞共同避免明显不合理行为与状态抖动。

### Intent 交付与调试

- Gameplay Intent 通过 UE 纯数据回调交付；本阶段未实现具体 StateTree、AIController 或 Gameplay 执行器。
- Intent 不是跨进程 ToolCall，不进入现有 `POST /v1/dialogue`。
- 提供 `ZL.Social.InspectDemo`，显示选定 NPC 的人格、Instant State、Last Event、Short Memory、候选分数和最终 Intent。
- 提供 `ZL.Social.Benchmark`，记录注册 NPC 数、事件查询候选数、通过感知数、Rule Evaluation 数和处理耗时。

## 数据与权威边界

- UE 是 Event、Perception、State、Memory 和 Intent 的唯一事实来源。
- Python SQLite Memory 继续只保存显式启用范围中的成功对话轮次。
- 社会 Memory 与 Dialogue Memory 不互相自动同步。
- `POST /v1/dialogue` 请求、响应、错误、超时和 Memory 语义保持不变。
- Gameplay 执行结果必须由 UE 产生；调试数据不得反向驱动状态。

## 未纳入范围

- Decision Endpoint 或 ToolCall 字段。
- Level 1 LLM 决策。
- Level 2 Important NPC 或 Level 3 Core NPC。
- Long Memory、Relationship、Reputation、Faction Standing 或 SaveGame 持久化。
- NPC→NPC 二级传播、谣言、Chain Budget 或 Faction Authority。
- 正式 Speech Bubble、Emoji、复杂对话或 AI Boss Director。
- MassEntity 正式集成和第三方群体模拟框架。
- Embedding、向量数据库、摘要、知识图谱或自动事实抽取。
- 真实 World LOS、楼层/区域传播和具体 Gameplay Intent 执行器。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M5-A01` | Milestone 1 至 4 的 Dialogue、Context、Memory、错误和超时路径保持回归通过；现有协议与 Python Runtime 行为不改变。 |
| `M5-A02` | `ZLASocialRuntime` 与 `ZL`、`ZLAIRuntime` 的依赖方向符合架构，社会模拟模块不依赖具体 UI、NPC Actor、Python 或 Provider 类型。 |
| `M5-A03` | Punch、Gunshot、Help 事件可以由 Gameplay 显式产生，字段、Tag、默认配置和非法输入行为确定且可测试。 |
| `M5-A04` | Event 只查询覆盖空间单元，候选数量由场景空间分布决定；自动化证明处理路径没有遍历全部注册 NPC。 |
| `M5-A05` | Direct、Visual、Auditory 感知过滤、距离衰减、过期和同 Event/NPC 去重行为确定且有边界测试。 |
| `M5-A06` | 六个 Personality Trait、四个 Instant State 和固定容量 Short Memory 的更新、衰减、覆盖及淘汰行为可重复。 |
| `M5-A07` | 同一 Punch 事件在不同人格配置下至少产生 Observe、Flee、Report 和 Confront 四种预期结果；相同输入重复运行结果一致。 |
| `M5-A08` | Extreme Gunshot 事件可以覆盖低优先级好奇行为；Intent 冷却和迟滞避免高频往返切换。 |
| `M5-A09` | 受影响 UE Target 编译、社会模拟自动化、现有 `ZLAIRuntime` 自动化和一个无界面 Gameplay 集成场景通过。 |
| `M5-A10` | 目标测试场景注册至少 100 个 Level 1 Agent；记录固定硬件与场景下的查询量和处理耗时，Inspector/日志不泄露对话、密钥或内部敏感数据。 |
