# Project State

本文件只记录当前项目快照。范围、任务明细、验收标准和流程规则分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [DocumentationRules.md](./DocumentationRules.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-07-31 |
| 当前里程碑 | Milestone 4：持久化对话 Memory |
| 里程碑状态 | `开发中` |
| 当前活动任务 | 无 |
| 下一候选任务 | `M4-03`：SQLite Repository 与配置 |
| 已知阻塞 | 无 |
| 验收进度 | 1/10 项已验证 |

## 实现基线

- Milestone 1 至 3 均已完成；最新历史范围和证据见 [Milestone3.md](./Milestones/Milestone3.md) 与 [Milestone3Validation.md](./Validation/Milestone3Validation.md)，更早记录由其历史链接追溯。
- 当前 UE→Python Service→Kimi 非流式链路、Provider 错误映射、瞬时 `context`、Context Builder 和本地 Stub/Fake 集成均已验证。
- 当前实现仍未读写 SQLite，但已包含两端 Memory 协议类型、校验、序列化和兼容请求入口。
- Milestone 4 的可选 `memory.scope_id` 协议、SQLite Memory 边界、工作包和验收模板已经定稿。
- `M4-02` 已完成：Python/UE 的可选 `memory.scope_id` 类型、边界和省略兼容已实现；`M4-A02` 已验证。
