# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-03 |
| 当前里程碑 | Milestone 12：社会后果与角色行为打磨 |
| 里程碑状态 | `验收中` |
| 当前活动任务 | `M12-T06` 集成回归、场景验收与文档收口 |
| 下一候选任务 | 无 |
| 已知阻塞 | 无；无界面 UE 自动化尚未取得完成标记，不能作为通过证据 |
| 最近验收 | Milestone 11：`M11-A01` 至 `M11-A12` 已归档 |

## 当前能力基线

- Milestone 1 至 11 已完成；M11 的配置驱动角色/NPC、角色输入、Notify 命中、NPC 击退与 ragdoll 范围已归档至 [Milestone11.md](../Milestones/Milestone11.md)。
- 社会沙盒稳定生成 4 个个人视角隔离的 NPC，具备 Speech/Action 分离输入、定向视觉、分级听觉、气泡、Inspector、逐 NPC 调度及有界公开历史。
- UE 现已为非 Guard NPC 维护容量 12 的个人社会事实、重复受击、道歉、交易尝试、实际执行动作与已确认报告；这些事实只进入该 NPC 的 Decision v2 Context。交易入口只返回 UE 权威互动立场，不创建经济系统。
- `/v2/decision` 已由 Python Schema/Route/Service、Kimi/Stub Planner 与 UE Client/沙盒计划执行器实现；Kimi 只从请求的有限能力实例中选择计划步骤，UE 二次校验并只展示实际 Speech、立场和动作结果。`/v1/decision` 保持兼容。

## 当前执行边界

- `/v1/decision` 保持现状；`/v2/decision` 以开放高层计划、`social_situation` 与请求级有限能力实例承载角色判断。
- UE 继续权威维护逐 NPC 感知、社会后果、关系、状态版本、调度和动作执行；Python 每次只返回一个 NPC 的现有结构化建议。
- M12 的最小交易入口仅验证互动立场；不建设货币、库存、物品、结算或经济系统。
- M12 已完成 Python 情境回归、真实 Kimi 抽样和 UE Development 编译；最终无界面自动化/地图证据仍在收集，详见 [Milestone12Validation.md](../Validation/Milestone12Validation.md)。
