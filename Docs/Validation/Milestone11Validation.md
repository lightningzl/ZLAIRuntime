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
| `M11-A01` 至 `M11-A08` | 未验证 | 待 M11 实现与验证完成后补充。 |

## M11-T02：JSON Schema、校验与受控预设

- 2026-09-02：`ZLEditor Win64 Development` 编译成功，新增本地 JSON 预设解析、受控目录限制、字段白名单、范围校验和导出实现均通过编译链接。
- 2026-09-02：NullRHI 执行 `ZL.Social.Sandbox.PresetValidation`，收集 1 项并通过；验证 `market_day`、`night_watch` 两套预设、稳定 ID/个人 Profile、路径穿越式预设名拒绝以及仅写入 Saved 的导出路径。
