# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-08-31 |
| 当前里程碑 | Milestone 7：可操作交互舞台与定向感知 |
| 里程碑状态 | `实施中` |
| 当前活动任务 | `M7-T02` 文本交互 UI 与输入边界 |
| 下一候选任务 | `M7-T03` Speech/Action Event 与个人 Observation |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 6：10/10 项已验证 |

## 当前能力基线

- Milestone 1 至 6 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- 当前已验证 UE→Python→Kimi Dialogue、瞬时 Context 和 SQLite 对话 Memory。
- 当前已完成纯 UE Event→Perception→State→Memory→Rule Decision→Gameplay Intent 闭环、单 Agent 调试命令和 120 Agent 聚合基准。
- 当前已完成有界 Event Chain、显式报告传播、稀疏 Relationship/Faction Authority、Important NPC Long Memory、关系感知规则决策、5 个 Important NPC 无界面纵向切片和 120+5 聚合基准。
- Milestone 6 保持现有 Dialogue 协议和 Python Runtime 行为不变；具体 Gameplay Intent 执行、Mass 正式集成、LLM Decision 与 ToolCall 尚未实现。
- Milestone 7 已按最终场景 1 定义为当前里程碑：交付玩家可操作平面场景、文本说话/行为输入、定向视觉、分级听觉、个人 Observation、气泡和开发视图；本阶段不修改 Dialogue 协议，也不接入 LLM Decision 或 NPC ToolCall。
