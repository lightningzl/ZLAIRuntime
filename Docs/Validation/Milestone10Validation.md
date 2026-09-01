# Milestone 10 Validation

## 文档职责

本文档只记录 Milestone 10 已实际执行的验证环境、命令、结果和验收证据。范围与验收标准见 [CurrentMilestone.md](../Current/CurrentMilestone.md)，任务状态见 [TaskBoard.md](../Current/TaskBoard.md)。

## 当前状态

- 状态：`进行中`
- 已完成：范围、协议边界、工作包与 `FS1-01` 至 `FS1-11` 验收映射审查
- 待记录：实现提交、Python/UE 自动化、构建、Stub/失败/性能场景、真实 Kimi 与现场演示证据

## 文档基线

- 2026-09-01：确认 Milestone 9 已归档，工作区开始时无未提交改动。
- 2026-09-01：确认 Milestone 10 沿用 Decision v1，不修改 `Protocol.md` 或两端协议类型。
- 2026-09-01：确认固定交付 Guard、Merchant、Rival、Civilian 4 个 NPC；逐 NPC 请求和全局最多 2 个并发均位于 UE 编排边界。
- 2026-09-01：`ZLEditor Win64 Development` 编译成功；4 NPC Profile、NPC 持有配置和个人 Context 接入通过 UHT、编译与链接。
- 2026-09-01：NullRHI 执行 `ZL.Social.Sandbox.PersonalDecisionContext`，收集 1 项并通过；验证 Rival/Guard 人物与关系不同、Observer 隔离和协议请求仍合法。
- 2026-09-01：`ZLEditor Win64 Development` 在多 NPC Scheduler、GameMode 分派和回调关联接入后编译成功。
- 2026-09-01：NullRHI 执行 `ZL.Social.Sandbox.MultiNpcDecisionBounds`，收集 1 项并通过；验证 4 NPC 注册上限、逐 NPC Pending 合并、稳定轮转、全局并发 2 和 Reset 清理。
- 2026-09-01：`ZLEditor Win64 Development` 在个人 Speech/Action 路由、旁观动作触发、逐 NPC 公开历史和冲突状态隔离接入后编译成功；全量场景回归留待 `M10-T07` 统一执行。
- 2026-09-01：Python 定向执行 Stub、Decision Context Builder 和 Kimi Planner 相关测试，12/12 通过；包含 4 NPC 相同输入产生 4 种稳定表达、Rival/Civilian Confidence 差异和 Kimi 单 NPC/旁观者约束断言。
- 2026-09-01：`ZLEditor Win64 Development` 在任意 NPC Tool Registry/Handler、动作结果、个人执行窗口、距离跨带和通用 Inspector 接入后编译成功；完整运行时回归与烟测进入 `M10-T07`。

后续只在实际执行验证后补充结果；未执行项目不得写为通过。
