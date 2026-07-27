# Current Milestone

历史里程碑定稿：

- [Milestone 1：UE 到 Python Service 最小闭环](./Milestones/Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](./Milestones/Milestone2.md)

历史文档仅用于追溯，不覆盖本文件定义的当前范围。

## Milestone 3：NPC 上下文与人格

目标：在保持现有非流式 `POST /v1/dialogue`、Provider 边界和 UE 最终 Gameplay 控制权不变的前提下，由 UE 为每次请求提供受限的 NPC 人格、当前世界状态和有限对话历史，Python Service 通过独立 Context Builder 将其组装为供应商无关的生成上下文，使 NPC 回复能够稳定体现当前角色与场景。

Milestone 2 的真实 Kimi 对话链路已经完成并作为本阶段基线。本阶段只处理单次请求携带的瞬时上下文，不建设持久化 Memory、服务端会话或 Tool Use。

## 本阶段范围

### UE5

- 保留现有无上下文请求入口，并增加可提交 `FZLDialogueContext` 的公开重载，旧调用方无需修改即可继续工作。
- 为 NPC 人格、世界状态、对话消息和完整上下文增加协议结构体，并严格按 [Protocol.md](./Protocol.md) 序列化。
- Gameplay/UI 负责从自身已知状态构造上下文快照；`ZLAIRuntime` 插件只校验、传输和解析，不读取具体 NPC Actor、关卡或 UI 内部状态。
- 上下文请求仍由 `UZLAIServiceSubsystem` 管理，继续满足唯一 `request_id`、一次完成回调、Game Thread 回调和无悬空引用约束。
- 更新最小演示入口，使其能提交一组确定性示例人格、世界状态和有限历史；不建设正式 UMG、对话编辑器或内容资产系统。
- 增加协议序列化、边界校验、兼容性和本地 Service 集成测试。

### Python

- 在 v1 Schema 中增加可选、结构化且有边界的 `context`；无 `context` 的 Milestone 1/2 请求继续保持兼容。
- 增加独立 Context Builder，把固定系统约束、NPC 人格、世界状态、有限历史和当前玩家输入组装为供应商无关的生成输入。
- 固定系统约束始终由 Python Service 维护；请求中的人格、世界状态、历史和玩家输入都作为数据处理，不得改变系统边界。
- Dialogue Service 负责业务校验和编排，只把校验后的上下文交给 Context Builder，再调用一次 Provider。
- Provider 接口接收 Context Builder 的内部结果，不接收 FastAPI/Pydantic 协议模型，也不自行解释 UE 字段。
- Kimi Provider 将内部生成上下文映射为一次非流式 Chat Completions 请求，保持无工具、无托管会话、无自动重试。
- Stub/Fake 路径提供确定性方式验证上下文透传和组装；自动化测试默认不访问外网、不读取真实密钥、不消耗 Token。
- 日志只记录 `request_id`、`npc_id`、Provider、上下文是否存在、历史条数和错误分类，不记录完整人格、世界事实、历史或玩家输入。

## 协议与边界基线

- Endpoint 和响应结构保持不变；请求新增可选 `context`，详细字段、顺序和长度限制以 [Protocol.md](./Protocol.md) 为准。
- `context` 是请求时快照，不是会话 ID、Memory 引用或服务端状态句柄。
- 对话历史按最旧到最新排列，只包含之前已完成的 `player`/`npc` 消息，不包含本次 `player_input`。
- Python 是协议输入校验的最终权威；UE 也应在发送前拒绝明显越界内容，避免无效网络请求。
- 结构或类型错误返回 `422 validation_error`；通过 Schema 后违反业务规则的内容返回 `400 invalid_request`。
- 上下文缺失时继续使用 Milestone 2 的无上下文生成行为；不得静默补造具体人格、世界事实或历史。

## 明确不做

- 不做 SQLite、文件数据库、长期或短期持久化 Memory、摘要 Memory、Chroma、FAISS 或向量检索。
- 不由 Python Service 保存、恢复或合并会话；不增加 `session_id`、`conversation_id` 或供应商托管会话状态。
- 不从 UE Actor、World、GameState、SaveGame 或数据库自动抓取上下文；Gameplay/UI 显式构造快照。
- 不设计正式内容资产、DataTable、编辑器面板、对话树、任务系统或完整 NPC 创作工作流。
- 不生成、解析或执行 Tool Call，不从自然语言回复中推断 Gameplay 指令。
- 不做流式输出、WebSocket、Realtime API、语音或逐 Token UI。
- 不增加多 Provider 热切换、自动故障转移、复杂重试、Prompt 缓存或生产级可观测性。
- 不让请求上下文覆盖固定系统约束、选择 Provider/模型、提供密钥或改变工具权限。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M3-A01` | 不含 `context` 的既有合法请求继续返回符合 v1 协议的成功响应，Milestone 2 的成功、错误和超时回归保持通过。 |
| `M3-A02` | UE 与 Python 对完整 `context` 的字段名、类型、角色枚举、顺序和边界限制一致；合法最小值和最大值可通过，越界或结构错误按协议拒绝。 |
| `M3-A03` | Context Builder 以确定性顺序组装固定系统约束、NPC 人格、世界状态、历史和当前输入；当前 `player_input` 不会被重复加入历史。 |
| `M3-A04` | 固定系统约束不能被人格、世界事实、历史或玩家输入改写；生成请求保持无工具、无托管会话、非流式且单次调用。 |
| `M3-A05` | Fake/Stub Provider 可验证人格、世界状态和历史均被完整、按序传入内部生成上下文，且 Provider 不依赖协议 Schema 或 UE 类型。 |
| `M3-A06` | Python 离线自动化覆盖无上下文兼容、完整上下文、各字段边界、非法角色、空白内容、历史顺序、Prompt 注入边界和全部既有错误路径；默认无外网、无真实 Key、无 Token 消耗。 |
| `M3-A07` | UE 保留旧请求入口并支持新上下文入口；序列化测试覆盖最小、完整、边界和非法上下文，失败时不发送 HTTP 且只完成一次失败回调。 |
| `M3-A08` | UE 编译、完整 `ZLAIRuntime` 自动化和本地 Stub/Fake Service 集成验证通过；无上下文与有上下文请求都能正常结束。 |
| `M3-A09` | 真实 Kimi 端到端验收至少覆盖人格、世界状态和历史三类受控变化，回复可见地符合提供的上下文，字段关联可通过 `request_id` 复查。 |
| `M3-A10` | 仓库、日志、错误响应和验收记录不包含 API Key、完整玩家输入、完整人格、完整世界事实、完整历史或模型原始响应；Service 重启后不存在可恢复的对话状态。 |

## 完成定义

`M3-02` 至 `M3-06` 的实现和适用自动化验证全部完成后，里程碑进入 `验收中`；完成 `M3-07` 并为 `M3-A01` 至 `M3-A10` 留下可复查证据后，Milestone 3 才能标记为 `已完成`。

验收记录统一写入 [Milestone3Validation.md](./Validation/Milestone3Validation.md)。任何持久化 Memory、Tool Use、正式 UI 或内容生产能力进入后续里程碑，不阻塞本阶段交付。
