# Milestone 4 Validation

## 状态

- 里程碑：Milestone 4：持久化对话 Memory
- 当前阶段：开发中
- 最后更新：2026-07-31
- 结论：部分通过（1/10）

本文件只记录实际执行过的验证和可复查证据。验收标准正文见 [CurrentMilestone.md](../CurrentMilestone.md)；未执行项目保持“未验证”。

## 环境记录

已完成 M4-02 协议类型与兼容入口验证；SQLite、Memory 编排和端到端验证尚未执行。

不得记录 API Key、完整 `scope_id`、完整玩家输入、完整持久化对话、数据库绝对路径、模型回复正文或可还原敏感数据的片段。

## 自动化验证

| 验证 | 命令或入口 | 结果 | 证据摘要 |
| --- | --- | --- | --- |
| Python 完整测试 | `pytest`（无缓存） | 通过 | 105 passed；M1-M3 回归与 M4-02 协议测试通过，M4-06 将在全部 Python 实现完成后重跑 |
| Python Memory 协议专项 | `pytest`：Memory/Context/API | 通过 | 53 passed；省略、边界、Unicode、空白、错误类型、未知字段和 Context 组合通过 |
| Python 依赖检查 | `pip check` | 通过 | No broken requirements found |
| Python 字节码编译 | 未执行 | 未验证 | 待 `M4-06` 记录 |
| SQLite 初始化与重启 | 未执行 | 未验证 | 待 `M4-06` 记录 |
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
| `M4-A03` | 未验证 | 待执行 |
| `M4-A04` | 未验证 | 待执行 |
| `M4-A05` | 未验证 | 待执行 |
| `M4-A06` | 未验证 | 待执行 |
| `M4-A07` | 未验证 | 待执行 |
| `M4-A08` | 未验证 | 待执行 |
| `M4-A09` | 未验证 | 待执行 |
| `M4-A10` | 未验证 | 待执行 |

## 最终结论

Milestone 4 尚未开始实现，当前无验收结论。
