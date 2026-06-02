# sCPU

一个 8-bit 的简化单周期 CPU，使用 Verilog 实现，通过 Verilator 仿真。

## Pins

- `clk`: 时钟输入
- `rst`: 同步复位输入
- `gpio[7:0]`: GPIO 输出寄存器

`gpio` 是寄存器输出，不是组合逻辑输出。只有执行 `out` 指令时，`gpio` 才会更新。

## Register File

- 4 个通用寄存器
- 每个寄存器宽度为 8 bit
- 编号为 `R[0]` 到 `R[3]`

## ISA

指令宽度为 8 bit。

### `add`

```text
7  6 5  4 3   2 1   0
+----+----+-----+-----+
| 00 | rd | rs1 | rs2 |
+----+----+-----+-----+
```

语义：

```text
R[rd] = R[rs1] + R[rs2]
```

### `out`

```text
7  6 5  4 3   2 1   0
+----+----+-----+-----+
| 01 | xx |  xx | rs2 |
+----+----+-----+-----+
```

语义：

```text
gpio = R[rs2]
```

该指令由控制器生成 `gpioWE`，把寄存器内容写入 `gpio` 寄存器。

### `li`

```text
7  6 5  4 3   2 1   0
+----+----+----------+
| 10 | rd |   imm    |
+----+----+----------+
```

语义：

```text
R[rd] = imm
```

高位补 0。

### `bner0`

```text
7  6 5   4 3   2 1   0
+----+----------+-----+
| 11 |   addr   | rs2 |
+----+----------+-----+
```

语义：

```text
if (R[0] != R[rs2]) PC = addr
```

## ROM Program

当前 ROM 程序会计算：

```text
1 + 2 + ... + 10 = 55
```

然后执行 `out r1`，把结果 `55` 输出到 `gpio`。

## Simulation

构建：

```bash
make -C npc/sCPU
```

运行仿真：

```bash
make -C npc/sCPU run
```

查看波形：

```bash
make -C npc/sCPU wave
```

波形文件输出为：

```text
npc/sCPU/build/wave.fst
```
