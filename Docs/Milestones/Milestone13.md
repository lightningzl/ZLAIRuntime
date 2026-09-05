# Milestone 13：NPC 内容配置、批量管理与场景投放

## 状态

- 状态：`已完成`
- 归档日期：2026-09-06
- 验收证据：[Milestone13Validation.md](../Validation/Milestone13Validation.md)

## 定稿范围

Milestone 13 建立了可复用 Persona Asset/Row、严格单条与批量 JSON、只允许配置来源的 DataRegistry 查询和 Asset/ID 双路径 NPC Spawner。编辑器批量写入在完整校验后以单一撤销事务覆盖或新增 DataTable 行；运行时只将静态 Persona 转换为既有 NPC Profile，个人社会事实继续由 UE 维护且不跨 NPC 共享。

最终编辑器工作流包括：Spawner 的 Persona Asset/已配置 Registry ID 互斥选择；ID 仅从明确配置且类型匹配的 Registry 获取，编辑器可按需加载该受限列表而不扫描任意资产；Persona Asset 支持文件或粘贴 JSON 导入；Persona DataTable 右键菜单支持粘贴批量 JSON、复制批量 JSON 与保存批量 JSON 文件。

验收 `M13-A01` 至 `M13-A06` 已完成。最终编译、自动化与回归证据以 [Milestone13Validation.md](../Validation/Milestone13Validation.md) 为准。

本里程碑不修改 UE/Python Decision 协议，不实现动态世界事实、谣言、个人世界认知、运行时配置覆盖或完整角色编辑器。
