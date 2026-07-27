# Project State

本文件只记录当前项目快照。范围、任务明细、验收标准和流程规则分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [DocumentationRules.md](./DocumentationRules.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-07-27 |
| 当前里程碑 | Milestone 4：持久化对话 Memory |
| 里程碑状态 | `待开发` |
| 当前活动任务 | 无 |
| 下一候选任务 | `M4-02`：两端 Memory 协议类型与兼容入口 |
| 已知阻塞 | 无 |
| 验收进度 | 0/10 项已验证 |

## 实现基线

- Milestone 1 至 3 均已完成；最新历史范围和证据见 [Milestone3.md](./Milestones/Milestone3.md) 与 [Milestone3Validation.md](./Validation/Milestone3Validation.md)，更早记录由其历史链接追溯。
- 当前 UE→Python Service→Kimi 非流式链路、Provider 错误映射、瞬时 `context`、Context Builder 和本地 Stub/Fake 集成均已验证。
- 当前实现仍完全无状态，不包含 SQLite、Memory Service、Memory 协议类型或数据库运行时文件。
- Milestone 4 的可选 `memory.scope_id` 协议、SQLite Memory 边界、工作包和验收模板已经定稿。
- `M4-01` 已完成文档准备；尚未修改 UE/Python 运行时代码，`M4-A01` 至 `M4-A10` 均为未验证。
