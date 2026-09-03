# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-03 |
| 当前里程碑 | Milestone 12：社会后果与角色行为打磨 |
| 里程碑状态 | `规划中` |
| 当前活动任务 | 无（范围与工作包已完成，等待实施启动） |
| 下一候选任务 | `M12-T02` 个人后果与关系更新 |
| 已知阻塞 | 无；若现有四个 Tool 不能完成经评审的可见链路，须先获得协议确认 |
| 最近验收 | Milestone 11：`M11-A01` 至 `M11-A12` 已归档 |

## 当前能力基线

- Milestone 1 至 11 已完成；M11 的配置驱动角色/NPC、角色输入、Notify 命中、NPC 击退与 ragdoll 范围已归档至 [Milestone11.md](../Milestones/Milestone11.md)。
- 社会沙盒稳定生成 4 个个人视角隔离的 NPC，具备 Speech/Action 分离输入、定向视觉、分级听觉、气泡、Inspector、逐 NPC 调度及有界公开历史。
- UE 已权威维护 NPC 生命、防卫、失能、冲突立场、受击私有 Observation 和既有 Relationship/Instant State；Kimi、Stub 和离线降级均使用单 NPC Decision v1。
- 当前体验缺口是已感知的攻击、缓和、报告和商人互动尚未稳定收敛为连续、角色差异化的社会后果；M12 将以此为唯一体验重点。

## 当前执行边界

- 沿用已确认的 Decision v1；不修改 `Protocol.md`，不新增 Tool、Intent、字段或批量请求。
- UE 继续权威维护逐 NPC 感知、社会后果、关系、状态版本、调度和动作执行；Python 每次只返回一个 NPC 的现有结构化建议。
- M12 的最小交易入口仅验证互动立场；不建设货币、库存、物品、结算或经济系统。
- 当前仅完成规划文档，尚未开始 M12 实现、自动化或真实模型验收。
