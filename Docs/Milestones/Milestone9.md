# Milestone 9：连续互动、冲突升级与缓和

## 状态

- 状态：`已完成`
- 开始日期：2026-09-01
- 前置里程碑：Milestone 1 至 8 已完成
- 验收证据：[Milestone9Validation.md](../Validation/Milestone9Validation.md)
- 场景来源：[最终场景 1：开放式社会交互沙盒](../Planning/FinalScenarios/Scenario1OpenSocialSandbox.md)

历史里程碑：

- [Milestone 1：UE 到 Python Service 最小闭环](../Milestones/Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](../Milestones/Milestone2.md)
- [Milestone 3：NPC 上下文与人格](../Milestones/Milestone3.md)
- [Milestone 4：持久化对话 Memory](../Milestones/Milestone4.md)
- [Milestone 5：确定性社会模拟基础](../Milestones/Milestone5.md)
- [Milestone 6：关系、长期记忆与重要 NPC](../Milestones/Milestone6.md)
- [Milestone 7：可操作交互舞台与定向感知](../Milestones/Milestone7.md)
- [Milestone 8：单 NPC LLM 具身反馈与受控动作](../Milestones/Milestone8.md)

长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，目标系统设计见 [SocialSimulationDesign.md](../Planning/SocialSimulationDesign.md)，当前已实现边界见 [Architecture.md](../Planning/Architecture.md)。

## 目标

在 Milestone 8 的单 Guard Decision 闭环上增加有界的连续重新判断和最小冲突 Gameplay。玩家可以通过语言、距离变化、停止、退让、道歉、求助或基础攻击随时改变局势；Guard 只依据自己实际感知的事件和最近外部表现更新表达与行动：

```text
玩家说话、移动或执行基础攻击
  -> UE 产生 Guard 实际感知到的显著变化或冲突结果
  -> 连续调度器合并触发、执行冷却并保留最新待判断事实
  -> Guard 的个人历史包含自己刚刚的表达与已执行动作
  -> Stub 或 Kimi 更新公开 Intent、Speech 与现有四个 Tool 建议
  -> UE 继续权威执行移动、停止、即时防卫、命中与伤害
  -> 新结果再次进入个人 Observation，并在需要时重新判断
  -> 停止、退让或道歉可以降低冲突并形成可见缓和
```

本里程碑继续只让 `npc_guard` 使用 LLM Decision。冲突属于现有开放互动循环的升级阶段，不建设独立战斗沙盒，也不扩展为多 NPC LLM 场景。

## 协议边界

- 本里程碑不修改 [Protocol.md](../Reference/Protocol.md)，不新增 Endpoint、字段、Intent、Tool、状态码或兼容规则。
- 继续使用 `respond`、`engage`、`disengage`、`hold` 和 FaceTarget、MoveToward、MoveAway、Stop；逃离可由 MoveAway 表达，其他即时防卫、攻击、命中、伤害和求助反馈由 UE Gameplay 规则负责。
- 多轮连续性只通过现有 `trigger`、`instant_state`、`relationship` 和 `recent_history` 传递。历史中的 NPC Speech 与 Action Result 必须是已经公开表达或真实发生的事实，不编码隐藏推理或 Chain-of-Thought。
- 如果实现证明必须新增协议能力，应停止对应工作包，先说明原因并取得明确确认；其他不依赖协议变更的工作继续推进。

## 玩家可操作成果

- 玩家可以继续自由说话，使用威胁、解释、道歉或求助等未枚举自然语言改变 Guard 的判断。
- 玩家可以靠近、停止、退让并通过行为模式发起一个有距离、目标和冷却校验的基础攻击；失败时得到明确反馈且不产生伤害。
- 玩家可在 Guard 请求等待、移动或冲突过程中改变路线，系统使用最新权威状态接受、拒绝或重新调度结果。
- 至少一条可重复 Stub 路径允许普通交流升级为警告、后退或基础冲突，并因玩家停止、退让或道歉再次缓和。

## 屏幕可见成果

- Guard 的对话气泡、动作气泡、位置、生命/受击状态和当前公开立场保持一致；不会只用文字假装攻击、防卫或退让已经发生。
- 距离跨阈值、玩家攻击、Guard 受击、动作完成、停止与新的说话内容可以触发重新判断，但不会逐帧请求 LLM。
- 开发视图显示最近触发原因、当前公开立场、冲突等级、生命值、连续调度状态、待处理触发、Decision 来源、Tool 校验和公开 Reason Code。
- Python 不可用、请求超时或结构无效时，Guard 仍执行有界的本地安全动作并显示 `LocalFallback` 来源。

## 本阶段范围

### 连续 Decision 调度

- 为单 Guard 增加 Event 驱动调度器，接收新 Speech、已执行 Action Result、距离跨阈值、计划完成、受击和当前计划失效等显著触发。
- 同一时刻最多一个请求在途；冷却窗口内合并重复触发，容量固定，只保留足以代表最新权威变化的待处理触发。
- 请求期间状态变化继续推进 Authority State Version；旧响应的 Speech 与 Tool 按现有独立处理和过期规则接受或拒绝。
- 重新判断不是逐帧控制；等待期间继续执行当前合法计划或确定性安全动作。

### 多轮外部连续性

- Guard 自己已经显示的 Speech 和实际开始、完成、取消、失败的 Action Result 进入自己的有界 Observation History。
- Planner 使用现有个人历史、即时状态和关系快照维持公开立场连续性；不得读取其他 NPC Observation、完整 World 或隐藏 Gameplay 状态。
- Stub 根据受控事实类别生成可重复的升级、维持或缓和结果；Kimi 继续允许自由表达，不把固定台词或固定 Tool 当作验收条件。

### 最小冲突 Gameplay

- 玩家行为白名单增加一个基础 Attack，必须通过目标、距离、可执行状态和冷却校验后才命中并产生伤害。
- Guard 具有有界生命、受击、短暂防卫和失去战斗能力状态；UE 决定命中、伤害、无敌窗口和动作结果。
- Guard 的即时安全规则可在高威胁或请求不可用时停止、后退、防卫或发出求助反馈；规则不假装 LLM 已作出决定。
- 攻击、受击、防卫、停止和失能结果形成个人可感知 Action Observation，并推进权威状态版本。

### 升级与缓和状态

- UE 维护公开、可调试且有界的冲突等级与 Guard 立场；变化只来自已感知威胁、距离、实际攻击/受击、玩家停止或退让，以及 Guard 可听见的新语言。
- 自然语言的社会含义由 Planner 判断；UE 不把任意词表直接当作世界事实。Stub 只为自动化使用有限可复现分类。
- 口头道歉不直接撤销伤害或强制清空警戒；是否缓和必须与当前距离、最近可见行为、关系和 Planner 结果相容。
- 重置恢复确定初始生命、位置、历史、调度器、冲突状态和请求 Generation。

### 可见反馈、测试与验收

- UI 和 Bubble 区分说话、受击、防卫、后退、求助、失能、拒绝和本地降级。
- UE 自动化覆盖调度合并、触发边界、历史裁剪、攻击校验、伤害零副作用拒绝、升级、缓和、过期和离线降级。
- Python 覆盖 Stub/Kimi 对连续历史的结构化处理、边界、错误分类和日志脱敏。
- Stub 场景自动化完成一条升级再缓和的纵向链路；真实 Kimi 人工验收只比较行为属性，不比较固定台词。

## 数据与权威边界

- UE 是位置、距离、视线、生命、命中、伤害、防卫、冲突等级、状态版本、动作执行和结果 Event 的唯一事实来源。
- Python 只使用 UE 提供的单 NPC 个人事实提出公开表达和现有高层建议，不执行攻击、不计算伤害、不直接修改关系或冲突状态。
- NPC 自己的先前 Speech 可以作为已公开事实进入个人历史；模型隐式推理、内部 Prompt 和未执行建议不得进入 Observation。
- 所有触发队列、历史、请求、冷却、攻击频率、生命、伤害、调试记录和连续重规划次数都有硬上限。

## 明确不做

- 不修改 Decision 或 Dialogue 协议，不新增 Flee、CallForHelp、Defend、Attack 等 LLM Tool。
- 不让其他 NPC 使用 LLM Decision，不实现 Milestone 10 的多 NPC 人物差异和旁观者 LLM 反应。
- 不建设完整战斗系统、连招、武器、GAS、正式动画、复杂导航、尸体、掉落、犯罪或复活系统。
- 不允许 LLM 决定命中、伤害、无敌、死亡、Transform 或逐帧动作。
- 不实现无限自主循环、并行请求、多 Tool、多步 Agent、后台计划、长期目标持久化或新的 Memory 数据库。
- 不新增 Embedding、向量数据库、语音、唇形同步、MassEntity 正式集成、多人同步或 Server Authority。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M9-A01` | Milestone 1 至 8 的适用 Dialogue、Decision、Memory、社会模拟和沙盒回归保持通过；`Protocol.md` 与两端协议类型无变更。 |
| `M9-A02` | 新 Speech、动作结果、距离阈值、计划完成、受击和计划失效能按规则触发单 Guard 重新判断；请求最多一个在途，重复触发被有界合并且不逐帧调用。 |
| `M9-A03` | 玩家可通过正常 UI 靠近、停止、退让和执行基础 Attack；目标、距离、状态或冷却非法时零伤害且反馈可见。 |
| `M9-A04` | UE 权威处理命中、伤害、防卫、失能和相关状态版本；Python 或模型输出不能绕过规则改变生命或世界状态。 |
| `M9-A05` | Stub 纵向路径能从普通交流升级到警告、后退或基础冲突，再因玩家停止、退让或道歉形成可见缓和；不依赖固定模型台词。 |
| `M9-A06` | Guard 的请求只包含它实际感知的 Trigger、自己的公开 Speech、真实 Action Result 和有界个人历史；未感知事实、未执行建议和隐藏推理不会进入上下文。 |
| `M9-A07` | 请求期间距离、生命、目标或计划变化会推进状态版本；过期 Tool 不执行，合法 Speech、Tool 拒绝与后续重新判断能独立呈现。 |
| `M9-A08` | Bubble 与开发视图能显示冲突等级、公开立场、生命、触发原因、调度状态、Decision 来源、动作结果和拒绝原因，并与实际世界状态一致。 |
| `M9-A09` | Python 离线、超时、无效结构和请求失败触发有界本地安全行为；玩家仍能输入、移动、攻击、停止和重置，且不会形成请求风暴。 |
| `M9-A10` | 受影响 UE Target 编译、Python/UE 回归、Stub 升级—缓和场景、失败路径和真实 Kimi 行为属性验收通过；Validation 保存可复查且脱敏的证据。 |

## 完成定义

1. [TaskBoard.md](./TaskBoard.md) 中 Milestone 9 工作包全部完成。
2. `M9-A01` 至 `M9-A10` 均有可复查验证证据。
3. Stub 与真实模型都完成至少一次基于个人视角的连续多轮重新判断；真实模型不要求固定台词或固定 Tool。
4. 至少一段可操作互动从普通交流自然升级为冲突并再次缓和，公开表达、真实动作和权威状态一致。
5. 非法攻击和过期建议零 Gameplay 副作用；Python 不可用时仍有可见、有界的安全降级。
6. Architecture、UE/Python Reference、Decision Log、Validation、Task Board 和 Project State 已按职责同步，Milestone 9 范围完成后归档。
