# Milestone 6 Validation

## 状态

- 里程碑：Milestone 6：关系、长期记忆与重要 NPC
- 当前阶段：已完成
- 最后更新：2026-08-28
- 结论：通过，10/10 项已验证

验收标准正文见已归档的 [Milestone6.md](../Milestones/Milestone6.md)。本文只记录实际执行过的验证。

## 验证环境

- Unreal Engine：5.8，Win64 Development Editor，NullRHI 无界面自动化。
- 编译器：Visual Studio 2022 14.44.35228，Windows SDK 10.0.22621.0。
- 构建工具报告硬件：24 个物理核心、32 个逻辑处理器、63.84 GB 内存。
- Python：3.12.13；pytest 9.1.1。
- AI Provider：`stub`；端到端回归使用系统临时目录中的隔离 SQLite 数据库，完成后已关闭 Service 并删除数据库、WAL 与 SHM。

## 已执行验证

| 范围 | 方法 | 结果 | 证据 |
| --- | --- | --- | --- |
| Python 全量回归 | 在 `PythonService` 执行 `.venv/Scripts/python -m pytest` | 通过 | 收集 166 项，166 项通过，耗时 2.61 秒 |
| UE 既有本地自动化 | 分别无界面执行 `ZLAIRuntime.Configuration`、`ZLAIRuntime.Failures` 与 `ZLAIRuntime.Protocol` | 通过 | Configuration 1 项、Failures 1 项、Protocol 6 项，共 8 项通过 |
| UE Service 端到端 | 启动隔离 Stub Service，执行 `ZLAIRuntime.Integration.ServiceClientCallbacks` | 通过 | 1 项通过；覆盖 Dialogue、Context、Memory、失败响应与 Game Thread 单次回调 |
| UE 社会模拟自动化 | 无界面执行 `Automation RunTests ZL.Social` | 通过 | 13 项通过；覆盖 Event Chain、传播、关系、Authority、Long Memory、历史感知决策、纵向切片、Inspector 和基准 |
| UE Target 编译 | 执行 `Build.bat ZLEditor Win64 Development` | 通过 | UHT、`ZLASocialRuntime`、`ZLAIRuntime` 与 `ZL` 编译链接成功；最终检查 Target 为最新状态 |
| Docs 本地链接 | 递归解析 Markdown 相对链接并检查目标存在 | 通过 | 所有本地 Markdown 链接可解析 |
| Docs JSON 示例 | 解析全部 `json` fenced code block | 通过 | 所有示例均为有效 JSON |
| 文档格式 | `git diff --check` | 通过 | 无空白错误；仅有 Git 行尾转换提示 |
| 协议边界 | 比较 `origin/main` 与里程碑分支的 `Docs/Reference/Protocol.md` | 通过 | 零差异；Dialogue 字段、错误、超时和 SQLite Memory 语义未改变 |

一次准备性运行在 Stub Service 尚未启动时执行了包含 Integration 的 `ZLAIRuntime` 总前缀，端到端项按预期得到 `network_error`；随后按测试前置条件启动隔离 Stub Service，单独复跑端到端项并通过。该准备性失败未涉及代码修改，也未作为通过证据。

## 120 + 5 聚合基准

固定场景注册 120 个 Level 1 Agent 与 5 个 Important NPC，产生 Punch，显式确认一次 Report 并向 5 个 Important NPC 创建 Social 派生 Event，处理 Relationship、Long Memory、Authority 与本地规则决策。2026-08-28 最终运行结果：

| 指标 | 结果 |
| --- | ---: |
| Level 1 Agent | 120 |
| Important NPC | 5 |
| 注册总数 | 125 |
| Social 派生 Event | 5 |
| 传播拒绝 | 0 |
| Root/Agent 去重 | 5 |
| 稀疏 Relationship 边 | 13 |
| Faction Standing | 1 |
| Long Memory 项 | 5 |
| Rule Evaluation | 13 |
| Rule Evaluation 耗时 | 0.018 ms |
| 社会模拟核心处理耗时 | 0.036 ms |

该数字只表示上述本机、Development Editor、NullRHI 和固定场景下的单次证据，不作为跨硬件性能承诺。Runtime 另有 10000 个注册 Agent、1024 个活动 Root、4096 条 Personal Relationship 边、1024 条 Faction Standing、Depth 2、单节点 Fan-out 6、Root Budget 32 上限，以及有界 Short/Long Memory 与 Top-K。

## 验收项

| 验收 ID | 状态 | 证据 |
| --- | --- | --- |
| `M6-A01` | 通过 | Python 166 项、UE 既有本地 8 项和 Service 端到端 1 项通过；协议零差异 |
| `M6-A02` | 通过 | `LifecycleAndDedup` 与 `BoundsAndConfirmation` 覆盖 Root/Parent、Depth、Budget、TTL、Fan-out、Causation、过期清理和 Root/Agent 去重 |
| `M6-A03` | 通过 | `IntentRequiresConfirmation` 证明 Report Intent 不消费传播副作用；显式确认后才创建 Social Event 并衰减 Confidence |
| `M6-A04` | 通过 | `SparseBoundsAndAuthority` 覆盖稀疏有向边、Trust/Affinity/Fear/Familiarity/Reputation、衰减、重复与 4096 边硬上限 |
| `M6-A05` | 通过 | 同一测试覆盖普通 Witness 拒绝、Authority/Confidence 校验、Root/Faction 去重与有界 Faction Standing |
| `M6-A06` | 通过 | `ImportantPromotionRetrievalAndBounds` 覆盖 Level 1 Short 6、Important Short 16/Long 8、提升、锚定、衰减淘汰与稳定 Top-K |
| `M6-A07` | 通过 | `RelationshipMemoryFactionHistory` 以相同人格和事件验证 Investigate、Assist、Confront 三种历史相关结果、Reason Code 和重复确定性 |
| `M6-A08` | 通过 | `ImportantNpcReportVerticalSlice` 注册 5 个 Important NPC 并只通过 `ZLASocialRuntime` 本地规则完成社会行为；模块不依赖 HTTP Client |
| `M6-A09` | 通过 | 同一纵向测试覆盖 Punch→Witness Report→显式确认→Guard Social/Long Memory→Authority/Relationship→Intent，同一 Root 重放不重复生效 |
| `M6-A10` | 通过 | 13 项社会自动化、120+5 基准、扩展 Inspector、ZLEditor 编译、Python/UE 回归与安全输出检查通过 |

## 范围边界确认

- 未修改 [Protocol.md](../Reference/Protocol.md)，未新增 Decision Endpoint、ToolCall 或 Python 社会状态。
- 未实现 LLM 社会决策、Level 3 Core NPC、Mass 正式集成、StateTree/AIController 行为执行、SaveGame、Embedding 或 Runtime Debugger UI。
- Inspector 与基准只输出结构化社会元数据和聚合数量，不输出 Dialogue、scope、Prompt、API Key 或凭据。
