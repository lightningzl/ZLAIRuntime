# Documentation Rules

## 目标

每份文档只维护一种事实。长期规划、当前执行、稳定参考、流程规则、历史范围和验证证据必须分离；需要了解同一事项时通过链接追溯唯一来源，不在多份文档复制完整正文。

## 目录职责

```text
Docs/
  Planning/    长期目标、目标设计、当前架构和重要决策
  Current/     当前里程碑、当前任务板和实时项目状态
  Reference/   协议、UE 类型、Python 模块和数据库设计
  Process/     文档、编码和 Git 工作规则
  Interview/   面试展示、复习资料和历史问答
  Milestones/  已完成里程碑的范围定稿
  Validation/  已实际执行的验证证据
```

除非出现至少两份职责相同且持续增长的文档，不继续增加目录层级。实现源码和运行配置不得放入 `Docs`。

## 文档责任边界

| 文档 | 唯一职责 | 不负责 |
| --- | --- | --- |
| `README.md` | Docs 导航和入口链接 | 保存项目事实、范围或状态 |
| `Planning/ProjectOverview.md` | 项目定位、长期目标、技术栈和高层演进 | 当前任务、协议字段、验证结果 |
| `Planning/SocialSimulationPlan.md` | 长期交付范围、里程碑顺序、MVP 和取舍 | 当前允许范围、任务状态、实现完成度 |
| `Planning/SocialSimulationDesign.md` | 目标社会模拟领域模型和长期技术设计 | 声明目标已实现、线上协议正文、当前任务 |
| `Planning/Architecture.md` | 当前已接受并实现的系统模块、边界和依赖 | 目标路线、文件清单、当前任务状态 |
| `Planning/DecisionLog.md` | 已接受或已取代的重要决策及原因 | 任务流水账、临时方案、单独改变范围或协议 |
| `Current/CurrentMilestone.md` | 当前范围、明确不做、验收标准和完成定义 | 任务状态、验证日志、历史里程碑正文 |
| `Current/TaskBoard.md` | 当前工作包、依赖、状态和完成条件 | 复制验收标准、验证命令和历史结果 |
| `Current/ProjectState.md` | 活动任务、下一步、阻塞和验收进度的简洁快照 | 任务明细、架构、协议和验证日志 |
| `Reference/Protocol.md` | UE/Python 线上字段、语义、状态码和兼容性 | Provider SDK、Prompt、Tool 执行规则、实现步骤 |
| `Reference/UEClasses.md` | 当前 UE AI Runtime 类型、职责和依赖 | 历史任务流水账、系统级路线和测试证据正文 |
| `Reference/PythonModules.md` | 当前 Python 模块、内部边界和运行配置 | UE 类型、HTTP 字段正文和历史任务流水账 |
| `Reference/DatabaseDesign.md` | SQLite Schema、字段、约束、索引、事务和生命周期 | HTTP 协议、Memory 编排和任务状态 |
| `Process/DocumentationRules.md` | 文档目录、职责、同步和归档规则 | 项目功能设计 |
| `Process/CodingStandards.md` | 通用代码风格、实现约束和测试原则 | 当前范围、架构选择、Git 状态 |
| `Process/GitWorkflow.md` | 分支、暂存、提交和提交前检查 | 模块职责和当前任务 |
| `Interview/SocialSimulationInterviewGuide.md` | 面试演示顺序、讲解和问题复习 | 定义系统事实、协议或完成状态 |
| `Interview/Archives/*` | 历史学习资料 | 当前实现说明和当前调试手册 |
| `Milestones/MilestoneN.md` | 已完成里程碑的范围定稿 | 当前范围或后来实现状态 |
| `Validation/MilestoneNValidation.md` | 实际命令、环境、结果和验收证据 | 重新定义标准、计划或任务状态 |
| `AGENTS.md` | Agent 必读入口和强制工作流 | 复制各文档完整规则 |

## 权威顺序

- 长期目标和里程碑顺序以 `SocialSimulationPlan.md` 为准。
- 目标系统设计以 `SocialSimulationDesign.md` 为准，但它不授权实施。
- 当前允许范围和验收标准以 `CurrentMilestone.md` 为准。
- 当前任务状态和依赖以 `TaskBoard.md` 为准。
- 当前活动任务、阻塞和进度以 `ProjectState.md` 为准。
- 当前已实现模块边界以 `Architecture.md` 为准。
- 通信格式、字段和状态码以 `Protocol.md` 为准。
- UE/Python 类型和模块职责以对应 Reference 文档为准。
- SQLite 物理结构和事务以 `DatabaseDesign.md` 为准。
- 已执行行为以代码、测试和 Validation 证据为准。

若目标设计、当前范围和实现不同，不视为冲突：目标设计说明最终方向，Current Milestone 决定本阶段允许做什么，Architecture/Reference 只描述已经接受和实现的现状。

## 避免重复

- Task Board 不保存测试数量、命令或完整验证记录，只引用验收 ID。
- Project State 不逐任务复述实现历史，只保留简洁能力摘要。
- UEClasses/PythonModules 不使用 Milestone 任务 ID 充当当前类型状态。
- Architecture 不写“本里程碑做/不做”；范围只写入 Current Milestone。
- Interview 文档只能链接系统事实，不得成为设计来源。
- 归档和 Validation 不随当前实现演进，只修正链接、勘误或脱敏问题。

## 里程碑工作流

1. 从总规划选择下一个纵向切片。
2. 将已完成的 Current Milestone 定稿复制到 `Milestones/MilestoneN.md`。
3. 在 `CurrentMilestone.md` 定义新范围、明确不做、稳定验收 ID 和完成定义。
4. 在 `TaskBoard.md` 拆分工作包，并同步 `ProjectState.md`。
5. 实现期间只执行当前里程碑任务。
6. 实际验证结果写入 `Validation/MilestoneNValidation.md`。
7. 所有验收 ID 通过后才将里程碑标记为已完成。

未来能力可以保留在总规划和总设计中，但不能提前加入当前 Task Board。

## 状态同步

以下变化必须在同一工作变更中同步 `TaskBoard.md` 与 `ProjectState.md`：

- 任务进入 `进行中`、`受阻`、`待验收` 或 `已完成`。
- 活动任务、下一候选任务、阻塞或验收进度变化。
- 当前里程碑进入 `验收中` 或 `已完成`。

阻塞必须写明原因、受影响任务和解除条件。没有活动任务时明确写“无”。

## 协议、架构与设计变更

- 修改协议字段、类型、语义、状态码或兼容规则前，必须先向用户说明并获得明确确认。
- 协议确认后同步 `Protocol.md`、Current Milestone、Task Board、Project State 和两端模块文档。
- 目标系统的重要设计变化更新 `SocialSimulationDesign.md`；长期范围或顺序变化更新 `SocialSimulationPlan.md`。
- 已实现模块边界发生变化时更新 `Architecture.md` 和对应 Reference 文档。
- 满足架构、长期维护、技术选型、重要取舍或面试讲解价值的决策写入 `DecisionLog.md`。

Decision 状态允许：`已接受`、`已取代`。被取代的条目保留历史正文，并明确链接或命名后继决策。

## 安全与编写规则

- 不记录密钥、完整玩家输入、完整对话、内部绝对路径、完整堆栈、原始 Provider 异常或临时调试日志。
- 配置示例只使用占位值；本地 `.env`、SQLite、WAL、SHM 和生成文件不得提交。
- 文档内 JSON 示例必须是有效 JSON。
- 新建、移动或改名文档时更新全部入站链接以及 `AGENTS.md` 中适用路径。
- 文档只记录已确认事实；未执行验证明确标记“未验证”。
- 历史文档出现过期实现描述时增加历史声明，不把它改写成当前实现。

## Agent 工作流

开始实施前：

1. 按仓库根目录 `AGENTS.md` 完整读取必读文档。
2. 检查分支、工作区、当前范围、任务状态和阻塞。
3. 确认任务属于 Current Milestone。
4. 将任务更新为 `进行中`，同步 Project State，再修改实现。

结束任务时：

1. 记录实际结果和可复查证据。
2. 更新 Task Board 与 Project State。
3. 更新受影响的 Architecture、Reference、Decision 或计划文档。
4. 按 Git Workflow 检查 diff；未获明确要求不得创建 commit、push、rebase、merge 或 PR。

## 文档验证

文档变更至少检查：

- 相对链接无断链。
- JSON 示例可解析。
- 当前里程碑 ID、任务状态和 Validation 引用一致。
- 术语与责任边界一致。
- `git diff --check` 通过。

仅文档变更不要求运行代码测试；未运行测试必须如实说明。
