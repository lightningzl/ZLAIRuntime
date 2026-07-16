# Git Workflow

## 目标

保持提交范围清晰、可审查、可回退，并避免将 UE/Python 生成文件、环境文件或密钥提交到仓库。

## 分支策略

- `main` 始终保持可运行，不直接在其上提交未验证功能。
- 每个任务使用独立短期分支。
- Agent 创建的分支统一使用 `codex/` 前缀。
- 推荐命名：
  - `codex/feat-ue-ai-client`
  - `codex/feat-python-service`
  - `codex/fix-http-timeout`
  - `codex/docs-protocol-notes`

## 提交流程

1. **确认范围**
   - 阅读 `AGENTS.md` 和四份核心项目文档。
   - 查看当前分支与工作区状态。
   - 区分本任务改动、用户已有改动和生成文件。
2. **实施与验证**
   - 只修改当前任务需要的文件。
   - 按改动类型执行最小充分验证。
   - 更新 `UEClasses.md`、`PythonModules.md` 或 `DecisionLog.md` 中适用的文档。
3. **提交前检查**
   - 查看完整 diff 和未跟踪文件。
   - 明确指定暂存文件，不使用会混入无关文件的批量暂存。
   - 检查暂存区 diff，确认无密钥、缓存、二进制产物或无关格式化。
4. **创建提交**
   - 一个提交只表达一个逻辑变更。
   - 功能、重构和无关格式化不得混在同一提交。
   - 提交信息遵守下方格式。
5. **提交后确认**
   - 记录 commit hash 和验证结果。
   - 确认剩余未提交内容均为预期内容。
   - 推送或创建 PR 必须由用户明确要求。

## Commit Message

格式：

```text
<type>(<scope>): <summary>
```

允许的 `type`：

- `feat`：新增能力
- `fix`：修复缺陷
- `refactor`：不改变外部行为的重构
- `test`：新增或调整测试
- `docs`：只修改文档
- `build`：构建、依赖或工程配置
- `chore`：其他维护工作

推荐的 `scope`：`ue`、`python`、`protocol`、`docs`、`repo`。

示例：

```text
feat(ue): add AI service HTTP client
feat(python): add stub dialogue endpoint
fix(ue): handle service timeout safely
docs(repo): define git workflow
```

摘要使用英文祈使句，不加句号，建议不超过 72 个字符。需要正文时说明“为什么”，避免复述代码。

## 验证要求

| 改动类型 | 提交前最低验证 |
| --- | --- |
| 仅文档 | 检查链接、路径、JSON 示例和 diff |
| Python | 相关测试；无测试时至少启动检查和一次成功/失败请求 |
| UE C++ | 对受影响 Target 完成编译；涉及运行时行为时执行对应编辑器或游戏内验证 |
| UE + Python 协作 | 两端分别验证，并完成一次端到端请求 |

无法执行某项验证时，必须在交付说明中明确列出，不得写成已通过。

## Agent 权限边界

- Agent 不自动创建 commit；只有用户明确要求“提交”时才提交。
- 用户要求提交时，Agent 只能提交当前任务文件，不得顺带提交用户已有改动。
- 未经明确要求，不执行 push、force push、rebase、merge 或创建 PR。
- 禁止使用破坏性命令丢弃未提交改动。
- 发现疑似密钥或超大二进制文件时停止暂存并报告。
