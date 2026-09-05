# Persona 配置指南

## 目的

本指南说明如何使用 Milestone 13 的 NPC Persona 内容管线。Persona 是 NPC 的**静态起点**：它定义身份、背景、性格、表达、目标及初始立场。攻击、报告、交易、关系变化和个人社会事实仍由 UE 运行时维护，不能写入 Persona。

## 配置路径

```text
Persona Asset / DataTable Row / JSON
             -> 已验证 Persona
             -> NPC Spawner
             -> Sandbox NPC Profile
             -> Inspector 与 Decision Context
```

使用 Persona Asset 时不依赖 DataRegistry。使用 Persona ID 时，ID 必须来自插件设置中显式声明且已缓存的 DataRegistry。

## Persona 字段

| 字段 | 必填 | 限制与用途 |
| --- | --- | --- |
| `stable_id` | 是 | 稳定且唯一；最长 64 个字符。DataTable 行名与该值保持一致。 |
| `display_name` | 是 | 场景中显示的名称；最长 64 个字符。 |
| `background_summary` | 是 | 给 Decision Context 的角色背景摘要；最长 512 个字符。 |
| `role` | 是 | NPC 身份；最长 128 个字符。 |
| `personality` | 是 | 1 至 8 项，每项最长 128 个字符。 |
| `speaking_style` | 是 | 表达倾向；最长 256 个字符。 |
| `goals` | 是 | 1 至 8 项，每项最长 128 个字符。 |
| `initial_relationship` | 是 | `trust`、`affinity` 为 `[-1, 1]`；`fear`、`familiarity` 为 `[0, 1]`。 |
| `initial_instant_state` | 是 | `fear`、`anger`、`curiosity`、`alert` 均为 `[0, 1]`。 |

## Persona Asset：单条配置

1. 在 Content Browser 新建 `ZL Social Persona Asset`。
2. 填写 `Persona` 中全部必填字段。
3. 可选择两种导入方式：在 `JSON` 区域指定文件后点击 `Import Persona Json`，或点击 `粘贴 JSON 导入 Persona`，在弹窗文本框中粘贴完整 JSON 后点击 `确认导入`。
4. 导入失败时 Asset 不会被改写；检查弹窗结果或 `Last Json Operation Result` 了解原因。

单条 JSON 的 `schema_version` 当前为 `1`，未知字段、缺失字段、越界数值或无效数组都会被拒绝。

```json
{
  "schema_version": 1,
  "stable_id": "npc_example_merchant",
  "display_name": "示例商人",
  "background_summary": "经营小型摊位，重视信誉与安全。",
  "role": "market merchant",
  "personality": ["pragmatic", "cautious"],
  "speaking_style": "礼貌、简洁，优先谈清条件。",
  "goals": ["保护摊位", "避免冲突"],
  "initial_relationship": {
    "trust": 0.1,
    "affinity": 0.1,
    "fear": 0.0,
    "familiarity": 0.2
  },
  "initial_instant_state": {
    "fear": 0.0,
    "anger": 0.0,
    "curiosity": 0.2,
    "alert": 0.1
  }
}
```

## DataTable 与批量 JSON

DataTable 的行结构必须为 `FZLSocialPersonaRow`。在 Content Browser 中选中该 DataTable，右键选择 `粘贴 Persona 批量 JSON`，在弹窗文本框中粘贴批量文件并点击 `确认导入`。同一菜单还提供 `复制 Persona 批量 JSON` 与 `导出 Persona 批量 JSON 到文件`：前者写入系统剪贴板，后者打开保存位置选择框。批量导入/导出由 `ZLASocialRuntimeEditor` 的 `FZLSocialPersonaDataTableTools` 提供：它在写入前验证整个批次，并在单一 UE 撤销事务中完成写入。

- 同一 `stable_id` 的行会覆盖。
- 新 `stable_id` 的行会新增。
- 批次内重复 ID、未知字段、无效类型或越界字段会让**整个批次**失败，DataTable 保持不变。
- 该右键入口只会显示在 Persona Row DataTable 上；它不使用 UE 原生 DataTable JSON 导入格式。

批量文件格式如下。每一项均使用与单条 Persona 相同的完整 Schema。

```json
{
  "schema_version": 1,
  "personas": [
    {
      "schema_version": 1,
      "stable_id": "npc_example_merchant",
      "display_name": "示例商人",
      "background_summary": "经营小型摊位，重视信誉与安全。",
      "role": "market merchant",
      "personality": ["pragmatic", "cautious"],
      "speaking_style": "礼貌、简洁。",
      "goals": ["保护摊位"],
      "initial_relationship": {"trust": 0.1, "affinity": 0.1, "fear": 0.0, "familiarity": 0.2},
      "initial_instant_state": {"fear": 0.0, "anger": 0.0, "curiosity": 0.2, "alert": 0.1}
    }
  ]
}
```

## DataRegistry

在 **Project Settings → ZL Social Persona → Persona Registry** 中，仅添加允许作为 Persona 来源的 DataRegistry。不要让任何运行时或详情面板逻辑扫描项目资产。

Registry 必须提供 `FZLSocialPersonaRow`，并且其 ID 与 Persona 的 `stable_id` 对应。运行时只读取已配置且已缓存的 Registry；ID 未命中、类型不匹配或在多个已配置 Registry 中冲突时，Spawner 会拒绝生成并报告原因。

## NPC Spawner

在关卡中放置 `ZL Social Sandbox NPC Spawner`，然后配置：

1. `Npc Class`：要生成的 Sandbox NPC 类。
2. 二选一：`Persona Asset`，或 `Persona Id`。`Persona Id` 是可搜索下拉框，只显示已在 `Persona Registries` 配置且类型匹配的有效 ID；编辑器会加载这些明确配置的 Registry，不扫描其他目录。选中 Asset 会自动清空 ID，选择 ID 会自动清空 Asset。
3. 可选：`Body Color`、`Initial Health` 与 `Spawn On Begin Play`。
4. 使用 Actor Transform 决定固定出生位置与朝向。

Spawner 以 Deferred Spawn 初始化 NPC。NPC 类、Persona 或 ID 解析任一项无效时不会产生半初始化 Actor；`Last Spawn Result` 会给出结果。配置不定义动画、导航、伤害、任意 Tool 或 Prompt。

## 验证清单

- Asset 导入后再次导出，字段应保持一致。
- 批量导入包含重复 ID 或未知字段时，DataTable 不应改变。
- Asset 和 Registry ID 两种方式都能生成正确名称和人物差异的 NPC。
- Inspector / Decision Context 应显示当前 NPC 的 Persona，而不显示其他 NPC 的私有社会事实。

相关实现与验证记录见 [Milestone13.md](../Milestones/Milestone13.md) 和 [Milestone13Validation.md](../Validation/Milestone13Validation.md)。
