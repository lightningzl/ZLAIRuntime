# Project State

## 当前快照

| 项目 | 当前值 |
| --- | --- |
| 最后更新 | 2026-09-06 |
| 当前里程碑 | Milestone 14：个人世界认知与动态背景反馈 |
| 里程碑状态 | `进行中` |
| 当前活动任务 | M14-T03 / M14-T04：世界事件感知与认知调试（进行中） |
| 下一候选任务 | M14-T05：协议确认后的 Decision 接入与回归（待启动） |
| 已知阻塞 | 无 |
| 最近验收 | Milestone 13：NPC 内容配置、批量管理与场景投放已归档 |

## 当前能力基线

- Milestone 1 至 13 已完成；M13 的 Persona 内容管线、DataRegistry/Spawner 投放与编辑器 JSON 工作流见 [Milestone13.md](../Milestones/Milestone13.md)。
- Persona Asset/Row、单条/批量 JSON、受限 Registry 查询、Asset/ID 双路径 Spawner 和编辑器内容制作入口已完成；转换后的 Persona 复用现有 NPC Profile，因此身份、人物、目标和初始状态已进入 Inspector 与 Decision Context，运行时个人社会事实继续按 NPC 隔离。
- 当前世界背景只能间接写入 NPC Profile 字段；尚未存在场景级世界规则配置、动态世界事实或逐 NPC 世界认知。

## 当前执行边界

- M14-T01 已完成：World Context 静态公开配置可经严格 JSON 导入导出并由 Setting 选择；动态世界事实与逐 NPC 认知仍未实现。
- M14-T02 已完成：World Fact 与逐 NPC Knowledge/Belief 已隔离建模；其感知、确认报告和有界传播入口尚未接入。
- M14-T03/T04 正在接入受控世界事件：当前事件仅让感知范围内 NPC 获得确认知识，Inspector 已显示个人来源、可信度与更新原因；报告和受限传播仍待接入。
- M14 若需增加 Decision Context 或 UE/Python 协议字段，必须先获得用户明确确认。
- 当前已实现架构仍以 [Architecture.md](../Planning/Architecture.md) 为准；M14 的目标设计不应写成已实现能力。
