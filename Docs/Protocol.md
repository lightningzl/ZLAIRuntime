# Protocol

## 基本约定

- Transport：HTTP
- Content-Type：`application/json`
- Encoding：UTF-8
- API Version：`v1`
- Endpoint：`POST /v1/dialogue`
- 字段使用 `snake_case`。
- 客户端生成 `request_id`，服务端在响应中原样返回，便于日志关联。
- 当前版本保持请求和响应字段结构稳定，并按兼容规则扩展 `provider` 允许值和错误码。

## 请求

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "npc_id": "npc_guard_01",
  "player_input": "这里发生了什么？"
}
```

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | 单次请求唯一 ID，推荐 UUID |
| `npc_id` | string | 是 | NPC 的稳定业务 ID |
| `player_input` | string | 是 | 玩家本次输入，不允许空字符串 |

当前阶段仍不发送人格、玩家历史、世界状态、Memory、Tool 定义或模型配置。Provider、模型和密钥均为 Python Service 内部配置，不得由 UE 请求指定。

## 成功响应

HTTP Status：`200 OK`

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "npc_id": "npc_guard_01",
  "reply": "城门刚刚关闭，请稍后再来。",
  "provider": "kimi"
}
```

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | 与请求一致 |
| `npc_id` | string | 是 | 与请求一致 |
| `reply` | string | 是 | 非空 NPC 回复文本 |
| `provider` | string | 是 | 生成回复的逻辑 Provider；当前允许 `stub`、`kimi`，不包含模型名 |

`stub` 仅用于显式离线开发和确定性验证；真实模型链路成功时必须返回 `kimi`。

当前响应不包含模型名、Token 用量、会话状态或 Tool Call。UE 只消费 `reply`，不得从回复文本中推断并执行 Gameplay 指令。

## 错误响应

非 `2xx` 响应统一使用以下结构：

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "error": {
    "code": "provider_timeout",
    "message": "dialogue provider timed out"
  }
}
```

| HTTP Status | `error.code` | 场景 |
| --- | --- | --- |
| `400` | `invalid_request` | 业务字段无效 |
| `422` | `validation_error` | JSON 缺字段或类型错误 |
| `429` | `provider_rate_limited` | 上游 Provider 拒绝请求，原因是当前额度或速率限制 |
| `502` | `provider_error` | 上游返回无效响应或发生未单独分类的 Provider 错误 |
| `503` | `provider_auth_error` | Provider 凭据缺失、无效或无权访问所配置模型 |
| `503` | `provider_unavailable` | Provider 暂时不可用，或当前 Provider 配置无法服务请求 |
| `504` | `provider_timeout` | Python Service 等待 Provider 超时 |
| `500` | `internal_error` | 与 Provider 无关的未预期服务端错误 |

错误 `message` 是稳定、脱敏、面向调用方的简短描述。错误响应和日志不得返回 API Key、上游原始异常正文、完整堆栈、内部路径或完整玩家输入。

UE 必须记录自己的 `request_id`、HTTP 状态与错误码，并以可恢复方式提示失败。UE 不需要把新增 Provider 错误码编译为枚举；未知错误码仍按通用 HTTP 失败处理。

## 超时责任

- Python Provider 超时负责终止单次上游生成请求，并返回 `504 provider_timeout`。
- UE HTTP 超时是端到端外层保护，配置值必须明确大于 Provider 超时。
- 单次请求无论由哪一层超时，都只能触发一次成功或失败完成回调。
- 当前协议语义不自动重试上游请求，避免重复计费和重复回复。

## 兼容性规则

- `v1` 内允许新增可选字段、`provider` 标识和错误码，不改变已有字段的类型或核心含义。
- `provider` 表示逻辑生成来源，不表示具体模型版本；模型替换不需要修改 UE 协议。
- 删除字段、修改字段类型或改变字段核心语义时创建新版本。
- UE 解析响应时忽略未知字段，并将未知服务错误码保留为字符串。
- 后续加入人格、世界状态、会话、Memory 或 Tool Call 前，必须重新评估是否仍适合在 `v1` 内兼容扩展。
