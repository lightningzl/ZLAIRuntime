# Milestone 4 Validation

## 状态

- 里程碑：Milestone 4：持久化对话 Memory
- 当前阶段：开发中
- 最后更新：2026-07-31
- 结论：部分通过（6/10）

本文件只记录实际执行过的验证和可复查证据。验收标准正文见 [CurrentMilestone.md](../CurrentMilestone.md)；未执行项目保持“未验证”。

## 环境记录

已完成 M4-02 至 M4-05 的协议、SQLite Repository、Memory Service、历史合并和 Dialogue Service 编排验证；维护入口、UE 完整集成和真实端到端验证尚未执行。

不得记录 API Key、完整 `scope_id`、完整玩家输入、完整持久化对话、数据库绝对路径、模型回复正文或可还原敏感数据的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `pytest` | 通过 | 154 passed；M1-M3 回归与 M4-02 至 M4-05 测试通过，M4-06 将在维护入口完成后重跑 |
| Python Memory 协议专项 | `pytest`：Memory/Context/API | 通过 | 53 passed；省略、边界、Unicode、空白、错误类型、未知字段和 Context 组合通过 |
| Python 依赖检查 | `pip check` | 通过 | No broken requirements found |
| Python 字节码编译 | `compileall` | 通过 | `app` 与 `tests` 编译通过；M4-06 将在全部 Python 实现完成后重跑 |
| SQLite Repository 专项 | `pytest`：Repository/Settings | 通过 | 32 passed；覆盖配置、Schema、索引、隔离、排序、幂等、回滚、重启、损坏路径和生命周期 |
| SQLite 初始化与重启 | 临时数据库 Repository 测试 | 通过 | 空库与重复初始化成功，关闭后重新打开可读取已提交轮次 |
| Memory Service 与 Builder 专项 | `pytest`：Memory/Builder/Repository | 通过 | 35 passed；覆盖二次隔离、预算、乱序恢复、最长精确边界去重、角色敏感、客户端优先、幂等写入、当前输入单次追加和真实临时库联调 |
| Dialogue Memory 编排专项 | `pytest`：编排/API/日志 | 通过 | 46 passed；覆盖无 Memory 零调用、读取→单次生成→成功写入、各失败阶段、空回复拒写、重复 ID、重启恢复、生命周期、错误映射和日志脱敏 |
| UE 编译 | `ZLEditor Win64 Development` | 通过 | M4-02 协议类型和四类兼容入口编译成功；完整最终编译待 M4-07 重跑 |
| UE 协议自动化 | `ZLAIRuntime.Protocol` | 通过 | 6/6；Memory 省略、单独/组合序列化、最大边界、空白和越界通过 |
| UE 完整自动化测试 | 未执行 | 未验证 | 待 `M4-07` 记录 |
| UE Game 本地 Memory 演示 | 未执行 | 未验证 | 待 `M4-07` 记录 |
| UE Game 真实 Kimi 验收 | 未执行 | 未验证 | 待 `M4-08` 记录 |
| 安全审计 | 未执行 | 未验证 | 待 `M4-08` 记录 |

## 验收证据

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M4-A01` | 未验证 | 待执行 |
| `M4-A02` | 通过 | Python 专项 53/53、UE 协议 6/6 和 `ZLEditor` 编译证明两端字段、可选语义、Unicode 边界及错误拒绝一致 |
| `M4-A03` | 通过 | 临时库测试证明 Schema v1、范围索引、重复初始化、重启读取和稳定已提交数据；忽略规则覆盖数据库、WAL、SHM、Journal、备份和临时文件 |
| `M4-A04` | 通过 | Repository 与 Memory Service 测试证明 `(scope_id, npc_id)` 双重隔离、最近轮次预算、稳定排序和完整轮次恢复 |
| `M4-A05` | 通过 | Memory Service 使用最长精确角色/内容边界重叠保留客户端快照；Context Builder 专项证明合并历史位于当前输入之前且当前输入只追加一次 |
| `M4-A06` | 通过 | Fake Memory/Provider 与真实临时库测试证明只在合法 Provider 成功后写入完整轮次；读取、Provider、空回复和写入失败不产生错误轮次，重复 ID 幂等 |
| `M4-A07` | 通过 | 应用依赖与源码扫描证明 Route→Dialogue Service→Memory Service→Repository 分层保持，SQL 仅在 SQLite Repository，Provider 未依赖 Memory、SQLite 或协议 Schema |
| `M4-A08` | 未验证 | 待执行 |
| `M4-A09` | 未验证 | 待执行 |
| `M4-A10` | 未验证 | 待执行 |

## 最终结论

Milestone 4 正在开发；Python Runtime Memory 主链路已通过，维护入口、UE 完整集成和真实端到端验收仍未完成。
