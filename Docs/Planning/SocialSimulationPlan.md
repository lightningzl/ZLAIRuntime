# ZLAI Social Simulation Runtime 总规划

## 文档职责

本文档是项目长期目标、交付范围、里程碑顺序和取舍原则的唯一规划来源。后续里程碑必须从本文裁剪，但只有 [CurrentMilestone.md](../Current/CurrentMilestone.md) 能定义当前允许实施的范围。

本文档不维护当前任务状态、实际验证结果、协议字段正文或实现文件清单。目标系统细节见 [SocialSimulationDesign.md](./SocialSimulationDesign.md)，当前实现边界见 [Architecture.md](./Architecture.md)。

## 项目目标

项目名称：**ZLAI Social Simulation Runtime**。

目标是制作一个小规模但系统深度高的 UE AI Gameplay Runtime，展示玩家行为如何通过事件、感知、状态、记忆、关系和决策改变 NPC 社会系统，而不是制作单纯的 AI Chat NPC。

最终 Demo 需要同时证明：

- UE Gameplay、AI、StateTree/Mass、动画与运行时工具能力。
- LLM 决策编排、结构化输出、失败降级和成本控制能力。
- 模块边界、协议、安全执行、性能预算和可验证架构能力。

## 实现状态入口

本文档不复制实时完成状态。当前已完成能力、活动里程碑和下一候选任务分别以 [ProjectState.md](../Current/ProjectState.md)、[CurrentMilestone.md](../Current/CurrentMilestone.md) 和 [TaskBoard.md](../Current/TaskBoard.md) 为准；已经接受并实现的模块边界见 [Architecture.md](./Architecture.md)。

## 核心原则

1. **UE 权威**：位置、状态、关系、伤害、移动、动画、生成和行为执行由 UE 决定。
2. **LLM 只做高层推理**：模型输出 Decision、Dialogue 和受约束 ToolCall，不直接控制世界。
3. **先确定性模拟，后接入 LLM**：Level 1 和所有降级路径不依赖外部服务。
4. **事件驱动**：Event 产生时进行空间查询和调度，不每帧扫描全部 NPC。
5. **有限预算**：传播深度、Fan-out、Memory、LLM 并发、Token 和 Tool 执行都必须有硬上限。
6. **可观察、可复现**：关键状态、事件链、决策来源和 Tool 校验可以在 Runtime Debugger 中检查。
7. **纵向切片优先**：每个里程碑都必须形成可运行、可测试的闭环。

## 最终 Demo 范围

### 场景与角色

- 一个市场街区或小型城镇广场。
- 100 至 120 个 Level 1 Mass NPC。
- 5 个 Level 2 Important NPC。
- 1 个完整 Level 3 Core NPC。
- Civilian、Merchant、Guard 三个基础阵营。

### 玩家行为

- Talk、Whisper、Shout。
- Punch、Shoot、Kill。
- Help、Steal。

### 关键展示闭环

1. 同一 Punch 事件使不同人格 NPC 产生围观、逃跑、报警、协助或对抗。
2. Gunshot 在大范围内触发 100+ NPC 的分层反应，但不进行全量每帧扫描。
3. Witness 将事件报告给 Important NPC，传播在深度和预算耗尽后终止。
4. Help、Steal、Attack 等事件改变 Trust、Relationship、Reputation 和后续行为。
5. Core NPC 使用有界记忆和世界快照产生高层 Decision 与合法 ToolCall。
6. UE 拒绝未知、过期、越权或上下文已失效的 ToolCall。
7. Python 离线、超时或输出无效时，NPC 使用确定性规则继续运行。

## MVP 取舍

MVP 只深度打磨一个场景、四类人格、七个主要 Intent 和少量 Tool。明确不做：

- 开放世界经济、任务生成和完整城市政治。
- 向量数据库、Embedding、知识图谱和自动事实抽取。
- 多人网络同步、生产级账号、云存档和数据合规系统。
- 任意模型动态创建新 Tool。
- 超过一个完整 Core NPC。
- AI Boss Director 的正式实现。

## 里程碑路线

### Milestone 5：确定性社会模拟基础

构建纯 UE 的 Event → Spatial Query → Perception → Instant State → Short Memory → Rule Decision → Gameplay Intent 闭环，并验证 100+ Level 1 NPC。不得修改现有 Dialogue 协议或接入 ToolCall。

### Milestone 6：关系与重要 NPC

增加 Long-Term State、Relationship、Faction Standing、Long Memory、事件二级传播和 5 个 Important NPC。所有行为仍可由规则完成，Python 只保留现有对话能力。

### Milestone 7：AI Decision 与 ToolCall

在单独获得协议确认后新增结构化 Decision 边界、Python Planner、UE Tool Registry/Executor、过期检测和失败降级。接入 Important/Core NPC，但不扩大 Level 1 的 LLM 调用。

### Milestone 8：Core NPC、Debugger 与交付

完成一个 Core NPC 的复杂对话与记忆组合、Runtime Decision Debugger、性能基准、受控真实模型验收和面试演示场景。

### 后续扩展

- AI Boss Director。
- 谣言可信度、多源确认和事件失真。
- 社会 Memory 的 SaveGame 持久化。
- 语义检索和摘要。
- Multiplayer Server Authority。

## 6–8 周建议节奏

| 周次 | 交付重点 |
| --- | --- |
| 1 | Milestone 5 数据模型、Event Router 和确定性测试框架 |
| 2 | Spatial Query、Perception Filter、人格和 Instant State |
| 3 | Level 1 Rule/StateTree、100+ NPC 性能基线 |
| 4 | Milestone 6 Relationship、Long Memory、事件链和 Important NPC |
| 5 | Tool Registry/Executor 与 Speech Bubble；准备协议提案 |
| 6 | 经确认后实现 Python Decision Planner 和 Level 2 集成 |
| 7 | Core NPC、完整异步决策闭环和失败降级 |
| 8 | Debugger、性能、自动化、验收记录和演示打磨 |

若必须压缩到 6 周，保留 100+ Level 1、3 个 Important NPC、1 个 Core NPC；Debugger 只实现单 NPC Inspector 和 Event Chain；不实现 Boss Director、语义 Memory 或复杂犯罪系统。

## Git 变更规划

每个提交只表达一个逻辑变更，并包含适用测试和文档。只有用户明确要求后才创建 commit。

```text
docs(docs): define social simulation roadmap
feat(ue): add gameplay event routing
feat(ue): add spatial event propagation
feat(ue): add personality and instant state
feat(ue): add bounded social memory
feat(ue): add mass npc rule decisions
feat(ue): add relationship propagation
docs(protocol): define decision contract
feat(python): add structured decision planner
feat(ue): add validated tool execution
feat(ue): integrate important and core npc decisions
feat(ue): add runtime decision debugger
test(ue): add social simulation performance scenarios
docs(docs): record final validation and demo guide
```

`docs(protocol)` 及后续 ToolCall 实现必须等待用户对协议方案的明确确认。

## 规划执行规则

- 同一时间只有一个 `CurrentMilestone.md`。
- 未来能力可以保留在本文，但不得提前加入当前 Task Board。
- 切换里程碑前必须归档已完成的当前范围。
- 每项验收标准使用稳定 ID，证据只写入对应 Validation 文档。
- 如果实现证明目标不合理，应先更新本文的取舍或顺序，再修改后续里程碑；不得用 ProjectState 反向改写规划。
