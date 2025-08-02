# 构建
在项目根目录下执行
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug -S . -B build
cmake --build build -j 编译 build/ 目录中的构建系统，实质上等同于进入该目录后执行 make -j 或 ninja -j
则会在build目录下生成complie_commands.json文件,实现C++代码提示和跳转
# 格式化
clang-format -i your_file.cpp 单个文件

find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i 整个项目
想实时格式化可启用 "editor.formatOnSave": true