# Python Modules

## 当前状态

Route、协议 Schema、集中 Settings、Provider 接口与 Factory、确定性 Stub Provider、OpenAI Responses API 适配器、可注入的 Dialogue Service、Provider 错误分类、协议错误映射和自动化回归测试已经完成。

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
      dialogue_service.py
    providers/
      __init__.py
      base.py
      errors.py
      factory.py
      openai_provider.py
      stub_provider.py
  tests/
    conftest.py
    test_dialogue_api.py
    test_openai_api_integration.py
    test_provider_error_api.py
    test_provider_factory.py
    test_settings.py
    test_openai_provider.py
```

## 模块职责

| 模块 | 状态 | 职责 |
| --- | --- | --- |
| `app.main` | M2-04 已调整 | 创建 FastAPI App，在启动阶段组装 Settings/Provider/Service、注册 Route，并统一映射业务、Provider 和内部异常；支持 Provider 注入且模块导入时不读取配置或访问网络 |
| `app.api.dialogue` | M2-02 已调整 | 提供 `POST /v1/dialogue` 的 HTTP 适配，将已经校验的请求交给应用持有的 Dialogue Service，不直接调用 Provider SDK |
| `app.core.settings` | M2-02 已实现 | 从环境读取 Provider、密钥、模型、超时和输出上限；完成类型、范围、组合与脱敏校验 |
| `app.schemas.dialogue` | M2-04 已调整 | 定义 v1 请求、成功响应和统一错误响应；声明 `stub`/`openai` 成功来源和全部协议错误码 |
| `app.services.dialogue_service` | M2-04 已调整 | 执行业务校验，构造最小生成输入，只调用一次注入的 Dialogue Provider，将内部结果转换为协议响应 |
| `app.providers.base` | M2-02 已实现 | 定义与 FastAPI、Pydantic 协议 Schema 和供应商 SDK 解耦的 Provider 接口与内部结果类型 |
| `app.providers.errors` | M2-04 已实现 | 定义鉴权、限流、超时、不可用、无效响应和通用 Provider 内部异常，不包含 HTTP 状态码 |
| `app.providers.factory` | M2-03 已调整 | 根据 Settings 创建 OpenAI 或显式 Stub Provider，并支持注入 OpenAI 构造器；不静默回退 |
| `app.providers.openai_provider` | M2-04 已调整 | 使用官方 OpenAI Python SDK 和 Responses API 完成一次非流式文本生成，提取非空回复并把 SDK 异常转换为内部 Provider 分类 |
| `app.providers.stub_provider` | M2-02 已实现 | 提供确定性离线回复，仅用于显式本地模式和联调，不满足真实 LLM 验收 |
| `tests.*` | M2-05 已完成 | 全局移除真实 `OPENAI_API_KEY` 并拦截非本机套接字连接；离线覆盖配置、Factory、Stub/OpenAI 成功响应、SDK 到 HTTP 的完整错误映射、API 回归、单次调用和脱敏 |

## 内部类型边界

Provider 接口概念上接收一次生成所需的最小内部输入，并返回：

- 非空 `reply` 文本。
- 稳定逻辑标识 `provider`。

Provider 接口不得接收 FastAPI `Request`、构造 `JSONResponse`、返回 Pydantic 协议模型，或暴露 OpenAI SDK 的 Request、Response、Usage、异常和 ID 类型。

Dialogue Service 负责把协议请求转换为 Provider 输入，并把内部结果转换为 `DialogueResponse`。HTTP 状态码只在应用错误映射层确定。

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
  -> app.providers.base
  -> app.schemas.dialogue

app.providers.factory
  -> app.providers.openai_provider
  -> app.providers.stub_provider

app.providers.openai_provider
  -> app.providers.base / errors
  -> OpenAI SDK
```

官方 OpenAI Python SDK 运行依赖锁定为兼容范围 `openai>=2.46,<3.0`。OpenAI Client 从 Settings 接收 API Key、超时并设置 `max_retries=0`；Provider 每次只调用一次 `responses.create`。

- Route 可以依赖 Schema 和 Service，但不能依赖具体 Provider。
- Service 只依赖 Provider 接口和内部异常，不依赖 OpenAI SDK 或 FastAPI HTTP 类型。
- Provider 实现不依赖 Route、协议 Schema 或 UE 字段结构。
- Settings 不依赖 Provider 实现；Factory 负责把配置转换为具体 Provider。
- 测试通过应用工厂、Factory 或 Service 构造参数注入 Fake Provider，不通过 monkeypatch 真实网络作为主要测试方式。

## 运行与配置边界

| 环境变量 | 规则 |
| --- | --- |
| `ZL_DIALOGUE_PROVIDER` | 默认 `openai`；只允许受支持值，`stub` 必须显式选择 |
| `OPENAI_API_KEY` | OpenAI 模式必填；不得写入日志、响应、UE 或 Git |
| `ZL_OPENAI_MODEL` | 默认 `gpt-5.6-luna`；只在 Python Provider 内消费 |
| `ZL_OPENAI_TIMEOUT_SECONDS` | 默认 `20` 秒；正数并小于 UE 外层基线 `30` 秒 |
| `ZL_OPENAI_MAX_OUTPUT_TOKENS` | 默认 `256`；正整数，硬上限 `4096` |

- 模块导入不得启动服务、读取网络或调用 OpenAI。
- OpenAI 模式在应用启动阶段完成配置校验；配置无效时明确失败。
- Stub 模式不需要 API Key 或外部网络，并继续支持单命令启动。
- 自动化测试不读取开发者真实密钥；测试环境应删除或屏蔽 `OPENAI_API_KEY`。
- `.env` 可作为开发者本地工具，但不得成为运行必需，也不得提交真实文件；可提交的示例只包含变量名和占位值。

## Prompt 基线

- 只维护一个集中、短小的静态指令。
- 输出为简洁纯文本 NPC 回复，不要求 JSON。
- 不声称知道未提供的人格、世界状态、玩家历史或 Memory。
- 不生成 Tool Call、Gameplay 指令或系统操作建议。
- 玩家输入是不可信数据，不能改变系统边界。
- `npc_id` 仅作稳定标识，不推导角色设定。

## 错误边界

Provider 内部异常按以下类别向上层表达：鉴权、限流、超时、不可用、无效/其他 Provider 错误。应用错误映射层再严格转换为 [Protocol.md](./Protocol.md) 的状态码和错误码。

原始 SDK 异常可用于内部分类，但不得作为对外 `message`，也不得在普通错误日志中完整输出。单次请求不自动重试。

## 验证要求

- Settings：默认值、合法覆盖、非法类型/范围、缺少密钥、显式 Stub 和脱敏。
- Provider：请求参数、模型选择、非流式输出提取、空输出和所有 SDK 异常分类。
- Service/API：字段透传、`provider`、业务校验、全部协议错误码和单次调用。
- 安全：无外网测试、无真实 Token、无密钥、无原始异常正文。
- 依赖：完整测试和 `pip check` 通过。
- 真实网络：仅在最终端到端验收中使用用户本地环境配置执行，并记录脱敏结果。

历史通信闭环验证保留在 [Milestone1Validation.md](./Validation/Milestone1Validation.md)，当前真实 LLM 接入结果记录到 [Milestone2Validation.md](./Validation/Milestone2Validation.md)。
