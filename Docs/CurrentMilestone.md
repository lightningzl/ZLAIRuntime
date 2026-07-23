# Current Milestone

历史里程碑定稿：[Milestone 1：UE 到 Python Service 最小闭环](./Milestones/Milestone1.md)。历史文档仅用于追溯，不覆盖本文件定义的当前范围。

## Milestone 2：真实 LLM 自由对话

目标：在保持 UE 请求入口和模块边界稳定的前提下，由 Python Service 通过 Kimi 开放平台 Chat Completions API 和 `kimi-k2.6` 生成真实、非流式的单轮 NPC 对话回复，并建立可配置、可测试、可诊断且不泄露密钥的 Provider 边界。

Milestone 1 的 Stub 通信闭环已经完成并作为本阶段基线；本阶段不重做 UE HTTP Client，也不提前引入人格、Memory 或 Tool Use。

## 本阶段范围

### UE5

- 继续通过 `UZLAIServiceSubsystem` 调用 `POST /v1/dialogue`，请求字段保持不变。
- 接受 `provider` 为 `stub` 或 `kimi`，不绑定模型名称。
- 保持对未知响应字段和未知服务错误码的兼容。
- 验证并展示真实 LLM 回复。
- 对 Provider 超时、鉴权失败、限流和上游故障保持可恢复，不崩溃且输出可定位信息。
- 调整 UE 外层请求超时，使其明确大于 Python Provider 超时，避免两层超时次序不确定。

### Python

- 在现有 Route、Schema、Service 分层内增加独立的 Dialogue Provider 边界。
- 保留 Stub Provider 作为显式离线开发模式；运行模式默认使用 Kimi Provider。
- 首个真实 Provider 使用 Kimi 开放平台的 OpenAI 兼容 Chat Completions API，并通过官方文档支持的 OpenAI Python SDK 调用。
- 模型通过环境配置提供；当前默认值为 `kimi-k2.6`，不得写入 UE 协议。
- API Key 仅从 `MOONSHOT_API_KEY` 读取，不接受请求字段、UE Config 或仓库文件中的密钥。
- 为模型名称、Provider 超时和最大输出 Token 数提供集中配置与边界校验。
- 使用最小静态指令生成简洁纯文本 NPC 回复；`npc_id` 在本阶段只作为稳定标识，不扩展为人格定义。
- 将 Provider 鉴权、限流、超时、不可用和无效响应转换为 [Protocol.md](./Protocol.md) 中的稳定错误。
- 日志保留 `request_id`、Provider、错误类别和必要状态，不记录密钥、完整玩家输入或上游原始异常正文。
- 通过依赖注入或应用工厂让自动化测试使用 Fake/Stub Provider，默认测试不访问外网、不消耗真实 Token。

## 配置基线

| 配置 | 必填 | 初始约定 |
| --- | --- | --- |
| `ZL_DIALOGUE_PROVIDER` | 否 | 默认 `kimi`；显式设置为 `stub` 可离线运行 |
| `MOONSHOT_API_KEY` | Kimi 模式必填 | 仅从进程环境读取 |
| `ZL_KIMI_MODEL` | 否 | 默认 `kimi-k2.6` |
| `ZL_KIMI_TIMEOUT_SECONDS` | 否 | 正数；必须小于 UE 外层请求超时 |
| `ZL_KIMI_MAX_OUTPUT_TOKENS` | 否 | 正整数并设置合理硬上限 |

具体默认超时和 Token 上限由 `M2-02` 实现并通过测试固定；如果实现需要改变上述配置名称或含义，必须同步更新本文件、README 和模块文档。

## 明确不做

- 不传递或设计 NPC 人格、说话风格、目标、完整世界状态或玩家历史。
- 不维护多轮对话上下文，不使用供应商托管的会话状态。
- 不做 SQLite、长期/短期 Memory、Chroma、FAISS 或向量检索。
- 不生成、解析或执行 Tool Call，不向模型开放内置工具或自定义工具。
- 不做流式输出、WebSocket、Realtime API、语音输入输出或 Token 逐字显示。
- 不做多 Provider 热切换、自动故障转移或复杂重试；单次玩家请求最多发起一次上游生成请求。
- 不做完整用量计费、Prompt 缓存、批处理、后台模式或生产级可观测性。
- 不做服务部署、用户鉴权、多人同步、限流网关或生产级运维。
- 不让 Python Service 直接读取或修改 UE 世界，也不从自然语言回复中推断 Gameplay 指令。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M2-A01` | Python Service 在 `stub` 模式下无需 API Key 或外部网络即可启动并响应。 |
| `M2-A02` | Python Service 在有效 Kimi 配置下可启动；Kimi 模式缺少或包含无效配置时给出明确、脱敏的失败信息。 |
| `M2-A03` | 使用合法 JSON 调用 `/v1/dialogue`，Kimi 模式返回 `200`、非空 `reply` 且结构符合协议。 |
| `M2-A04` | `request_id` 和 `npc_id` 在真实 Provider 请求链路中保持一致，成功响应的 `provider` 为 `kimi`。 |
| `M2-A05` | Provider 鉴权失败、限流、超时、不可用和无效响应均映射为规定的 HTTP 状态与结构化错误，且不泄露密钥、堆栈或上游原始异常正文。 |
| `M2-A06` | Python 自动化测试通过 Fake/Stub Provider 覆盖成功和全部 Provider 错误路径，默认测试不访问外网、不需要 API Key、不消耗真实 Token。 |
| `M2-A07` | UE 能继续解析 `stub` 与 `kimi` 成功响应，并对新增错误码保持兼容。 |
| `M2-A08` | UE 能发起真实模型请求并可见地展示回复；Provider 失败时 UE 不崩溃并显示可定位信息。 |
| `M2-A09` | UE 外层超时大于 Provider 超时，真实超时路径只有一次完成回调且无悬空引用。 |
| `M2-A10` | 在一次可复现验收中完成至少 3 条不同玩家输入的真实端到端请求，两端正常结束且仓库、日志和响应中均无密钥。 |

## 完成定义

`M2-02` 至 `M2-07` 的实现和适用自动化验证全部完成后，里程碑进入 `验收中`；完成 `M2-08` 并为 `M2-A01` 至 `M2-A10` 留下可复查证据后，Milestone 2 才能标记为 `已完成`。

验收记录统一写入 [Milestone2Validation.md](./Validation/Milestone2Validation.md)。任何超出本文件范围的能力进入后续 Milestone，不阻塞本阶段交付。
