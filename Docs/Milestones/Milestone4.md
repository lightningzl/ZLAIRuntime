# Milestone 4 Archive

本文档保存 Milestone 4 完成时的 `CurrentMilestone.md` 定稿内容。它只用于历史追溯，不定义当前开发范围；当前范围以 [CurrentMilestone.md](../Current/CurrentMilestone.md) 为准。

历史里程碑定稿：

- [Milestone 1：UE 到 Python Service 最小闭环](./Milestone1.md)
- [Milestone 2：真实 LLM 自由对话](./Milestone2.md)
- [Milestone 3：NPC 上下文与人格](./Milestone3.md)

历史文档仅用于追溯，不覆盖本文件定义的当前范围。

## Milestone 4：持久化对话 Memory

目标：在保持 UE 为 Gameplay 状态事实来源、Provider 供应商无关边界和 UE 最终 Gameplay 控制权不变的前提下，为 `POST /v1/dialogue` 增加显式启用的本地持久化对话 Memory。Python Service 使用 SQLite 按玩家记忆范围与 NPC 隔离保存成功完成的对话轮次，在后续请求中按确定性预算检索并交给 Context Builder，使 NPC 在 Service 重启后仍能延续有限的跨请求对话记忆。

Milestone 3 的瞬时 `context`、真实 Kimi 链路和无状态兼容路径作为本阶段基线。Memory 是可选能力：请求省略 `memory` 时必须继续保持既有无状态行为。

## 本阶段范围

### UE5

- 在现有 v1 请求类型中增加可选 `memory`，其中只包含由 Gameplay/UI 提供的稳定、不透明 `scope_id`。
- Memory 以 `(scope_id, npc_id)` 作为隔离键；`ZLAIRuntime` 只负责校验和传输，不解释玩家账号、存档槽或业务身份。
- 保留所有既有无 Memory 请求入口；增加接受 Memory 范围的公开入口或参数对象，复用现有请求、HTTP、超时和单次完成逻辑。
- Gameplay/UI 显式决定是否启用 Memory，并负责为同一玩家或存档提供稳定 `scope_id`；插件不得自动读取账号、SaveGame、Actor、World 或平台身份。
- 增加协议序列化、边界校验、兼容性和本地 Service 集成测试。
- 增加最小脱敏演示入口，验证同一范围连续对话、不同范围隔离和 Service 重启恢复；不建设正式 Memory UI。

### Python

- 在 v1 Schema 中增加可选、受限的 `memory.scope_id`；省略时不得读取或写入 Memory。
- 增加独立 Memory Service 与 SQLite Repository。Dialogue Service 只依赖 Memory Service 接口，不直接执行 SQL。
- SQLite 保存成功完成的对话轮次，至少包含唯一 `request_id`、`scope_id`、`npc_id`、玩家输入、NPC 回复和稳定排序信息。
- `request_id` 在持久层唯一；重复请求不得产生重复 Memory 轮次。
- 每次生成前按 `(scope_id, npc_id)` 检索最近的已完成轮次，按最旧到最新交给 Context Builder；检索数量由有界配置控制。
- 持久化 Memory 位于客户端瞬时 `dialogue_history` 之前。两者发生精确重叠时去除重复消息，保留客户端提供的较新快照；当前 `player_input` 仍只出现一次。
- 只有 Provider 成功并产生合法回复后才写入本轮；Schema、业务、Provider 或持久层失败不得留下半轮记录。
- Repository 管理建表、索引、事务和连接生命周期；数据库不可用时请求通过现有 `500 internal_error` 安全失败，不泄露 SQL、路径或对话正文。
- 提供本地维护入口以检查统计信息和按范围清理测试数据；它不是 UE Runtime 协议，也不得输出完整对话正文。
- 自动化默认使用临时 SQLite 数据库、Fake/Stub Provider、无外网、无真实密钥和无 Token。

## 协议与数据基线

- Endpoint 与成功/错误响应结构保持不变；请求新增可选 `memory`，详细字段和边界以 [Protocol.md](../Reference/Protocol.md) 为准。
- `memory` 存在即表示本次请求允许读取并在成功后写入对应范围；省略时严格无状态。
- `scope_id` 是不透明隔离标识，不是文件名、表名、SQL 片段、认证凭据或可由 Service 推导的玩家资料。
- Memory 范围由 `(scope_id, npc_id)` 共同确定；不同任一字段都不得互相读取记录。
- SQLite 是 Python Service 的内部实现，不向 UE 暴露数据库路径、行 ID、迁移版本或查询接口。
- 持久化历史、瞬时历史和当前输入的合并必须确定、可测试并受预算限制；不得把数据库内容提升为系统指令。
- 现有错误包络保持不变；数据库异常使用脱敏 `500 internal_error`。

## 明确不做

- 不做 Chroma、FAISS、Embedding、语义检索、向量索引、混合检索或自动相关性评分。
- 不做 LLM 摘要 Memory、事实抽取、人格自动更新、遗忘权重、情绪模型或知识图谱。
- 不保存完整 NPC 人格、世界快照、Provider 原始响应、SDK 对象、Token 用量或模型推理内容。
- 不建设用户账号、鉴权、云同步、多设备同步、远程数据库、备份恢复或生产级加密密钥管理。
- 不增加供应商托管会话、`conversation_id`、自动重试、流式输出、WebSocket 或 Realtime API。
- 不提供 UE 正式 Memory 浏览、编辑、删除 UI；本阶段只有脱敏演示和 Python 本地维护入口。
- 不生成、解析或执行 Tool Call，不从 Memory 或自然语言回复中推断 Gameplay 指令。
- 不允许 Python 从 UE World、SaveGame、账号系统或平台服务主动抓取身份和状态。

## 验收标准

| ID | 标准 |
| --- | --- |
| `M4-A01` | 省略 `memory` 的全部既有合法请求保持无状态，Milestone 1 至 3 的成功、错误、超时、上下文和单次回调回归通过。 |
| `M4-A02` | UE 与 Python 对 `memory.scope_id` 的字段名、可选语义、类型和边界一致；合法边界可通过，缺字段、错误类型、空白或越界内容按协议拒绝。 |
| `M4-A03` | SQLite Repository 可确定性初始化 Schema 和索引，数据库文件不进入 Git；临时库和重启后的同一库均可读取已提交数据。 |
| `M4-A04` | Memory 严格按 `(scope_id, npc_id)` 隔离，检索只返回最近的有界已完成轮次，并以最旧到最新的稳定顺序交给上层。 |
| `M4-A05` | Context Builder 确定性合并持久化历史、客户端瞬时历史和当前输入；精确重叠不重复，客户端历史优先，当前 `player_input` 只出现一次。 |
| `M4-A06` | 只有合法 Provider 成功结果写入一条完整轮次；失败请求不写入，重复 `request_id` 不产生重复记录，事务失败不留下半轮。 |
| `M4-A07` | Route、Dialogue Service、Context Builder 和 Provider 保持既有依赖边界；SQL 只存在于 Repository，Provider 不依赖 Memory、SQLite 或协议 Schema。 |
| `M4-A08` | Python 离线自动化覆盖 Schema、初始化、事务、排序、预算、隔离、合并、幂等、失败回滚、维护入口和日志脱敏；默认无外网、无真实 Key、无 Token。 |
| `M4-A09` | UE 保留旧入口并支持显式 Memory 入口；编译、完整 `ZLAIRuntime` 自动化和本地 Stub 集成覆盖无 Memory、连续 Memory、隔离、非法范围和单次回调。 |
| `M4-A10` | 真实端到端验收证明同一范围在 Service 重启后可利用既有记忆，不同 scope/NPC 不串线；仓库、日志、错误响应和验收记录无密钥、数据库文件、完整对话或原始模型响应。 |

## 完成定义

`M4-02` 至 `M4-07` 的实现和适用离线验证全部完成后，里程碑进入 `验收中`；完成 `M4-08` 并为 `M4-A01` 至 `M4-A10` 留下可复查证据后，Milestone 4 才能标记为 `已完成`。

验收记录见 [Milestone4Validation.md](../Validation/Milestone4Validation.md)。Milestone 4 已于 2026-08-03 完成；本文件为归档定稿，不随当前协议和实现继续演进。
