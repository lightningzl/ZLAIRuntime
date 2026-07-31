# Task Board

## 用途

本文件将 [CurrentMilestone.md](./CurrentMilestone.md) 中的当前里程碑拆成可执行工作包，并记录任务状态、依赖和完成条件。它不改变里程碑范围，也不替代协议、架构或验收标准。

当前执行队列只维护 Milestone 4。Milestone 1 至 3 已完成并归档；后续 Milestone 不得提前加入执行队列。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| `待开始` | 范围和前置条件明确，但尚未实施 |
| `进行中` | 已开始实施；同一任务只能有一个主要执行者 |
| `受阻` | 无法继续，且已记录阻塞原因和解除条件 |
| `待验收` | 实施完成，正在执行或等待规定验证 |
| `已完成` | 产物、适用验证和相关文档均已完成 |

任务只有在完成表中“完成条件”并留下可复查的验证记录后，才能标记为 `已完成`。未执行的验证必须写为“未验证”，不能根据代码存在推定通过。

## Milestone 4 工作包

| ID | 状态 | 工作包 | 主要产物 | 依赖 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M4-01` | `已完成` | 里程碑范围、协议与开发准备定稿 | M3 归档、M4 范围、v1 Memory 协议、架构、任务板、状态、模块、决策和验收模板 | Milestone 3 | 用户确认协议扩展；开发前置文档职责清晰，链接和术语一致 |
| `M4-02` | `已完成` | 两端 Memory 协议类型与兼容入口 | Python Schema、UE Memory 结构体、序列化、校验和兼容测试 | `M4-01` | 无 Memory 请求不变；合法范围可往返；非法结构和边界按协议拒绝 |
| `M4-03` | `待开始` | SQLite Repository 与配置 | Schema 初始化、索引、连接/事务、数据库路径和检索预算配置、临时库测试 | `M4-01` | 初始化和重启可重复；数据库不入库；Repository 外无 SQL |
| `M4-04` | `待开始` | Memory Service 与历史合并 | Memory 内部类型、隔离检索、稳定排序、预算、重叠消除和幂等写入 | `M4-02`、`M4-03` | 满足隔离、顺序、预算、成功后写入、重复请求和失败回滚约束 |
| `M4-05` | `待开始` | Dialogue Service 与 Context Builder 接线 | 读取→合并→生成→成功写入编排、错误映射和脱敏日志 | `M4-04` | 无 Memory 路径零读写；Provider 边界不变；数据库错误安全失败 |
| `M4-06` | `待开始` | Python 离线回归与维护入口 | 完整自动化、本地统计/清理命令、模块文档和安全验证 | `M4-05` | 为 `M4-A01` 至 `M4-A08` 提供可复查的离线证据 |
| `M4-07` | `待开始` | UE Memory 演示与集成验证 | 兼容公开入口、脱敏演示、本地 Stub 集成、自动化和 UE 模块文档 | `M4-02`、`M4-05` | 旧/新入口、隔离、非法范围和单次回调通过；受影响 Target 编译成功 |
| `M4-08` | `待开始` | 真实端到端验收与交付记录 | 真实 Kimi 连续对话、重启恢复、范围隔离、安全审计和 M4 验收记录 | `M4-06`、`M4-07` | `M4-A01` 至 `M4-A10` 均有可复查证据 |

## 工作包明细

### M4-01：里程碑范围、协议与开发准备定稿

- 将完成时的 Milestone 3 范围定稿归档到 [Milestone3.md](./Milestones/Milestone3.md)。
- 将 `CurrentMilestone.md` 切换到 Milestone 4，定义范围、明确不做、验收标准和完成定义。
- 经用户确认，在 v1 请求中增加可选 `memory.scope_id`，响应和 Endpoint 保持不变。
- 定义 `(scope_id, npc_id)` 隔离、成功后写入、无 Memory 兼容、SQLite Repository 和 Context Builder 合并边界。
- 同步架构、模块、决策、项目状态和验收模板。

验证记录（2026-07-27）：用户确认 v1 `memory` 协议方案；Milestone 3 已归档，Milestone 4 开发准备文档已完成。仅修改文档，未修改运行时代码，代码测试未执行。

### M4-02：两端 Memory 协议类型与兼容入口

- Python Schema 增加只包含 `scope_id` 的可选 Memory 对象。
- UE 插件增加对应 `USTRUCT` 和明确的“未提供/已提供”表示。
- 两端实现一致的 Unicode 长度和首尾空白校验。
- 请求省略 Memory 时完全不发送该字段，不发送 `null` 或空对象。
- 保留旧请求入口、成功响应、错误响应和未知字段兼容行为。

验证：两端覆盖省略、最小/最大边界、空白、越界、缺字段、错误类型、未知字段和完整上下文组合。

验证记录（2026-07-31）：Python 新增可选 `DialogueMemory`、显式 `null` 拒绝、Unicode 边界和空白业务校验；UE 新增 `FZLDialogueMemory`、`bHasMemory`、Memory/Context 兼容重载及 JSON 校验/序列化。Python 完整离线测试 105/105、`pip check`、`ZLEditor Win64 Development` 编译和 `ZLAIRuntime.Protocol` 6/6 均通过；`M4-A02` 已验证。

### M4-03：SQLite Repository 与配置

- 新增只负责 SQLite 的 Repository 和持久化内部类型。
- 使用显式 Schema 版本、建表和索引初始化；重复启动不得破坏已有数据。
- 以 `request_id` 唯一约束防止重复轮次，以 `(scope_id, npc_id, sequence)` 支持隔离和稳定检索。
- 所有 SQL、连接、提交、回滚和行映射限制在 Repository。
- 数据库路径与最大检索轮数通过 Python Settings 注入；测试使用临时目录。
- 更新忽略规则，确保数据库、WAL、SHM、备份和临时文件不进入 Git。

验证：空库初始化、重复初始化、重启读取、排序、索引、唯一约束、事务回滚、损坏/不可写路径和关闭生命周期。

### M4-04：Memory Service 与历史合并

- 新增与 SQLite 解耦的 Memory Service 接口和对话轮次内部类型。
- 严格按 `(scope_id, npc_id)` 检索最近的有界已完成轮次，再恢复为最旧到最新。
- 将持久化消息放在客户端瞬时历史之前。
- 对持久化尾部与客户端历史前部执行精确角色/内容重叠消除，客户端历史优先。
- 保证当前输入只追加一次，Memory 内容仍作为不可信数据。
- Provider 成功后以唯一 `request_id` 写入完整轮次；重复写入为幂等，失败事务不留半轮。

验证：空 Memory、连续轮次、预算截断、四类范围隔离、稳定排序、完整/部分/无重叠、同文非重叠、重复 ID 和回滚。

### M4-05：Dialogue Service 与 Context Builder 接线

- Dialogue Service 在 Memory 启用时执行检索、合并、单次 Provider 生成和成功写入。
- 无 Memory 请求不创建数据库查询或写入，保持现有无状态路径。
- Context Builder 接收已合并的供应商无关历史，不依赖 SQLite 或 Repository。
- Provider 接口和 Kimi/Stub/Fake 实现继续只接收生成上下文。
- 数据库异常映射为脱敏 `500 internal_error`；不得包含 SQL、数据库路径或对话正文。
- 日志只记录 request/NPC ID、Memory 是否启用、检索轮数、写入结果和错误分类，不记录 scope ID 或对话正文。

验证：Fake Memory/Provider 覆盖调用顺序、无 Memory 零调用、成功后写入、各失败阶段、单次 Provider 调用和日志脱敏。

### M4-06：Python 离线回归与维护入口

- 补齐 Schema、Repository、Memory Service、Builder、Dialogue Service、API 和日志测试。
- 全局继续移除真实密钥并拦截非本机网络。
- 提供只输出脱敏统计的本地检查入口，以及按精确 scope/NPC 清理的维护入口。
- 维护入口不得成为 HTTP Runtime API，不得输出完整对话。
- 执行完整 Python 测试、依赖检查和字节码编译。
- 更新 `PythonModules.md` 与 Python Service README。

验证：为 `M4-A01` 至 `M4-A08` 留下自动化证据；数据库文件只存在于临时或忽略目录。

### M4-07：UE Memory 演示与集成验证

- 保留既有无 Memory 请求重载，增加显式 Memory 范围入口并共享发送/完成逻辑。
- Gameplay/UI 提供稳定 scope，插件不读取账号、SaveGame、World、Actor 或平台身份。
- 增加固定脱敏演示，覆盖同范围连续对话、不同 scope 隔离、不同 NPC 隔离和重启恢复。
- 协议测试覆盖省略、合法边界、非法内容和与完整 `context` 的组合。
- 本地 Stub Service 集成验证旧/新入口、成功/失败、非法范围不发 HTTP 和单次回调。
- 编译受影响 UE Target，执行完整 `ZLAIRuntime` 自动化和无界面 Game 演示。

验证：为 `M4-A09` 留下编译、自动化和运行证据；不扩展为正式 Gameplay/UI 系统。

### M4-08：真实端到端验收与交付记录

- 使用有效但不入库的 Kimi 配置执行受控连续对话。
- 证明同一 `(scope_id, npc_id)` 的后续请求可利用已保存轮次。
- 重启 Service 后再次请求，证明 SQLite 数据可恢复。
- 分别改变 scope 和 NPC，证明不存在跨范围串线。
- 审计 Git、数据库忽略、日志、错误响应和验收记录。
- 执行 Python 完整测试、依赖检查、UE 编译和完整自动化。
- 将实际证据记录到 [Milestone4Validation.md](./Validation/Milestone4Validation.md)。

验证：只按 [CurrentMilestone.md](./CurrentMilestone.md) 中 `M4-A01` 至 `M4-A10` 记录证据；未执行或失败的项目保持未验证。

## 推荐执行顺序

```text
M4-01 -> M4-02 --+
                  +-> M4-04 -> M4-05 --+-> M4-06 --+
M4-01 -> M4-03 --+                     |            +-> M4-08
                                        +-> M4-07 --+
```

`M4-02` 与 `M4-03` 可以并行；`M4-06` 与 `M4-07` 在 Service 编排稳定后可分别推进；`M4-08` 必须等待两端实现和离线验证完成。

## 历史归档

| 范围 | 状态 | 范围定稿 | 验收记录 |
| --- | --- | --- | --- |
| Milestone 1：最小通信闭环 | `已完成` | [Milestone1.md](./Milestones/Milestone1.md) | [Milestone1Validation.md](./Validation/Milestone1Validation.md) |
| Milestone 2：真实 LLM 自由对话 | `已完成` | [Milestone2.md](./Milestones/Milestone2.md) | [Milestone2Validation.md](./Validation/Milestone2Validation.md) |
| Milestone 3：NPC 上下文与人格 | `已完成` | [Milestone3.md](./Milestones/Milestone3.md) | [Milestone3Validation.md](./Validation/Milestone3Validation.md) |
