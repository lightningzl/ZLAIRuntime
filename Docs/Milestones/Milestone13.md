# Milestone 13：NPC 内容配置、批量管理与场景投放

## 状态

- 状态：`已完成`
- 完成日期：2026-09-04
- 验收证据：[Milestone13Validation.md](../Validation/Milestone13Validation.md)

## 定稿范围

Milestone 13 建立了可复用 Persona Asset/Row、严格单条与批量 JSON、只允许配置来源的 DataRegistry 查询和 Asset/ID 双路径 NPC Spawner。编辑器批量写入在完整校验后以单一撤销事务覆盖或新增 DataTable 行；运行时只将静态 Persona 转换为既有 NPC Profile，个人社会事实继续由 UE 维护且不跨 NPC 共享。

本里程碑不修改 UE/Python Decision 协议，不实现动态世界事实、谣言、个人世界认知、运行时配置覆盖或完整角色编辑器。
