# Agent 协作规则

本文件适用于仓库内的所有 Agent 开发任务。

## 开始任务前

必须先完整阅读以下文档：

1. `Docs/Planning/ProjectOverview.md`
2. `Docs/Planning/SocialSimulationPlan.md`
3. `Docs/Planning/SocialSimulationDesign.md`
4. `Docs/Planning/Architecture.md`
5. `Docs/Reference/Protocol.md`
6. `Docs/Current/CurrentMilestone.md`
7. `Docs/Current/ProjectState.md`
8. `Docs/Current/TaskBoard.md`
9. `Docs/Process/DocumentationRules.md`

## 实施范围

- 长期目标和顺序遵守 `Docs/Planning/SocialSimulationPlan.md`，目标技术设计遵守 `Docs/Planning/SocialSimulationDesign.md`。
- 只完成 `Docs/Current/CurrentMilestone.md` 中列入当前里程碑的任务；总规划和总设计不直接授权提前实施未来能力。
- 不得实现该文档“明确不做”部分或其他未列入当前范围的功能。
- 遵守 `Docs/Planning/Architecture.md` 中当前已实现的模块边界和依赖方向。
- UE 与 Python Service 的通信必须遵守 `Docs/Reference/Protocol.md`。

## 完成任务后

按 `Docs/Process/DocumentationRules.md` 更新受影响的状态、模块、架构、协议、决策和验收文档。协议修改必须先向用户说明修改原因并等待明确确认；未经确认，不得直接修改 `Docs/Reference/Protocol.md` 或实现与当前协议不一致的通信格式。

如果上述文档尚不存在，在首次需要更新时按 `Docs/Process/DocumentationRules.md` 的目录与职责边界创建。

## Git 提交流程

- 所有 Git 操作遵守 `Docs/Process/GitWorkflow.md`。
- 开始修改前检查当前分支和工作区，保留并隔离用户已有改动。
- 提交前必须检查工作区 diff、暂存文件列表和暂存区 diff，并完成适用验证。
- 使用 Conventional Commits：`<type>(<scope>): <summary>`。
- 仅在用户明确要求“提交”后创建 commit；不得自动 push、rebase、merge 或创建 PR。
- 暂存时明确指定本任务文件，不得将无关改动或生成文件一并提交。
