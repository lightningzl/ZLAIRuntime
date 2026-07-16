# Protocol

## 基本约定

- Transport：HTTP
- Content-Type：`application/json`
- Encoding：UTF-8
- API Version：`v1`
- Endpoint：`POST /v1/dialogue`
- 字段使用 `snake_case`。
- 客户端生成 `request_id`，服务端在响应中原样返回，便于日志关联。

## 请求

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "npc_id": "npc_guard_01",
  "player_input": "这里发生了什么？"
}
```

字段说明：

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | 单次请求唯一 ID，推荐 UUID |
| `npc_id` | string | 是 | NPC 的稳定业务 ID |
| `player_input` | string | 是 | 玩家本次输入，不允许空字符串 |

当前阶段不发送人格、玩家历史、世界状态、Memory 或 Tool 定义。这些字段在对应里程碑中通过协议版本演进加入。

## 成功响应

HTTP Status：`200 OK`

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "npc_id": "npc_guard_01",
  "reply": "城门刚刚关闭，请稍后再来。",
  "provider": "stub"
}
```

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | 与请求一致 |
| `npc_id` | string | 是 | 与请求一致 |
| `reply` | string | 是 | NPC 回复文本 |
| `provider` | string | 是 | 当前固定为 `stub`，便于确认未调用真实 LLM |

当前响应不包含 Tool Call。UE 只消费 `reply`，不得从回复文本中推断并执行 Gameplay 指令。

## 错误响应

非 `2xx` 响应统一使用以下结构：

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "error": {
    "code": "invalid_request",
    "message": "player_input must not be empty"
  }
}
```

| HTTP Status | `error.code` | 场景 |
| --- | --- | --- |
| `400` | `invalid_request` | 业务字段无效 |
| `422` | `validation_error` | JSON 缺字段或类型错误 |
| `500` | `internal_error` | 未预期的服务端错误 |

错误响应不得返回堆栈、密钥或内部路径。UE 应记录 `request_id` 和错误码，并以可恢复方式提示失败。

## 兼容性规则

- `v1` 内允许新增可选字段，不改变已有字段含义。
- 删除字段、修改类型或改变语义时创建新版本。
- UE 解析响应时应忽略未知字段。
