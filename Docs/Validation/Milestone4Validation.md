# Milestone 4 Validation

## 状态

- 里程碑：Milestone 4：持久化对话 Memory
- 当前阶段：已完成
- 最后更新：2026-08-03
- 结论：通过（10/10）

本文件只记录实际执行过的验证和可复查证据。历史验收标准正文见 [Milestone4.md](../Milestones/Milestone4.md)；未执行项目保持“未验证”。

## 环境记录

M4-02 至 M4-08 的 Python/UE 实现、离线回归、本地 Stub 集成和真实 Kimi 端到端验收均已完成。

不得记录 API Key、完整 `scope_id`、完整玩家输入、完整持久化对话、数据库绝对路径、模型回复正文或可还原敏感数据的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `pytest` | 通过 | 166 passed；M1-M3 回归与 M4-02 至 M4-06 全部 Python 路径通过 |
| Python Memory 协议专项 | `pytest`：Memory/Context/API | 通过 | 53 passed；省略、边界、Unicode、空白、错误类型、未知字段和 Context 组合通过 |
| Python 依赖检查 | `pip check` | 通过 | No broken requirements found |
| Python 字节码编译 | `compileall` | 通过 | `app` 与 `tests` 最终编译通过 |
| SQLite Repository 专项 | `pytest`：Repository/Settings | 通过 | 32 passed；覆盖配置、Schema、索引、隔离、排序、幂等、回滚、重启、损坏路径和生命周期 |
| SQLite 初始化与重启 | 临时数据库 Repository 测试 | 通过 | 空库与重复初始化成功，关闭后重新打开可读取已提交轮次 |
| Memory Service 与 Builder 专项 | `pytest`：Memory/Builder/Repository | 通过 | 35 passed；覆盖二次隔离、预算、乱序恢复、最长精确边界去重、角色敏感、客户端优先、幂等写入、当前输入单次追加和真实临时库联调 |
| Dialogue Memory 编排专项 | `pytest`：编排/API/日志 | 通过 | 46 passed；覆盖无 Memory 零调用、读取→单次生成→成功写入、各失败阶段、空回复拒写、重复 ID、重启恢复、生命周期、错误映射和日志脱敏 |
| Repository/Settings/Maintenance 专项 | `pytest`：配置、持久层、维护入口 | 通过 | 44 passed；统计无标识、精确清理、显式确认、幂等、SQL 风格数据安全、错误脱敏和非 HTTP 边界通过 |
| Memory 维护命令 | `python -m app.memory.maintenance --help` | 通过 | 本机 `stats`/`clear` 入口可用；清理要求 `--confirm` |
| UE 编译 | `ZLEditor Win64 Development` | 通过 | M4-07 增量编译 9 个 Action 成功，Memory 演示与扩展集成测试源文件均参与编译和链接 |
| UE 协议自动化 | `ZLAIRuntime.Protocol` | 通过 | 6/6；Memory 省略、单独/组合序列化、最大边界、空白和越界通过 |
| UE 完整自动化测试 | `Automation RunTests ZLAIRuntime` | 通过 | 9/9；旧请求、Context、协议、失败分类及新增 Memory 本地集成全部成功，退出码 0 |
| UE Stub Memory 集成 | 本地 Stub + 临时 SQLite | 通过 | 连续 Memory、不同 scope、不同 NPC 和非法 Memory 单次回调通过；聚合统计为 4 轮、2 个 scope、3 个 scope/NPC 分区 |
| UE Game 本地 Memory 演示 | `ZL.AI.DialogueMemoryDemo seed` | 通过 | 无界面 Game 收到一次 Stub 成功回调；只记录脱敏元数据，固定 Stub 不承担语义标记判断 |
| UE Game 真实 Kimi 验收 | `ZL.AI.DialogueMemoryDemo` 四个受控场景 | 通过 | seed 命中；Service 重启后 recall 命中；不同 scope 与不同 NPC 均未命中；所有响应 Provider 为 `kimi` |
| Python 安全审计 | 源码、Git 跟踪、日志与自动化扫描 | 通过 | 无数据库文件入库；SQL 仅在 Repository；维护入口非 HTTP；自动化屏蔽真实 Key 和非本机网络，响应与日志脱敏 |

## 验收证据

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M4-A01` | 通过 | Python 完整测试 166/166 与 UE 完整自动化 9/9 共同证明 M1-M3 成功、错误、超时、Context、单次回调及无 Memory 兼容路径未回归 |
| `M4-A02` | 通过 | Python 专项 53/53、UE 协议 6/6 和 `ZLEditor` 编译证明两端字段、可选语义、Unicode 边界及错误拒绝一致 |
| `M4-A03` | 通过 | 临时库测试证明 Schema v1、范围索引、重复初始化、重启读取和稳定已提交数据；忽略规则覆盖数据库、WAL、SHM、Journal、备份和临时文件 |
| `M4-A04` | 通过 | Repository 与 Memory Service 测试证明 `(scope_id, npc_id)` 双重隔离、最近轮次预算、稳定排序和完整轮次恢复 |
| `M4-A05` | 通过 | Memory Service 使用最长精确角色/内容边界重叠保留客户端快照；Context Builder 专项证明合并历史位于当前输入之前且当前输入只追加一次 |
| `M4-A06` | 通过 | Fake Memory/Provider 与真实临时库测试证明只在合法 Provider 成功后写入完整轮次；读取、Provider、空回复和写入失败不产生错误轮次，重复 ID 幂等 |
| `M4-A07` | 通过 | 应用依赖与源码扫描证明 Route→Dialogue Service→Memory Service→Repository 分层保持，SQL 仅在 SQLite Repository，Provider 未依赖 Memory、SQLite 或协议 Schema |
| `M4-A08` | 通过 | 166 项离线自动化覆盖 Schema、初始化、事务、排序、预算、隔离、合并、幂等、回滚、维护入口和日志脱敏；测试无真实 Key、外网或 Token |
| `M4-A09` | 通过 | UE 旧/新公开入口编译通过；本地 Stub 集成覆盖连续 Memory、scope/NPC 隔离、非法范围不发 HTTP 和所有回调恰好一次；无界面 Game 演示收到成功回调 |
| `M4-A10` | 通过 | 真实 Kimi seed 与重启后 recall 均命中固定虚构标记，不同 scope/NPC 均未命中；最终临时库聚合为 4 轮、2 个 scope、3 个分区，Git 与日志审计无密钥、数据库、完整对话或原始回复 |

## 最终结论

Milestone 4 已完成。`M4-A01` 至 `M4-A10` 全部通过；持久化 Memory、无状态兼容、双重隔离、确定性合并、事务写入、维护入口、UE 集成和真实 Kimi 重启恢复均有可复查的脱敏证据。
