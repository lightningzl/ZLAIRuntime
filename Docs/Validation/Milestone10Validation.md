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
- 2026-09-01：Python 全量回归 198/198 通过，用时 1.92 秒。
- 2026-09-01：NullRHI + 内存 DDC 执行 `Automation RunTests ZL.Social`，收集 28 项，28/28 `Success`；包含既有社会模拟、感知、冲突、Tool、个人 Context 和新增多 NPC 并发边界。
- 2026-09-01：本机隔离 Stub Service 在线时执行 `Automation RunTests ZLAIRuntime`，收集 13 项，13/13 `Success`；Dialogue/Decision 协议、Client、失败分类和真实 HTTP 回调保持通过。
- 2026-09-01：最终 `ZLEditor Win64 Development` 编译成功；UHT、ZLASocialRuntime、ZLAIRuntime、ZL 和新增烟测入口完成链接。
- 2026-09-01：默认地图运行 `-ZLSandboxMultiNpcSmoke`，结果 `Success`；生成 4 个稳定 NPC，4/4 接受 `provider=stub` Speech，采样时在途 0 且全程由硬上限 2 约束。
- 2026-09-01：本地 Service 离线且限帧运行 `-ZLSandboxDecisionFallbackSmoke`，结果 `Success`；约 2.02 秒获得 `network_error`，Guard 显示 `provider=local`，推进防卫状态版本且位置不变。

后续只在实际执行验证后补充结果；未执行项目不得写为通过。
