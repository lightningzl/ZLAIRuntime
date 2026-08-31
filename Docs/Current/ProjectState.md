# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-08-31 |
| 当前里程碑 | 无；Milestone 7 已完成并归档 |
| 里程碑状态 | `无活动里程碑` |
| 当前活动任务 | 无 |
| 下一候选任务 | Milestone 8 范围定义（需要用户确认协议与 ToolCall 边界） |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 7：10/10 项已验证 |

## 当前能力基线

- Milestone 1 至 7 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- 当前已验证 UE→Python→Kimi Dialogue、瞬时 Context 和 SQLite 对话 Memory。
- 当前已完成纯 UE Event→Perception→State→Memory→Rule Decision→Gameplay Intent 闭环、单 Agent 调试命令和 120 Agent 聚合基准。
- 当前已完成有界 Event Chain、显式报告传播、稀疏 Relationship/Faction Authority、Important NPC Long Memory、关系感知规则决策、5 个 Important NPC 无界面纵向切片和 120+5 聚合基准。
- Milestone 7 已交付专用可操作平面场景、文本说话/行为 UI、Speech/Action 分离事件、定向视觉、分级听觉、逐 NPC Observation、受控玩家行为、气泡和开发视图。
- Milestone 7 保持现有 Dialogue 协议和 Python Runtime 行为不变；LLM Decision、NPC ToolCall、复杂导航、战斗与 Mass 正式集成尚未实现。
