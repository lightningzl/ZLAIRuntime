# Decision Log

新增或调整决策前，必须遵守 [DocumentationRules.md](./DocumentationRules.md) 中的“Decision Log 规则”。

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
已接受

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
已接受

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
