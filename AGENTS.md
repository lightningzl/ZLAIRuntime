# Agent 协作规则

本文件适用于仓库内的所有 Agent 开发任务。

## 开始任务前

必须先完整阅读以下文档：

1. `Docs/ProjectOverview.md`
2. `Docs/Architecture.md`
3. `Docs/Protocol.md`
4. `Docs/CurrentMilestone.md`
5. `Docs/ProjectState.md`
6. `Docs/TaskBoard.md`

## 实施范围

- 只完成 `Docs/CurrentMilestone.md` 中列入当前里程碑的任务。
- 不得实现该文档“明确不做”部分或其他未列入当前范围的功能。
- 遵守 `Docs/Architecture.md` 中定义的模块边界和依赖方向。
- UE 与 Python Service 的通信必须遵守 `Docs/Protocol.md`。

## 完成任务后

1. 涉及 UE C++ 类型、职责或接口时，更新 `Docs/UEClasses.md`。
2. 涉及 Python 模块、职责或接口时，更新 `Docs/PythonModules.md`。
3. 如果做出新的架构选择或改变既有架构选择，更新 `Docs/DecisionLog.md`，记录背景、决定和原因。
4. 如果实现需要修改协议，必须先向用户说明修改原因并等待确认；未经确认，不得直接修改 `Docs/Protocol.md` 或实现与当前协议不一致的通信格式。

如果上述文档尚不存在，在首次需要更新时于 `Docs` 目录创建，并保持内容简洁、面向后续 Agent 协作。

## Git 提交流程

- 所有 Git 操作遵守 `Docs/GitWorkflow.md`。
- 开始修改前检查当前分支和工作区，保留并隔离用户已有改动。
- 提交前必须检查工作区 diff、暂存文件列表和暂存区 diff，并完成适用验证。
- 使用 Conventional Commits：`<type>(<scope>): <summary>`。
- 仅在用户明确要求“提交”后创建 commit；不得自动 push、rebase、merge 或创建 PR。
- 暂存时明确指定本任务文件，不得将无关改动或生成文件一并提交。
