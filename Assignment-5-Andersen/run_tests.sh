#!/bin/bash
# ============================================================
# 自动批量测试 Andersen 指针分析
# 作者：zhao yi
# ============================================================

# 项目路径设置
PROJECT_DIR="/home/zyl94/assignment-5-andersen-zyi952694-crypto/Assignment-5-Andersen"
TEST_DIR="$PROJECT_DIR/Test-Cases"
RESULT_DIR="$PROJECT_DIR/Results"
ANDERSEN_EXE="$PROJECT_DIR/andersen"

# 确保输出目录存在
mkdir -p "$RESULT_DIR"

# 检查 anderson 可执行文件是否存在
if [ ! -f "$ANDERSEN_EXE" ]; then
    echo "❌ 未找到可执行文件：$ANDERSEN_EXE"
    echo "请先确保已编译完成 (make 或 cmake --build .)"
    exit 1
fi

echo "============================================"
echo " 开始运行 Andersen 指针分析测试 "
echo " 测试目录: $TEST_DIR"
echo " 可执行文件: $ANDERSEN_EXE"
echo "============================================"

# 遍历所有 .c 测试文件
for cfile in "$TEST_DIR"/*.c; do
    filename=$(basename "$cfile" .c)
    bcfile="$TEST_DIR/$filename.bc"
    resultfile="$RESULT_DIR/${filename}_result.txt"

    echo ""
    echo ">>> 编译 $filename.c 为 LLVM bitcode..."
    clang -O0 -emit-llvm -c "$cfile" -o "$bcfile"

    if [ ! -f "$bcfile" ]; then
        echo "编译失败：$filename.c"
        continue
    fi

    echo ">>> 运行 Andersen 分析: $filename.bc ..."
    "$ANDERSEN_EXE" "$bcfile" > "$resultfile" 2>&1

    if [ $? -eq 0 ]; then
        echo "分析完成，结果保存到: $resultfile"
    else
        echo "分析执行失败，请检查输出文件: $resultfile"
    fi
done

echo ""
echo "============================================"
echo " 所有测试完成！结果保存在: $RESULT_DIR/"
echo "============================================"
