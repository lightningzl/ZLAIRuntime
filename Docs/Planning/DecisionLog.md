# Decision Log

## 2026-09-03：Decision v2 采用开放高层计划与请求级有限能力实例

决定：
Milestone 12 新增独立 `POST /v2/decision`，不修改既有 `/v1/decision`。v2 由 UE 为单个 NPC 提供个人 `social_situation` 与本次有限的 `available_capabilities`；LLM 可开放地判断目标、公开理由、Speech、连续表现参数和最多四个短期步骤，但步骤只能逐字回显 UE 本次提供的能力实例。UE 逐步校验、执行并把实际结果回流为新的个人事实。

原因：
- 单 Intent/单 Tool 不能表达“害怕但嘴硬、边后退边寻求守卫、同时拒绝交易”等连续、可组合的角色策略。
- 直接让模型任意创建动作、动画或世界事实会破坏 UE 的世界权威，也会让“我要报告”被错误地显示为“报告已送达”。
- 请求级能力实例允许随场景、距离、角色状态和已注册 Handler 变化开放能力，而不让协议变成全局任意函数调用。

取舍：
- v2 增加 Context、计划生命周期、步骤预算和两端 Schema/测试工作；实现期间须同时保持 v1 回归。
- LLM 的目标、理由和表现建议本身不产生 Gameplay；UE 没有注册或未通过校验的步骤只能失败/拒绝，不能伪造结果。
- 表情与手势仅映射至已注册资源/参数，不能由模型加载、生成或修改二进制资产。

状态：
已接受；协议已文档化，尚未实现。

重要性：
重要

## 2026-09-01：公开冲突立场保持为 UE 有界状态机

Milestone 9 以 `Calm`、`Alert`、`Escalated`、`Recovering` 表示单 Guard 的公开立场。攻击、距离、停止、已接受的高层 Intent 和服务失败是唯一输入；模型不直接改变生命、命中、防卫或状态机。服务失败的确定性安全结果是停止当前计划、进入防卫并显示 `LocalFallback`，使离线时仍具备可见且有界的 Gameplay 行为。

新增或调整决策前，必须遵守 [DocumentationRules.md](../Process/DocumentationRules.md) 中的“Decision Log 规则”。

## 2026-07-16：使用 PythonService 作为 AI Runtime

决定：
UE 不直接调用 LLM。AI 推理、未来的 Prompt 组装和模型供应商适配统一放在 PythonService 中，UE 只通过服务协议请求结果。

原因：
- 隔离 UE Gameplay Runtime 与变化频繁的 AI SDK、模型和推理逻辑。
- 允许 Python 和 UE 独立测试、替换实现和演进。

取舍：
- 增加一个独立进程、网络边界以及相应的启动和错误处理成本。
- 放弃 UE 直接调用模型所带来的较少组件数量。

状态：
已接受

重要性：
重要

## 2026-09-03：社会沙盒 NPC 受击沿用 Combat Damageable 与死亡 ragdoll 边界

决定：
`AZLSocialSandboxNpc` 实现 `ICombatDamageable`。普通有效命中按 `ACombatEnemy::ApplyDamage` 施加 CharacterMovement 击退；只有生命归零、进入失能时由 `HandleDeath` 禁用 Capsule/移动并启用 Mesh ragdoll。NPC 不再以受击蒙太奇表现命中。

原因：
- 命中权威仍在社会沙盒 GameMode/NPC，复用 Damageable 接口使伤害后的物理表现能与 Combat 资产和通知链保持一致。
- 将完整 ragdoll 限于失能，避免非致命命中永久脱离 CharacterMovement 和既有社会行动流程。
- 角色蓝图只需配置有效骨骼网格和 Physics Asset，不必为受击额外制作动画资源。

取舍：
- Reset 会显式恢复 Capsule 碰撞、行走模式与非物理 Mesh；Physics Asset 缺失时 ragdoll 视觉效果由用户资源配置决定。
- 此项不改变攻击伤害、协议、Tool、导航或 StateTree 语义。

状态：
已接受

重要性：
重要

## 2026-09-03：社会沙盒 NPC 采用 ACharacter 与标准 AIController 基础

决定：
`AZLSocialSandboxNpc` 从 `AActor` 升级为 `ACharacter`，复用 UE 内置 Capsule、Mesh 和 CharacterMovement，并在放置或生成时自动创建标准 `AAIController`。现有社会沙盒继续由 GameMode 与 NPC 的受控规则计算移动；本决策不引入导航、Behavior Tree、StateTree 或新的 Tool/协议语义。

原因：
- 骨骼角色本身更适合建立在 Character 的内置 Mesh 与 Movement 基础上，避免自建 Actor 组件与标准角色生命周期分离。
- `AAIController` 只能 Possess `APawn`，以 `ACharacter` 为基类可保留后续 AIController、NavMesh 和行为资产的接入空间。
- 当前移动与安全校验已经由 UE 权威规则实现，保持该执行语义可将基类升级与未来导航行为解耦。

取舍：
- NPC 蓝图的网格配置位置从自定义 `CharacterMesh` 变为内置 `Mesh`；已有蓝图资源由用户自行迁移配置。
- 自动具备 AIController 不代表当前 Tool 改为寻路；若将来接入 `MoveTo`、导航或行为树，必须在新范围中定义并验证其世界语义。

状态：
已接受

重要性：
重要

## 2026-07-16：第一里程碑使用 HTTP 和 JSON

决定：
UE 与 PythonService 在第一里程碑使用版本化 HTTP/JSON 通信，不使用 WebSocket；当前入口为 `POST /v1/dialogue`。

原因：
- 请求/响应模型满足当前非流式最小闭环，开发和调试成本较低。
- JSON 协议清晰，便于两端独立验证和面试演示。

取舍：
- 当前方案不支持双向推送和流式 Token。
- 如果后续里程碑证明需要持续连接或流式输出，需要重新评估 WebSocket 等方案。

状态：
已接受

重要性：
重要

## 2026-07-16：先使用确定性 Stub 验证闭环

决定：
最小通信闭环阶段使用确定性 Stub 生成回复，`provider` 固定为 `stub`，暂不接入真实 LLM、Memory 或 Tool Use。

原因：
- 先独立验证 UE HTTP Client、协议、超时和错误处理。
- 避免模型网络、API Key 和输出不确定性干扰当前验收。

取舍：
- 当前阶段不能展示真实生成式对话能力。
- 后续接入 LLM 时仍需补充供应商适配、配置和模型错误处理。

状态：
已接受

重要性：
重要

## 2026-07-16：Python 按 Route、Schema 和 Service 分层

决定：
FastAPI Route 负责 HTTP 适配，Pydantic Schema 定义协议字段，Service 层提供与 FastAPI 解耦的 Stub 回复逻辑。

原因：
- 保持协议校验、传输逻辑和回复逻辑的模块边界清晰。
- 后续替换 Stub Provider 时无需改变 UE 协议或 Route 职责。

取舍：
- 当前代码量很小时会增加少量文件和接口。
- 需要避免为未来需求继续拆分不必要的层级。

状态：
已接受

重要性：
重要

## 2026-07-16：使用 Monorepo 管理 UE、PythonService 和 Docs

决定：
UE 工程、PythonService 和项目文档保存在同一个 Git 仓库中，并通过顶层目录隔离。

原因：
- 协议和两端实现可以在同一个变更中同步审查。
- Demo 的克隆、搭建和面试展示路径更集中。

取舍：
- 仓库需要严格忽略 UE 生成目录、Python 环境和大型非源码资源。
- 如果未来服务需要独立发布和权限管理，可能需要拆分仓库。

状态：
已接受

重要性：
重要

## 2026-07-17：分离任务执行状态与项目状态快照

决定：
使用 `TaskBoard.md` 维护当前里程碑的工作包、依赖和任务状态，使用 `ProjectState.md` 汇总当前活动任务、下一步、阻塞和验收进度。两者不重新定义 `CurrentMilestone.md` 的范围或 `Protocol.md` 的通信契约。

原因：
- 让开发者和 Agent 能快速恢复当前上下文，同时保持任务状态只有一个事实来源。
- 避免把高频变化的执行信息混入里程碑范围、架构或协议文档。

取舍：
- 任务状态变化时需要同步更新 Task Board 和 Project State。
- 如果维护流程未被遵守，两份状态文档可能短暂不一致，因此必须以实现和验证证据校正状态。

状态：
已接受

重要性：
重要

## 2026-07-17：UE AI Runtime 使用独立项目插件

决定：
UE 侧 AI Service Client、通信协议类型及后续同类 AI Runtime 能力统一放在项目级 `ZLAIRuntime` Runtime Plugin 中。游戏主模块 `ZL` 仅依赖插件公开接口，不直接拥有 HTTP/JSON 实现。

原因：
- 将可复用的 AI Runtime 能力与具体 Gameplay、UI 和模板示例代码隔离。
- 保持插件不反向依赖游戏模块，便于独立编译、测试以及未来迁移到其他 UE 项目。
- 让后续配置、失败处理和演示入口沿稳定模块边界演进。

取舍：
- 增加插件描述文件、独立模块和公开接口的维护成本。
- 游戏模块需要显式依赖插件模块后才能使用其类型，不能直接访问插件私有实现。

状态：
已接受

重要性：
重要

## 2026-07-22：通过 Provider 边界接入 OpenAI Responses API

决定：
首个真实 LLM Provider 使用官方 OpenAI Python SDK 和 Responses API。具体供应商调用封装在 Python Service 的 Dialogue Provider 实现中，Dialogue Service 只依赖 Provider 接口；UE、Route 和协议 Schema 不依赖 OpenAI SDK 类型。模型通过环境配置，初始默认使用适合成本敏感对话场景的 `gpt-5.6-luna`，后续更换模型不改变 UE 协议。

原因：
- 延续“UE 不直接调用 LLM”和 Route、Schema、Service 分层的既有边界。
- Provider 接口允许通过 Fake/Stub 离线测试，避免自动化测试产生费用或依赖外部网络。
- Responses API 是 OpenAI 当前推荐用于后续多轮与 Tool Use 演进的统一接口，能够让后续里程碑沿同一供应商边界扩展。
- 模型配置与协议解耦，避免模型版本变化迫使 UE 更新。

取舍：
- 真实 LLM 接入仍会引入一个具体供应商 SDK 和外部网络依赖。
- Provider 抽象增加少量模块与依赖注入成本，但本阶段只实现 OpenAI 和显式 Stub，不建设多供应商框架。
- 默认模型可能随成本、延迟和质量评估调整，因此真实验收必须记录实际模型配置，协议只暴露 `openai`。

状态：
已取代；当前 Provider 决策见“2026-07-23：首个真实 Provider 切换为 Kimi K3”。

重要性：
重要

参考：
- [OpenAI Models](https://developers.openai.com/api/docs/models)
- [OpenAI Model Guidance](https://developers.openai.com/api/docs/guides/latest-model)

## 2026-07-22：真实 LLM 接入保持非流式 HTTP v1 协议

决定：
真实 LLM 接入继续使用 `POST /v1/dialogue` 的单次 HTTP 请求/响应，不增加流式传输或新端点。请求字段保持不变；在 v1 内兼容扩展 `provider` 标识和结构化 Provider 错误码。`provider` 表示逻辑生成来源而不是模型版本，UE 将 Provider 和错误码保留为字符串。

原因：
- 当前目标是隔离并验证真实模型调用，现有请求/响应链路已经足以演示单轮自由对话。
- UE 现有实现使用字符串解析 `provider` 和错误码，可以兼容新增值而无需扩大公开类型。
- 保持非流式链路能把供应商、配置、超时和错误映射风险与流式 UI、取消和增量协议风险分开验证。

取舍：
- 玩家必须等待完整回复，无法逐 Token 展示。
- Provider 错误仍通过通用 HTTP 失败路径交给 UE，UE 不提供供应商专用错误枚举。
- 后续加入人格、会话、Memory 或 Tool Call 时仍需重新评估 v1 兼容性；本决策不预先授权这些字段。

状态：
已接受

重要性：
重要

## 2026-07-23：首个真实 Provider 切换为 Kimi K3

决定：
Milestone 2 的首个真实 LLM Provider 从 OpenAI Responses API 切换为 Kimi 国内开放平台的 `kimi-k3`。Python Service 使用 Kimi 官方文档支持的 OpenAI 兼容 Python SDK，通过 `https://api.moonshot.cn/v1` 的 Chat Completions API 发起请求；密钥仅从 `MOONSHOT_API_KEY` 读取。协议成功响应使用逻辑标识 `kimi`，但 UE、Route 和 Dialogue Service 仍不依赖具体 SDK 类型。

原因：
- Kimi 平台提供更适合当前开发环境的账户充值与支付路径，降低真实端到端验收的外部阻塞。
- Kimi 官方提供 OpenAI 兼容接口，可保留现有 Provider 边界、依赖注入、错误分类和大部分离线测试结构。
- `kimi-k3` 是明确的模型配置，仍与 UE 协议解耦，后续模型版本调整不要求修改 UE 请求字段。

取舍：
- 继续保留名为 `openai` 的 Python 依赖，因为它是 Kimi 官方支持的兼容客户端；实际供应商由固定 Base URL 和 Provider 实现决定。
- Kimi K3 始终启用思考，本阶段使用低 `reasoning_effort` 并限制最大输出 Token，仍可能比非思考模型具有更高延迟。
- 本次只替换首个真实 Provider，不建设多供应商热切换、自动故障转移或供应商专用 UE 类型。

状态：
已取代；当前默认模型决策见“2026-07-23：Milestone 2 验收默认模型降级为 Kimi K2.6”。

重要性：
重要

参考：
- [Kimi API Overview](https://platform.kimi.com/docs/overview)
- [Kimi Chat Completions API](https://platform.kimi.com/docs/api/chat)

## 2026-07-23：Milestone 2 验收默认模型降级为 Kimi K2.6

决定：
真实端到端验收期间，`kimi-k3` 出现持续限流。账户模型列表明确开放 `kimi-k2.6`，且相同密钥、Base URL、Provider 适配器和最小输入下真实生成成功，因此将 `ZL_KIMI_MODEL` 的默认值由 `kimi-k3` 调整为 `kimi-k2.6`。环境变量仍可显式覆盖模型，UE 协议和成功响应中的逻辑 Provider 标识 `kimi` 均不变。

原因：
- 已充值账户与密钥可正常调用 `kimi-k2.6`，说明鉴权、余额和国内 API 地址有效。
- 降级模型能将 K3 热点限流与本地实现问题分开，保证 Milestone 2 可以形成可复现的真实端到端验收证据。
- 模型名始终只属于 Python Provider 配置，调整默认值不应扩散到 UE 请求或 v1 协议字段。
- K2.6 在默认思考模式下可能让短输出上限被推理内容占用而返回空 `content`；NPC 单句回复不需要深度推理，因此 K2.x 请求显式关闭思考。

取舍：
- 本阶段不实现自动模型故障转移或重试；每个玩家请求仍只发起一次上游生成请求。
- K3 恢复稳定后可通过 `ZL_KIMI_MODEL=kimi-k3` 手动验证，无需修改代码或协议。

状态：
已接受

重要性：
重要

参考：
- [Kimi 模型列表](https://platform.kimi.com/docs/models)
- [Kimi Chat Completions API](https://platform.kimi.com/docs/api/chat)

## 2026-07-27：v1 使用客户端提供的受限瞬时上下文

决定：
Milestone 3 继续使用 `POST /v1/dialogue`，在现有 v1 请求中增加可选 `context`。该对象完整包含 NPC 人格、当前世界状态和最多 8 条按序对话历史，并对每个字符串和数组设置明确上限。旧请求可以省略 `context`；响应结构、Provider 标识和错误包络保持不变。

上下文由 UE Gameplay/UI 根据自身已知状态显式构造，每次请求携带完整快照。Python Service 不保存会话、不读取 UE World，也不引入 `session_id`、数据库或供应商托管会话。

原因：
- UE 是 Gameplay 状态的事实来源，显式快照可以避免 Python 反向依赖 UE 内部类型或主动读取世界。
- 可选字段符合 v1 的兼容扩展规则，Milestone 1/2 客户端和回归请求可以继续工作。
- 严格的字段、角色和长度边界让两端能够确定性校验输入，并控制 Prompt 大小和日志风险。
- 将持久化 Memory 与瞬时上下文分开，可以先验证人格、世界状态和有限历史的实际价值，再决定后续存储方案。

取舍：
- 每次请求会重复传输上下文，Gameplay/UI 需要负责构造一致的快照。
- Service 重启或客户端未携带历史时不会保留对话连续性。
- v1 的固定结构不支持任意嵌套 Gameplay 数据；新增语义需要再次评估协议兼容性。

状态：
已接受

重要性：
重要

## 2026-07-27：由独立 Context Builder 组装模型输入

决定：
Python Service 新增独立 Context Builder。它以确定性顺序组合固定系统约束、NPC 人格、世界状态、有限历史和当前玩家输入，输出供应商无关的内部生成上下文。Dialogue Service 负责调用 Builder 和 Provider；Kimi Provider 只负责把内部上下文映射为 SDK 请求。

固定系统约束只由 Python 维护。请求中的人格、世界事实、历史和玩家输入全部作为数据处理，不能启用 Tool、Memory、供应商会话或覆盖系统边界。

原因：
- 避免 Route、Service 和 Provider 分别拼接 Prompt，保持单一职责和可测试的依赖方向。
- 供应商无关的内部类型让 Stub/Fake 能离线验证上下文顺序，也避免 Kimi SDK 类型扩散到业务层。
- 集中的系统/数据边界便于测试 Prompt 注入、当前输入重复和日志泄露风险。

取舍：
- 增加一个内部模块和类型转换步骤。
- Builder 的输出结构需要与 Provider 接口共同演进，但不得反向改变公开 HTTP 协议。

状态：
已接受

重要性：
重要

## 2026-07-27：使用显式范围和 SQLite 持久化对话 Memory

决定：
Milestone 4 继续使用 `POST /v1/dialogue`，在 v1 请求中增加可选 `memory` 对象，其中只包含稳定、不透明的 `scope_id`。请求提供 Memory 时，Python Service 以 `(scope_id, npc_id)` 隔离读取和保存对话；请求省略 Memory 时严格保持既有无状态行为。

Python Service 新增独立 Memory Service 与 SQLite Repository。Repository 独占 Schema、索引、查询和事务；Memory Service 负责隔离、预算、顺序、重叠消除和幂等轮次语义；Dialogue Service 协调读取、Context Builder、单次 Provider 调用和成功后写入。只保存成功完成的玩家输入与 NPC 回复，不保存完整上下文、Provider 原始响应或推理内容。

原因：
- 可选 Memory 保持 Milestone 1 至 3 客户端和无状态请求兼容，不会让所有请求隐式产生持久化副作用。
- `(scope_id, npc_id)` 同时表达玩家/存档范围和 NPC 隔离，避免只按 NPC ID 导致不同玩家串线。
- SQLite 足以支撑单机 Demo 的结构化历史、事务、唯一约束和重启恢复，且无需提前引入外部数据库服务。
- 独立 Memory Service 和 Repository 保持 SQL、Prompt 编排、Provider 和 UE Runtime 边界清晰，为后续评估摘要或向量检索留下替换点。

取舍：
- Gameplay/UI 必须提供稳定 scope，Service 不负责账号、存档或身份生命周期。
- 本地 SQLite 保存对话明文，当前只适合受控 Demo 环境；生产级加密、备份、权限和合规删除需要后续设计。
- 最近轮次检索不能提供语义相关性；本阶段明确不引入 Embedding、向量库或 LLM 摘要。
- v1 响应不暴露 Memory 命中数量或数据库状态，运行验证依赖脱敏日志、测试和本地维护入口。

状态：
已接受

重要性：
重要

## 2026-08-03：将项目演进为分层社会模拟 Runtime

决定：
项目长期目标从以对话链路为主的 AI NPC Demo 演进为 `ZLAI Social Simulation Runtime`。玩家行为先进入 UE 权威的 Event、Perception、State、Memory、Relationship 和 Rule Decision 链路；LLM 只服务少量 Important/Core NPC 的高层 Decision 和 Dialogue。Level 1 NPC 不调用 LLM，所有层级必须提供确定性降级。

原因：
- 高级 UE Gameplay/AI 岗位更需要证明运行时系统、性能、状态和行为执行能力，而不只是模型接入。
- 三级 NPC 可以把成本和不确定性集中在少量叙事角色，同时保留 100+ NPC 的群体表现。
- UE 权威和统一执行边界能够安全处理过期、非法或不可用的 AI 输出。

取舍：
- 增加社会模拟、调试和性能验证工作，完整 Demo 需要多个里程碑。
- MVP 只做一个小场景、一个 Core NPC 和有限行为，不建设开放世界社会系统。

状态：
已接受

重要性：
重要

## 2026-08-03：先实现纯 UE 确定性社会模拟

决定：
Milestone 5 只实现纯 UE 的 Event、空间查询、感知、Instant State、Short Memory 和 Rule/Utility Decision，不修改现有 Dialogue 协议，也不提前实现 ToolCall、Level 2/3 或 NPC 间二级传播。结构化 AI Decision 留到后续独立里程碑，并再次获得协议确认。

原因：
- 先验证 100+ NPC、事件传播和人格差异，可以将 Gameplay/性能风险与 LLM/协议风险分开。
- Python 不可用时仍保持完整的基础社会反应和可测试降级路径。
- 避免一次里程碑同时引入新 UE 模块、群体模拟、协议和模型结构化输出。

取舍：
- Milestone 5 尚不能展示模型驱动 Gameplay。
- Relationship、Long Memory 和二级社会传播需要后续里程碑继续完成。

状态：
已接受

重要性：
重要

## 2026-08-03：按文档生命周期重组 Docs

决定：
Docs 使用 `Planning`、`Current`、`Reference`、`Process`、`Interview`、`Milestones` 和 `Validation` 分离长期规划、实时执行、稳定参考、流程、展示、历史范围与证据。根 `Docs/README.md` 只提供导航。总规划和总设计位于同一目录，Current Milestone、Task Board 和 Project State 位于同一目录。

原因：
- 防止 Task Board、Project State 和 Validation 重复保存同一验收事实。
- 防止目标设计被误读为当前已经实现或已经授权的范围。
- 让 Agent 能从固定入口恢复长期方向、当前任务和强制流程。

取舍：
- 现有文档路径发生一次性变化，需要同步全部链接和 Agent 入口。
- 目录层级增加一层，因此必须保留根导航并避免继续过度拆分。

状态：
已接受

重要性：
重要

## 2026-08-28：将报告意图与社会传播成功分离

决定：
Milestone 6 中的 `Intent.Report` 只表示 NPC 希望报告，不直接修改接收者 Memory、Relationship、Reputation 或 Faction Standing。只有 UE Gameplay 通过显式确认入口证明报告已经发生后，才能创建带 Root/Parent、Depth、Budget、来源与 Confidence 的 Social 派生 Event。Important NPC 继续使用确定性规则消费报告；本阶段不修改 Dialogue 协议，也不新增 LLM Decision 或 ToolCall。

原因：
- 决策意图不是 Gameplay 执行结果；移动失败、目标离开或报告被中断时不应产生“信息已经送达”的虚假事实。
- 显式确认保持 UE 权威，并为未来 StateTree、AIController 或 Tool Executor 提供稳定的完成边界。
- Root Event、传播预算和来源链只有在真实传播发生时更新，能够可靠执行去重、Confidence 衰减和 Authority 校验。

取舍：
- Milestone 6 的无界面纵向切片只能模拟显式报告完成，不能展示找守卫、移动、动画或对话表现。
- Gameplay 层必须保存待报告上下文，并在实际执行成功后调用确认入口；只生成 `Intent.Report` 不再足以完成传播。

状态：
已接受

重要性：
重要

## 2026-08-31：由最终场景倒推底座并要求里程碑成果可操作、可显化

决定：
后续社会模拟路线以 [最终场景 1：开放式社会交互沙盒](./FinalScenarios/Scenario1OpenSocialSandbox.md) 为首个体验来源。玩家开放语言、行为描述、定向视觉、分级听觉、个人视角、LLM 人物反馈和 UE 受控执行构成目标闭环；威胁、基础战斗、逃跑、求助和缓和属于同一开放互动连续谱，不另建重复的战斗沙盒。

Milestone 7 至 10 按玩家可操作、屏幕可见的纵向切片推进：交互舞台与感知、单 NPC LLM 具身反馈、连续互动与冲突变化、3 至 5 NPC 最终场景交付。未来里程碑不能只用后台类型、接口、日志或无界面测试作为完成成果。100+ NPC 继续作为独立确定性性能证明，不作为最终场景 1 的人数要求。

原因：
- 现有 Milestone 5/6 证明了确定性底座和性能，但没有直接表现 LLM 在开放语境、人物一致性和连续反馈上的优势。
- 从技术模块出发容易产生不可玩的能力清单；从玩家能做什么、能看到什么倒推，才能约束底座只服务真实体验。
- 冲突和战斗是社会互动可能升级到的状态，独立复制一套沙盒会重复感知、人物、记忆和执行循环。
- 每步都有可操作入口和可见结果，可以更早发现设计问题，也便于现场演示和阶段验收。

取舍：
- 未来路线由两个大里程碑拆为四个玩家可见纵向切片，文档、场景 UI 和表现层工作增加。
- 第一个最终场景优先 3 至 5 个 NPC 的反馈质量，100+ NPC 与复杂群体传播不在同一场景中同时展示。
- 结构化 Decision 与 ToolCall 仍需单独确认协议；本决策只重排目标和顺序，不修改当前线上协议。

状态：
已接受

重要性：
重要

## 2026-08-31：使用独立 Decision 契约和 UE 最终 Tool 校验

决定：
Milestone 8 在现有 `POST /v1/dialogue` 旁新增独立 `POST /v1/decision`。请求只携带一个明确选定 NPC 的状态版本、TTL、单个个人 Trigger、人物/关系/即时状态、有界个人历史，以及 UE 本次允许建议的固定 Tool 与目标。响应返回结构化 Intent、可选 Speech、最多一个 ToolCall、Confidence 和逻辑 Provider。

第一阶段 Tool 固定为 `face_target`、`move_toward`、`move_away` 和 `stop`，不允许任意参数对象、多 Tool 数组、动态函数或脚本。Python 只能提出建议；UE 收到响应后仍重新校验请求关联、状态版本、本地 TTL、注册、Capability、目标、距离、冷却、速率和幂等。Speech 可以独立于被拒绝的 Tool 显示。

原因：
- Dialogue 文本适合开放表达，但从自然语言解析 Gameplay 指令会混淆表达与权威执行，无法稳定拒绝未知或过期动作。
- 独立端点保持现有 Dialogue/Memory 客户端兼容，也让 Decision 的个人视角、状态版本和 Tool 边界可以独立测试。
- 固定单 Tool 建议足以完成 Milestone 8 的可见具身闭环，同时避免提前引入多步 Agent 循环和无限动作面。
- UE 二次校验保证 Python 延迟、模型错误或世界变化不会直接修改 Gameplay 事实。

取舍：
- UE 与 Python 需要维护一套新的共享 Schema 和契约测试。
- 单次响应只能建议一个动作，连续重新规划留到 Milestone 9。
- `ttl_ms` 使用 UE 本地单调时间判定，服务端不能替代客户端判断过期。

状态：
已接受

重要性：
重要

## 2026-09-01：连续 Decision 使用事件触发与有界最新合并

决定：
Milestone 9 的单 Guard 连续判断继续使用现有 Decision v1，不增加协议字段或 Tool。UE 只在 Guard 实际感知到新 Speech、已完成玩家行为、距离跨稳定阈值、受击、计划完成或计划失效等显著变化时调度请求。调度器固定最多一个请求在途和一个最新 Pending Trigger，执行冷却，并限制没有新玩家输入时的连续自动重规划次数。

Guard 已公开的 Speech 和真实 Action Result 可以作为个人历史事实进入后续请求；未执行 Tool、隐藏目标、模型隐式推理和其他 NPC Observation 不进入历史。距离检测允许每帧检查数值，但只有跨阈值时产生 Observation 和调度，不进行逐帧 LLM 控制。

原因：
- 连续互动需要对世界变化重新判断，但逐帧请求会造成成本、竞态和请求风暴。
- 只保留最新待处理事实能在异步响应期间吸收变化，同时维持固定内存和确定完成语义。
- 沿用现有 Trigger、History、Intent 与四个 Tool 足以表达当前高层变化；命中、伤害和即时安全动作继续由 UE 权威执行。
- 公开 Speech 与真实动作历史可以保持角色外部立场连续性，而不保存或传输 Chain-of-Thought。

取舍：
- 中间的重复触发可能被更新事实覆盖，调试视图只保留合并计数和最新原因。
- 单 Pending 与冷却不适合高并发群体规划；Milestone 9 仍只服务一个 Guard。
- 当前历史表达外部事实，不持久化隐藏长期计划；更复杂目标记忆需要未来独立设计。

状态：
已接受

重要性：
重要

## 2026-09-01：基础攻击与伤害保持 UE 即时权威

决定：
Milestone 9 将玩家 Attack 加入本地受控 Action 白名单，但不加入 Decision 协议或 LLM Tool。GameMode 在产生 Action Observation 前以固定距离、冷却、目标和可执行状态校验攻击；NPC 在 UE 内维护生命、防卫减伤、受击无敌窗口、失能和状态版本。Python 只能看到已发生的有界 Action Result，不能决定命中、伤害、生命、无敌、失能或攻击频率。

原因：
- 冲突升级需要可见且真实的受击后果，但将即时命中交给异步模型会破坏 UE 的权威与可复现性。
- 独立的纯数据攻击校验使非法距离、冷却和失能攻击能证明零 Gameplay 副作用。
- 维持现有四个 Decision Tool 可避免未经确认的协议扩展，同时将高层应对策略与即时伤害分层。

取舍：
- 当前只有最小 Attack/生命/防卫/失能，不包含武器、连招、GAS、动画或完整战斗系统。
- 防卫状态在后续升级—缓和工作包中由规则和可见状态驱动；本提交只提供权威状态与伤害边界。

状态：
已接受

重要性：
重要

## 2026-09-01：多 NPC Decision 编排留在 UE 并保持单 NPC 协议

决定：
Milestone 10 不增加批量 Decision 协议。Guard、Merchant、Rival 和 Civilian 继续各自发送现有单 NPC `/v1/decision` 请求；UE 为每个 NPC 维护一个在途和一个最新 Pending，并将全局并发固定为 2。NPC 只有实际听见或看见 Trigger 后才进入自己的调度器；回调、状态、公开历史、冲突和 Tool 执行全部按稳定 NPC ID 隔离。

原因：
- 单 NPC 请求天然保持个人视角，避免批量 Context 把未感知事实广播给所有 NPC。
- UE 掌握感知、状态版本、调度优先级与动作执行，最适合执行全局成本和竞态控制。
- 沿用已验证的 Decision v1 可以复用两端 Schema、错误分类和 Tool Registry，不扩大协议攻击面。
- 全局并发 2 与逐 NPC 最新合并足以支撑 4 NPC 最终场景，同时对成本、延迟和请求风暴给出硬上限。

取舍：
- 多 NPC 不会在一次模型调用中共同规划，也不具有隐式共享对话上下文。
- 同时触发时由 UE 稳定轮转，部分 NPC 会等待并使用最新 Pending，而不是并行无限扩张。
- NPC 间自主对话、群体目标和共享计划需要未来独立设计，不能通过当前个人请求暗中实现。

状态：
已接受

重要性：
重要
