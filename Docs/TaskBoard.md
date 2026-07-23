# Task Board

## 用途

本文件将 [CurrentMilestone.md](./CurrentMilestone.md) 中的当前里程碑拆成可执行工作包，并记录任务状态、依赖和验收证据。它不改变里程碑范围，也不替代协议或架构文档。

当前执行队列只维护 Milestone 2。Milestone 1 已完成并保留归档摘要；后续 Milestone 不得提前加入执行队列。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| `待开始` | 范围和前置条件明确，但尚未实施 |
| `进行中` | 已开始实施；同一任务只能有一个主要执行者 |
| `受阻` | 无法继续，且已记录阻塞原因和解除条件 |
| `待验收` | 实施完成，正在执行或等待规定验证 |
| `已完成` | 产物、适用验证和相关文档均已完成 |

任务只有在完成表中“完成条件”并留下可复查的验证记录后，才能标记为 `已完成`。未执行的验证必须写为“未验证”，不能根据代码存在推定通过。

## Milestone 2 工作包

| ID | 状态 | 工作包 | 主要产物 | 依赖 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M2-01` | `已完成` | 范围、协议与架构定稿 | Milestone、协议、架构、任务板、状态、模块和决策文档 | Milestone 1 | 开发前置文档相互一致，范围和协议扩展已获用户确认 |
| `M2-02` | `已完成` | Provider 配置与抽象 | Settings、Provider 接口、Provider Factory、Stub/Fake 注入边界 | `M2-01` | 配置可校验；真实/Stub 模式可明确选择；Service 不依赖具体 Provider SDK 类型 |
| `M2-03` | `已完成` | 初版真实 Provider | SDK 依赖、供应商 API 适配器、最小静态指令 | `M2-02` | 有效配置可生成非空纯文本回复，供应商类型不泄漏出 Provider 边界 |
| `M2-04` | `已完成` | Dialogue Service 与错误映射 | Provider 编排、协议成功响应、分类异常与脱敏日志 | `M2-03` | 成功及鉴权、限流、超时、不可用、无效响应均符合协议且单次请求不重试 |
| `M2-05` | `已完成` | Python 离线自动化测试 | 配置、Provider、Service、API 成功与错误路径测试 | `M2-02`、`M2-04` | 默认测试无外网、无 API Key、无真实 Token 消耗并覆盖全部 M2 Python 验收路径 |
| `M2-06` | `已完成` | UE 兼容与失败处理验证 | Provider 兼容测试、新错误码联调、超时配置与演示入口验证 | `M2-04` | UE 可展示真实 Provider 回复并安全处理全部 Provider 失败；外层超时次序明确 |
| `M2-07` | `已完成` | 切换至 Kimi K3 Provider | Kimi 配置、Chat Completions 适配器、协议标识、两端兼容测试和文档 | `M2-06` | 默认真实 Provider 为 `kimi-k3`；离线自动化和 UE 兼容验证通过 |
| `M2-08` | `已完成` | 真实端到端验收与交付记录 | 真实请求证据、密钥审计、依赖检查、Milestone 2 验收记录 | `M2-07` | `M2-A01` 至 `M2-A10` 均有可复查证据 |

## 工作包明细

### M2-01：范围、协议与架构定稿

- 将 `CurrentMilestone.md` 切换到 Milestone 2，定义范围、明确不做、验收标准和完成定义。
- 保持 `POST /v1/dialogue` 请求字段不变，扩展 `provider` 允许值和 Provider 错误码。
- 定义真实 Provider、Stub 离线模式、配置、Prompt、超时和密钥边界。
- 同步 `Architecture.md`、`ProjectOverview.md`、`ProjectState.md`、`PythonModules.md`、`UEClasses.md`、`CodingStandards.md` 和 `DecisionLog.md`。
- 创建 Milestone 2 验收记录模板。

验证：检查文档链接、JSON 示例、任务依赖、术语、协议状态码和跨文档范围一致性。

验证记录（2026-07-22）：开发前置文档已按用户确认的 Milestone 2 方案完成整理；文档一致性和链接检查结果见本次工作交付。未修改运行时代码，未执行代码测试。

### M2-02：Provider 配置与抽象

- 新增集中 Settings，读取 `ZL_DIALOGUE_PROVIDER`、供应商 API Key、模型、Provider 超时和最大输出 Token 数。
- 配置值必须有类型、范围和跨层超时校验；日志或异常不得包含密钥值。
- 定义与 FastAPI、协议 Schema 和供应商 SDK 解耦的 Dialogue Provider 接口。
- 提供 Provider Factory；默认选择真实 Provider，只有显式配置才选择 Stub。
- 保留确定性 Stub Provider，并允许应用工厂或 Service 构造函数注入 Fake Provider。
- 真实 Provider 模式配置无效时明确失败，不得静默回退 Stub。
- 更新 `PythonService/README.md`，说明安装、环境变量、真实 Provider 与离线启动方式。

验证：覆盖默认值、合法覆盖、无效类型/范围、缺少密钥、显式 Stub 模式和密钥脱敏。

验证记录（2026-07-22）：集中 Settings、Provider 接口与内部类型、显式 Factory、确定性 Stub Provider、应用启动组装和 Fake 注入边界已完成。Python 离线测试 `27 passed`，应用字节码编译和 `pip check` 通过；文档链接、密钥形态扫描与 `git diff --check` 通过。OpenAI SDK 调用和 Provider 错误映射按任务依赖分别留给 `M2-03` 与 `M2-04`。

### M2-03：初版真实 Provider

- 添加官方 OpenAI Python SDK 运行依赖，并锁定兼容版本范围。
- 使用 Responses API 完成一次非流式文本生成。
- 初始默认模型为 `gpt-5.6-luna`，但调用始终读取集中配置。
- 集中维护最小静态指令：只生成简洁纯文本回复，不声称具备未提供的人格、世界状态、记忆或 Gameplay 能力。
- `npc_id` 仅作为稳定标识；不从 ID 推导角色设定。
- 不启用托管工具、自定义工具、会话状态、流式输出或自动重试。
- 只向上层返回非空文本和 `openai` Provider 标识；SDK 响应类型不得泄漏。

验证：使用 Fake SDK Client 或等价注入验证请求参数、模型配置、输出提取、空/无效输出和一次调用约束；真实网络验证留给最终验收任务。

验证记录（2026-07-23）：官方 OpenAI Python SDK 兼容范围、Responses API 非流式适配器、集中静态指令和 Factory 接线已完成。Fake SDK Client 验证了模型、指令、输入、输出 Token 上限、超时、`max_retries=0`、单次调用、无工具/流式/会话参数、`npc_id` 不推导人格、输出裁剪以及空/无效输出拒绝。Python 离线测试 `34 passed`，`pip check` 通过；未访问真实 OpenAI API，未消耗 Token。

### M2-04：Dialogue Service 与错误映射

- 将现有 Stub 回复逻辑改为通过 Provider 接口生成回复。
- 保留空输入业务校验和字段透传。
- 将 Provider 内部错误映射为 `provider_auth_error`、`provider_rate_limited`、`provider_timeout`、`provider_unavailable` 或 `provider_error`。
- 保持 `internal_error` 仅表示非 Provider 的未预期错误。
- 成功响应返回逻辑 Provider 标识，不返回模型名、Token 用量或供应商响应 ID。
- 日志只记录 `request_id`、Provider、错误分类和必要状态；不记录完整玩家输入或上游异常正文。
- 单次请求最多调用 Provider 一次，不自动重试。

验证：逐项验证 [Protocol.md](./Protocol.md) 中全部状态码、错误结构、字段透传、脱敏和单次调用语义。

验证记录（2026-07-23）：OpenAI SDK 鉴权、权限、限流、超时、连接、服务端、模型不可用、响应校验和其他 API 错误已转换为供应商无关异常；应用层严格映射为协议规定的 `429`、`502`、`503`、`504` 与稳定错误码。Fake Provider/API 测试验证 `request_id`、固定脱敏消息、日志字段和单次调用，响应与日志均不包含上游原始异常正文。Python 离线测试 `48 passed`。

### M2-05：Python 离线自动化测试

- 保留 Milestone 1 的协议和业务回归覆盖。
- 增加 Settings 与 Provider Factory 测试。
- 增加真实 Provider 请求构造和输出提取测试。
- 增加 API 层真实 Provider/Stub 成功响应测试。
- 增加鉴权、限流、超时、不可用、无效响应和内部错误测试。
- 测试中拦截外部网络，并确认无需真实 API Key。
- 执行 `pip check` 并记录依赖结果。

验证：执行完整 Python 测试；测试必须可重复、无真实 Token 消耗且不依赖当前账户状态。

验证记录（2026-07-23）：测试套件通过自动夹具删除进程中的 `OPENAI_API_KEY`，并拒绝所有非本机套接字连接；使用 Fake SDK 串联验证 OpenAI 成功、鉴权、限流、超时、不可用和无效响应从 Provider 到 HTTP 协议层的结果，且每条路径仅发起一次生成调用。完整离线测试 `56 passed`，`pip check` 报告无依赖冲突。

### M2-06：UE 兼容与失败处理验证

- 确认 `FZLDialogueResponse::Provider` 继续接受任意字符串，测试 `stub` 与真实 Provider。
- 确认 `FZLServiceError::Code` 保留服务返回的新增错误码，不引入 Provider 专用 UE 枚举。
- 为新增 `429`、`502`、`503`、`504` 响应补充协议/失败处理测试。
- 调整默认 UE 请求超时，使其明确大于 Python Provider 超时并记录理由。
- 使用本地 Service 的 Stub/Fake 模式验证成功和 Provider 失败回调均只执行一次。
- 保持控制台演示入口，不新增正式 UMG 或 NPC Gameplay 集成。
- 同步更新 [UEClasses.md](./UEClasses.md)。

验证：编译受影响 UE Target，执行完整 `ZLAIRuntime` 自动化测试，并在 Game 模式验证一条成功和一条 Provider 失败路径。

验证记录（2026-07-23）：`ZLEditor Win64 Development` 编译成功；本机 Stub Service 下完整 `ZLAIRuntime` 自动化报告为 6/6 成功、0 失败。协议测试接受 `stub`、`openai` 和未来字符串 Provider；失败处理测试覆盖 `429`、`502`、`503`、`504` 并保留协议错误码；本地 HTTP 集成测试确认成功与失败路径均只完成一次回调。无界面 Game 模式通过 `ZL.AI.DialogueDemo` 分别验证了 Stub 成功回复和 Fake Provider 的 `503 provider_unavailable` 可定位失败。UE 请求超时调整为 30 秒，明确大于 Python Provider 默认 20 秒。

### M2-07：切换至 Kimi K3 Provider

- 默认真实 Provider 改为 Kimi，逻辑 Provider 标识改为 `kimi`。
- 使用官方文档支持的 OpenAI 兼容 SDK 调用 Kimi Chat Completions API，国内开放平台 Base URL 为 `https://api.moonshot.cn/v1`。
- 默认模型改为 `kimi-k3`，密钥改从 `MOONSHOT_API_KEY` 读取。
- 保持单次、非流式、无工具、无会话状态和不自动重试约束。
- 同步 Python/UE 自动化、协议、配置、模块和决策文档。

验证：运行完整 Python 离线测试、依赖检查和 UE 自动化，确认不访问外网、不需要真实 Key、不消耗 Token；真实网络验证留给 `M2-08`。

验证记录（2026-07-23）：默认真实 Provider、配置和协议标识已切换为 Kimi；适配器通过 Kimi 国内开放平台 Base URL 使用 `chat.completions.create`，默认模型为 `kimi-k3`，并保持单次、非流式、无工具、低推理强度和 `max_retries=0`。Python 离线测试 `56 passed`，`pip check` 通过；`ZLEditor Win64 Development` 编译成功，本机 Stub Service 下完整 `ZLAIRuntime` 自动化 6/6 成功。未执行生成式真实 Kimi 请求，未消耗 Token。

### M2-08：真实端到端验收与交付记录

阻塞解除（2026-07-23）：宿主 Windows 用户环境已配置 `MOONSHOT_API_KEY`；验收进程从用户环境读取密钥，不将值写入仓库、命令输出或验证记录。

- K3 在真实验收期间出现持续限流；账户模型列表确认 `kimi-k2.6` 可用，最小真实生成请求成功，因此当前默认模型降级为 `kimi-k2.6`。`ZL_KIMI_MODEL` 仍可显式覆盖模型，UE 协议不变。

- 使用有效但不入库的 `MOONSHOT_API_KEY` 启动 Kimi 模式 Service。
- 在同一次 UE Game 会话中提交至少 3 条不同玩家输入。
- 记录 UE 请求、Service 调用、`provider: kimi` 响应和 UE 展示结果之间的关联证据。
- 验证 Service 停止或 Provider 失败时 UE 可恢复。
- 审计 Git diff、已跟踪文件、日志和响应，确认不存在 API Key。
- 执行 Python 测试、依赖检查、UE 编译和完整自动化测试。
- 将每项结果记录到 [Milestone2Validation.md](./Validation/Milestone2Validation.md)，同步模块文档和 Project State。

验证：按 [CurrentMilestone.md](./CurrentMilestone.md) 中 `M2-A01` 至 `M2-A10` 逐项记录证据；未执行或失败的项目必须保持未验证。

验证记录（2026-07-23）：Kimi 国内 API 鉴权、余额和账户模型列表验证通过；因 K3 热点限流，将默认模型降级为账户已开放且真实生成成功的 `kimi-k2.6`，并为 K2.x 简短对话显式关闭思考。合法 HTTP 请求返回 `200`、非空回复、字段透传和 `provider: kimi`。绕过 UE 本机代理后，同一无界面 Game 会话中的 3 条不同输入均经 Service 返回 `200`，UE 收到 3 条一一对应回复且无失败。真实 1 毫秒 Provider 超时返回一次 `504 provider_timeout`，UE 只完成一次失败回调并正常退出。Python 离线测试 `57 passed`、`pip check` 通过；`ZLEditor Win64 Development` 编译成功，完整 `ZLAIRuntime` 自动化 6/6 成功。详细脱敏证据见 [Milestone2Validation.md](./Validation/Milestone2Validation.md)。

## 推荐执行顺序

```text
M2-01 -> M2-02 -> M2-03 -> M2-04 --+-> M2-05 --+
                                     |            +-> M2-07 -> M2-08
                                     +-> M2-06 --+
```

`M2-05` 与 `M2-06` 可在 Service 行为稳定后分别推进；`M2-07` 完成供应商切换，`M2-08` 必须等待两端实现与自动化验证完成。

## Milestone 1 归档摘要

| 范围 | 状态 | 验证 |
| --- | --- | --- |
| `M1-01` 至 `M1-07`：UE 到 Python Service 最小闭环 | `已完成` | [范围定稿](./Milestones/Milestone1.md)；[验收记录](./Validation/Milestone1Validation.md) |

Milestone 1 的范围定稿、验证记录、Git 历史、[UEClasses.md](./UEClasses.md) 与 [PythonModules.md](./PythonModules.md) 共同构成归档，不再进入当前执行队列。任务状态和同步规则见 [DocumentationRules.md](./DocumentationRules.md)。
