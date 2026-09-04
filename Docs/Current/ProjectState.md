# Project State

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-04 |
| 当前里程碑 | Milestone 13：NPC 内容配置、批量管理与场景投放 |
| 里程碑状态 | `已完成` |
| 当前活动任务 | 无 |
| 下一候选任务 | Milestone 14：个人世界认知与动态背景反馈（未启动） |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 13：NPC 内容配置、批量管理与场景投放已完成 |

## 当前能力基线

- Milestone 1 至 12 已完成；M12 的个人社会后果、Decision v2、最小交易立场和 Kimi/Stub/UE 回归见 [Milestone12.md](../Milestones/Milestone12.md)。
- Persona Asset/Row、单条/批量 JSON、受限 Registry 查询、Asset/ID 双路径 Spawner 已完成；转换后的 Persona 复用现有 NPC Profile，因此身份、人物、目标和初始状态已进入 Inspector 与 Decision Context，运行时个人社会事实继续按 NPC 隔离。
- 当前世界背景只能间接写入 NPC Profile 字段；尚未存在场景级世界规则配置、动态世界事实或逐 NPC 世界认知。

## 当前执行边界

- M13 先建设静态 NPC Persona 与场景投放管线，不实现动态世界认知；后者已规划为 M14。
- M13 不修改 `/v1/decision` 或 `/v2/decision`。任何新协议字段必须先获得用户确认。
- 当前已实现架构仍以 [Architecture.md](../Planning/Architecture.md) 为准；M13/M14 的目标设计不应写成已实现能力。
