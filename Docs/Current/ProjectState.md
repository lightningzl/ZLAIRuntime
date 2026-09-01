# Project State

本文件只记录当前项目快照。范围、任务明细和长期路线分别以 [CurrentMilestone.md](./CurrentMilestone.md)、[TaskBoard.md](./TaskBoard.md) 和 [SocialSimulationPlan.md](../Planning/SocialSimulationPlan.md) 为准。

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-01 |
| 当前里程碑 | Milestone 10：最终场景 1 多 NPC 交付 |
| 里程碑状态 | `进行中` |
| 当前活动任务 | `M10-T07`：自动化、构建与多 NPC 烟测 |
| 下一候选任务 | `M10-T08`：Kimi 验收、演示与里程碑收口 |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 9：10/10 项已验证 |

## 当前能力基线

- Milestone 1 至 9 已完成；范围与证据见 [Milestones](../Milestones/) 和 [Validation](../Validation/)。
- UE/Python 已具有真实 Kimi Dialogue、SQLite 对话 Memory、独立 Decision v1、Kimi/Stub Planner 和脱敏失败映射。
- 纯 UE 社会模拟已具有 Event、空间查询、个人感知、状态、关系、长期记忆、规则决策、5 Important NPC 无界面切片和 120+5 基准。
- 可操作社会沙盒稳定生成 4 个 NPC，并具有 Speech/Action 分离输入、定向视觉、分级听觉、逐 NPC Observation、气泡和个人 Inspector 基础。
- 当前只有 Guard 接入 LLM Decision、四个受控移动 Tool、连续调度、基础 Attack、生命/防卫/失能、冲突升级—缓和和本地失败降级。
- 当前场景 4 个 NPC 已固定为 Guard、Merchant、Rival 和 Civilian，并分别具有身份、人物、表达风格、目标、初始关系/状态和可见颜色；个人 Decision Context 已消费选定 NPC Profile。
- 当前已接入逐 NPC Scheduler、稳定轮转和全局最多 2 个 Decision 在途；实际听清 Speech 或看见已完成 Action 的 NPC 才进入自己的有界队列，回调按 NPC 与重置代次隔离。
- 当前逐 NPC 公开历史、冲突状态和本地失败已按稳定 ID 隔离；旁观者只有看见其他 NPC 已执行动作后才重新判断，未听清或未看见的 Event 不进入 Planner。
- 当前 Stub 已按 Guard、Merchant、Rival、Civilian Profile 与关系生成 4 种可复现表达；Kimi 固定约束要求保持当前 NPC 身份、人物和个人视角，并禁止推断其他 NPC 的感知与决定。
- 当前任意 NPC 已可独立接收和执行通过 UE 校验的四个 Tool，并产生可见移动/动作与个人公开历史；每个 NPC 的距离跨带、生命、防卫、冲突状态和 Inspector 结果彼此隔离。
- Milestone 9 最终基线为 Python 197/197、UE `ZL.Social` 27/27、受影响 UE Target 编译、Stub/离线烟测和默认地图真实 Kimi 验收通过。

## 当前执行边界

- 当前只实施 [CurrentMilestone.md](./CurrentMilestone.md) 定义的 4 NPC 最终场景交付与 `FS1-01` 至 `FS1-11` 验收。
- 沿用 2026-08-31 已确认的 Decision v1；不修改 `Protocol.md`，不新增 Tool、Intent、字段或批量请求。
- UE 继续权威维护逐 NPC 感知、关系输入、生命、冲突、状态版本、调度和动作执行；Python 每次只返回一个 NPC 的现有结构化建议。
- 按 `M10-T01` 至 `M10-T08` 的依赖顺序实施、验证和同步状态；每个工作包分步提交，完成后合并到 `main` 并统一推送。
