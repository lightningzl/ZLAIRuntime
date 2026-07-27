# Python Modules

## 当前状态

Milestone 2 的 Route、协议 Schema、集中 Settings、Provider 接口与 Factory、确定性 Stub Provider、Kimi Chat Completions API 适配器、可注入的 Dialogue Service、Provider 错误分类、协议错误映射和自动化回归测试已经完成。

Milestone 3 的 v1 可选 `context` Schema、字段边界和空白业务校验已经实现。独立 Context Builder 和 Provider 内部生成上下文仍按 M3-03/M3-04 待实现。

## 目标目录

以下是职责级计划；实现任务可以调整文件名，但不得改变模块边界，调整后必须同步本文档。

```text
PythonService/
  .python-version
  pyproject.toml
  requirements.txt
  requirements-dev.txt
  README.md
  app/
    __init__.py
    main.py
    api/
      __init__.py
      dialogue.py
    core/
      __init__.py
      settings.py
    schemas/
      __init__.py
      dialogue.py
    services/
      __init__.py
      context_builder.py
      dialogue_service.py
    providers/
      __init__.py
      base.py
      errors.py
      factory.py
      kimi_provider.py
      stub_provider.py
  tests/
    conftest.py
    test_dialogue_api.py
    test_context_builder.py
    test_kimi_api_integration.py
    test_provider_error_api.py
    test_provider_factory.py
    test_settings.py
    test_kimi_provider.py
```

## 模块职责

| 模块 | 状态 | 职责 |
| --- | --- | --- |
| `app.main` | M2-04 已调整 | 创建 FastAPI App，在启动阶段组装 Settings/Provider/Service、注册 Route，并统一映射业务、Provider 和内部异常；支持 Provider 注入且模块导入时不读取配置或访问网络 |
| `app.api.dialogue` | M2-02 已调整 | 提供 `POST /v1/dialogue` 的 HTTP 适配，将已经校验的请求交给应用持有的 Dialogue Service，不直接调用 Provider SDK |
| `app.core.settings` | M2-02 已实现 | 从环境读取 Provider、密钥、模型、超时和输出上限；完成类型、范围、组合与脱敏校验 |
| `app.schemas.dialogue` | M3-02 已调整 | 定义 v1 请求、可选上下文、成功响应和统一错误响应；声明所有字段边界、角色枚举、`stub`/`kimi` 成功来源和协议错误码 |
| `app.services.context_builder` | M3-03 待实现 | 把固定系统约束、NPC 人格、世界状态、有限历史和当前输入组装为确定性的供应商无关生成上下文；不访问网络、数据库、Settings 或具体 Provider |
| `app.services.dialogue_service` | M3-02 已增加上下文业务校验；M3-04 待接线 | 拒绝空白上下文字段；后续调用 Context Builder，只调用一次注入的 Dialogue Provider，将内部结果转换为协议响应 |
| `app.providers.base` | M3-04 待调整 | 定义与 FastAPI、Pydantic 协议 Schema、UE 类型和供应商 SDK 解耦的 Provider 接口、内部生成上下文与结果类型 |
| `app.providers.errors` | M2-04 已实现 | 定义鉴权、限流、超时、不可用、无效响应和通用 Provider 内部异常，不包含 HTTP 状态码 |
| `app.providers.factory` | M2-07 已调整 | 根据 Settings 创建 Kimi 或显式 Stub Provider，并支持注入 Kimi 构造器；不静默回退 |
| `app.providers.kimi_provider` | M3-04 待调整 | 使用 OpenAI 兼容 Python SDK 和 Kimi Chat Completions API，把内部生成上下文映射为一次非流式消息生成，提取非空回复并分类 SDK 异常 |
| `app.providers.stub_provider` | M3-04 待调整 | 提供确定性离线回复和上下文验证能力，仅用于显式本地模式和联调，不满足真实 LLM 验收 |
| `tests.*` | M3-02 已增加 Schema/API 边界；M3-05 待扩展 | 全局移除真实 `MOONSHOT_API_KEY` 并拦截非本机套接字连接；已覆盖上下文 Schema 与 API，后续覆盖 Builder、Service、Provider 映射、日志和安全边界 |

## 内部类型边界

Context Builder 输出一次生成所需的供应商无关内部输入，概念上包含：

- 固定系统约束。
- 结构化的 NPC 人格和世界状态数据。
- 按最旧到最新排列的有限 `player`/`npc` 历史。
- 只出现一次的当前玩家输入。

Provider 接口接收该内部输入，并返回：

- 非空 `reply` 文本。
- 稳定逻辑标识 `provider`。

Provider 接口不得接收 FastAPI `Request`、构造 `JSONResponse`、返回 Pydantic 协议模型、依赖 UE 类型，或暴露供应商 SDK 的 Request、Response、Usage、异常和 ID 类型。

Dialogue Service 负责业务校验和编排；协议请求到内部生成上下文的转换只通过 Context Builder 完成。Service 把 Provider 结果转换为 `DialogueResponse`，HTTP 状态码只在应用错误映射层确定。

## 依赖方向

```text
app.main
  -> app.core.settings
  -> app.providers.factory
  -> app.api.dialogue

app.api.dialogue
  -> app.services.dialogue_service
  -> app.schemas.dialogue

app.services.dialogue_service
  -> app.services.context_builder
  -> app.providers.base
  -> app.schemas.dialogue

app.services.context_builder
  -> app.providers.base

app.providers.factory
  -> app.providers.kimi_provider
  -> app.providers.stub_provider

app.providers.kimi_provider
  -> app.providers.base / errors
  -> OpenAI-compatible SDK -> Kimi API
```

OpenAI 兼容 Python SDK 运行依赖锁定为 `openai>=2.46,<3.0`。Kimi Client 从 Settings 接收 API Key、固定 Base URL、超时并设置 `max_retries=0`；Provider 每次只调用一次 `chat.completions.create`。

- Route 可以依赖 Schema 和 Service，但不能依赖 Context Builder 内部类型或具体 Provider。
- Service 依赖 Context Builder、Provider 接口和内部异常，不依赖供应商 SDK 或 FastAPI HTTP 类型。
- Context Builder 可以接收已校验的协议数据，但输出必须是自有内部类型；不得依赖具体 Provider、Settings、网络或数据库。
- Provider 实现不依赖 Route、协议 Schema 或 UE 字段结构。
- Settings 不依赖 Provider 实现；Factory 负责把配置转换为具体 Provider。
- 测试通过应用工厂、Factory 或 Service 构造参数注入 Fake Provider，不通过 monkeypatch 真实网络作为主要测试方式。

## 运行与配置边界

| 环境变量 | 规则 |
| --- | --- |
| `ZL_DIALOGUE_PROVIDER` | 默认 `kimi`；只允许受支持值，`stub` 必须显式选择 |
| `MOONSHOT_API_KEY` | Kimi 模式必填；不得写入日志、响应、UE 或 Git |
| `ZL_KIMI_MODEL` | 默认 `kimi-k2.6`；只在 Python Provider 内消费；K2.x 简短对话关闭思考 |
| `ZL_KIMI_TIMEOUT_SECONDS` | 默认 `20` 秒；正数并小于 UE 外层基线 `30` 秒 |
| `ZL_KIMI_MAX_OUTPUT_TOKENS` | 默认 `256`；正整数，硬上限 `4096` |

- 模块导入不得启动服务、读取网络或调用 Kimi。
- Kimi 模式在应用启动阶段完成配置校验；配置无效时明确失败。
- Stub 模式不需要 API Key 或外部网络，并继续支持单命令启动。
- 自动化测试不读取开发者真实密钥；测试环境应删除或屏蔽 `MOONSHOT_API_KEY`。
- `.env` 可作为开发者本地工具，但不得成为运行必需，也不得提交真实文件；可提交的示例只包含变量名和占位值。

## Context Builder 与 Prompt 基线

- 只维护一组集中、短小、可测试的固定系统约束。
- 输出为简洁纯文本 NPC 回复，不要求 JSON。
- 只使用本次请求明确提供的人格、世界状态、历史和当前输入；缺失 `context` 时不得补造具体设定。
- 不生成 Tool Call、Gameplay 指令或系统操作建议。
- 人格、世界事实、历史和玩家输入均为不可信数据，必须以明确边界和确定性顺序表示，不能改变系统约束。
- 历史仅接受 `player`/`npc`，当前输入只出现一次且位于历史之后。
- `npc_id` 仅作稳定标识，不推导角色设定；Provider 不自行重新解释协议字段。
- Builder 不保存输入或输出，不生成会话 ID，不访问持久化介质。

## 错误边界

Provider 内部异常按以下类别向上层表达：鉴权、限流、超时、不可用、无效/其他 Provider 错误。应用错误映射层再严格转换为 [Protocol.md](./Protocol.md) 的状态码和错误码。

原始 SDK 异常可用于内部分类，但不得作为对外 `message`，也不得在普通错误日志中完整输出。单次请求不自动重试。

## 验证要求

- Settings：默认值、合法覆盖、非法类型/范围、缺少密钥、显式 Stub 和脱敏。
- Schema：无上下文兼容、完整上下文、所有字段边界、非法角色和未知字段兼容。
- Context Builder：确定性顺序、角色映射、空可选数组、当前输入不重复、注入边界和无状态。
- Provider：消息映射、模型选择、非流式输出提取、无工具/会话参数、空输出和所有 SDK 异常分类。
- Service/API：字段透传、`provider`、业务校验、全部协议错误码和单次调用。
- 安全：无外网测试、无真实 Token、无密钥、无完整上下文日志、无原始异常正文。
- 依赖：完整测试和 `pip check` 通过。
- 真实网络：仅在最终端到端验收中使用用户本地环境配置执行，并记录脱敏结果。

历史通信闭环和真实 LLM 接入结果分别保留在 [Milestone1Validation.md](./Validation/Milestone1Validation.md) 与 [Milestone2Validation.md](./Validation/Milestone2Validation.md)。Milestone 3 的实际验证结果只记录到 [Milestone3Validation.md](./Validation/Milestone3Validation.md)。
