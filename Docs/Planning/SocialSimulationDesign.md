# ZLAI Social Simulation Runtime 总设计

## 文档职责

本文档定义目标系统的领域模型、运行链路和长期模块设计，是总规划的技术设计配套文档。它不声明功能已经实现，不定义当前里程碑范围，也不替代 UE/Python 通信协议。

当前允许实施的范围见 [CurrentMilestone.md](../Current/CurrentMilestone.md)，当前已实现架构见 [Architecture.md](./Architecture.md)，线上字段以 [Protocol.md](../Reference/Protocol.md) 为准。

目标系统必须服务于 [最终场景 1：开放式社会交互沙盒](./FinalScenarios/Scenario1OpenSocialSandbox.md)。场景先定义玩家可操作体验和观察属性，本设计再为其提供领域模型与技术边界。

## 总体架构

```text
Player Speech / Action Text + Embodied Movement
  -> UE Input Adapter + Gameplay Event
  -> Event Router + Spatial Query
  -> Directional Sight / Bounded Hearing
  -> Per-NPC Observation
  -> Instant State Update
  -> Social Memory Update
  -> Relationship / Reputation Update
  -> Decision Scheduler
       |- Level 1 Rule + StateTree
       |- Level 2 Rule Shortlist + AI Decision
       `- Level 3 Deep Context + AI Decision
  -> UE Tool Registry / Intent Executor
  -> StateTree / GAS / AIController
  -> Speech Bubble / Action Bubble / Gameplay Result
  -> New Gameplay Event
```

## 场景驱动与可显化要求

- 自然语言输入可以开放，但真实世界动作必须映射到有限、已注册、可校验的能力。
- 说话输入与行为描述必须分离；NPC 不会“听见”玩家输入的行为命令，只能观察实际执行结果。
- 每个 NPC 只接收自己的 Observation，不把完整 World Snapshot 当作所有 NPC 的共同知识。
- LLM 生成的语言通过对话气泡显化；已经执行或正在执行的动作通过角色移动、动作气泡或状态标识显化。
- 内部状态、隐藏目标和模型隐式推理不向玩家公开；开发 Inspector 只显示输入来源、结构化结果、校验结果和公开 Reason Code。
- 冲突、基础战斗、逃跑、求助和缓和复用同一感知—判断—执行循环。LLM 负责高层目标、策略和表达，UE 负责即时移动、防卫、命中、伤害与合法性。
- 每个未来里程碑必须提供场景入口和玩家操作路径；只有自动化、日志或无界面测试不能单独构成里程碑完成。

## 权威边界

UE 是 Gameplay 和社会模拟的唯一事实来源：

- NPC 位置、存活、阵营、能力和当前行为。
- Event、Instant State、Relationship、Reputation 和 Faction Standing。
- Tool 权限、参数、目标有效性、距离、冷却和执行结果。
- Movement、Animation、Damage、Spawning、GAS 和 StateTree。

Python 只负责少量 Level 2/3 的高层意图、结构化表达和对话。Python 不读取 UE World，不直接修改关系或状态，也不假定 ToolCall 已执行。

Event 或显著可感知变化到达后，UE 立即更新状态和记忆，不等待 LLM。异步回复必须携带可用于过期判断的状态版本；世界已经变化时 UE 丢弃、继续当前安全动作或重新规划。

## 目标 UE 模块

| 模块 | 职责 |
| --- | --- |
| `ZLAIRuntime` | 已有 Dialogue/Memory Client；未来增加独立 Decision Client 和响应解析 |
| `ZLASocialRuntime` | Event、空间传播、感知、人格、状态、关系、社会 Memory、规则决策和调度 |
| `ZLAIRuntimeDebugger` | Developer Module；NPC Inspector、事件链、决策和性能可视化 |
| `ZL` | 玩家行为适配、NPC/Mass 配置、具体 StateTree/GAS/动画 Handler、场景和 UI |

通用 Tool 注册与校验边界可放入 `ZLASocialRuntime`；具体 Gameplay Handler 由 `ZL` 注册，避免插件反向依赖游戏模块。

## 目标 Python 模块

在现有 Dialogue/Memory 路径旁增加独立 Decision 纵向切片：

```text
app/
  api/decision.py
  schemas/decision.py
  services/decision_service.py
  services/decision_context_builder.py
  planners/base.py
  planners/kimi_planner.py
  planners/stub_planner.py
```

UE 向 Decision Service 提供已经裁剪的人格、状态、Memory、Relationship、World Facts、候选 Intent 和允许 Tools。Python 不主动查询社会状态，不持有 Gameplay 权威副本。

## 三级 NPC

### Level 1：Mass NPC

- 目标数量 100 至 120。
- 不调用 LLM。
- 使用 MassEntity、StateTree、Gameplay Tags 和确定性 Utility Rule。
- 普通更新按 LOD 降频；高严重度 Event 即时调度。
- 行为候选：Ignore、Observe、Flee、Investigate、Report、Assist、Confront。

状态机建议：

```text
Idle -> React -> Act -> Recover -> Idle
                 |- Observe
                 |- Flee
                 |- Investigate
                 |- Report
                 |- Assist
                 `- Confront
```

事件优先级基线：

```text
Gunshot > Kill > Direct Attack > Punch > Steal/Help > Speech
```

### Level 2：Important NPC

目标数量 5。常规行为继续使用规则；只有高重要度、规则候选接近、需要生成对话或涉及长期策略时才进入 AI Decision。UE 最多提供三个合法候选 Intent。超时或失败时使用规则第一候选。

### Level 3：Core NPC

MVP 只做 1 个。增加更大的 Memory 预算、个人目标、关键关系和复杂对话，但继续复用 Level 2 的 Tool 校验与执行路径。

## Event 模型

Event 使用不可变根信息和受限传播元数据：

| 字段 | 语义 |
| --- | --- |
| `event_id` | 当前事件唯一 ID |
| `root_event_id` | 整条传播链根 ID |
| `parent_event_id` | 当前传播父节点 |
| `type_tag` | Gameplay Tag 事件类型 |
| `source` / `target` | 来源与直接对象 |
| `position` | UE 权威位置 |
| `radius` | 最大传播范围 |
| `severity` | 事件严重度 |
| `noise` | 听觉强度 |
| `channels` | Direct、Visual、Auditory、Social |
| `faction_impact` | 阵营影响候选值 |
| `chain_depth` | 当前传播深度 |
| `chain_budget` | 剩余传播预算 |
| `timestamp` / `expiry` | 生命周期 |
| `causation_id` | Gameplay/Decision 因果关联 |

范围由数据资产配置。建议基线：Whisper 2m、Talk 8m、Shout 25m、Punch 10m、Steal 8m、Gunshot 100m、Help 12m；Kill 的视觉和听觉范围由实际死亡与武器事件共同决定。

## Spatial Query 与过滤

NPC 按位置注册到二维 Uniform Grid；只在跨 Cell 时更新索引。Event 产生时只查询覆盖 Cell，再依次执行距离、视线、听觉、状态、阵营、阈值和去重过滤。

```text
effective_intensity =
severity * channel_gain * distance_falloff * occlusion_factor
```

直接受害者使用 Direct Channel，不受普通半径过滤。视觉需要检查观察者朝向、视野角和距离；听觉需要区分小声、正常、大喊和面向单个近距离对象的耳边说话。禁止每帧对全部 NPC 扫描 Event。

## 传播限制

MVP 默认：

- 最大 Chain Depth 为 2。
- 单节点最大社会传播对象为 6。
- Root Event 维护接收去重。
- 每个事件有 TTL 和 Chain Budget。
- 低 Importance 事件不得生成二级传播。
- 同一 NPC 对同一 Root Event 只报告一次。
- Tool 执行产生的即时结果继承 Root 与剩余预算。

阵营声望不由每个目击者直接全局修改。Witness 先产生 Report，Guard/Faction Authority 再确认并更新。

## Personality、State 与 Relationship

人格数据使用 DataAsset 定义 Archetype，DataTable 保存批量 NPC 实例覆盖。核心 Trait 为 `[0,1]`：

- Brave
- FearSensitivity
- Curiosity
- Justice
- Aggression
- Social

Instant State 为 Fear、Anger、Curiosity、Alert，按秒或分钟衰减。Gameplay Tags 只表达跨过阈值后的离散状态，连续值保留在状态数据中。

Long-Term State 包含 Trust、Reputation、Relationship 和 Faction Standing。关系使用稀疏边，不为 100+ NPC 建立完整 N×N 图。

行为分数：

```text
Utility(Action) =
EventBase
+ PersonalityWeights
+ InstantStateWeights
+ RelationshipWeights
+ Occupation/FactionBonus
- Risk
- DistanceCost
- CooldownPenalty
```

硬约束先于 Utility。枪击等 Extreme 事件可以覆盖普通好奇心，避免人格导致明显不合理行为。

## Memory

必须区分两个域：

- **UE Social Memory**：Gameplay 决策事实，UE 权威。
- **Python Dialogue Memory**：对话连续性，沿用现有 SQLite。

Social Memory 至少包含 Root Event、类型、主客体、位置、时间、可信度、情绪影响、Importance、Relationship Delta 和来源通道。

```text
importance =
0.35 * severity
+ 0.25 * personal_relevance
+ 0.15 * novelty
+ 0.15 * relationship_impact
+ 0.10 * emotional_arousal
```

建议容量：Level 1 Short 6；Level 2 Short 16、Long 8；Level 3 Short 32、Long 24。Short Memory 使用固定环形缓冲区；Long Memory 只接收高 Importance 或明确锚定事件。

```text
effective_importance = base_importance * exp(-decay_rate * elapsed_time)
```

检索先按事件、人物、地点和时间过滤，再按 Importance、Recency、Query Relevance 和 Emotional Match 排序，返回有界 Top-K。MVP 不使用 Embedding。

## Decision 流程

```text
Event Accepted
  -> Instant State
  -> Social Memory
  -> Relationship Candidate Delta
  -> Rule Candidate Intents
  -> Tier/Budget Check
  -> Rule Decision or Async AI Decision
  -> UE Validation
  -> Gameplay Execution
  -> Execution Result Event
```

Decision 调度除 Event 外，也可以由距离跨阈值、目标开始或停止靠近、动作完成、受击、玩家新的说话内容和当前计划失效触发。LLM 不逐帧控制角色；请求等待期间继续执行已有计划或确定性安全动作。

未来 Decision 响应可包含 `decision_id`、状态版本、Intent、可选 Speech、可选 ToolCall、Confidence 和 Expiry，但这些字段只有在协议获得明确确认后才能写入 [Protocol.md](../Reference/Protocol.md)。不得返回或保存 Chain-of-Thought。

对于需要更丰富角色表现的场景，Decision 应演进为“开放高层计划 + 有限执行原语”：UE 发送该 NPC 已感知的社会事实、人物、关系、即时状态、近期经历和当前可用能力；LLM 自主判断目标、表达、表现倾向与短期步骤组合；UE 只校验并执行本次显式提供的能力实例。目标、理由和表情倾向不是已发生的世界事实，步骤成功/失败必须由 UE 回流后才进入个人记忆。该模型允许角色策略开放组合，同时保留移动、攻击、报告、互动立场和表现资产的 UE 权威与安全边界。

## Tool 验证

UE 对每个 ToolCall 检查：注册、Capability、参数 Schema、Target、状态版本、距离、导航、视线、当前状态、冷却、速率、幂等和 Authority。

最终场景 1 的初始 Tool 候选按可见闭环逐步开放：Speak、FaceTarget、MoveToward、MoveAway、Stop、ObserveTarget、FleeFrom、CallForHelp、Defend。未知或非法 Tool 不执行 Gameplay；如 Speech 合法，可只显示 Bubble 和白名单 Emoji。攻击、命中、伤害和逐帧战斗动作继续由 UE Gameplay 实现，不由 LLM 直接调用任意能力。

## Dialogue 与表达

玩家 Speech Event 支持 Whisper、Talk、Shout 和定向 InEar；模式影响传播范围、Noise、目标约束和旁观者可听性。玩家可以不指定目标，NPC 根据称呼、朝向、距离和上下文判断是否指向自己。NPC 表达包含 Text、Emotion Tag、Expression Tag 和白名单 Emoji ID。表达可以独立于 Tool 执行；Tool 被拒绝不必丢弃合法的纯表达。

玩家行为文本先解析为候选动作和可选目标，再由 UE 校验并执行；无法映射或非法的行为必须给玩家明确反馈，不得仅通过叙述假装世界已经改变。

## Runtime Debugger

NPC Inspector 显示 Identity、Occupation、Faction、AI Level、Personality、Instant/Long State、Memory、Relationship、Last Event、候选 Intent、Decision 来源、ToolCall、校验结果、Request ID 和延迟。

Event Graph 显示 Root、Parent、Depth、Budget、感知通道、过滤结果和子事件。性能面板显示 NPC 数、Event 数、Spatial Query 候选/通过数量、Rule 次数、LLM 队列、延迟和 Tool 拒绝数量。

Debugger 不显示完整 Prompt、API Key、原始 Provider 异常或模型隐式推理。

## 性能与失败预算

- Level 1 不调用 LLM。
- 规则只在 Event、状态阈值或行为完成时重新评估。
- LLM 请求队列有界，并发建议不超过 2。
- 每个 Important/Core NPC 有决策冷却。
- 回复超过 Expiry 或状态版本不匹配时丢弃。
- Python 不可用时所有 NPC 保持确定性降级能力。
- 自动化默认使用 Stub/Fake、临时数据和本机网络。

## AI Boss Director 扩展边界

Boss Director 只消费 Combat Snapshot 并输出 AssignRole、RequestReinforcement、ChangePhase、FocusTarget 或 RetreatGroup 等战术意图。GAS Effect、伤害、Spawn、动画和 StateTree Transition 仍由 UE 执行。该模块不属于 MVP。
