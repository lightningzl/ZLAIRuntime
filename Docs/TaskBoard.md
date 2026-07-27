# Task Board

## 用途

本文件将 [CurrentMilestone.md](./CurrentMilestone.md) 中的当前里程碑拆成可执行工作包，并记录任务状态、依赖和完成条件。它不改变里程碑范围，也不替代协议、架构或验收标准。

当前执行队列只维护 Milestone 3。Milestone 1 和 Milestone 2 已完成并归档；后续 Milestone 不得提前加入执行队列。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| `待开始` | 范围和前置条件明确，但尚未实施 |
| `进行中` | 已开始实施；同一任务只能有一个主要执行者 |
| `受阻` | 无法继续，且已记录阻塞原因和解除条件 |
| `待验收` | 实施完成，正在执行或等待规定验证 |
| `已完成` | 产物、适用验证和相关文档均已完成 |

任务只有在完成表中“完成条件”并留下可复查的验证记录后，才能标记为 `已完成`。未执行的验证必须写为“未验证”，不能根据代码存在推定通过。

## Milestone 3 工作包

| ID | 状态 | 工作包 | 主要产物 | 依赖 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M3-01` | `已完成` | 里程碑范围、协议与开发准备定稿 | M2 归档、M3 范围、v1 上下文协议、架构、任务板、状态、模块、决策和验收模板 | Milestone 2 | 用户确认协议扩展；全部开发前置文档职责清晰、链接和术语一致 |
| `M3-02` | `已完成` | 两端协议类型与边界校验 | Python Schema、UE 上下文结构体、序列化与校验函数、兼容测试 | `M3-01` | 无上下文请求保持兼容；合法上下文可往返；非法结构和边界按协议拒绝 |
| `M3-03` | `已完成` | Python Context Builder | 供应商无关内部类型、固定系统约束、确定性上下文组装与单元测试 | `M3-02` | 组装顺序稳定；当前输入不重复；请求数据不能覆盖系统边界 |
| `M3-04` | `已完成` | Dialogue Service 与 Kimi Provider 接线 | Service 编排、内部生成输入、Kimi 消息映射、Stub/Fake 上下文路径 | `M3-03` | Provider 不依赖协议模型；上下文完整按序传入；保持一次非流式无工具调用 |
| `M3-05` | `已完成` | Python 离线回归与安全验证 | Schema、Builder、Service、Provider、API 和日志测试 | `M3-02`、`M3-04` | 覆盖全部 M3 Python 验收路径，默认无外网、无真实密钥、无 Token 消耗 |
| `M3-06` | `已完成` | UE 上下文入口、演示与集成验证 | 兼容重载、最小上下文演示、自动化和本地 HTTP 集成测试、UE 模块文档 | `M3-02`、`M3-04` | 旧/新入口均可用；非法上下文不发请求；UE 编译与完整插件自动化通过 |
| `M3-07` | `已完成` | 真实端到端验收与交付记录 | 真实 Kimi 受控场景、状态重启检查、安全审计和 M3 验收记录 | `M3-05`、`M3-06` | `M3-A01` 至 `M3-A10` 均有可复查证据 |

## 工作包明细

### M3-01：里程碑范围、协议与开发准备定稿

- 将完成时的 Milestone 2 范围定稿归档到 [Milestone2.md](./Milestones/Milestone2.md)。
- 将 `CurrentMilestone.md` 切换到 Milestone 3，定义范围、明确不做、验收标准和完成定义。
- 经用户确认，在 v1 请求中增加可选、受限的 `context`，响应和 Endpoint 保持不变。
- 定义 UE 上下文快照所有权、Python Context Builder、Provider 输入边界和无服务端会话约束。
- 同步架构、模块、编码标准、决策、项目状态和验收模板。

验证记录（2026-07-27）：用户确认 v1 `context` 协议方案；Milestone 2 已归档，Milestone 3 开发准备文档已完成。仅修改文档，未修改运行时代码，代码测试未执行。

### M3-02：两端协议类型与边界校验

- 在 Python Schema 中增加 `NpcContext`、`WorldContext`、`DialogueHistoryMessage` 和 `DialogueContext`。
- 使用显式长度、数组数量和 `player`/`npc` 枚举约束；保留未知字段兼容策略。
- 对空白字符串和其他结构校验后业务违规返回稳定的 `400 invalid_request`。
- 在 UE 插件中增加对应 `USTRUCT`，字段名和容器类型与协议一致。
- 扩展请求序列化器；`context` 缺失时完全省略字段，不发送 `null` 或空占位对象。
- UE 在网络调用前执行同等的显式边界检查，并通过现有 Client 错误路径完成一次失败回调。
- 保留现有无上下文请求序列化和响应/错误解析行为。

验证：Python 与 UE 分别覆盖最小值、最大值、越界、非法角色、缺字段、未知字段、空历史和完整上下文；对照 [Protocol.md](./Protocol.md) 逐字段复核。

验证记录（2026-07-27）：Python 新增四类上下文 Schema、显式数组/文本边界和空白业务校验；UE 新增对应 `USTRUCT`、可选上下文标记、请求校验和完整 JSON 序列化。Python 完整离线测试 `82 passed`，`pip check` 与字节码编译通过；`ZLEditor Win64 Development` 编译成功，`ZLAIRuntime.Protocol` 自动化 5/5 通过。最大边界、Unicode code point、未知字段、非法角色、空白内容和无上下文兼容均有自动化证据。

### M3-03：Python Context Builder

- 新增供应商无关的 Context Builder 模块和内部生成输入类型。
- 固定系统约束只在 Python 内维护，明确禁止 Tool、Memory、世界修改和越权指令。
- 以确定性结构组装 NPC ID/人格、世界状态、按序历史和当前玩家输入。
- 将请求字符串标记并作为数据放入上下文，不把它们拼接成可覆盖系统约束的自由指令。
- 当前 `player_input` 只作为最后一条当前输入出现一次。
- Builder 不访问 Settings、网络、数据库、FastAPI Request 或具体 Provider SDK。

验证：快照式/结构化断言组装结果与顺序；覆盖空 goals/facts/history、最大边界、角色映射、当前输入不重复和注入样例。

验证记录（2026-07-27）：新增不可变的 `DialogueGenerationContext`/`DialogueGenerationMessage` 内部类型和无状态 Context Builder；固定系统约束、JSON 上下文数据、历史消息及当前输入保持独立边界。专项测试 5/5、Python 完整离线测试 `87 passed`，覆盖无上下文、完整上下文、空可选数组、角色映射、同文旧消息、当前输入追加顺序和 Prompt 注入样例。

### M3-04：Dialogue Service 与 Kimi Provider 接线

- Dialogue Service 将通过验证的协议请求转换为 Builder 输入，再调用一次 Provider。
- Provider 接口改为接收供应商无关的内部生成上下文，不暴露 Pydantic、FastAPI、UE 或 Kimi SDK 类型。
- Kimi Provider 把固定系统约束和对话消息映射到 Chat Completions `messages`。
- Stub/Fake Provider 接收相同内部类型，并提供确定性验证上下文是否存在、顺序是否正确的方式。
- 保持 `max_retries=0`、非流式、无工具、无托管会话和现有错误分类。
- 不把完整 Prompt、人格、世界事实、历史或玩家输入写入日志。

验证：Fake SDK 覆盖消息顺序、角色映射、模型/超时/输出上限、一次调用、无工具/会话参数、空/无效输出和全部既有错误映射。

验证记录（2026-07-27）：Dialogue Service 已通过 Context Builder 生成内部输入；Provider 接口移除旧的 `npc_id + player_input` 请求类型，只接受 `DialogueGenerationContext`。Kimi Provider 按固定系统约束、标记的 JSON 数据、历史和当前输入映射消息，保持 `max_retries=0`、单次、非流式、无工具和无托管会话。相关专项测试 62/62、Python 完整离线测试 `88 passed`；Fake Provider 和 Fake SDK 均验证完整上下文及顺序。

### M3-05：Python 离线回归与安全验证

- 保留 Milestone 1/2 的协议、配置、Provider 和错误路径回归。
- 增加上下文 Schema 与业务校验的参数化边界测试。
- 增加 Builder、Service、Stub/Fake、Kimi 请求映射和 API 集成测试。
- 测试中继续删除真实 `MOONSHOT_API_KEY` 并拒绝非本机网络。
- 验证普通和错误日志只包含允许的元数据，不出现完整上下文或上游原始异常。
- 执行完整 Python 测试与 `pip check`。

验证：测试必须可重复、无外网、无真实 Token，并为 `M3-A01` 至 `M3-A06` 提供自动化证据。

验证记录（2026-07-27）：补齐所有上下文数组的空白内容参数化测试，以及成功/Provider 错误日志脱敏测试；请求日志只记录允许的关联元数据。Python 完整离线测试 `92 passed`，`pip check` 与字节码编译通过；测试全局移除真实密钥并阻止非本机套接字连接，未访问真实 Provider、未消耗 Token。`M3-A06` 已验证，`M3-A01` 的 UE 回归部分留待 M3-06。

### M3-06：UE 上下文入口、演示与集成验证

- 保留 `SendDialogueRequest(NpcId, PlayerInput, ...)`，增加接受 `FZLDialogueContext` 的重载；两个入口共享同一发送与完成逻辑。
- Gameplay/UI 显式构造快照，插件不得依赖具体 NPC Actor、World、GameState、UI 或 DataTable。
- 更新 `ZL.AI.DialogueDemo` 或增加同级最小命令，使用固定脱敏示例提交完整上下文。
- 协议测试覆盖无上下文、完整上下文、边界与非法内容。
- 本地 Stub/Fake Service 集成验证旧/新入口、成功/失败和单次回调。
- 编译受影响 UE Target，执行完整 `ZLAIRuntime` 自动化和无界面 Game 演示。

验证：为 `M3-A07`、`M3-A08` 留下编译、自动化和运行证据；不将示例扩展成正式 Gameplay/UI 系统。

验证记录（2026-07-27）：保留旧 `NpcId + PlayerInput` 入口并新增完整 `FZLDialogueContext` 重载，两个入口共享请求验证、序列化、HTTP 和完成逻辑；新增三类受控上下文的脱敏 Game 演示命令。`ZLEditor Win64 Development` 编译成功，完整 `ZLAIRuntime` 自动化 8/8 通过；本地 Stub 集成覆盖旧/新成功、服务端 `400` 和非法上下文本地失败，非法上下文只回调一次且未产生 HTTP。无界面 Game 上下文演示成功返回 `provider=stub`，进程正常退出。`M3-A01`、`M3-A07`、`M3-A08` 已验证。

### M3-07：真实端到端验收与交付记录

- 使用有效但不入库的 `MOONSHOT_API_KEY` 启动 Kimi 模式 Service。
- 设计三组受控请求，分别只改变人格、世界状态和历史，记录脱敏场景摘要。
- 通过 `request_id` 关联 UE 请求、Service 状态、`provider: kimi` 响应和 UE 可见结果。
- 重启 Service 后确认没有可恢复的对话历史或服务端会话状态。
- 审计 Git diff、已跟踪文件、普通/错误日志和响应，确认不存在密钥或完整上下文。
- 执行 Python 完整测试、依赖检查、UE 编译和完整自动化。
- 将实际证据记录到 [Milestone3Validation.md](./Validation/Milestone3Validation.md)。

验证：只按 [CurrentMilestone.md](./CurrentMilestone.md) 中 `M3-A01` 至 `M3-A10` 记录证据；未执行或失败的项目保持未验证。

验证记录（2026-07-27）：使用不入库的进程级密钥和 `kimi-k2.6` 完成人格、世界状态、历史三类真实 UE→Service→Kimi→UE 受控验收；三类场景均返回 `provider=kimi`、受控匹配为真且 request ID 一一对应。Service 重启后的无上下文请求返回 `200`，未出现重启前场景标记。最终 Python 离线测试 `92 passed`、`pip check`、字节码编译、`ZLEditor` 编译和完整 `ZLAIRuntime` 自动化 8/8 均通过；仓库、日志、错误响应和验收记录安全审计通过。`M3-A01` 至 `M3-A10` 全部验证，Milestone 3 已完成。

## 推荐执行顺序

```text
M3-01 -> M3-02 -> M3-03 -> M3-04 --+-> M3-05 --+
                                     |            +-> M3-07
                                     +-> M3-06 --+
```

`M3-05` 与 `M3-06` 在 Service/Provider 内部生成输入稳定后可分别推进；`M3-07` 必须等待两端实现和离线验证完成。

## 历史归档

| 范围 | 状态 | 范围定稿 | 验收记录 |
| --- | --- | --- | --- |
| Milestone 1：最小通信闭环 | `已完成` | [Milestone1.md](./Milestones/Milestone1.md) | [Milestone1Validation.md](./Validation/Milestone1Validation.md) |
| Milestone 2：真实 LLM 自由对话 | `已完成` | [Milestone2.md](./Milestones/Milestone2.md) | [Milestone2Validation.md](./Validation/Milestone2Validation.md) |
