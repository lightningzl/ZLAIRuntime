# Project State

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-04 |
| 当前里程碑 | Milestone 13：NPC 内容配置、批量管理与场景投放 |
| 里程碑状态 | `规划中` |
| 当前活动任务 | 无（等待启动实施） |
| 下一候选任务 | `M13-T01` Persona 数据模型与插件 Setting |
| 已知阻塞 | 无；DataRegistry、编辑器导入和 Spawner 尚未实现 |
| 最近验收 | Milestone 12：社会后果与角色行为打磨已完成 |

## 当前能力基线

- Milestone 1 至 12 已完成；M12 的个人社会后果、Decision v2、最小交易立场和 Kimi/Stub/UE 回归见 [Milestone12.md](../Milestones/Milestone12.md)。
- 当前 NPC 人物配置只存在于项目级 JSON 预设和 C++ 默认 Profile；尚无可复用 Persona Asset、DataRegistry、批量编辑器工作流或场景 Spawner。
- 当前世界背景只能间接写入 NPC Profile 字段；尚未存在场景级世界规则配置、动态世界事实或逐 NPC 世界认知。

## 当前执行边界

- M13 先建设静态 NPC Persona 与场景投放管线，不实现动态世界认知；后者已规划为 M14。
- M13 不修改 `/v1/decision` 或 `/v2/decision`。任何新协议字段必须先获得用户确认。
- 当前已实现架构仍以 [Architecture.md](../Planning/Architecture.md) 为准；M13/M14 的目标设计不应写成已实现能力。
