# Task Board

## 用途

本文件将 [CurrentMilestone.md](./CurrentMilestone.md) 中的当前里程碑拆成可执行工作包，并记录任务状态、依赖和验收证据。它不改变里程碑范围，也不替代协议或架构文档。

当前只维护 Milestone 1。超出当前里程碑的工作不得加入执行队列。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| `待开始` | 范围和前置条件明确，但尚未实施 |
| `进行中` | 已开始实施；同一任务只能有一个主要执行者 |
| `受阻` | 无法继续，且已记录阻塞原因和解除条件 |
| `待验收` | 实施完成，正在执行或等待规定验证 |
| `已完成` | 产物、适用验证和相关文档均已完成 |

任务只有在完成表中“完成条件”并留下可复查的验证记录后，才能标记为 `已完成`。未执行的验证必须写为“未验证”，不能根据代码存在推定通过。

## Milestone 1 工作包

| ID | 状态 | 工作包 | 主要产物 | 依赖 | 完成条件 |
| --- | --- | --- | --- | --- | --- |
| `M1-01` | `已完成` | Python Service 骨架 | `PythonService/app/`、依赖声明、启动说明 | 无 | Service 可通过一条明确命令在本地启动 |
| `M1-02` | `已完成` | Python 协议与错误处理 | Route、Schema、Service、统一异常映射 | `M1-01` | 合法请求返回协议响应；非法请求返回规定错误结构且 Service 不崩溃 |
| `M1-03` | `已完成` | Python 自动化测试 | 成功与错误路径测试 | `M1-02` | 测试覆盖字段透传、Stub Provider、缺字段、类型错误和空输入并全部通过 |
| `M1-04` | `待验收` | UE AI Service Client | `ZLAIRuntime` Runtime Plugin、Subsystem、协议结构体、HTTP/JSON 请求与响应解析 | 无 | UE 能构造协议请求并以明确的成功或失败结果完成异步回调 |
| `M1-05` | `待开始` | UE 配置与失败处理 | Base URL、超时配置、分类错误模型和日志 | `M1-04` | 网络失败、超时、非 `2xx` 和解析失败均不崩溃，并输出可定位信息 |
| `M1-06` | `待开始` | UE 最小演示入口 | 对话触发入口、最小可见结果载体 | `M1-04`、`M1-05` | UE 可发起请求并可见地展示 `reply` 或失败信息 |
| `M1-07` | `待开始` | 端到端验收与交付记录 | 演示步骤、验收结果、依赖检查 | `M1-03`、`M1-06` | Milestone 1 全部验收项有证据，包含至少 10 次连续请求验证 |

## 工作包明细

### M1-01：Python Service 骨架

- 创建 `PythonService` 顶层目录。
- 按 Route、Schema、Service 分层建立最小 FastAPI App。
- 声明 Python 版本、依赖和单命令启动方式。
- 不加入 LLM SDK、数据库、向量库或 Tool 框架。

验证：安装依赖后执行启动命令，确认进程可启动且应用可加载。

验证记录（2026-07-17）：使用 Python 3.12.13 创建本地虚拟环境并安装 `requirements.txt`；应用导入检查通过；按 `README.md` 中的单命令启动 Uvicorn 后，`GET /docs` 返回 `200`，进程可正常停止。`POST /v1/dialogue` 返回 `404`，符合该接口留待 `M1-02` 实现的范围边界。

### M1-02：Python 协议与错误处理

- 实现 `POST /v1/dialogue`。
- 严格按 [Protocol.md](./Protocol.md) 校验请求与构造响应。
- 原样返回 `request_id` 和 `npc_id`，固定返回 `provider: stub`。
- 为业务校验、Schema 校验和未预期错误提供统一响应结构。
- Stub 回复保持确定性，Service 层不依赖 FastAPI HTTP 类型。

验证：分别发送合法请求、空输入、缺字段和错误类型请求，检查状态码和 JSON 结构。

验证记录（2026-07-17）：实际启动 Uvicorn 后，合法请求返回 `200`，`request_id` 与 `npc_id` 原样透传，`provider` 为 `stub`；空 `player_input` 返回 `400 invalid_request`；缺少 `player_input` 和数值类型 `player_input` 均返回 `422 validation_error`。另验证 Stub 回复具有确定性，`500 internal_error` 响应不包含原始异常文本或内部路径，Service 进程可正常停止。

### M1-03：Python 自动化测试

- 覆盖合法请求和字段透传。
- 覆盖 `provider` 与确定性回复。
- 覆盖空 `player_input`、缺少必填字段和字段类型错误。
- 覆盖内部错误不会泄露堆栈、密钥或内部路径。
- 确认测试及运行均不需要 LLM API Key 或外部网络。

验证：执行完整 Python 测试命令并记录结果。

验证记录（2026-07-17）：使用 Python 3.12.13 执行 `PythonService/.venv/Scripts/python -m pytest -c PythonService/pyproject.toml`，共收集并通过 10 个测试，耗时 0.68 秒且无警告；覆盖成功字段透传、确定性 Stub 回复、空输入、三个必填字段缺失、三个字段类型错误、内部错误脱敏，以及无 API Key 和禁止外部网络条件下的成功请求。随后执行 `pip check`，未发现依赖冲突。

### M1-04：UE AI Service Client

- 实现独立的 `UZLAIServiceSubsystem`。
- 定义请求、成功响应和服务错误结构体。
- 在客户端生成唯一 `request_id`。
- 完成 JSON 序列化、HTTP POST、响应反序列化和未知字段兼容。
- 使用异步成功/失败接口返回结果，不让 HTTP 细节泄漏到 Gameplay/UI。
- 同步维护 [UEClasses.md](./UEClasses.md)。

验证：编译受影响的 UE Target，并验证序列化与反序列化路径。

验证记录（2026-07-17）：`ZLAIRuntime` 项目级 Runtime Plugin 已创建，包含 `UZLAIServiceSubsystem`、三类协议结构体和独立 JSON 协议转换函数。使用 UE 5.8 与 MSVC `14.44.35228` 编译 `ZLEditor Win64 Development` 成功；执行 `ZLAIRuntime.Protocol` 自动化测试，共发现并通过 3 项，覆盖请求序列化、成功响应必填字段与未知字段兼容、服务错误解析。实际 Python Service 异步成功/失败回调尚待验收，因此任务保持 `待验收`。

### M1-05：UE 配置与失败处理

- 通过 `UZLAIServiceSettings` 和 UE Config 提供 Base URL 与请求超时。
- 区分网络失败、超时、非 `2xx` 和响应解析失败。
- 失败日志包含 `request_id`、错误类别和必要状态信息。
- 异步完成时检查上下文有效性，不持有 NPC Actor 强引用。

验证：分别模拟 Service 未启动、请求超时、非 `2xx` 和无效响应，确认 UE 可恢复且不崩溃。

### M1-06：UE 最小演示入口

- 提供一次最小对话请求触发方式。
- 为调用方提供 `npc_id` 和 `player_input`。
- 将成功响应中的 `reply` 输出到明确的屏幕、UMG 或日志载体。
- 以同样可见的方式报告失败，但不把屏幕日志当作错误处理本身。

验证：在 UE 中实际触发一次成功请求和一次失败请求。

### M1-07：端到端验收与交付记录

- 记录从 UE 输入、HTTP 请求、Service 响应到 UE 展示的可复现步骤。
- 对照 [CurrentMilestone.md](./CurrentMilestone.md) 逐项记录验收结果。
- 连续完成至少 10 次请求，观察两端稳定性和资源释放。
- 检查运行时依赖不存在数据库、向量库、真实 LLM 或 Tool Use。
- 同步维护 [PythonModules.md](./PythonModules.md)、[UEClasses.md](./UEClasses.md) 和 [ProjectState.md](./ProjectState.md)。

验证：Python 测试、UE 编译/运行验证和端到端请求均有明确结果。

## 推荐执行顺序

```text
M1-01 -> M1-02 -> M1-03 --+
                              +-> M1-07
M1-04 -> M1-05 -> M1-06 --+
```

Python 与 UE 两条链路可以并行；`M1-07` 必须等待两条链路完成。

## 维护规则

1. 开始任务前，将对应工作包改为 `进行中`，并同步更新 [ProjectState.md](./ProjectState.md)。
2. 任务受阻时，状态改为 `受阻`，在 Project State 中记录阻塞原因、影响和解除条件。
3. 实施完成但验证尚未结束时使用 `待验收`，不得提前标记 `已完成`。
4. 标记 `已完成` 时，同一变更中更新适用的模块文档和 Project State。
5. 任务拆分可以细化实现步骤，但不得扩大 [CurrentMilestone.md](./CurrentMilestone.md) 的范围。
6. 协议字段或语义需要改变时，停止实施并先向用户确认；未经确认不得修改协议或实现偏离协议的通信。
7. 每个任务遵守 [GitWorkflow.md](./GitWorkflow.md)，仅在用户明确要求后创建 commit。
