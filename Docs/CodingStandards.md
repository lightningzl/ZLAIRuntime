# Coding Standards

## 通用规则

- 只实现 [CurrentMilestone.md](./CurrentMilestone.md) 的当前范围。
- 模块通过公开接口协作，不跨层访问内部类型或状态。
- 优先编写小而明确的函数；错误路径必须可观察、可恢复。
- 日志不得包含密钥、完整堆栈、内部路径或不必要的玩家输入。
- 代码、测试和相关文档在同一任务中同步更新。

## UE5 C++

- 遵循 Unreal 命名：`A` Actor、`U` UObject、`F` Struct、`E` Enum、`I` Interface、`b` Bool。
- 项目类型使用 `ZL` 前缀；文件名与主要类型名一致。
- UObject 成员使用 `UPROPERTY` 管理生命周期；对象引用优先使用 `TObjectPtr`。
- 头文件尽量前置声明；只在 `.cpp` 引入具体依赖。
- Blueprint 暴露保持最小化，并明确使用 `BlueprintCallable`、`BlueprintPure` 或 `BlueprintAssignable` 的原因。
- HTTP 和 JSON 细节只存在于 AI Service Client 层，不泄漏到 Gameplay/UI。
- 异步回调不得假设请求对象或调用方仍然存活；完成回调统一回到 Game Thread。
- 网络失败、超时、取消、非 `2xx` 和解析失败必须走明确失败路径，不能导致崩溃。
- 日志使用项目 Log Category，并包含 `request_id`；不以屏幕日志代替错误处理。
- 新增模块依赖时同步更新 `ZL.Build.cs`，避免加入当前里程碑不需要的依赖。

## Python

- 目标版本由项目依赖声明统一指定；使用类型标注和 UTF-8。
- 文件、函数和变量使用 `snake_case`，类使用 `PascalCase`，常量使用 `UPPER_SNAKE_CASE`。
- FastAPI Route 只负责 HTTP 适配；业务回复逻辑放在 Service 层；协议字段由 Pydantic Schema 定义。
- 对外响应不得直接返回内部异常文本或堆栈。
- Stub 回复必须确定、可测试，且不读取数据库、不访问外网、不需要 API Key。
- 模块导入不得产生启动服务、写文件或网络访问等副作用。
- 公共函数保持短小，并为协议边界、错误映射和非直观逻辑提供必要说明。

## 协议与测试

- JSON 字段、状态码和错误码以 [Protocol.md](./Protocol.md) 为唯一依据。
- UE 解析响应时忽略未知字段，但缺失必填字段时必须失败。
- Python 至少覆盖成功请求、缺字段、类型错误和空 `player_input`。
- UE 至少覆盖序列化、反序列化，以及网络失败、超时和非 `2xx` 路径。
- 无法执行验证时，在交付说明中明确列出，不得标记为已通过。

## 格式化与提交

- 避免顺手格式化无关文件。
- 提交前检查 diff、生成文件、密钥和行尾变化。
- Git 操作遵守 [GitWorkflow.md](./GitWorkflow.md)。
