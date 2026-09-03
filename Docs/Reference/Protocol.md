# Protocol

## 基本约定

- Transport：HTTP
- Content-Type：`application/json`
- Encoding：UTF-8
- API Version：`v1`（兼容运行）与 `v2`（当前运行的 Decision 计划契约）
- Endpoints：`POST /v1/dialogue`、`POST /v1/decision`、`POST /v2/decision`
- 字段使用 `snake_case`。
- 客户端生成 `request_id`，服务端在响应中原样返回，便于日志关联。
- v1 允许新增可选字段、`provider` 允许值和错误码；不得改变已有字段的类型或核心语义。
- 未知请求和响应字段按兼容规则忽略；已知嵌套对象中的字段仍必须满足本协议的类型与边界。

## 请求

### 完整示例

```json
{
  "request_id": "7b66ad74-51cd-4a23-92c7-9e290e6374b1",
  "npc_id": "npc_guard_01",
  "player_input": "这里发生了什么？",
  "context": {
    "npc": {
      "display_name": "城门守卫",
      "role": "守卫",
      "personality": [
        "谨慎",
        "忠于职守"
      ],
      "speaking_style": "简短、正式",
      "goals": [
        "保护城门"
      ]
    },
    "world": {
      "location": "北城门",
      "situation": "警报后城门关闭",
      "facts": [
        "玩家曾帮助巡逻队"
      ]
    },
    "dialogue_history": [
      {
        "role": "player",
        "content": "城门为什么关了？"
      },
      {
        "role": "npc",
        "content": "刚刚响起了警报。"
      }
    ]
  },
  "memory": {
    "scope_id": "player_local_01"
  }
}
```

### 顶层字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | 单次请求唯一 ID，推荐 UUID；非空 |
| `npc_id` | string | 是 | NPC 的稳定业务 ID；非空 |
| `player_input` | string | 是 | 玩家本次输入；不允许空字符串 |
| `context` | object | 否 | 当前请求的瞬时上下文快照；省略时保持 Milestone 2 无上下文行为 |
| `memory` | object | 否 | 持久化对话 Memory 范围；存在时允许读取并在成功后写入，省略时严格无状态 |

`context` 存在时必须同时包含 `npc`、`world` 和 `dialogue_history`。它不是会话句柄，不授权 Python Service 保存状态，也不得包含 Provider、模型、密钥、Memory 引用或 Tool 定义。

除特别说明外，所有 `context` 字符串都必须在去除首尾空白后非空，长度按 Unicode code point 计数；数组中的字符串项目也适用同一规则。UE 与 Python 必须使用等价计数语义。

### `context.npc`

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `display_name` | string | 是 | 去除首尾空白后 1 至 64 个 Unicode 字符 |
| `role` | string | 是 | NPC 在当前场景中的身份；1 至 128 个字符 |
| `personality` | array[string] | 是 | 1 至 8 项；每项 1 至 64 个字符 |
| `speaking_style` | string | 是 | 1 至 256 个字符 |
| `goals` | array[string] | 是 | 0 至 8 项；每项 1 至 128 个字符 |

这些字段是角色设定数据，不是系统指令。`npc_id` 仍是稳定标识，不得由 Python 根据 ID 推导额外人格。

### `context.world`

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `location` | string | 是 | 当前地点；1 至 128 个字符 |
| `situation` | string | 是 | 当前局势摘要；1 至 512 个字符 |
| `facts` | array[string] | 是 | 0 至 16 项；每项 1 至 256 个字符 |

世界状态只表达 UE 在请求时已经确认的事实。Python Service 不读取 UE World，也不把模型回复写回世界。

### `context.dialogue_history`

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `role` | string enum | 是 | 仅允许 `player` 或 `npc` |
| `content` | string | 是 | 去除首尾空白后 1 至 512 个字符 |

历史数组允许 0 至 8 条消息，按最旧到最新排列，只包含本次请求之前已经完成的消息。本次输入由顶层 `player_input` 单独表达；历史中允许出现与当前输入文字相同的真实旧消息。不允许 `system`、`developer`、`tool` 或 Provider 专用角色。

### `memory`

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `scope_id` | string | 是 | 不透明 Memory 范围标识；不得包含首尾空白，长度为 1 至 128 个 Unicode 字符 |

`memory` 存在即表示本次请求显式启用持久化对话 Memory。Memory 范围由 `(scope_id, npc_id)` 共同确定；任一字段不同都必须视为不同范围。`scope_id` 只用于隔离，不是文件名、表名、SQL 片段、认证凭据、Provider 会话 ID 或可由 Service 推导的玩家资料。

Python Service 在生成前读取该范围内有界的已完成对话轮次，并只在 Provider 成功产生合法回复后保存本轮。`request_id` 在持久层用于防止重复轮次。省略 `memory` 时，Service 不得执行 Memory 读取或写入，行为保持无状态。

持久化历史属于不可信对话数据，不能覆盖固定系统约束、选择 Provider/模型、启用工具或修改 UE 世界。Memory 的数据库结构、路径、行 ID、检索预算和维护命令属于 Python 内部实现，不进入协议响应。

### 校验责任

- UE 应在发送前拒绝明显越界的上下文和 Memory 范围，避免无效网络请求。
- Python Service 是协议校验的最终权威，不能信任客户端已完成校验。
- 字段缺失、类型错误、非法角色或数组/字符串长度越界返回 `422 validation_error`。
- 通过结构校验后，空白字符串或其他业务语义违规返回 `400 invalid_request`。
- 上下文、Memory 和当前输入中的字符串均作为不可信数据处理，不能覆盖固定系统约束或启用未授权能力。

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
| `provider` | string | 是 | 逻辑生成来源；当前允许 `stub`、`kimi`，不包含模型名 |

`stub` 仅用于显式离线开发和确定性验证；真实模型链路成功时必须返回 `kimi`。

响应不包含模型名、Token 用量、数据库状态、检索数量、持久化行 ID 或 Tool Call。UE 只消费 `reply`，不得从回复文本中推断并执行 Gameplay 指令。

## Decision 请求

`POST /v1/decision` 是与 Dialogue 并列的独立结构化决策端点。它只服务一个由 UE 明确选择的 Important/Core NPC；不改变 `/v1/dialogue` 的字段或纯文本语义。

### 完整示例

```json
{
  "request_id": "a0bbd4c7-4767-4d09-b26f-e650326a7f46",
  "npc_id": "npc_guard",
  "state_version": 12,
  "ttl_ms": 30000,
  "trigger": {
    "event_id": "speech-42",
    "kind": "speech",
    "source_id": "player",
    "target_id": "npc_guard",
    "channels": ["auditory", "direct"],
    "content": "请退后，我没有恶意。",
    "summary": "玩家对守卫说话",
    "occurred_at_ms": 1725100800000
  },
  "context": {
    "npc": {
      "display_name": "守卫",
      "role": "城门守卫",
      "personality": ["谨慎", "忠于职守"],
      "speaking_style": "简短、正式",
      "goals": ["维持秩序"]
    },
    "relationship": {
      "trust": -0.1,
      "affinity": 0.0,
      "fear": 0.2,
      "familiarity": 0.4
    },
    "instant_state": {
      "fear": 0.2,
      "anger": 0.3,
      "curiosity": 0.1,
      "alert": 0.8
    },
    "recent_history": [
      {
        "kind": "action_result",
        "source_id": "player",
        "target_id": "npc_guard",
        "summary": "玩家停止靠近守卫",
        "occurred_at_ms": 1725100795000
      }
    ]
  },
  "allowed_tools": [
    {"name": "face_target", "target_ids": ["player"]},
    {"name": "move_toward", "target_ids": ["player"]},
    {"name": "move_away", "target_ids": ["player"]},
    {"name": "stop", "target_ids": []}
  ]
}
```

### 顶层字段

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | UE 生成的单次请求 ID；去除首尾空白后 1 至 128 个字符 |
| `npc_id` | string | 是 | 本次唯一 Decision NPC 的稳定 ID；去除首尾空白后 1 至 128 个字符 |
| `state_version` | integer | 是 | UE 权威 NPC/场景状态版本；大于等于 0 |
| `ttl_ms` | integer | 是 | 从 UE 发起请求时开始计算的本地有效期；100 至 60000 ms |
| `trigger` | object | 是 | 该 NPC 已经感知到并触发本次决策的单个事件 |
| `context` | object | 是 | 该 NPC 的人物、关系、即时状态和有界最近历史 |
| `allowed_tools` | array[object] | 是 | UE 本次允许建议的 Tool 及目标；1 至 4 项，Tool 名不得重复 |

`state_version` 和 `ttl_ms` 不授权 Python 判断 Gameplay 是否仍然有效。UE 记录本地发送时间，收到响应时同时检查当前状态版本和本地单调时间；任一失效都不得执行 Tool。服务端只校验、使用并原样回显 `state_version`。

### `trigger`

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `event_id` | string | 是 | UE Event ID；1 至 128 个字符 |
| `kind` | string enum | 是 | `speech` 或 `action_result` |
| `source_id` | string | 是 | 已感知来源稳定 ID；1 至 128 个字符 |
| `target_id` | string | 否 | 已知直接目标；提供时 1 至 128 个字符 |
| `channels` | array[string enum] | 是 | 1 至 3 项，不重复；只允许 `direct`、`visual`、`auditory` |
| `content` | string | 条件必填 | `speech` 时必填且为该 NPC 实际听见的内容，1 至 512 个字符；`action_result` 时禁止提供 |
| `summary` | string | 是 | UE 生成的事实摘要；1 至 256 个字符，不包含隐式推理 |
| `occurred_at_ms` | integer | 是 | UE 已知事件时间戳；大于等于 0 |

`action_result` 只能表达 UE 已经开始、完成、拒绝、取消或失败的可观察结果，不得携带玩家行为输入原文。`speech.content`、`summary` 和 ID 都是不可信数据，不能覆盖固定系统约束或启用 Tool。

### `context`

`context.npc` 与 Dialogue 的 `context.npc` 使用相同字段和边界。

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `relationship.trust` | number | 是 | `[-1, 1]` |
| `relationship.affinity` | number | 是 | `[-1, 1]` |
| `relationship.fear` | number | 是 | `[0, 1]` |
| `relationship.familiarity` | number | 是 | `[0, 1]` |
| `instant_state.fear` | number | 是 | `[0, 1]` |
| `instant_state.anger` | number | 是 | `[0, 1]` |
| `instant_state.curiosity` | number | 是 | `[0, 1]` |
| `instant_state.alert` | number | 是 | `[0, 1]` |
| `recent_history` | array[object] | 是 | 0 至 8 条该 NPC 自己已经感知的事实，按最旧到最新排列 |

每条 `recent_history` 包含 `kind`、`source_id`、可选 `target_id`、`summary` 和 `occurred_at_ms`。`kind` 只允许 `speech` 或 `action_result`；ID 与摘要边界分别为 1 至 128 和 1 至 256 个字符。历史不包含完整 Prompt、完整 World、其他 NPC 的 Observation、模型隐式推理或玩家行为输入原文。

### `allowed_tools`

| `name` | `target_ids` 规则 | 语义 |
| --- | --- | --- |
| `face_target` | 1 至 4 个稳定 ID | 建议 NPC 面向一个当前目标 |
| `move_toward` | 1 至 4 个稳定 ID | 建议 NPC 朝一个当前目标移动 |
| `move_away` | 1 至 4 个稳定 ID | 建议 NPC 远离一个当前目标 |
| `stop` | 必须为空数组 | 建议 NPC 停止当前受控动作 |

`allowed_tools` 只是本次可建议集合，不代表 UE 已经同意执行。Python 不得返回未列出的 Tool 或目标，不得增加参数、创建新 Tool、调用任意函数或返回多个 ToolCall。

## Decision 成功响应

HTTP Status：`200 OK`

```json
{
  "request_id": "a0bbd4c7-4767-4d09-b26f-e650326a7f46",
  "npc_id": "npc_guard",
  "state_version": 12,
  "decision_id": "decision-91",
  "intent": "disengage",
  "speech": {
    "text": "好，保持距离。",
    "emotion": "wary"
  },
  "tool_call": {
    "call_id": "tool-91",
    "name": "move_away",
    "target_id": "player"
  },
  "confidence": 0.82,
  "provider": "kimi"
}
```

| 字段 | 类型 | 必填 | 边界与语义 |
| --- | --- | --- | --- |
| `request_id` | string | 是 | 与请求一致 |
| `npc_id` | string | 是 | 与请求一致 |
| `state_version` | integer | 是 | 与请求一致；UE 仍与当前权威版本重新比较 |
| `decision_id` | string | 是 | 服务端生成的 Decision ID；1 至 128 个字符 |
| `intent` | string enum | 是 | `respond`、`engage`、`disengage` 或 `hold` |
| `speech` | object | 否 | 可独立显示的结构化表达 |
| `tool_call` | object | 否 | 最多一个受约束动作建议 |
| `confidence` | number | 是 | `[0, 1]`；只供调试和策略参考，不绕过 UE 硬校验 |
| `provider` | string enum | 是 | `stub` 或 `kimi` |

`speech` 与 `tool_call` 至少存在一个。`speech.text` 为去除首尾空白后 1 至 512 个字符；可选 `speech.emotion` 为 1 至 64 个字符，只能作为 UE 白名单表达提示。

`tool_call.call_id` 为 1 至 128 个字符，并用于 UE 幂等检查；`name` 只允许四个已定义 Tool。`face_target`、`move_toward` 和 `move_away` 必须提供 1 至 128 字符的 `target_id`，`stop` 禁止提供 `target_id`。Tool 与目标还必须出现在对应请求的 `allowed_tools` 中。

Speech 与 Tool 独立处理：Speech 合法但 Tool 未注册、越权、参数非法、目标失效、状态过期、距离失效、冷却中或重复时，UE 可以显示 Speech，但不得执行 Tool。响应不得包含 Chain-of-Thought、任意参数对象、多 Tool 数组、脚本、函数名或“已执行”声明。

## Decision v2：开放计划与有限执行原语

`POST /v2/decision` 是经确认的下一代单 NPC Decision 契约。它保留 v1 的个人视角、状态版本、TTL、单个 Trigger 和 UE 最终权威，但以“开放的高层计划 + UE 本次明确提供的有限执行能力”取代 v1 的固定 Intent 与单 ToolCall。`/v1/decision` 保持不变；在 v2 实现和验证前，客户端与服务端继续使用 v1。

### 核心语义

- UE 只发送该 NPC 已感知的事实、人物、关系、即时状态、近期历史和本次可用能力；不得发送其他 NPC 私有 Observation、隐藏世界状态、Prompt 或模型推理。
- LLM 可以自由判断态度、目标、表达、表情倾向和计划组合；`objective`、`public_reason`、Speech 与表现参数都不是 UE 已执行事实。
- UE 只执行响应中回显的 `capability_id`，且该 ID、目标和参数必须来自本次请求的 `available_capabilities`。任何未提供、过期、越权、不可达或失败的步骤都不能被表述为已经发生。
- 每个步骤的实际 `started`、`completed`、`failed` 或 `rejected` 结果由 UE 产生新的个人事实后，才可能进入下一次 v2 请求。

### v2 请求增量

v2 顶层继续包含 `request_id`、`npc_id`、`state_version`、`ttl_ms`、`trigger` 和 `context`；前述公共字段沿用 v1 的类型、边界和个人视角约束。v2 的 `context` 新增 `social_situation`，并以 `available_capabilities` 替代 v1 的 `allowed_tools`。

```json
{
  "request_id": "9e3477ff-5f28-4d2c-a2a4-c1fdce7b44f5",
  "npc_id": "npc_merchant",
  "state_version": 31,
  "ttl_ms": 30000,
  "trigger": {
    "event_id": "hit-17",
    "kind": "action_result",
    "source_id": "player",
    "target_id": "npc_merchant",
    "channels": ["direct", "visual"],
    "summary": "玩家再次击中商人",
    "occurred_at_ms": 1788399600000
  },
  "context": {
    "npc": {"display_name": "商人", "role": "货摊商人", "personality": ["谨慎"], "speaking_style": "克制而精明", "goals": ["保护货物"]},
    "relationship": {"trust": -0.7, "affinity": -0.4, "fear": 0.8, "familiarity": 0.5},
    "instant_state": {"fear": 0.8, "anger": 0.6, "curiosity": 0.0, "alert": 1.0},
    "recent_history": [],
    "social_situation": [
      {"kind": "received_harm", "subject_id": "player", "summary": "玩家在 20 秒内第二次攻击你", "occurred_at_ms": 1788399600000, "salience": 0.95},
      {"kind": "public_warning", "subject_id": "player", "summary": "你已警告玩家停止", "occurred_at_ms": 1788399590000, "salience": 0.7}
    ]
  },
  "available_capabilities": [
    {"capability_id": "keep_distance_from_player", "kind": "move_away", "target_ids": ["player"]},
    {"capability_id": "seek_nearby_guard", "kind": "move_toward", "target_ids": ["npc_guard"]},
    {"capability_id": "refuse_trade", "kind": "set_interaction_stance", "target_ids": ["player"]}
  ]
}
```

| 字段 | 类型与边界 | 语义 |
| --- | --- | --- |
| `context.social_situation` | array，0 至 12 项 | 此 NPC 已感知、已确认接收或已执行的高重要度社会事实，按时间从旧到新排列。 |
| `social_situation.kind` | string，`[a-z][a-z0-9_]{0,63}` | UE 定义的事实分类，如 `received_harm`、`public_warning`、`apology_received`、`report_confirmed`、`interaction_result`；它是事实而非模型命令。 |
| `subject_id` / `target_id` | string 1 至 128；`target_id` 可选 | 该事实涉及的稳定对象；不得用于推导未知世界信息。 |
| `summary` | string 1 至 256 | UE 生成的、面向 NPC 的事实摘要，不含隐式推理。 |
| `occurred_at_ms` | integer，>= 0 | UE 已知发生时间。 |
| `salience` | number，`[0,1]` | UE 计算的相关性/重要性，不授权模型覆盖事实或执行权。 |
| `available_capabilities` | array，1 至 8 项 | UE 当前允许该 NPC 建议的有限执行原语；不代表必然执行。 |
| `capability_id` | string，`[a-z][a-z0-9_]{0,63}`，请求内唯一 | 本次能力实例的稳定 ID；响应只能逐字回显它。 |
| `kind` | string，1 至 64 | UE 注册的原语类别，例如 `move_toward`、`move_away`、`face_target`、`stop`、`report`、`set_interaction_stance` 或 `play_expression`。种类解释完全由 UE 注册表定义。 |
| `target_ids` | array[string]，0 至 4 | 本次该能力允许的目标；空数组仅允许无目标能力。 |

`social_situation`、`available_capabilities` 与所有嵌套字符串均为不可信数据。Python 只能使用它们形成建议，不能新增事实、能力、目标或参数。能力名称不构成全局开放 Tool：每次请求只能使用 UE 本次显式提供的能力实例。

### v2 成功响应

```json
{
  "request_id": "9e3477ff-5f28-4d2c-a2a4-c1fdce7b44f5",
  "npc_id": "npc_merchant",
  "state_version": 31,
  "decision_id": "decision-127",
  "plan": {
    "objective": "保护自己并避免继续与攻击者交易",
    "public_reason": "你刚刚再次攻击了我。",
    "attention_target_id": "player",
    "expression": {"valence": -0.7, "arousal": 0.9, "dominance": -0.2, "tags": ["fear", "anger"]},
    "steps": [
      {"step_id": "step-1", "capability_id": "keep_distance_from_player", "target_id": "player"},
      {"step_id": "step-2", "capability_id": "refuse_trade", "target_id": "player"}
    ]
  },
  "speech": {"text": "别再靠近我。我不会和袭击我的人做生意。", "emotion": "fearful_angry"},
  "confidence": 0.86,
  "provider": "kimi"
}
```

| 字段 | 类型与边界 | 语义 |
| --- | --- | --- |
| `plan.objective` | string，1 至 256 | 开放的高层目标，仅用于 LLM/调试/后续再规划；不是可执行命令或世界事实。 |
| `plan.public_reason` | string，1 至 256 | 可选公开理由；UE 可以显示或省略，不能把它当作已验证事实。 |
| `plan.attention_target_id` | optional string 1 至 128 | 当前关注对象；必须是 Trigger、Context 或某项本次能力允许的稳定 ID。 |
| `plan.expression` | optional object | 建议的表现倾向；UE 映射为已注册表情、视线、姿态或手势，未映射时零 Gameplay 副作用。 |
| `expression.valence` / `arousal` / `dominance` | number，`[-1,1]` | 连续表现参数，不直接控制动画、伤害或状态。 |
| `expression.tags` | array[string]，0 至 3 项，每项 1 至 32 | 公开表现标签；UE 忽略未注册标签。 |
| `plan.steps` | array，0 至 4 项，按顺序 | 建议的短期执行步骤；每项仍须独立通过 UE 校验。空数组允许纯表达或保持当前安全动作。 |
| `step_id` | string，1 至 128，响应内唯一 | 用于 UE 的步骤结果关联与幂等。 |
| `capability_id` | string | 必须逐字等于请求中的一个 `available_capabilities.capability_id`。 |
| `target_id` | optional string | 必须出现在对应能力的 `target_ids`；空目标能力禁止提供。 |
| `speech` / `confidence` / `provider` | 沿用 v1 的字段边界 | Speech 可以独立显示；Confidence 不绕过 UE 校验；Provider 仅允许 `stub` 或 `kimi`。 |

响应不得声明步骤已完成、报告已送达、交易已成功、伤害已产生或其他世界事实。`plan.steps` 数组不是无限 Agent 循环：UE 至多接受本次响应中的 4 个步骤，并在状态版本变化、TTL 到期、任一步失败或新高优先级事实到达时取消剩余步骤并重新判断。

### v2 校验与兼容性

- UE 在接受任一步骤前检查响应关联、当前状态版本、本地 TTL、能力实例、目标、Capability、位置/可达性、当前状态、冷却、速率、步骤幂等和计划总预算。
- `report`、`set_interaction_stance` 与 `play_expression` 都必须由 UE 注册具体 Handler；没有 Handler、目标失效或校验失败时，UE 记录稳定原因且不改变世界。
- `play_expression` 只能选择已注册的表现资产或参数映射，不能加载路径、生成资产或改变战斗结果。
- `/v1/decision` 不修改、不删除，也不与 `/v2/decision` 混用字段；部署可独立支持 v1 与 v2。v2 的 Provider 不可用时返回脱敏错误，不能把请求降级为 v1。
- v2 的未知顶层可选字段按 v1 兼容规则忽略；未知 `capability_id`、重复步骤、非法目标、越界表现参数或超出计划预算属于 `502 planner_invalid_response`。

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
| `400` | `invalid_request` | 已通过 Schema 的业务内容无效，包括空白内容或上下文语义违规 |
| `422` | `validation_error` | JSON 缺字段、类型错误、非法枚举或字段边界越界 |
| `429` | `provider_rate_limited` | 上游 Provider 因额度或速率限制拒绝请求 |
| `502` | `provider_error` | 上游返回无效响应或发生未单独分类的 Provider 错误 |
| `502` | `planner_invalid_response` | Decision Planner 返回缺字段、越界、未知 Tool/目标或其他无法形成合法 Decision 的结构 |
| `503` | `provider_auth_error` | Provider 凭据缺失、无效或无权访问所配置模型 |
| `503` | `provider_unavailable` | Provider 暂时不可用，或当前 Provider 配置无法服务请求 |
| `504` | `provider_timeout` | Python Service 等待 Provider 超时 |
| `500` | `internal_error` | 与 Provider 无关的未预期服务端错误，包括 Memory 数据库不可用或事务失败 |

错误 `message` 是稳定、脱敏、面向调用方的简短描述。错误响应和日志不得返回 API Key、上游原始异常正文、完整堆栈、内部路径、完整 `scope_id`、完整玩家输入、完整上下文或持久化对话。

UE 必须记录自己的 `request_id`、HTTP 状态与错误码，并以可恢复方式提示失败。UE 不需要把新增 Provider 错误码编译为枚举；未知错误码仍按通用 HTTP 失败处理。

## 超时责任

- Python Provider 超时负责终止单次上游生成请求，并返回 `504 provider_timeout`。
- UE HTTP 超时是端到端外层保护，配置值必须明确大于 Provider 超时。
- 单次请求无论由哪一层超时，都只能触发一次成功或失败完成回调。
- 当前协议语义不自动重试上游请求，避免重复计费和重复回复。

## 兼容性规则

- `context` 是 v1 的可选兼容扩展；旧客户端可继续只发送 `request_id`、`npc_id` 和 `player_input`。
- `memory` 是 v1 的可选兼容扩展；省略时严格保持既有无状态语义，提供时才允许按 `(scope_id, npc_id)` 读取和写入持久化对话。
- v1 内允许新增可选字段、`provider` 标识和错误码，不改变已有字段的类型或核心含义。
- `provider` 表示逻辑生成来源，不表示具体模型版本；模型替换不需要修改 UE 协议。
- 删除字段、修改字段类型、把当前可选字段改为必填或改变字段核心语义时创建新版本。
- UE 解析响应时忽略未知字段，并将未知服务错误码保留为字符串。
- 后续增加 Memory 管理端点、服务端会话句柄、Tool Call 或流式事件前，必须重新评估是否仍适合在 `v1` 内兼容扩展。
- `/v1/decision` 是独立端点，不改变 `/v1/dialogue` 的请求、响应、Memory 或纯文本语义；旧客户端和只实现 Dialogue 的服务部署可以继续使用原端点。
- Decision v1 允许增加可选表达字段、Intent 允许值和错误码；增加 Tool 名、改变现有 Tool 参数语义、允许多个 ToolCall 或改变 UE 权威校验责任时必须重新确认协议兼容性。
