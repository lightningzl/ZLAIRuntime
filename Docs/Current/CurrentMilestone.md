# Current Milestone

历史里程碑定稿：

- [Milestone 1：UE 到 Python Service 最小闭环](../Milestones/Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](../Milestones/Milestone2.md)
- [Milestone 3：NPC 上下文与人格](../Milestones/Milestone3.md)
- [Milestone 4：持久化对话 Memory](../Milestones/Milestone4.md)

历史文档只用于追溯，不覆盖本文件定义的当前范围。长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，目标系统设计见 [SocialSimulationDesign.md](../Planning/SocialSimulationDesign.md)。

## Milestone 5：确定性社会模拟基础

状态：`已完成`（2026-08-03，验收证据见 [Milestone5Validation.md](../Validation/Milestone5Validation.md)）。

目标：在不修改现有 UE/Python Dialogue 协议、不引入 ToolCall 或新的 LLM 决策路径的前提下，建立纯 UE、事件驱动、可测试的社会模拟纵向切片：

```text
Gameplay Event
  -> Spatial Query
  -> Perception Filter
  -> Instant State
  -> Short Social Memory
  -> Personality Rule Decision
  -> UE Intent Execution
```

本阶段先证明 100+ 普通 NPC 可以对同一事件产生不同、确定性且有性能预算的反应。现有 Dialogue、Context 和 SQLite Memory 作为回归基线，不承担社会模拟状态存储。

## 本阶段范围

### 模块与数据

- 新增独立 UE Runtime Module `ZLASocialRuntime`，不把社会模拟职责塞入现有 HTTP Client。
- `ZL` Gameplay 通过公开接口产生 Event、配置场景并执行具体 Gameplay Intent。
- 使用 Gameplay Tags 表达 Event、Instant State 和 Intent 类型。
- 使用 DataAsset 定义 Event/Personality Archetype，DataTable 可用于批量 NPC 实例覆盖。
- 社会模拟模块不得依赖 Python SDK、Prompt、HTTP Schema、具体 Widget 或具体 NPC Actor 实现。

### Event 与传播

- 支持至少 Punch、Gunshot、Help 三个受控事件；为 Talk/Shout/Steal/Kill 预留 Tag 和数据扩展点，但不要求本阶段完成全部表现。
- Event 至少包含唯一 ID、类型、来源、目标、位置、半径、严重度、Noise、感知通道、时间和过期信息。
- 使用事件触发的空间索引查询，不每帧扫描全部 NPC。
- 支持 Direct、Visual、Auditory 三种感知通道。
- 对同一 Event/NPC 执行确定性去重和优先级排序。
- 本阶段只处理直接感知，不生成 NPC→NPC 二级社会传播。

### NPC 与规则

- Level 1 NPC 不调用 LLM。
- Personality 使用 Brave、FearSensitivity、Curiosity、Justice、Aggression、Social 六个 `[0,1]` Trait。
- Instant State 至少包含 Fear、Anger、Curiosity、Alert，并具有有界更新与衰减。
- Short Social Memory 使用固定容量，不进行持久化、摘要或语义检索。
- Rule/Utility System 产生 Ignore、Observe、Flee、Report、Assist、Confront 中的合法 Intent。
- 硬约束、事件优先级和行为冷却先于 Utility，避免明显不合理行为和状态抖动。

### 执行与调试

- Gameplay Intent 只通过 UE 接口交付，由 `ZL` 的 StateTree/AIController/Gameplay 适配层执行。
- 本阶段的 Intent 不是跨进程 ToolCall，不进入现有 `POST /v1/dialogue`。
- 提供最小 Runtime Debug 视图或调试命令，显示选定 NPC 的人格、Instant State、Last Event、Short Memory、候选分数和最终 Intent。
- 提供聚合性能统计：注册 NPC 数、事件查询候选数、通过感知数、Rule Evaluation 数和处理耗时。

## 数据与权威边界

- UE 是 Event、Perception、State、Memory 和 Intent 的唯一事实来源。
- Python SQLite Memory 继续只保存显式启用范围中的成功对话轮次。
- 社会 Memory 与 Dialogue Memory 不互相自动同步。
- 当前 `POST /v1/dialogue` 请求、响应、错误、超时和 Memory 语义保持不变。
- 任何 Gameplay 执行结果都必须由 UE 产生；调试数据不得反向驱动状态。

## 明确不做

- 不修改 [Protocol.md](../Reference/Protocol.md)，不新增 Decision Endpoint 或 ToolCall 字段。
- 不调用 LLM 进行 Level 1 决策。
- 不实现 Level 2 Important NPC 或 Level 3 Core NPC。
- 不实现 Long Memory、Relationship、Reputation、Faction Standing 或 SaveGame 持久化。
- 不实现 NPC→NPC 二级传播、谣言、Chain Budget 或 Faction Authority。
- 不实现正式 Speech Bubble、Emoji、复杂对话或 AI Boss Director。
- 不引入 MassEntity 之外的新第三方群体模拟框架；如 Mass 集成阻塞，可先用轻量测试 Agent 验证同一模块接口。
- 不引入 Embedding、向量数据库、摘要、知识图谱或自动事实抽取。

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
| `M5-A08` | Extreme Gunshot 事件可以按硬约束覆盖低优先级好奇行为；Intent 冷却和迟滞避免高频往返切换。 |
| `M5-A09` | 受影响 UE Target 编译、社会模拟自动化、现有 `ZLAIRuntime` 自动化和一个无界面 Gameplay 集成场景通过。 |
| `M5-A10` | 目标测试场景注册至少 100 个 Level 1 Agent；记录固定硬件与场景下的查询量和处理耗时，Debugger/日志不泄露对话、密钥或内部敏感数据。 |

## 完成定义

实现工作包完成并通过适用离线验证后，里程碑进入 `验收中`。只有 `M5-A01` 至 `M5-A10` 均有可复查证据时，Milestone 5 才能标记为 `已完成`。

验证证据统一写入 `Docs/Validation/Milestone5Validation.md`。该文件在首次产生实际验证结果时创建；不得提前填写未执行结果。
