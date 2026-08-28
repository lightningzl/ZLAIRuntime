# Milestone 6：关系、长期记忆与重要 NPC

## 状态

- 状态：`已完成`
- 完成日期：2026-08-28
- 验收证据：[Milestone6Validation.md](../Validation/Milestone6Validation.md)

本文档是 Milestone 6 完成时的范围定稿，只用于历史追溯，不定义当前允许实施的范围。

## 前置里程碑

- [Milestone 1：UE 到 Python Service 最小闭环](./Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](./Milestone2.md)
- [Milestone 3：NPC 上下文与人格](./Milestone3.md)
- [Milestone 4：持久化对话 Memory](./Milestone4.md)
- [Milestone 5：确定性社会模拟基础](./Milestone5.md)

长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，目标系统设计见 [SocialSimulationDesign.md](../Planning/SocialSimulationDesign.md)。

目标：在 Milestone 5 的纯 UE 确定性社会模拟基础上，增加有界的 NPC→NPC 社会传播、个人关系、Faction Standing、Important NPC Long Memory 和关系感知规则决策，形成以下可测试纵向切片：

```text
Authoritative Gameplay Event
  -> Direct Perception + Short Memory
  -> Relationship Candidate Delta
  -> Rule Intent.Report
  -> Explicit Gameplay Report Confirmation
  -> Bounded Social Event Propagation
  -> Important NPC Social/Long Memory
  -> Authority Validation + Relationship/Faction Update
  -> Deterministic Investigate/Report/Assist/Confront Intent
```

本阶段继续保持所有社会事实、传播、关系、Memory 和最终 Intent 由 UE 掌握。Important NPC 的社会决策仍使用本地规则；Python 只保留已经验证的 Dialogue 能力，不新增 Decision Endpoint、ToolCall 或社会状态存储。

## 本阶段范围

### Event Chain 与二级传播

- 为社会 Event 增加稳定传播元数据：`root_event_id`、`parent_event_id`、`chain_depth`、`chain_budget`、`causation_id` 和 Social 来源信息。
- 增加 Social 感知通道；二手接收者必须区分 Direct、Visual、Auditory 和 Social，不能把报告当成亲眼目击。
- 使用显式 Gameplay Report API 创建派生 Event；`Intent.Report` 本身不得直接修改接收者 Memory、Relationship 或 Faction Standing。
- 派生 Event 保留不可变 Root 事实、报告者、原始感知来源和衰减后的 Confidence，不覆盖原始 Event。
- 默认最大 Chain Depth 为 2，单节点最大 Fan-out 为 6；Root Budget、TTL、每个 Root/Agent 去重和每个 Reporter/Root 单次报告均有硬限制。
- 低 Importance、过期、预算耗尽、深度超限或重复传播不得产生新事件。
- 当前传播只支持结构化事实与 Confidence 衰减，不实现谣言文字生成、事实失真或多源确认推理。

### Long-Term State、Relationship 与 Faction Standing

- 使用稳定 Agent/Faction ID 表示关系主体，不在 Social Runtime 中保存具体 Actor 引用。
- Personal Relationship 使用稀疏有向边，只为发生过相关交互的 `(observer_id, subject_id)` 创建状态，不建立完整 N×N 图。
- Relationship 至少保存有界的 Trust、Affinity、Fear、Familiarity、最后更新时间和最近因果 Event；具体数值更新必须确定、可重复并可衰减或回归中性。
- Reputation 表示 Important NPC/Authority 对目标的有界长期评估；Faction Standing 只由具备 Authority Capability 的确认路径更新。
- 普通 Witness 可以产生 Report 和个人 Relationship Delta，但不能直接全局修改 Faction Standing。
- Direct 受害、Visual Witness 和 Social Report 必须因来源、Confidence 与 Personal Relevance 不同产生不同 Relationship Delta。
- Help、Punch、Steal、Kill 的长期影响预留统一规则入口；本阶段至少完成 Help 与 Punch 的确定性正负关系闭环。

### Important NPC 与有界 Long Memory

- 增加 Level 2 Important NPC 标识、Occupation/Faction、Authority 与报告接收能力；目标场景注册 5 个 Important NPC。
- Important NPC 常规社会行为继续使用 UE Rule/Utility，不调用 LLM Decision。
- Level 1 Agent 保持容量为 6 的 Short Memory；Important NPC 使用建议容量 Short 16、Long 8，容量必须可配置但有硬上限。
- Long Memory 只接收高 Importance 或显式 Anchored Event；低重要度事件保留在 Short Memory 或按规则淘汰。
- Long Memory 保存 Root Event、主客体、地点、时间、来源通道、Confidence、Importance、Relationship Delta 和因果来源。
- 实现不依赖 Embedding 的有界检索：先按 Event、人物、Faction、地点和时间过滤，再按 Importance、Recency、Query Relevance 与 Emotional Match 稳定排序，返回 Top-K。
- 本阶段 Long Memory 只存在于 UE Runtime，不写入 SaveGame、Python SQLite 或 Dialogue Memory。

### Rule Decision 与可观察性

- Rule Decision 增加 `Investigate` Intent，并允许 Relationship、Reputation、Faction Standing、相关 Memory、Occupation/Faction 和来源 Confidence 参与候选评分。
- 相同人格面对相同事件时，不同个人历史、关系和信息来源必须能够产生不同且可解释的 Intent。
- 能力硬约束、Authority、事件优先级、冷却和迟滞继续先于最终切换，避免重复报告和行为抖动。
- Intent Command 或只读 Debug Snapshot 提供稳定 Reason Code/Score Contribution，不暴露模型隐式推理，也不反向驱动状态。
- 扩展最小 Inspector/调试命令，显示 Root/Parent、Depth/Budget、来源通道、Confidence、Relationship、Faction Standing、Short/Long Memory 和最终 Intent。
- 聚合统计增加传播创建/拒绝数、Root 去重数、Relationship 边数、Long Memory 项数和规则评估耗时。

### Important NPC 纵向演示

- 提供无界面确定性场景：玩家 Punch NPC，Witness 直接感知并产生 Report，Gameplay 显式确认报告完成，Important Guard 接收 Social Event 并获得二手 Memory。
- Guard 依据报告 Confidence、个人/阵营关系和 Authority 规则产生 Investigate、Report 或 Confront 等合法 Intent。
- 同一 Root Event 不得因重复报告导致重复 Relationship、Reputation 或 Faction Standing 更新。
- 场景只验证数据闭环与 Intent 交付，不声称已经实现找守卫、移动、动画、StateTree 或 AIController 执行。

## 数据与权威边界

- UE 是 Event Chain、Perception、Relationship、Reputation、Faction Standing、Social Memory 和 Intent 的唯一事实来源。
- 派生传播只能由明确的 UE Gameplay 确认入口产生；规则选择或调试读取不得自行制造 Gameplay 成功结果。
- Social Memory 与 Python Dialogue Memory 保持隔离，不自动同步、摘要或互相提升。
- 当前 [Protocol.md](../Reference/Protocol.md)、`POST /v1/dialogue` 请求/响应、错误、超时和 SQLite Memory 语义保持不变。
- Important NPC 可以继续使用现有纯文本 Dialogue，但本阶段不把 Relationship、Faction Standing 或 Social Memory 自动注入 Dialogue 请求。
- 所有列表、图边、传播深度、Fan-out、Budget、Memory、检索 Top-K 和每次处理工作量必须有硬上限。

## 明确不做

- 不新增或修改 Decision Endpoint、ToolCall、结构化 AI Decision 或现有 Dialogue 协议。
- 不调用 LLM 进行社会决策，不实现 Level 3 Core NPC。
- 不实现具体 StateTree、Behavior Tree、AIController、导航、动画、GAS 或完整 Report/Flee/Confront Gameplay 执行器。
- 不正式集成 MassEntity、Representation LOD 或 World Partition；继续通过独立 Agent 数据接口验证规则边界。
- 不实现 Social Memory SaveGame、数据库持久化、Embedding、向量检索、摘要或知识图谱。
- 不实现谣言文本生成、传播失真、多源事实合并、自动事实抽取或开放式自然语言报告。
- 不实现完整城市犯罪系统、经济、任务、政治、复杂阵营层级或全局 N×N Relationship 图。
- 不实现三维空间索引、楼层/房间传播、真实声学或真实 World LOS 接入。
- 不实现正式 Runtime Debugger UI、Speech Bubble、Emoji 或完整面试场景表现。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M6-A01` | Milestone 1 至 5 的 Dialogue、Context、Dialogue Memory、社会模拟、错误和超时回归保持通过；`Protocol.md` 与 Python Runtime 行为不改变。 |
| `M6-A02` | Event Chain 的 Root/Parent、Depth、Budget、TTL、Fan-out、Causation 和 Root/Agent 去重行为确定；深度、预算、过期和重复边界有自动化覆盖。 |
| `M6-A03` | `Intent.Report` 不直接产生成功副作用；只有显式 Gameplay 确认才创建 Social 派生 Event，且保留来源链并确定性衰减 Confidence。 |
| `M6-A04` | Personal Relationship 使用稀疏有向边；Trust、Affinity、Fear、Familiarity 和 Reputation 更新有界、可重复，不随注册 NPC 数量预建 N×N 状态。 |
| `M6-A05` | 普通 Witness 不能直接修改 Faction Standing；具备 Authority Capability 的 Important NPC 只有在合法、未重复且 Confidence 达标的确认路径中才能更新。 |
| `M6-A06` | Level 1 Short Memory 仍有界；Important NPC Short/Long Memory 的提升、衰减、容量、淘汰和稳定 Top-K 检索均有边界测试，且不访问 Python SQLite。 |
| `M6-A07` | Relationship、Faction、Occupation、相关 Memory 和来源 Confidence 能够进入 Rule Decision；相同人格在不同历史下至少产生三种可解释且重复一致的预期 Intent。 |
| `M6-A08` | 确定性场景注册 5 个 Important NPC；所有社会行为在 Python 未启动时仍由本地规则完成，不产生 Decision HTTP 请求。 |
| `M6-A09` | 无界面纵向切片完成 Punch→Witness Report→显式确认→Important Guard Social Memory→Authority/Relationship 更新→Intent 交付，同一 Root 重放不重复生效。 |
| `M6-A10` | 120 个 Level 1 与 5 个 Important NPC 的有界传播/关系/Memory 基准、最小 Inspector、受影响 Target 编译和适用 UE/Python 回归通过；Validation 记录固定环境且不泄露对话、scope、Prompt 或凭据。 |

## 完成定义

实现工作包完成并通过适用离线验证后，里程碑进入 `验收中`。只有 `M6-A01` 至 `M6-A10` 均有可复查证据时，Milestone 6 才能标记为 `已完成`。

实际验证证据见 [Milestone6Validation.md](../Validation/Milestone6Validation.md)。
