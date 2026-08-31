# Python Modules

## 文档职责

本文档只记录当前 Python Service 模块、内部边界、运行配置和通用验证要求。HTTP 字段正文见 [Protocol.md](./Protocol.md)，SQLite 物理结构见 [DatabaseDesign.md](./DatabaseDesign.md)，当前范围见 [CurrentMilestone.md](../Current/CurrentMilestone.md)。

## 当前目录

```text
PythonService/
  pyproject.toml
  requirements.txt
  requirements-dev.txt
  README.md
  app/
    main.py
    api/dialogue.py
    core/settings.py
    schemas/dialogue.py
    services/
      context_builder.py
      dialogue_service.py
      memory_service.py
    memory/
      base.py
      maintenance.py
      models.py
      sqlite_repository.py
    providers/
      base.py
      errors.py
      factory.py
      kimi_provider.py
      stub_provider.py
  tests/
```

目录树只表示职责级结构，不作为完整文件清单。测试文件按被测边界拆分，实际文件以仓库为准。

## 模块职责

| 模块 | 职责 |
| --- | --- |
| `app.main` | 创建 FastAPI App，组装 Settings、Repository、Memory Service、Provider 和 Dialogue Service，并管理关闭生命周期 |
| `app.api.dialogue` | 提供 `POST /v1/dialogue` HTTP 适配，将已校验请求交给 Dialogue Service |
| `app.core.settings` | 读取并校验 Provider、超时、输出预算、数据库路径和 Memory 检索预算 |
| `app.schemas.dialogue` | 定义 v1 Dialogue、Context、Memory、成功响应和错误边界 |
| `app.schemas.decision` | 定义经确认的 v1 Decision、个人上下文、允许 Tool、结构化 Speech/Tool 建议和硬边界；当前尚未注册 Route |
| `app.services.context_builder` | 将固定约束、Context、合并历史和当前输入组装为供应商无关生成上下文 |
| `app.services.dialogue_service` | 协调业务校验、可选 Memory、单次 Provider 调用、合法响应和成功后完整写入 |
| `app.services.memory_service` | 执行范围复核、检索预算、稳定排序、精确重叠消除和幂等轮次语义 |
| `app.memory.base` | 定义 Repository 接口和脱敏持久化异常 |
| `app.memory.models` | 定义不可变的 Memory 内部类型，不暴露数据库行 |
| `app.memory.sqlite_repository` | 独占 SQLite Schema、索引、连接、SQL、事务、回滚、统计和精确清理 |
| `app.memory.maintenance` | 提供本机聚合统计和精确范围清理，不注册 HTTP Route |
| `app.providers.base` | 定义与 FastAPI、协议 Schema、UE 和具体 SDK 解耦的 Provider 接口 |
| `app.providers.errors` | 定义 Provider 内部错误分类，不携带 HTTP 状态码 |
| `app.providers.factory` | 根据 Settings 创建 Kimi 或显式 Stub Provider，不静默回退 |
| `app.providers.kimi_provider` | 映射供应商无关上下文，调用 Kimi Chat Completions，并分类 SDK 异常 |
| `app.providers.stub_provider` | 提供确定性离线回复，只用于显式本地模式和测试 |

## 内部类型边界

Context Builder 输出一次生成需要的供应商无关输入：固定系统约束、可选 NPC/World Context、有限历史和只出现一次的当前玩家输入。

Provider 接口只接收内部生成上下文，并返回非空 `reply` 和逻辑 `provider`。它不得接收 FastAPI `Request`、Pydantic 协议模型、UE 类型或暴露供应商 SDK 对象。

Dialogue Service 负责业务编排，不依赖 FastAPI HTTP 类型、SQL 或具体 SDK。Memory Service 只依赖 Repository 接口；SQLite Repository 是唯一允许执行 SQL 的模块。Context Builder 不读取网络、数据库、Settings 或环境变量。

## 依赖方向

```text
app.main
  -> settings
  -> provider factory
  -> repository + memory service
  -> dialogue service
  -> api route

api.dialogue
  -> schemas.dialogue
  -> services.dialogue_service

services.dialogue_service
  -> services.context_builder
  -> services.memory_service
  -> providers.base

services.memory_service
  -> memory.base / memory.models

memory.sqlite_repository
  -> memory.base / memory.models
  -> sqlite3

providers.kimi_provider
  -> providers.base / providers.errors
  -> OpenAI-compatible SDK -> Kimi API
```

## 运行配置

| 环境变量 | 当前规则 |
| --- | --- |
| `ZL_DIALOGUE_PROVIDER` | 默认 `kimi`；`stub` 必须显式选择 |
| `MOONSHOT_API_KEY` | Kimi 模式必填；不得进入日志、响应、UE 或 Git |
| `ZL_KIMI_MODEL` | 默认 `kimi-k2.6`；只由 Python Provider 消费 |
| `ZL_KIMI_TIMEOUT_SECONDS` | 默认 20 秒，必须小于 UE 外层 30 秒 |
| `ZL_KIMI_MAX_OUTPUT_TOKENS` | 默认 256，硬上限 4096 |
| `ZL_MEMORY_DATABASE_PATH` | 默认位于忽略的 `PythonService/data`；只由 Repository 消费 |
| `ZL_MEMORY_MAX_TURNS` | 默认最近 4 个完整轮次，范围 1 至 16 |

OpenAI 兼容 SDK 运行依赖为 `openai>=2.46,<3.0`。Kimi Client 禁用 SDK 自动重试；每次请求只发起一次上游生成。

模块导入不得启动服务、写文件或访问网络。Stub 模式不需要真实 Key。自动化测试屏蔽真实密钥并拦截非本机网络。

## Context 与 Memory 边界

- 固定系统约束集中维护，所有请求内容按不可信数据处理。
- `npc_id` 只作稳定标识，不用于推导人格。
- 持久化历史位于客户端历史之前，精确重叠由 Memory Service 消除。
- 省略 `memory` 时不读取或写入数据库。
- 只有 Provider 成功产生合法回复后才保存完整轮次。
- 当前不保存 NPC 人格、World Context、Prompt、Token、ToolCall、模型推理或 Gameplay 状态。
- 当前不生成或执行 ToolCall。

## 错误与日志

Provider 错误分为鉴权、限流、超时、不可用、无效响应和通用错误，再由应用边界映射为协议状态。Decision 契约允许脱敏 `502 planner_invalid_response`，其 Route 映射将在 Planner 工作包接入。Memory Repository 错误映射为脱敏 `500 internal_error`。

日志只记录请求关联、NPC ID、Context/Memory 开关、数量和错误分类；不记录完整 scope、玩家输入、Context、历史、回复、SQL、数据库路径或原始 SDK 异常。

## 验证要求

- Schema：边界、空白、未知字段和兼容路径。
- Service：调用顺序、单次 Provider、无 Memory 零读写和失败不写入。
- Repository：初始化、隔离、排序、预算、幂等、事务回滚和关闭。
- Provider：映射、空回复、异常分类、超时和禁用自动重试。
- 安全：无真实 Key、无外网副作用、日志和错误脱敏。

历史证据见 [Milestone2Validation.md](../Validation/Milestone2Validation.md)、[Milestone3Validation.md](../Validation/Milestone3Validation.md) 和 [Milestone4Validation.md](../Validation/Milestone4Validation.md)。
