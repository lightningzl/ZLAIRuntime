# Milestone 1 代码理解问题与答案

本文整理 Milestone 1 最小通信闭环的 8 个理解检查问题及参考答案，覆盖模块职责、调用链、数据结构、错误处理、设计原因、后续扩展、模块边界和故障排查。

## 1. 模块职责

### 问题

`ZLAIDialogueDemo.cpp`、`UZLAIServiceSubsystem`、`api/dialogue.py` 和 `dialogue_service.py` 各自负责什么？为什么不把 HTTP 请求、JSON 解析和回复生成全部写在 `ZLAIDialogueDemo.cpp` 中？

### 答案

- `ZLAIDialogueDemo.cpp` 是 Demo/表现层入口：解析控制台参数、获取 `UZLAIServiceSubsystem`、绑定成功与失败回调，并将结果显示到屏幕和日志。
- `UZLAIServiceSubsystem` 是 UE 通信流程协调者：生成 `request_id`、读取 Service 地址和超时、发起 HTTP 请求、接收结果、分类错误，并在 Game Thread 触发回调。
- `ZLAIServiceProtocol` 是 UE 协议转换层：在 UE 结构体与 JSON 之间进行序列化和解析。
- `api/dialogue.py` 是 Python HTTP 适配层：注册 `POST /v1/dialogue`，接收 FastAPI 已校验的 `DialogueRequest`，再将请求交给 Service 层。
- `dialogue_service.py` 是 Python 业务层：检查业务规则并生成确定性的 Stub 回复，不处理 FastAPI 请求对象或 HTTP 状态码。

分层可以避免入口文件臃肿，明确依赖方向，并让 UI、HTTP、协议转换和业务逻辑分别测试与复用。未来替换回复生成方式时，不需要把供应商逻辑泄漏到 UE 入口。

## 2. 完整调用链

### 问题

当 UE 执行 `ZL.AI.DialogueDemo npc_guard_01 What happened here?` 后，请按顺序说明从 `RunDialogueDemo()` 到 Python 的 `build_dialogue_response()`，再回到 UE 成功回调的完整调用链。

### 答案

```text
ZL.AI.DialogueDemo控制台命令
    -> RunDialogueDemo()
    -> 解析NpcId和PlayerInput
    -> UZLAIServiceSubsystem::SendDialogueRequest()
    -> 生成request_id并构造FZLDialogueRequest
    -> ZLAIServiceProtocol::SerializeDialogueRequest()
    -> IHttpRequest::ProcessRequest()
    -> Uvicorn接收POST /v1/dialogue
    -> FastAPI匹配路由并用Pydantic构造DialogueRequest
    -> create_dialogue()
    -> build_dialogue_response()
    -> 返回DialogueResponse
    -> FastAPI序列化为HTTP JSON响应
    -> UE OnProcessRequestComplete Lambda
    -> AsyncTask切回Game Thread
    -> UZLAIServiceSubsystem::CompleteRequest()
    -> ZLAIServiceProtocol::TryParseDialogueResponse()
    -> 校验request_id和npc_id
    -> 触发RunDialogueDemo()绑定的成功Lambda
    -> 屏幕和LogZL显示reply
```

收到 HTTP 回包后并不是直接进入 `CompleteRequest()`。首先执行 `OnProcessRequestComplete` 中绑定的 Lambda，由它提取状态码、响应 Body 和超时信息，再切回 Game Thread 调用 `CompleteRequest()`。

## 3. 数据结构与字段来源

### 问题

`FZLDialogueRequest`、Python 的 `DialogueRequest` 和网络中的 JSON 是什么关系？`request_id`、`npc_id`、`player_input` 分别在哪里产生、转换和校验？

### 答案

`FZLDialogueRequest` 是请求在 UE 进程内的结构，`DialogueRequest` 是同一协议数据在 Python 进程内的结构，JSON 是两个进程之间的传输格式。两端不共享内存，只共享 `Protocol.md` 定义的字段约定。

```text
UE控制台参数
    -> FZLDialogueRequest
    -> 请求JSON
    -> Python DialogueRequest
    -> Python DialogueResponse
    -> 响应JSON
    -> UE FZLDialogueResponse
```

| 字段 | 产生位置 | 转换位置 | 校验位置 |
| --- | --- | --- | --- |
| `request_id` | `SendDialogueRequest()` 使用 `FGuid::NewGuid()` 生成 | `SerializeDialogueRequest()` 写入 JSON | Python 校验类型；UE `CompleteRequest()` 检查响应值与原请求一致 |
| `npc_id` | `RunDialogueDemo()` 从 `Args[0]` 取得 | `SerializeDialogueRequest()` 写入 JSON | Python 校验类型；UE `CompleteRequest()` 检查响应值与原请求一致 |
| `player_input` | `RunDialogueDemo()` 将 `Args[1...]` 拼接得到 | `SerializeDialogueRequest()` 写入 JSON | Python Schema 校验类型；`build_dialogue_response()` 检查其不是空字符串 |

`TryParseDialogueResponse()` 负责解析成功响应，不负责校验发送前的请求。

## 4. 错误处理

### 问题

如果 `player_input` 为空、Python Service 没有启动、Service 返回 HTTP 500、Service 返回 HTTP 200 但 JSON 缺少 `reply`，分别由哪段逻辑处理，最终得到什么结果？

### 答案

| 情况 | 处理位置 | 最终结果 |
| --- | --- | --- |
| `player_input == ""` | `build_dialogue_response()` 抛出 `InvalidDialogueRequest`，由 `_handle_invalid_request()` 映射 | HTTP 400；UE `Category=Http`、`Code=invalid_request` |
| Service 未启动且直接连接失败 | UE `CompleteRequest()` 的传输失败分支 | `Category=Network`、`Code=network_error`、状态码 0 |
| 请求超时 | UE `CompleteRequest()` 的传输失败分支 | `Category=Timeout`、`Code=timeout` |
| Python 未预期异常 | `_handle_internal_error()` 返回标准错误 | HTTP 500；UE `Category=Http`、`Code=internal_error` |
| 非 2xx 且 Body 不符合错误协议 | UE 无法解析标准 Service Error | `Category=Http`、`Code=http_error`，保留 HTTP 状态码 |
| HTTP 200 但缺少 `reply` | `TryParseDialogueResponse()` 返回 `false` | `Category=Parse`、`Code=parse_error`、状态码 200 |

某些本机网络环境可能由中间层把连接失败转换成 HTTP 502，此时 UE 会将其识别为 `Http/http_error/502`，而不是 `Network/network_error`。

Python 中的 `application.add_exception_handler(Exception, _handle_internal_error)` 是通用 500 兜底。`Exception` 是 Python 内置异常基类；请求处理中未被更具体处理器匹配的异常会触发该处理器。具体异常对象由 FastAPI 传入 `_handle_internal_error(request, exception)`。

## 5. 为什么分离 Route 与 Service

### 问题

为什么 Python 要把 HTTP Route 写在 `create_dialogue()`，把业务规则和回复生成写在 `build_dialogue_response()`？如果直接在 `create_dialogue()` 中完成所有处理，会有什么问题？

### 答案

`create_dialogue()` 只负责 HTTP 适配：接收 FastAPI 已校验的输入、保存请求上下文并调用 Service。`build_dialogue_response()` 负责业务规则和回复生成，不依赖 FastAPI 的 `Request`、`JSONResponse` 或 HTTP 状态码。

这样可以：

- 分别测试 Route、Schema 和业务逻辑；
- 从命令行、后台任务或其他入口复用业务层；
- 替换 Stub/Provider 时保持 HTTP 路由和协议稳定；
- 避免模型调用、Prompt、超时、重试和响应解析全部堆进 Route。

真实 Provider 通常涉及异步 I/O，未来可能需要把 Route 改为 `async def` 并 `await` Service，但 Route 的地址、输入输出协议和职责仍应保持稳定。

## 6. 后续如何扩展为真实 LLM

### 问题

如果后续要把固定 `STUB_REPLY` 替换为真实 LLM，应新增哪些模块？哪些接口应该保持稳定？为什么 UE 不应该知道具体模型供应商？

### 答案

后续可以新增统一 Provider 接口、具体 Provider 实现和 Provider 内部数据结构，例如：

```text
app/
  providers/
    base.py
    openai_provider.py
  schemas/
    provider.py
  services/
    dialogue_service.py
```

- Provider 接口定义统一的 `generate_reply()` 能力。
- 具体 Provider 负责组装供应商请求、发送请求和解析供应商回包。
- Dialogue Service 校验业务输入、调用 Provider，并把 Provider 结果转换为项目自己的 `DialogueResponse`。
- Route 只返回 `DialogueResponse`，由 FastAPI 序列化并发送给 UE。

Provider 不应直接发送 UE 消息，也不应依赖 UE 协议之外的 Gameplay 类型。UE 只依赖项目自己的稳定 JSON 协议，因此更换 OpenAI、本地模型或其他供应商时，UE 调用方不需要理解供应商格式。

当前 Milestone 1 明确禁止真实 LLM，且 `provider` 固定为 `stub`。未来允许其他 Provider 值属于协议变化，必须进入后续里程碑，并在获得用户确认后更新 `Protocol.md`、两端 Schema 和测试。

## 7. 哪些逻辑不能放在 UZLAIServiceSubsystem

### 问题

哪些逻辑不能放进 `UZLAIServiceSubsystem`？这些逻辑分别属于哪一层？

### 答案

| 逻辑 | 所属模块 |
| --- | --- |
| NPC 回复气泡、字幕、颜色和显示时长 | UE UI/UMG |
| NPC 移动、转向、战斗和任务行为 | UE Gameplay、AIController、StateTree 等模块 |
| 动画、表情和姿态切换 | Animation Blueprint、AnimInstance 或角色动画组件 |
| 语音播放和空间音频 | UE Audio/Voice 组件 |
| 文本转语音生成 | Python TTS Provider 或独立语音 Service |
| Prompt 拼接和模型调用 | Python Dialogue Service/Provider |
| Memory 存储与检索 | 未来独立 Memory Service |
| Tool Call 规划 | 未来 Python Tool Planner |
| Tool 参数校验和 Gameplay 执行 | UE Tool Registry/Executor |

`UZLAIServiceSubsystem` 应保持为通信模块，只负责配置、协议请求、HTTP、JSON、错误分类和成功/失败回调。它不应该依赖具体 Widget、NPC Actor、动画、模型供应商或数据库。

## 8. 出错时从哪里排查

### 问题

如果 UE 已显示“请求已发送”，但始终没有显示 NPC 回复，应按照什么顺序排查？

### 答案

1. 查看 UE Output Log 中的 `LogZL` 和 `LogZLAIRuntime`，使用 `request_id` 关联请求。
2. 根据错误类别确定方向：
   - `network_error`：检查 Service 进程、端口和 URL；
   - `timeout`：检查 Service 是否卡住和 UE 超时配置；
   - `http`：检查 HTTP 状态码和 Python 错误响应；
   - `parse_error`：检查响应 JSON 结构和字段关联。
3. 检查 `DefaultGame.ini` 或 Project Settings 中的 `ServiceBaseUrl` 与超时。
4. 确认 Uvicorn 正在监听 `127.0.0.1:8000`，并检查 `/docs` 是否能访问。
5. 查看 Uvicorn 是否记录 `POST /v1/dialogue`，确认 Python 是否真正收到请求。
6. 检查 Python 返回的是 200、400、422 还是 500。
7. HTTP 200 仍出现 `parse_error` 时，检查响应是否包含 `request_id`、`npc_id`、`reply`、`provider`，以及前两个字段是否与请求一致。
8. 如果既没有成功日志也没有失败日志，检查异步期间 `GameInstance` 或 `UZLAIServiceSubsystem` 是否已销毁。`WeakThis` 和 `WeakGameInstance` 失效时会安全放弃回调。

“请求已发送”只表示 `IHttpRequest::ProcessRequest()` 成功启动异步请求，不代表 Python 已收到，也不代表请求已经成功。当前 UE 失败日志默认记录请求 ID、错误类别、错误码和 HTTP 状态，不打印原始响应 JSON。

## 复习重点

```text
未获得HTTP响应
    -> Network或Timeout

获得HTTP响应但状态码非2xx
    -> Http

获得HTTP 200但JSON不符合协议
    -> Parse
```
