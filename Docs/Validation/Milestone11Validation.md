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
| `M11-A07` | 通过 | 已编译的可选接口只在伤害接受后通知，未绑定资源无副作用。 |
| `M11-A08` | 通过 | UE Target 编译、29 项自动化、两套预设烟测与文档收口完成。 |

## M11-T02：JSON Schema、校验与受控预设

- 2026-09-02：`ZLEditor Win64 Development` 编译成功，新增本地 JSON 预设解析、受控目录限制、字段白名单、范围校验和导出实现均通过编译链接。
- 2026-09-02：NullRHI 执行 `ZL.Social.Sandbox.PresetValidation`，收集 1 项并通过；验证 `market_day`、`night_watch` 两套预设、稳定 ID/个人 Profile、路径穿越式预设名拒绝以及仅写入 Saved 的导出路径。

## M11-T03 至 T04：场景接入与无资源表现接口

- 2026-09-03：`ZLEditor Win64 Development` 编译成功；场景现在仅在提供 `-ZLSandboxPreset=<受控预设名>` 时从完整有效 JSON 快照生成玩家与 NPC，未提供或被拒绝时保持既有 4 NPC 默认场景。
- 2026-09-03：同次编译确认攻击展示只在现有目标、距离、冷却及伤害校验成功后调用可选接口；未绑定任何蓝图或资源时接口不改变攻击、伤害或镜头行为。
- 2026-09-03：最终合并后再次执行 `ZL.Social.Sandbox.PresetValidation`；日志显示收集 1 项并通过。命令行进程输出仅记录平台 SDK 探测，测试完成结果以 `ZL/Saved/Logs/ZL.log` 为准。
- 2026-09-03：`ZL.Social` 全量 NullRHI 回归收集 29 项并全部通过。
- 2026-09-03：`-ZLSandboxPreset=market_day -ZLSandboxPresetSmoke` 与 `night_watch` 两套真实地图烟测均通过；后者同时核对玩家/NPC 稳定 ID、显示名、出生位置、NPC Profile 与初始生命。
