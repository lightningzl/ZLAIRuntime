# Milestone 13 验证记录

## M13-T01：Persona 数据模型与插件 Setting

| 范围 | 实际结果 |
| --- | --- |
| UE 编译 | `ZLEditor Win64 Development` 在 Persona Runtime 类型、DataRegistry 插件依赖和 Setting 改动后编译成功。 |
| 编辑器内容制作入口编译 | 2026-09-06，Persona Asset 粘贴 JSON 导入、Persona DataTable 右键粘贴批量 JSON 导入、Spawner 已配置 Registry ID 搜索选择（按需加载已配置 Registry，不扫描其他资产）在 `ZLEditor Win64 Development` 编译并链接成功。 |
| DataTable 批量导出入口编译 | 2026-09-06，Persona DataTable 右键菜单的复制批量 JSON 与保存批量 JSON 文件入口在 `ZLEditor Win64 Development` 编译并链接成功。 |
| Persona 与 JSON 边界自动化 | NullRHI 运行 `ZL.Social.Persona.Validation`，发现 1 项并以 `Success` 完成；覆盖字段边界、单条/批量 JSON 往返、未知字段与重复稳定 ID 拒绝。 |
| Registry 查询编译 | `UZLSocialPersonaSettings` 的已配置 Registry 类型过滤和稳定 ID 枚举已通过 `ZLEditor Win64 Development` 编译；尚无项目 Persona Registry 资产，因此不将下拉结果标记为运行时通过。 |
| 双路径 Spawner | Persona 到 Sandbox Profile 的受控适配，以及 Asset/已配置 Registry ID 两条 Deferred Spawn 路径均已通过 `ZLEditor Win64 Development` 编译；Persona 自动化回归以 `Success` 完成。Registry ID 需要已缓存且类型匹配，冲突或未命中会拒绝生成。 |
| Editor 批量工具编译 | `ZLASocialRuntimeEditor` 的 Persona DataTable 批量解析、覆盖/新增、导出与撤销事务边界已通过 `ZLEditor Win64 Development` 编译。 |
| 社会回归 | NullRHI 运行 `ZL.Social`，共完成 30 项，均为 `Success`。 |
| Python 回归 | `.venv` 运行全套 `python -m pytest -q`，共 `205 passed`；同步了既有 `/v2/decision` 路由的维护测试期望。 |

## 验收进度

- `M13-A01` 至 `M13-A06`：通过。Persona 静态内容模型、单条/批量 JSON、受限 Registry、Asset/ID Spawner、现有 Profile/Decision Context 复用，以及 M12 Stub/Kimi/离线回归边界均有实现或自动化证据。

`M13-A01`、`M13-A02` 仍需单条 JSON、DataRegistry 查询与编辑器工作流完成后再整体验收；本记录不将未实现部分标记为通过。
