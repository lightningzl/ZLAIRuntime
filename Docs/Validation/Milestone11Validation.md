# Milestone 11 验证记录

## 文档职责

本文件只记录 Milestone 11 已实际执行的验证命令、环境与结果。范围和验收标准见 [CurrentMilestone.md](../Current/CurrentMilestone.md)。

## 基线

- 2026-09-02：Milestone 10 已归档至 [Milestone10.md](../Milestones/Milestone10.md)，Milestone 11 当前范围与任务板已建立。
- 2026-09-02：确认 M11 不修改 [Protocol.md](../Reference/Protocol.md)、UE/Python 协议 Schema、Tool 或 Intent。
- 2026-09-02：确认蓝图、动画、蒙太奇、材质、地图和其他二进制资源由用户配置；本文件不会把未执行的资源配置记录为通过。

## 验收状态

| 验收项 | 状态 | 证据 |
| --- | --- | --- |
| `M11-A01` | 通过 | 协议与 Tool 无变更；`ZL.Social` 29/29 通过。 |
| `M11-A02` 至 `M11-A06` | 通过 | 两套预设解析、路径拒绝、公开导出和两次真实地图烟测通过。 |
| `M11-A07` | 通过 | 攻击/受击接口只在权威校验接受后通知；玩家/NPC 自身的蒙太奇、Section、播放速率未绑定时无副作用。 |
| `M11-A08` | 通过 | UE Target 编译、29 项自动化、两套预设烟测、无资源回退与文档收口完成。 |
| `M11-A09` | 通过 | NPC 已改为 `ACharacter`，具备内置 Mesh/Capsule/CharacterMovement 与自动 AIController Possess 条件；29 项社会沙盒自动化回归通过。 |

## M11-T02：JSON Schema、校验与受控预设

- 2026-09-02：`ZLEditor Win64 Development` 编译成功，新增本地 JSON 预设解析、受控目录限制、字段白名单、范围校验和导出实现均通过编译链接。
- 2026-09-02：NullRHI 执行 `ZL.Social.Sandbox.PresetValidation`，收集 1 项并通过；验证 `market_day`、`night_watch` 两套预设、稳定 ID/个人 Profile、路径穿越式预设名拒绝以及仅写入 Saved 的导出路径。

## M11-T03 至 T04：场景接入与无资源表现接口

- 2026-09-03：`ZLEditor Win64 Development` 编译成功；场景现在仅在提供 `-ZLSandboxPreset=<受控预设名>` 时从完整有效 JSON 快照生成玩家与 NPC，未提供或被拒绝时保持既有 4 NPC 默认场景。
- 2026-09-03：同次编译确认攻击展示只在现有目标、距离、冷却及伤害校验成功后调用可选接口；未绑定任何蓝图或资源时接口不改变攻击、伤害或镜头行为。
- 2026-09-03：最终合并后再次执行 `ZL.Social.Sandbox.PresetValidation`；日志显示收集 1 项并通过。命令行进程输出仅记录平台 SDK 探测，测试完成结果以 `ZL/Saved/Logs/ZL.log` 为准。
- 2026-09-03：`ZL.Social` 全量 NullRHI 回归收集 29 项并全部通过。
- 2026-09-03：`-ZLSandboxPreset=market_day -ZLSandboxPresetSmoke` 与 `night_watch` 两套真实地图烟测均通过；后者同时核对玩家/NPC 稳定 ID、显示名、出生位置、NPC Profile 与初始生命。

## M11-T06 至 T07：骨骼角色、攻击与输入配置

- 2026-09-03：`ZLEditor Win64 Development` 编译成功。玩家移除 `BodyMesh` 与 `FacingArrow`，改用 `ACharacter::Mesh`；NPC 移除同名占位组件，改用 `CharacterMesh`。两类角色可各自设置骨骼网格、动画蓝图、Attack/Hit Montage、可选 Section 和播放速率。
- 2026-09-03：GameMode 增加 `SandboxPlayerClass` 与 `SandboxNpcClass`，由用户的 GameMode 蓝图选择角色蓝图；玩家增加 Mapping Context、Move/Look/Attack Action 配置。资源缺失时不播放蒙太奇，并保留既有轴映射和 UI 攻击入口。
- 2026-09-03：使用 NullRHI 与内存 DDC 执行 `Automation RunTests ZL.Social`，收集 29 项且全部 `Success`，进程退出码 0；覆盖攻击校验、NPC 行动、预设、场景配置以及既有社会模拟回归。

## M11-T08 至 T09：NPC Character 与 AIController 基础

- 2026-09-03：`AZLSocialSandboxNpc` 从 `AActor` 改为 `ACharacter`，复用内置 Capsule、Mesh 与 CharacterMovement，并设为在放置或生成时自动使用标准 `AAIController`。既有受控移动仍由社会沙盒规则计算和执行，不把本次基类升级扩展为导航或行为树实现。
- 2026-09-03：`ZLEditor Win64 Development` 编译成功；NullRHI 与内存 DDC 执行 `Automation RunTests ZL.Social`，29 项全部 `Success`，进程退出码 0。
