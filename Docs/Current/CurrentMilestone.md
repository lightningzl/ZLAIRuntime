# Milestone 10：最终场景 1 多 NPC 交付

## 状态

- 状态：`已完成`
- 开始日期：2026-09-01
- 前置里程碑：Milestone 1 至 9 已完成
- 验收证据：[Milestone10Validation.md](../Validation/Milestone10Validation.md)
- 场景来源：[最终场景 1：开放式社会交互沙盒](../Planning/FinalScenarios/Scenario1OpenSocialSandbox.md)

历史范围已归档至 [Milestone 1 至 9](../Milestones/)。长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，目标系统设计见 [SocialSimulationDesign.md](../Planning/SocialSimulationDesign.md)，当前已实现边界见 [Architecture.md](../Planning/Architecture.md)。

## 目标

把 Milestone 9 的单 Guard 连续互动扩展为最终场景 1 的 4 NPC 可操作交付。Guard、Merchant、Rival 和 Civilian 使用稳定但不同的身份、人物、关系与经历；每个 NPC 只依据自己的 Speech/Action Observation、公开表达和真实动作结果判断。同一玩家输入可以形成不同但可解释的语言、移动、冲突立场或确定性旁观反馈：

```text
玩家选择目标或不选目标并说话、移动或攻击
  -> UE 为 4 个 NPC 分别计算视觉、听觉、清晰度和目标判断
  -> 每个 NPC 只保存自己的 Observation 与公开历史
  -> 有资格的 Important NPC 进入逐 NPC 有界 Decision 调度
  -> Stub 或 Kimi 使用该 NPC 的人物、关系、状态和个人历史
  -> UE 独立校验并执行该 NPC 的 Speech 与现有四个 Tool
  -> 旁观者只对自己实际感知的内容产生规则或 Decision 反馈
  -> Inspector、气泡、移动和冲突状态显示各自结果
```

## 交付后修正：玩家可见中文化

- 将社会交互舞台的操作说明、下拉选项、状态反馈、角色名称/生命标识、气泡、Inspector 与本地 Stub 台词统一为简体中文。
- Kimi Decision 固定要求以简体中文输出公开 Speech；角色身份、个人上下文和结构化字段保持不变。
- 协议字段、Tool 名称、稳定 Reason Code、命令行参数和内部日志标识仍使用既有机器可读值，不作为玩家可见文本。
- 本修正只改变呈现与公开 Speech 语言，不新增能力、不修改 `Protocol.md`、不改变感知、调度或 Tool 执行语义。
- 玩家可点击场景中的任意 NPC；左侧目标栏与个人 Inspector 自动切换到被点选的角色。

本里程碑正式验收 `FS1-01` 至 `FS1-11`。真实模型验收比较感知一致、人物差异、连续性和动作合法性，不比较固定台词或固定 Tool。

## 协议边界

- 沿用已确认的 [Decision v1](../Reference/Protocol.md)，不新增 Endpoint、字段、Intent、Tool、状态码或兼容规则。
- 每个请求仍只对应一个 NPC；多 NPC 编排由 UE 在协议外完成，Python 不读取完整 World 或其他 NPC 的 Observation。
- 继续只允许 FaceTarget、MoveToward、MoveAway、Stop。求助、防卫、攻击、命中和伤害仍由 UE Gameplay 规则表达和执行。
- 全局最多 2 个 Decision 请求在途；每个 NPC 最多 1 个在途和 1 个最新 Pending，保持固定冷却和自动重规划上限。
- 如果实现证明必须改变协议，停止依赖该变更的部分并记录原因；本里程碑其他工作继续推进。

## 玩家可操作成果

- 玩家可以在 Guard、Merchant、Rival 和 Civilian 之间自由选择目标，也可以不指定目标，连续输入开放语言。
- 玩家可以使用 Whisper、Talk、Shout、InEar，执行 Face、Approach、MoveAway、Stop 和基础 Attack；非法或无法识别的行为明确反馈且不伪造世界变化。
- 玩家可以改变距离、朝向和冲突程度，并在不同 NPC 之间切换互动，不需要遵循固定剧情。
- Reset 和 Stub 演示提供确定、可重复的多 NPC 起点；真实 Kimi 使用相同场景入口。

## 屏幕可见成果

- 4 个 NPC 具有不同名称、人物身份、颜色/标识、初始关系与位置，并分别显示 Speech、Action、生命、立场和 Decision 来源。
- 同一说话事件对每个 NPC 分别显示看见、听见、听清和是否认为自己是目标；未感知 NPC 不产生引用该内容的反馈。
- 选中任意 NPC 时，个人视角 Inspector 显示其最近 Trigger、历史来源、调度状态、Provider、Intent、Tool 校验、公开 Reason Code 和延迟。
- Kimi 离线、超时或输出无效时，各 NPC 继续使用有界本地反馈；失败不会形成请求风暴或破坏其他 NPC 状态。

## 本阶段范围

### 多 NPC 身份、状态与个人上下文

- 固定交付 4 个 NPC：Guard、Merchant、Rival 和 Civilian；身份、人格、表达风格、目标、初始关系和公开经历使用受控配置。
- 每个 NPC 独立维护 Observation Buffer、Authority State Version、公开历史、生命、防卫、冲突状态、Decision 调试快照和 Tool 幂等状态。
- Decision Context Builder 从目标 NPC 配置和个人历史构造请求，不使用 Guard 常量或其他 NPC 数据。

### 多 NPC 调度与动作执行

- 每个 NPC 使用独立单在途/单 Pending 调度状态；GameMode 使用固定全局并发上限和稳定顺序分派请求。
- 新 Speech、已感知动作、距离跨阈值、受击、计划完成和计划失效只触发实际感知该变化的 NPC。
- Response 与 Tool 按 NPC、Request、Generation、State Version、TTL 和当前权威快照关联；一个 NPC 的回调不得改变另一个 NPC。
- 现有四个 Tool 复用于任意注册 NPC，仍只面向玩家或 Stop；移动、Transform、冷却和结果 Observation 由 UE 权威执行。

### 人物差异与旁观反应

- Stub 依据 NPC 身份、关系、当前状态和个人事实生成可重复但不同的公开反应，用于自动化验证人物差异和旁观边界。
- Kimi 固定约束要求以当前 NPC 身份说话，只使用个人 Context，不假装知道未感知事实或动作已执行。
- 未被话语指向但实际听见/看见事件的 NPC 可以产生有界旁观反馈；未听见或未看见时不得由广播式共享触发。
- 公开表达、实际移动、生命和冲突状态保持一致；内部数值和模型隐式推理不向玩家公开。

### 场景交付、调试与验收

- UI 支持目标切换、4 NPC 概览、个人 Inspector、重置和 Stub/Kimi 共用操作路径。
- 增加可重复的多 NPC Stub 场景烟测，覆盖有目标、无目标、旁观、人物差异、冲突升级—缓和、Tool 拒绝和离线降级。
- UE 自动化覆盖逐 NPC 隔离、调度公平与并发上限、回调关联、状态版本、攻击零副作用和 Inspector 数据。
- Python 覆盖多种人物 Context 的结构化处理、Stub 差异、Kimi 约束、错误映射和脱敏。
- Validation 保存 `FS1-01` 至 `FS1-11`、性能、真实 Kimi 和演示路径的可复查证据。

## 数据与权威边界

- UE 是 NPC 注册、位置、朝向、感知、关系输入、生命、命中、伤害、防卫、冲突状态、状态版本、调度、动作执行和结果 Event 的唯一事实来源。
- Python 每次只消费一个 NPC 的受控人物和个人事实，返回现有结构化表达与单个高层建议；不保存社会世界副本、不跨 NPC 共享 Context、不执行 Tool。
- NPC 公开 Speech 和真实 Action Result 只进入该 NPC 自己的历史；旁观者必须通过自己的感知重新获得事件。
- NPC 数、Observation、历史、调试记录、请求并发、Pending、自动重规划、Tool Call 和失败反馈均有硬上限。

## 明确不做

- 不修改 Dialogue/Decision 协议，不新增 Tool、Intent、多 Tool、批量 Decision 或服务端会话。
- 不实现 NPC 间自由对话、无限自主循环、群体计划、长期目标持久化或跨 NPC 隐式知识共享。
- 不建设完整战斗、武器、GAS、复杂导航、正式动画、犯罪、任务、经济或高质量美术系统。
- 不让 LLM 决定感知、目标是否合法、命中、伤害、状态版本、Transform、调度优先级或逐帧动作。
- 不实现 MassEntity 正式场景集成、语音、唇形、Embedding、向量数据库、多人同步或 Server Authority。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M10-A01` | Milestone 1 至 9 的适用回归保持通过；`Protocol.md` 和两端协议类型无变更。 |
| `M10-A02` | 场景稳定生成 4 个差异化 NPC，并为每个 NPC 独立维护人物、关系、状态、Observation、历史、调度和调试数据。 |
| `M10-A03` | 有目标、无目标、Whisper、Talk、Shout 和 InEar 均按逐 NPC 视觉/听觉/目标规则传播；未感知事实不进入请求或反馈。 |
| `M10-A04` | 多 NPC Decision 每个请求只含对应 NPC 个人 Context；全局并发不超过 2，每 NPC 不超过 1 个在途和 1 个 Pending，回调严格关联。 |
| `M10-A05` | Guard、Merchant、Rival 和 Civilian 对相同已感知输入在 Stub 中产生可复现差异，Kimi 验收体现身份、关系或历史造成的合理差异。 |
| `M10-A06` | 每个 NPC 的合法 Speech 与 Tool 独立处理；Tool 只有经 UE 当前目标、版本、TTL、距离、状态、冷却和幂等校验后才改变该 NPC 的世界状态。 |
| `M10-A07` | 玩家可在不同 NPC 间连续说话、接近、离开、停止和基础攻击；冲突可升级并因停止、退让或道歉缓和，且生命与动作仍由 UE 权威维护。 |
| `M10-A08` | 气泡、角色移动、生命/立场标识与个人 Inspector 能现场展示每个 NPC 的感知、公开表达、动作、Decision 来源和校验结果。 |
| `M10-A09` | Stub、离线、超时和无效响应路径均保持场景可操作、反馈有界、状态隔离且无请求风暴。 |
| `M10-A10` | `FS1-01` 至 `FS1-11` 均有自动化或人工证据；真实模型验收只比较行为属性并确认 `provider=kimi`。 |
| `M10-A11` | 受影响 UE Target 编译、Python/UE 全量回归、Stub 多 NPC 烟测、失败路径、性能采样和演示指南均完成并脱敏记录。 |
| `M10-A12` | 社会交互舞台所有玩家可见的固定提示和 Stub Speech 均为简体中文；Kimi 指令明确约束中文 Speech，且协议/稳定机器标识不变。 |
| `M10-A13` | 点击场景中的有效 NPC 后，左侧目标选择和个人 Inspector 同步切换到该 NPC；点击非 NPC 不改变当前选择。 |

## 完成定义

1. [TaskBoard.md](./TaskBoard.md) 中 Milestone 10 工作包全部完成。
2. `M10-A01` 至 `M10-A11` 和 `FS1-01` 至 `FS1-11` 均有可复查证据。
3. 4 个 NPC 可以从同一正常 UI 路径接受开放互动，个人感知与请求 Context 隔离成立。
4. Stub 可重复证明人物差异、旁观边界、动作落地和升级—缓和；真实 Kimi 完成多 NPC 行为属性验收。
5. 非法/过期 Tool 和攻击保持零 Gameplay 副作用；服务失败不破坏场景或形成请求风暴。
6. Architecture、Reference、Decision Log、Validation、演示指南、Task Board、Project State 与最终场景状态按职责同步并归档。
