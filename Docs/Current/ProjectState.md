# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-08-31 |
| 当前里程碑 | Milestone 8：单 NPC LLM 具身反馈与受控动作 |
| 里程碑状态 | `受阻（等待协议确认）` |
| 当前活动任务 | 无 |
| 下一候选任务 | `M8-T02` Decision 协议确认与契约基线 |
| 已知阻塞 | `M8-T02` 及后续实现等待用户明确确认 Decision 协议方案 |
| 最近验收 | Milestone 7：10/10 项已验证 |

## 当前能力基线

- Milestone 1 至 7 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- 当前已验证 UE→Python→Kimi Dialogue、瞬时 Context 和 SQLite 对话 Memory。
- 当前已完成纯 UE Event→Perception→State→Memory→Rule Decision→Gameplay Intent 闭环、单 Agent 调试命令和 120 Agent 聚合基准。
- 当前已完成有界 Event Chain、显式报告传播、稀疏 Relationship/Faction Authority、Important NPC Long Memory、关系感知规则决策、5 个 Important NPC 无界面纵向切片和 120+5 聚合基准。
- Milestone 7 已交付专用可操作平面场景、文本说话/行为 UI、Speech/Action 分离事件、定向视觉、分级听觉、逐 NPC Observation、受控玩家行为、气泡和开发视图。
- 当前协议仍只有 `/v1/dialogue` 纯文本路径；LLM Decision、结构化 Tool 建议、NPC Tool Executor、过期拒绝和 Python 离线具身降级尚未实现。

## 当前执行边界

- 已进入 Milestone 8 文档整理阶段，允许完善范围、验收、任务依赖和协议方案。
- 在用户明确确认 Decision 协议前，不修改 `Protocol.md`，不实现未经确认的网络字段，不从 Dialogue 文本驱动 Gameplay。
- 协议确认后，按 `M8-T02` 至 `M8-T08` 依赖顺序实施、验证和同步状态。
