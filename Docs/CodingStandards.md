# Coding Standards

## 通用规则

- 模块通过公开接口协作，不跨层访问内部类型或状态。
- 优先编写小而明确的函数；错误路径必须可观察、可恢复。
- 日志不得包含密钥、完整堆栈、内部路径或不必要的玩家输入。
- 当前范围、文档同步和验证记录遵守 [DocumentationRules.md](./DocumentationRules.md)。

## UE5 C++

- 遵循 Unreal 命名：`A` Actor、`U` UObject、`F` Struct、`E` Enum、`I` Interface、`b` Bool。
- 项目类型使用 `ZL` 前缀；文件名与主要类型名一致。
- UObject 成员使用 `UPROPERTY` 管理生命周期；对象引用优先使用 `TObjectPtr`。
- 头文件尽量前置声明；只在 `.cpp` 引入具体依赖。
- Blueprint 暴露保持最小化，并明确使用 `BlueprintCallable`、`BlueprintPure` 或 `BlueprintAssignable` 的原因。
- 异步回调不得假设请求对象或调用方仍然存活；完成回调统一回到 Game Thread。
- 网络失败、超时、取消、非 `2xx` 和解析失败必须走明确失败路径，不能导致崩溃。
- 日志使用项目 Log Category，并包含 `request_id`；不以屏幕日志代替错误处理。
- 新增模块依赖时同步更新对应 `.Build.cs`。

## Python

- 目标版本由项目依赖声明统一指定；使用类型标注和 UTF-8。
- 文件、函数和变量使用 `snake_case`，类使用 `PascalCase`，常量使用 `UPPER_SNAKE_CASE`。
- 对外响应不得直接返回内部异常文本或堆栈。
- API Key 只能从进程环境读取，不得进入请求 Schema、UE Config、日志、异常消息、测试数据或仓库文件。
- 模块导入不得产生启动服务、写文件或网络访问等副作用。
- 公共函数保持短小，并为协议边界、错误映射和非直观逻辑提供必要说明。

## 协议与测试

- 协议、模块边界和当前验收要求分别以 `Protocol.md`、`Architecture.md` 和 `CurrentMilestone.md` 为准。
- 测试必须可重复、隔离外部副作用，并覆盖当前范围要求的成功与失败路径。

## 格式化与提交

- 避免顺手格式化无关文件。
- Git 操作遵守 [GitWorkflow.md](./GitWorkflow.md)。
