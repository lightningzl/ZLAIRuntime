# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-08-31 |
| 当前里程碑 | 无（Milestone 6 已归档） |
| 里程碑状态 | `未选择` |
| 当前活动任务 | 无 |
| 下一候选任务 | Milestone 7“可操作交互舞台与定向感知”范围定义（不修改 Dialogue 协议） |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 6：10/10 项已验证 |

## 当前能力基线

- Milestone 1 至 6 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- 当前已验证 UE→Python→Kimi Dialogue、瞬时 Context 和 SQLite 对话 Memory。
- 当前已完成纯 UE Event→Perception→State→Memory→Rule Decision→Gameplay Intent 闭环、单 Agent 调试命令和 120 Agent 聚合基准。
- 当前已完成有界 Event Chain、显式报告传播、稀疏 Relationship/Faction Authority、Important NPC Long Memory、关系感知规则决策、5 个 Important NPC 无界面纵向切片和 120+5 聚合基准。
- Milestone 6 保持现有 Dialogue 协议和 Python Runtime 行为不变；具体 Gameplay Intent 执行、Mass 正式集成、LLM Decision 与 ToolCall 尚未实现。
- 后续路线已按最终场景 1 重排：先交付可操作舞台与感知，再接入单 NPC LLM 具身反馈、连续冲突变化和 3 至 5 NPC 场景验收；当前仍未选择活动里程碑。
