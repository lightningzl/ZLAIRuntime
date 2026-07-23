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
| `M2-02` | `已完成` | Provider 配置与抽象 | Settings、Provider 接口、Provider Factory、Stub/Fake 注入边界 | `M2-01` | 配置可校验；OpenAI/Stub 模式可明确选择；Service 不依赖具体 Provider SDK 类型 |
| `M2-03` | `已完成` | OpenAI Provider | 官方 SDK 依赖、Responses API 适配器、最小静态指令 | `M2-02` | 有效配置可生成非空纯文本回复，供应商类型不泄漏出 Provider 边界 |
| `M2-04` | `已完成` | Dialogue Service 与错误映射 | Provider 编排、协议成功响应、分类异常与脱敏日志 | `M2-03` | 成功及鉴权、限流、超时、不可用、无效响应均符合协议且单次请求不重试 |
| `M2-05` | `待开始` | Python 离线自动化测试 | 配置、Provider、Service、API 成功与错误路径测试 | `M2-02`、`M2-04` | 默认测试无外网、无 API Key、无真实 Token 消耗并覆盖全部 M2 Python 验收路径 |
| `M2-06` | `待开始` | UE 兼容与失败处理验证 | Provider 兼容测试、新错误码联调、超时配置与演示入口验证 | `M2-04` | UE 可展示 OpenAI 回复并安全处理全部 Provider 失败；外层超时次序明确 |
| `M2-07` | `待开始` | 真实端到端验收与交付记录 | 真实请求证据、密钥审计、依赖检查、Milestone 2 验收记录 | `M2-05`、`M2-06` | `M2-A01` 至 `M2-A10` 均有可复查证据 |

## 工作包明细

### M2-01：范围、协议与架构定稿

- 将 `CurrentMilestone.md` 切换到 Milestone 2，定义范围、明确不做、验收标准和完成定义。
- 保持 `POST /v1/dialogue` 请求字段不变，扩展 `provider` 允许值和 Provider 错误码。
- 定义 OpenAI Provider、Stub 离线模式、配置、Prompt、超时和密钥边界。
- 同步 `Architecture.md`、`ProjectOverview.md`、`ProjectState.md`、`PythonModules.md`、`UEClasses.md`、`CodingStandards.md` 和 `DecisionLog.md`。
- 创建 Milestone 2 验收记录模板。

验证：检查文档链接、JSON 示例、任务依赖、术语、协议状态码和跨文档范围一致性。

验证记录（2026-07-22）：开发前置文档已按用户确认的 Milestone 2 方案完成整理；文档一致性和链接检查结果见本次工作交付。未修改运行时代码，未执行代码测试。

### M2-02：Provider 配置与抽象

- 新增集中 Settings，读取 `ZL_DIALOGUE_PROVIDER`、`OPENAI_API_KEY`、`ZL_OPENAI_MODEL`、Provider 超时和最大输出 Token 数。
- 配置值必须有类型、范围和跨层超时校验；日志或异常不得包含密钥值。
- 定义与 FastAPI、协议 Schema 和 OpenAI SDK 解耦的 Dialogue Provider 接口。
- 提供 Provider Factory；默认选择 OpenAI，只有显式配置才选择 Stub。
- 保留确定性 Stub Provider，并允许应用工厂或 Service 构造函数注入 Fake Provider。
- OpenAI 模式配置无效时明确失败，不得静默回退 Stub。
- 更新 `PythonService/README.md`，说明安装、环境变量、OpenAI 与离线启动方式。

验证：覆盖默认值、合法覆盖、无效类型/范围、缺少密钥、显式 Stub 模式和密钥脱敏。

验证记录（2026-07-22）：集中 Settings、Provider 接口与内部类型、显式 Factory、确定性 Stub Provider、应用启动组装和 Fake 注入边界已完成。Python 离线测试 `27 passed`，应用字节码编译和 `pip check` 通过；文档链接、密钥形态扫描与 `git diff --check` 通过。OpenAI SDK 调用和 Provider 错误映射按任务依赖分别留给 `M2-03` 与 `M2-04`。

### M2-03：OpenAI Provider

- 添加官方 OpenAI Python SDK 运行依赖，并锁定兼容版本范围。
- 使用 Responses API 完成一次非流式文本生成。
- 初始默认模型为 `gpt-5.6-luna`，但调用始终读取集中配置。
- 集中维护最小静态指令：只生成简洁纯文本回复，不声称具备未提供的人格、世界状态、记忆或 Gameplay 能力。
- `npc_id` 仅作为稳定标识；不从 ID 推导角色设定。
- 不启用托管工具、自定义工具、会话状态、流式输出或自动重试。
- 只向上层返回非空文本和 `openai` Provider 标识；SDK 响应类型不得泄漏。

验证：使用 Fake SDK Client 或等价注入验证请求参数、模型配置、输出提取、空/无效输出和一次调用约束；真实网络验证留给 `M2-07`。

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
- 增加 OpenAI Provider 请求构造和输出提取测试。
- 增加 API 层 OpenAI/Stub 成功响应测试。
- 增加鉴权、限流、超时、不可用、无效响应和内部错误测试。
- 测试中拦截外部网络，并确认无需 `OPENAI_API_KEY`。
- 执行 `pip check` 并记录依赖结果。

验证：执行完整 Python 测试；测试必须可重复、无真实 Token 消耗且不依赖当前账户状态。

### M2-06：UE 兼容与失败处理验证

- 确认 `FZLDialogueResponse::Provider` 继续接受任意字符串，测试 `stub` 与 `openai`。
- 确认 `FZLServiceError::Code` 保留服务返回的新增错误码，不引入 Provider 专用 UE 枚举。
- 为新增 `429`、`502`、`503`、`504` 响应补充协议/失败处理测试。
- 调整默认 UE 请求超时，使其明确大于 Python Provider 超时并记录理由。
- 使用本地 Service 的 Stub/Fake 模式验证成功和 Provider 失败回调均只执行一次。
- 保持控制台演示入口，不新增正式 UMG 或 NPC Gameplay 集成。
- 同步更新 [UEClasses.md](./UEClasses.md)。

验证：编译受影响 UE Target，执行完整 `ZLAIRuntime` 自动化测试，并在 Game 模式验证一条成功和一条 Provider 失败路径。

### M2-07：真实端到端验收与交付记录

- 使用有效但不入库的 `OPENAI_API_KEY` 启动 OpenAI 模式 Service。
- 在同一次 UE Game 会话中提交至少 3 条不同玩家输入。
- 记录 UE 请求、Service 调用、`provider: openai` 响应和 UE 展示结果之间的关联证据。
- 验证 Service 停止或 Provider 失败时 UE 可恢复。
- 审计 Git diff、已跟踪文件、日志和响应，确认不存在 API Key。
- 执行 Python 测试、依赖检查、UE 编译和完整自动化测试。
- 将每项结果记录到 [Milestone2Validation.md](./Validation/Milestone2Validation.md)，同步模块文档和 Project State。

验证：按 [CurrentMilestone.md](./CurrentMilestone.md) 中 `M2-A01` 至 `M2-A10` 逐项记录证据；未执行或失败的项目必须保持未验证。

## 推荐执行顺序

```text
M2-01 -> M2-02 -> M2-03 -> M2-04 --+-> M2-05 --+
                                     |            +-> M2-07
                                     +-> M2-06 --+
```

`M2-05` 与 `M2-06` 可在 Service 行为稳定后分别推进；`M2-07` 必须等待两端实现与自动化验证完成。

## Milestone 1 归档摘要

| 范围 | 状态 | 验证 |
| --- | --- | --- |
| `M1-01` 至 `M1-07`：UE 到 Python Service 最小闭环 | `已完成` | [范围定稿](./Milestones/Milestone1.md)；[验收记录](./Validation/Milestone1Validation.md) |

Milestone 1 的范围定稿、验证记录、Git 历史、[UEClasses.md](./UEClasses.md) 与 [PythonModules.md](./PythonModules.md) 共同构成归档，不再进入当前执行队列。任务状态和同步规则见 [DocumentationRules.md](./DocumentationRules.md)。
