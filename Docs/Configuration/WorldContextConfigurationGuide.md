# World Context 配置指南

## 目的

World Context 是场景的静态公开背景：世界规则、势力背景和初始公开常识。它不表示运行中的警戒、宵禁、攻击结果，也不会自动让全部 NPC 获知动态事件。

## 配置步骤

1. 在 Content Browser 新建 `ZL Social World Context Asset`。
2. 填写唯一的 `Stable Id`、`Display Name`，以及可选的 `Rules`、`Factions`、`Public Knowledge`。
3. 在 **Project Settings → ZL Social Persona → World Context** 设置 `Default World Context`。
4. 使用 Asset 的 JSON 区域导入或导出配置。可指定文件后点击 `Import World Context Json`，也可点击 `粘贴 JSON 导入世界背景`，直接粘贴完整 JSON 字符串。导入先完整验证，失败时不会改写当前 Asset。

每组条目最多 16 项；稳定 ID 最长 64 个字符，显示名最长 96 个字符，摘要最长 512 个字符。规则、势力和公开常识在各自列表内必须使用唯一 ID。

## JSON 示例

```json
{
  "schema_version": 1,
  "stable_id": "market_district",
  "display_name": "市场区",
  "rules": [
    {"stable_id": "curfew", "summary": "夜间适用已公布的宵禁规则。"}
  ],
  "factions": [
    {"faction_id": "city_guard", "display_name": "城卫队", "background_summary": "负责市场区公共秩序。"}
  ],
  "public_knowledge": [
    {"stable_id": "market_hours", "summary": "市场通常在白天营业。"}
  ]
}
```

未知字段、缺失字段、重复 ID 或超出边界的内容都会被拒绝。

## 运行时世界事件

在 Social Sandbox 中可通过控制台使用以下受控入口：

- `TriggerWorldEvent curfew|hazard|alert`：创建 90 秒有效的 UE 确认事实；只有当前 Shout 感知范围内的 NPC 获得确认知识。
- `ReportWorldEvent <报告者ID> <接收者ID>`：只向指定接收者交付报告者已知的事实，可信度不超过 `0.8`。
- `SpreadWorldRumor <报告者ID> <接收者ID>`：只允许单跳传闻；已经是传闻的知识不能再次传播，可信度最多 `0.5`。

Inspector 只显示当前选中 NPC 的个人世界认知，包括来源、可信度和更新原因。动态知识可被反驳或在时效结束后遗忘。

## 边界

- 不要把动态事件写进 World Context Asset。
- World Context 不覆盖 NPC 的运行时关系、攻击结果、个人 Observation 或 Knowledge。
- 当前 World Context 和个人 Knowledge 不进入 UE/Python Decision Context；协议未修改。

相关实现与验证记录见 [Milestone14.md](../Milestones/Milestone14.md) 和 [Milestone14Validation.md](../Validation/Milestone14Validation.md)。
