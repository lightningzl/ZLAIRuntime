# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-01 |
| 当前里程碑 | Milestone 9：连续互动、冲突升级与缓和 |
| 里程碑状态 | `进行中` |
| 当前活动任务 | `M9-T05` Python 连续 Planner 行为 |
| 下一候选任务 | `M9-T04` 升级、缓和与本地安全行为 |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 8：10/10 项已验证 |

## 当前能力基线

- Milestone 1 至 8 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- 当前已验证 UE→Python→Kimi Dialogue、瞬时 Context 和 SQLite 对话 Memory。
- 当前已完成纯 UE Event→Perception→State→Memory→Rule Decision→Gameplay Intent 闭环、单 Agent 调试命令和 120 Agent 聚合基准。
- 当前已完成有界 Event Chain、显式报告传播、稀疏 Relationship/Faction Authority、Important NPC Long Memory、关系感知规则决策、5 个 Important NPC 无界面纵向切片和 120+5 聚合基准。
- Milestone 7 已交付专用可操作平面场景、文本说话/行为 UI、Speech/Action 分离事件、定向视觉、分级听觉、逐 NPC Observation、受控玩家行为、气泡和开发视图。
- Milestone 8 已交付 UE `/v1/decision` 异步 Client、个人上下文、单 Guard Decision、四个固定 Tool 的 Registry/Handler、真实移动、过期拒绝和可见本地降级。
- 当前单 Guard Decision 已支持新 Speech、玩家已完成 Action、距离跨阈值和计划完成触发；调度固定一个在途、一个最新 Pending、0.75 秒冷却和最多 3 次连续自动重规划，并将 Guard 自己的公开 Speech/真实 Action Result 合并进有界个人历史。
- 当前已实现玩家 Attack 白名单、目标/距离/冷却校验、Guard 生命、受击无敌、防卫减伤、失能、状态版本推进和可见生命/受击反馈；这些即时结果由 UE 权威处理并反馈为个人 Action Observation。
- 尚未实现冲突等级、规则安全应对、升级—缓和策略与 Planner 的连续行为。
- 最终 Milestone 8 证据：Python 194/194、UE Social 24/24、UE AI Runtime 13/13、ZLEditor Win64 Development 编译均通过；Stub、状态失效、服务离线和真实 Kimi 默认地图纵向链路通过。

## 当前执行边界

- 当前只实施 [CurrentMilestone.md](./CurrentMilestone.md) 定义的单 Guard 连续互动、最小冲突 Gameplay 和升级—缓和闭环。
- 沿用 2026-08-31 已确认的 Decision v1；本里程碑不修改 `Protocol.md`，不新增 Tool、Intent 或字段。
- UE 继续权威维护生命、命中、伤害、防卫、冲突等级、状态版本和动作结果；Python 只返回现有结构化表达和高层建议。
- 按 `M9-T01` 至 `M9-T08` 的依赖顺序实施、验证和同步状态；工作包分步提交到里程碑集成分支，完整验证后统一合并和推送。
