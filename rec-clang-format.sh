find . -path "./3rd-party" -prune -o \
-type f \( -name "*.h" -o -name "*.hpp" -o -name "*.c" -o -name "*.cc" -o -name "*.cpp" \) \
-exec clang-format -i {} \;