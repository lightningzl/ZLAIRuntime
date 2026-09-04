# Task Board

## 当前状态

- 当前里程碑：Milestone 13——NPC 内容配置、批量管理与场景投放
- 里程碑状态：`已完成`
- 当前活动工作包：无
- 下一工作包：Milestone 14（需另行启动）
- 协议状态：本里程碑不修改 UE/Python 协议；如需要新增 Context 字段，必须先取得用户确认。

## 工作包

| ID | 工作包 | 主要成果 | 依赖 | 状态 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M13-T01` | Persona 数据模型与插件 Setting | Persona Asset/Row、字段校验、DataRegistry Setting 与稳定 ID 查询边界 | M12 | `已完成` | 满足 `M13-A01`、`M13-A02` 的模型部分 |
| `M13-T02` | Asset 单条 JSON 工作流 | Asset 编辑器导入/导出、Schema 校验和失败不写入语义 | `M13-T01` | `已完成` | 满足 `M13-A01` |
| `M13-T03` | DataTable/DataRegistry 批量工作流 | JSON 批量覆盖/新增、导出、逐行报告、ID 下拉 | `M13-T01` | `已完成` | 满足 `M13-A02`、`M13-A03` |
| `M13-T04` | NPC Spawner 与 Persona 解析 | 可放置 Spawner、类 + Asset/ID、固定位置与安全失败反馈 | `M13-T01` | `已完成` | 满足 `M13-A04` |
| `M13-T05` | 沙盒接入与回归 | Persona 注入 Inspector/Decision Context、编辑器与运行时验证、M12 回归和文档收口 | `M13-T02`、`M13-T03`、`M13-T04` | `已完成` | 满足 `M13-A05`、`M13-A06` |

## 推荐实施顺序

```text
M13-T01 -> (M13-T02 + M13-T03 + M13-T04) -> M13-T05
```

## 工作规则

- Persona 是配置数据，不是运行时记忆；攻击、报告、交易、关系变化和个人社会事实仍由 UE 在运行时维护。
- Asset 和 DataRegistry 共享同一字段语义；JSON 仅是导入导出格式，不成为第三套独立配置来源。
- 只有插件 Setting 声明的 DataRegistry 可被查询；不得在详情面板或运行时任意扫描项目资产。
- 实际验证写入 `Milestone13Validation.md`；TaskBoard 只维护工作状态。
