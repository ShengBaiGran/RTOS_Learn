# VSCode 配置文件原理（面向后续学习）

## 0. 总体架构图（概念）

```text
你按 F5
  -> VSCode 读取 .vscode/launch.json
  -> Cortex-Debug 启动 gdb + openocd
  -> gdb 连接 openocd (localhost:3333)
  -> openocd 通过 CMSIS-DAP 与 MCU 通信
```

```text
你按 Ctrl+Shift+B 或 Run Task
  -> VSCode 读取 .vscode/tasks.json
  -> 在指定 cwd 和 env 下执行 make/openocd
```

---

## 1. `.vscode/extensions.json` 原理

作用：

- 只是“推荐扩展列表”，不参与编译和调试。
- 打开工程时，VSCode 会提示安装这些推荐扩展。

你当前配置：

```json
{
  "recommendations": [
    "ms-vscode.cpptools",
    "marus25.cortex-debug",
    "ms-vscode.makefile-tools"
  ]
}
```

学习重点：

- 这是团队协作入口，保证大家用同一类工具链。

---

## 2. `.vscode/tasks.json` 原理（任务系统）

### 2.1 核心概念

- 每个 `task` 就是一个可重复执行的命令模板。
- `label` 是任务名。
- `command` + `args` 是实际命令。
- `options` 决定工作目录和环境变量。
- `dependsOn` 可以把多个任务串起来。

### 2.2 关键字段解释（对应你当前文件）

`version`：

- 任务配置格式版本，固定用 `2.0.0`。

`label`：

- 任务名字，比如 `Build`、`Flash (OpenOCD)`。

`type: "shell"`：

- 表示用 shell 执行命令。

`command`：

- 执行入口，如 `make -j8` 或 `openocd`。

`options.cwd`：

- 指定命令在哪个目录执行。
- 你这里是 `${workspaceFolder}`，防止在错误目录跑命令。

`options.env.PATH`：

- 给任务单独注入工具链路径。
- 作用是即使 VSCode 从 Dock 启动（环境变量不完整），任务依然能找到 `arm-none-eabi-*`。

`group`：

- 把任务标记为默认 build 任务。
- 这样 `Ctrl+Shift+B` 会直接运行 `Build`。

`problemMatcher`：

- 把编译输出映射为 VSCode “问题列表”。
- `$gcc` 能识别 gcc 风格错误位置。

`dependsOrder: "sequence"` + `dependsOn`：

- 顺序串联任务。
- 例如 `Build + Flash + Run` 先编译，再烧录。

---

## 3. `.vscode/launch.json` 原理（调试系统）

### 3.1 核心概念

- `configurations` 是多个“调试场景”。
- 每个场景由 `name` 标识。
- `request` 决定调试模式：`launch` 或 `attach`。

### 3.2 `launch` 与 `attach` 区别

`launch`：

- VSCode 负责启动 OpenOCD 和 GDB。
- 适合日常一键调试。

`attach`：

- 假设 OpenOCD 已在外部运行。
- VSCode 只连接已有 gdb server。

### 3.3 关键字段逐个解释

`type: "cortex-debug"`：

- 指定使用 Cortex-Debug 调试后端。

`servertype`：

- `openocd`：由扩展启动 OpenOCD。
- `external`：连接外部已运行 server。

`serverpath`：

- OpenOCD 可执行文件路径。

`armToolchainPath` / `gdbPath`：

- 明确指定 ARM 工具链和 GDB 路径，避免 PATH 不一致。

`configFiles`：

- OpenOCD 脚本列表。
- `interface/cmsis-dap.cfg` 描述调试器接口。
- `target/stm32g4x.cfg` 描述目标芯片族。

`interface: "swd"`：

- 调试协议选 SWD（2 线），常用且稳定。

`device`：

- 设备标识，给扩展用于 UI/识别。

`executable`：

- 要调试的 ELF 文件路径。

`svdFile`：

- 外设寄存器描述文件路径。
- 用于调试器里显示寄存器树。

`preLaunchTask`：

- 调试前先执行任务（你是 `Build`）。

`runToEntryPoint: "main"`：

- 下载后自动运行到 `main` 断下，便于开始调试。

`openOCDLaunchCommands`：

- 启动 OpenOCD 时附加命令。
- 你设置 `adapter speed 1000`，更稳。

`postLaunchCommands`（Fault 场景）：

- 调试启动后自动下断点。
- 你配置了 `HardFault_Handler` 与 `Error_Handler`，排错效率很高。

---

## 4. 这三种配置在你项目中的职责分工

- `STM32G474 Launch (DAPLink + OpenOCD)`：日常调试主入口。
- `STM32G474 Launch (Fault Breakpoints)`：怀疑异常/死机时使用，自动捕捉故障。
- `STM32G474 Attach (OpenOCD running on :3333)`：你手动开了 OpenOCD 时使用。

---

## 5. 为什么这样配置“比默认更稳”

- 把路径写死到 `armToolchainPath/gdbPath`，不受 shell 环境影响。
- 任务里覆盖 `PATH`，解决 Dock 启动 VSCode 的环境差异。
- 降 `adapter speed`，优先稳定性。
- 引入 `svdFile`，寄存器可视化学习更直观。

---

## 6. 建议的学习路径（按阶段）

### 阶段 A：会用

- 熟练 `Build`、`Build + Flash + Run`、`F5` 三个动作。

### 阶段 B：会改

- 修改 `launch.json` 中 `openOCDLaunchCommands` 观察稳定性变化。
- 修改 `preLaunchTask`，理解调试前动作。

### 阶段 C：会设计

- 新增一个 `Launch (No Build)` 配置。
- 新增一个 `Attach` 到不同端口的配置。
- 为后续 FreeRTOS 增加专门断点策略（任务创建、调度相关）。

---

## 7. 高频误区

- 误区：`extensions.json` 能控制编译；事实：它只做推荐，不执行任务。
- 误区：`launch.json` 会自动修复编译问题；事实：它依赖 ELF，编译不过就无法调试。
- 误区：`attach` 与 `launch` 一样；事实：`attach` 不会替你启动 OpenOCD。
