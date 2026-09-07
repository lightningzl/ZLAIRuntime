# Project State

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-07 |
| 当前里程碑 | Milestone 15：配置驱动场景与测试隔离 |
| 里程碑状态 | `进行中` |
| 当前活动任务 | `M15-T01` 移除运行时隐式场景与预设入口 |
| 下一候选任务 | `M15-T02` 通用 NPC 注册与调度 |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 13：NPC 内容配置、批量管理与场景投放已归档 |

## 当前能力基线

- Milestone 1 至 13 已完成；M13 的 Persona 内容管线、DataRegistry/Spawner 投放与编辑器 JSON 工作流见 [Milestone13.md](../Milestones/Milestone13.md)。
- Persona Asset/Row、单条/批量 JSON、受限 Registry 查询、Asset/ID 双路径 Spawner 和编辑器内容制作入口已完成；转换后的 Persona 复用现有 NPC Profile，因此身份、人物、目标和初始状态已进入 Inspector 与 Decision Context，运行时个人社会事实继续按 NPC 隔离。
- 当前世界背景只能间接写入 NPC Profile 字段；尚未存在场景级世界规则配置、动态世界事实或逐 NPC 世界认知。

## 当前执行边界

- M14 已完成并归档至 [Milestone14.md](../Milestones/Milestone14.md)：World Context、逐 NPC Knowledge/Belief、受控事件、指定报告、单跳传闻与 Inspector 已完成；协议与 Decision Context 未改动。
- M15 正在移除 GameMode 的隐式场景投放和测试入口，普通地图改为仅使用已放置的 PlayerStart、环境 Actor 与 NPC Spawner。
- 当前已实现架构仍以 [Architecture.md](../Planning/Architecture.md) 为准；M14 的目标设计不应写成已实现能力。
