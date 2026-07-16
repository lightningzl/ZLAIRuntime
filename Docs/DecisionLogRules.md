# Decision Log Rules

## 记录范围

创建或更新 [DecisionLog.md](./DecisionLog.md) 时，只记录会影响以下方面的决策：

- 项目架构
- 长期可维护性
- 模块边界
- 技术取舍
- 协作规则
- 面试讲解价值

不要记录每一个小型实现细节。

使用以下标准判断某项内容是否应写入 Decision Log：

> 如果未来开发者、评审者或面试官可能合理地询问“为什么要这样设计？”，则应记录到 `DecisionLog.md`。
> 如果它只是局部实现细节、临时调试选择、命名细节或常规代码组织，则不要记录。

## 应加入 DecisionLog.md

应记录以下类型的决策：

- 选择 PythonService 作为 AI Runtime，而不是让 UE 直接调用 LLM。
- 将 Prompt 组装保留在 PythonService 内部。
- 让 AI 返回 Tool Call 建议，由 UE 负责校验和执行。
- 第一里程碑选择 HTTP，而不是 WebSocket。
- 初始结构化 Memory 存储选择 SQLite。
- 让 UE 负责确定性的 Gameplay 执行。
- 使用包含 UnrealProject、PythonService 和 Docs 的 Monorepo 结构。
- 避免将大型 Marketplace 资源提交到 Git。

## 不应加入 DecisionLog.md

不要记录以下小型细节：

- 函数名
- 变量名
- 按键绑定
- 临时调试 UI 选择
- Log Category 名称
- 小型响应字段新增，除非它会影响协议设计
- 常规目录放置，除非它反映了架构决策
- 一次性实现细节

## 决策分级

写入 `DecisionLog.md` 前，必须先完成分类：

- **A级决策**：架构、边界、技术选型或长期取舍。必须加入 Decision Log。
- **B级决策**：可能影响后续工作的里程碑级选择。仅当未来可能需要解释时加入。
- **C级细节**：常规实现细节。不得加入 Decision Log。

如果无法确定，不要自动写入。应在回复中说明该决策，并询问用户是否需要记录。

## 必需格式

新增决策时使用以下格式：

```md
## YYYY-MM-DD：简短的决策标题

决定：
清楚描述所做的决策。

原因：
- 说明为什么选择此设计。
- 说明它在架构或项目层面的收益。

取舍：
- 说明成本、限制或被放弃的替代方案。

状态：
已接受

重要性：
重要
```
