# Milestone 8：单 NPC LLM 具身反馈与受控动作

## 状态

- 状态：`受阻（等待协议确认）`
- 开始日期：2026-08-31
- 前置里程碑：Milestone 1 至 7 已完成
- 场景来源：[最终场景 1：开放式社会交互沙盒](../Planning/FinalScenarios/Scenario1OpenSocialSandbox.md)

历史里程碑：

- [Milestone 1：UE 到 Python Service 最小闭环](../Milestones/Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](../Milestones/Milestone2.md)
- [Milestone 3：NPC 上下文与人格](../Milestones/Milestone3.md)
- [Milestone 4：持久化对话 Memory](../Milestones/Milestone4.md)
- [Milestone 5：确定性社会模拟基础](../Milestones/Milestone5.md)
- [Milestone 6：关系、长期记忆与重要 NPC](../Milestones/Milestone6.md)
- [Milestone 7：可操作交互舞台与定向感知](../Milestones/Milestone7.md)

长期路线见 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md)，目标系统设计见 [SocialSimulationDesign.md](../Planning/SocialSimulationDesign.md)，当前已实现边界见 [Architecture.md](../Planning/Architecture.md)。

## 目标

在 Milestone 7 的可操作社会沙盒中选定一个具名 Important/Core NPC，把它自己的 Observation、人物、关系和有界最近历史送入独立 AI Decision 链路。Python Planner 返回结构化表达和受约束动作建议，UE 完成状态版本、目标、参数、距离、能力和时效校验后，才把合法结果表现为气泡与真实移动：

```text
玩家自由说话或描述基础行为并继续移动
  -> UE 更新目标 NPC 的个人 Observation 与权威状态版本
  -> 独立 Decision 请求只携带裁剪后的个人视角和允许动作
  -> Stub 或真实 Planner 返回结构化 Speech / Intent / Tool 建议
  -> UE Tool Registry / Executor 重新校验当前世界状态
  -> 合法 Speech 显示气泡，合法 Tool 执行真实移动
  -> 拒绝、过期、失败或执行结果进入开发视图和受控 Event
  -> Python 不可用时使用可见的确定性降级
```

本里程碑只证明一个 NPC 的端到端闭环，不把 LLM 扩展到全部占位 NPC，也不建设连续冲突系统。

## 协议前置条件

- 当前 [Protocol.md](../Reference/Protocol.md) 只定义 `POST /v1/dialogue` 的纯文本请求与响应，不允许 UE 从回复文本推断 Gameplay 指令。
- Milestone 8 需要独立的结构化 Decision 契约，以便传输个人视角、权威状态版本、允许动作、结构化表达、Intent 和可选 Tool 建议，并定义过期与错误语义。
- 具体 Endpoint、字段、类型、边界、错误码和兼容规则必须先向用户说明并获得明确确认，再同步到 `Protocol.md` 和两端实现。
- 在协议确认前，只允许完成范围、任务、验收和协议方案整理；不得修改 `Protocol.md`，不得实现与当前协议不一致的网络格式，也不得从现有 Dialogue 文本驱动 NPC 动作。

## 玩家可操作成果

- 玩家可以在 Milestone 7 场景中选定一个具名 NPC，自由输入说话内容或描述 Face、Approach、MoveAway、Stop 范围内的基础行为。
- 玩家在请求等待期间仍可移动和转向，使距离、目标或状态发生变化，并观察旧建议被接受或拒绝。
- Stub 与真实模型模式都能完成至少一次“自由输入 -> NPC 自然语言气泡 -> 合法动作实际执行”的闭环。
- Python Service 离线、超时或返回无效结构时，场景仍可继续操作，NPC 显示明确的确定性降级反馈。

## 屏幕可见成果

- 目标 NPC 的自然语言通过对话气泡显示，来源可区分为真实 Planner、Stub Planner 或本地降级。
- Face、Approach、MoveAway 或 Stop 建议只有实际开始执行后才显示动作气泡或状态；被拒绝的动作不伪装成已执行。
- 开发视图显示请求 ID、目标 NPC、状态版本、Decision 来源、公开 Intent、Tool 名称、校验结果、拒绝 Reason Code 和耗时。
- 开发视图不显示完整 Prompt、模型隐式推理、API Key、原始 Provider 异常、未裁剪历史或完整持久化范围。

## 本阶段范围

### 独立 Decision 契约与客户端

- 在协议确认后新增独立 Decision Endpoint；现有 `/v1/dialogue` 的字段、语义和纯文本能力保持兼容。
- UE 构造有界 Decision 请求、异步调用、解析结构化成功/错误响应，并确保每个请求只完成一次。
- 请求关联稳定 NPC ID、请求时状态版本和有效期；UE 收到回复时使用当前权威状态重新判断是否过期。
- Decision 请求不自动发送完整 World、全部 NPC Observation、完整社会图或未裁剪 Memory。

### 单 NPC 个人视角上下文

- 只为一个稳定 ID 的 Important/Core NPC 启用 LLM Decision；其余 NPC 保持 Milestone 7 的确定性占位反馈。
- 输入由该 NPC 自己的最新 Observation、人物设定、与玩家的关系摘要、即时状态和有界最近历史组成。
- 未看见、未听见或已过期的事件不得作为该 NPC 的已知事实；玩家行为输入原文不得冒充已观察到的 Gameplay 结果。
- 上下文、候选 Intent、允许 Tool、历史条数和所有字符串均有硬上限。

### Python Planner

- 在现有 Dialogue Service 旁建立独立 Decision Route、Schema、Service、Context Builder 和 Planner 抽象，不让 Route 直接依赖具体 Provider SDK。
- Stub Planner 提供确定、可重复的 Speech/Intent/Tool 组合，用于协议、失败和场景自动化。
- Kimi Planner 使用与 Provider 隔离的内部生成上下文，输出经过 Schema 校验的结构化结果；不返回或保存 Chain-of-Thought。
- Planner 不读取 UE World，不执行 Tool，不假定建议已经成功，也不修改 UE 社会状态。

### UE Tool Registry 与校验

- 通用注册、Schema 与无 Actor 的校验边界位于 `ZLASocialRuntime`；具体 Pawn/AIController Gameplay Handler 由 `ZL` 注册。
- 本里程碑只开放 FaceTarget、MoveToward、MoveAway 和 Stop；协议名称与参数在协议确认后定稿。
- 每个 Tool 建议至少校验注册、Capability、参数、目标、状态版本、有效期、距离、可执行状态、冷却、速率和幂等。
- 未知、越权、非法参数、目标失效、状态过期、距离失效、冷却中或重复调用均不得产生 Gameplay 副作用。

### 执行、结果与降级

- Gameplay Handler 只在 UE 校验通过后执行朝向或移动，并产生 Started、Completed、Rejected、Cancelled 或 Failed 中适用的受控结果。
- Tool 被拒绝时，仍可显示已经独立通过校验的合法 Speech；表达与动作执行状态不得混为一谈。
- 请求失败、超时、无效结构或过期时使用确定性安全反馈，不阻塞玩家输入、移动、重置或其他 NPC 的本地规则。
- Tool 执行结果形成新的 UE Event/Observation 输入，但本阶段不因结果自动建立无限 LLM 重规划循环。

### 自动化与人工验收

- Python 覆盖 Decision Schema、Stub、Kimi 结构化映射、错误分类、超时和日志脱敏。
- UE 覆盖请求边界、状态版本、过期、注册/Capability/参数/目标/距离/冷却/幂等校验和结果状态。
- 端到端自动化使用 Stub 完成一次可见 Speech 与真实动作，并覆盖至少一个拒绝、一个过期和一个 Python 不可用降级。
- 真实模型人工验收至少完成一次合法结构化 Decision，不把固定台词或固定动作选择写成通过条件。

## 数据与权威边界

- UE 继续是位置、朝向、状态版本、目标、能力、冷却、导航可达性、动作执行和结果 Event 的唯一事实来源。
- Python 只返回结构化高层建议；任何 Tool 建议在 UE 校验和执行前都不是世界事实。
- 现有 Dialogue Memory 与 UE Social Memory 保持分域；本里程碑不建立自动事实抽取、向量检索或 Python Gameplay 状态副本。
- `ZLASocialRuntime` 不依赖 `ZL`、Widget、Actor、HTTP 或 Provider；`ZLAIRuntime` 不反向依赖具体 Gameplay Handler。
- 所有队列、并发、请求大小、上下文、响应、重试、有效期、Tool 次数和调试记录都有硬上限。

## 明确不做

- 不让多个 NPC 同时使用 LLM Decision，不实现 Milestone 10 的 3 至 5 个差异化 LLM NPC。
- 不实现 Milestone 9 的连续重新规划、冲突升级、攻击、伤害、防卫、逃跑、求助或道歉后缓和。
- 不允许模型创建 Tool、调用任意 Blueprint/C++ 函数、直接设置 Transform、修改关系/状态或绕过 UE Authority。
- 不从 `/v1/dialogue` 的自然语言回复解析 Gameplay 指令，不改变既有 Dialogue/Memory 核心语义。
- 不实现流式 Token、并行 Tool、多步 Agent 循环、后台自主规划、语音、唇形同步或正式动画资产。
- 不新增 Embedding、向量数据库、知识图谱、自动事实抽取或新的 Memory 持久化语义。
- 不正式集成 MassEntity、复杂导航、三维遮挡、多人同步或 Server Authority。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M8-A01` | Milestone 1 至 7 的适用 Dialogue、Context、Memory、社会模拟和社会沙盒回归保持通过；既有 `/v1/dialogue` 字段与纯文本语义不改变。 |
| `M8-A02` | Decision 协议在实现前获得明确确认；`Protocol.md`、Python Schema、UE 类型和契约测试对 Endpoint、字段、边界、错误与兼容规则保持一致。 |
| `M8-A03` | 只有一个具名 Important/Core NPC 启用 Decision；请求只使用其个人 Observation、人物、关系、即时状态和有界历史，未感知事实与行为输入原文不会成为其已知 Gameplay 事实。 |
| `M8-A04` | Stub Planner 能通过正常 UI/场景路径产生合法结构化 Speech 和 Tool 建议；NPC 显示自然语言气泡并实际执行 Face、Approach、MoveAway 或 Stop 中至少一个动作。 |
| `M8-A05` | 真实 Kimi Planner 至少完成一次自由输入到合法结构化 Decision 的人工闭环；验收不依赖固定模型台词或固定动作选择。 |
| `M8-A06` | UE 对 Tool 执行注册、Capability、参数、目标、状态版本、有效期、距离、可执行状态、冷却、速率和幂等校验；未知或非法建议零 Gameplay 副作用，公开拒绝原因可见。 |
| `M8-A07` | 玩家在请求期间改变距离、目标或状态后，过期或上下文失效的 Tool 不执行；合法 Speech 与 Tool 拒绝可以独立呈现。 |
| `M8-A08` | Python 离线、Provider 超时、服务错误、无效结构和本地请求失败均触发有界确定性降级；玩家仍能移动、继续输入、重置并观察明确来源。 |
| `M8-A09` | Decision 队列、并发、上下文、历史、响应、有效期、Tool 调用和调试记录均有硬上限；日志与 Inspector 不泄露 Prompt、隐式推理、凭据、完整输入、scope 或原始 Provider 异常。 |
| `M8-A10` | 受影响 UE Target 编译、Python/UE 单元与回归、Stub 端到端、场景级自动化和人工操作路径通过；Validation 保存可复查证据。 |

## 完成定义

1. [TaskBoard.md](./TaskBoard.md) 中 Milestone 8 工作包全部完成。
2. `M8-A01` 至 `M8-A10` 均有可复查验证证据。
3. Stub 与真实模型都完成一次“个人视角输入 -> 结构化 Decision -> UE 校验 -> 气泡与真实动作”的可操作闭环。
4. 非法或过期动作零 Gameplay 副作用且拒绝原因可见；Python 不可用时场景保持可操作。
5. 协议、Architecture、UE/Python Reference、Decision Log、Validation 和当前状态文档已按职责同步，Milestone 8 范围定稿归档。
