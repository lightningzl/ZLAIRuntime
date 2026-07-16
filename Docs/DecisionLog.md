# Decision Log

新增或调整决策前，必须遵守 [DecisionLogRules.md](./DecisionLogRules.md)。

## 2026-07-16：使用 PythonService 作为 AI Runtime

决定：
UE 不直接调用 LLM。AI 推理、未来的 Prompt 组装和模型供应商适配统一放在 PythonService 中，UE 只通过服务协议请求结果。

原因：
- 隔离 UE Gameplay Runtime 与变化频繁的 AI SDK、模型和推理逻辑。
- 允许 Python 和 UE 独立测试、替换实现和演进。

取舍：
- 增加一个独立进程、网络边界以及相应的启动和错误处理成本。
- 放弃 UE 直接调用模型所带来的较少组件数量。

状态：
已接受

重要性：
重要

## 2026-07-16：第一里程碑使用 HTTP 和 JSON

决定：
UE 与 PythonService 在第一里程碑使用版本化 HTTP/JSON 通信，不使用 WebSocket；当前入口为 `POST /v1/dialogue`。

原因：
- 请求/响应模型满足当前非流式最小闭环，开发和调试成本较低。
- JSON 协议清晰，便于两端独立验证和面试演示。

取舍：
- 当前方案不支持双向推送和流式 Token。
- 如果后续里程碑证明需要持续连接或流式输出，需要重新评估 WebSocket 等方案。

状态：
已接受

重要性：
重要

## 2026-07-16：先使用确定性 Stub 验证闭环

决定：
Milestone 1 使用确定性 Stub 生成回复，`provider` 固定为 `stub`，暂不接入真实 LLM、Memory 或 Tool Use。

原因：
- 先独立验证 UE HTTP Client、协议、超时和错误处理。
- 避免模型网络、API Key 和输出不确定性干扰当前验收。

取舍：
- 当前阶段不能展示真实生成式对话能力。
- 后续接入 LLM 时仍需补充供应商适配、配置和模型错误处理。

状态：
已接受

重要性：
重要

## 2026-07-16：Python 按 Route、Schema 和 Service 分层

决定：
FastAPI Route 负责 HTTP 适配，Pydantic Schema 定义协议字段，Service 层提供与 FastAPI 解耦的 Stub 回复逻辑。

原因：
- 保持协议校验、传输逻辑和回复逻辑的模块边界清晰。
- 后续替换 Stub Provider 时无需改变 UE 协议或 Route 职责。

取舍：
- 当前代码量很小时会增加少量文件和接口。
- 需要避免为未来需求继续拆分不必要的层级。

状态：
已接受

重要性：
重要

## 2026-07-16：使用 Monorepo 管理 UE、PythonService 和 Docs

决定：
UE 工程、PythonService 和项目文档保存在同一个 Git 仓库中，并通过顶层目录隔离。

原因：
- 协议和两端实现可以在同一个变更中同步审查。
- Demo 的克隆、搭建和面试展示路径更集中。

取舍：
- 仓库需要严格忽略 UE 生成目录、Python 环境和大型非源码资源。
- 如果未来服务需要独立发布和权限管理，可能需要拆分仓库。

状态：
已接受

重要性：
重要

## 2026-07-17：分离任务执行状态与项目状态快照

决定：
使用 `TaskBoard.md` 维护当前里程碑的工作包、依赖和任务状态，使用 `ProjectState.md` 汇总当前活动任务、下一步、阻塞和验收进度。两者不重新定义 `CurrentMilestone.md` 的范围或 `Protocol.md` 的通信契约。

原因：
- 让开发者和 Agent 能快速恢复当前上下文，同时保持任务状态只有一个事实来源。
- 避免把高频变化的执行信息混入里程碑范围、架构或协议文档。

取舍：
- 任务状态变化时需要同步更新 Task Board 和 Project State。
- 如果维护流程未被遵守，两份状态文档可能短暂不一致，因此必须以实现和验证证据校正状态。

状态：
已接受

重要性：
重要

## 2026-07-17：UE AI Runtime 使用独立项目插件

决定：
UE 侧 AI Service Client、通信协议类型及后续同类 AI Runtime 能力统一放在项目级 `ZLAIRuntime` Runtime Plugin 中。游戏主模块 `ZL` 仅依赖插件公开接口，不直接拥有 HTTP/JSON 实现。

原因：
- 将可复用的 AI Runtime 能力与具体 Gameplay、UI 和模板示例代码隔离。
- 保持插件不反向依赖游戏模块，便于独立编译、测试以及未来迁移到其他 UE 项目。
- 让后续配置、失败处理和演示入口沿稳定模块边界演进。

取舍：
- 增加插件描述文件、独立模块和公开接口的维护成本。
- 游戏模块需要显式依赖插件模块后才能使用其类型，不能直接访问插件私有实现。

状态：
已接受

重要性：
重要
