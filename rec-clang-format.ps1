Get-ChildItem -Recurse -Include *.h, *.hpp, *.c, *.cc, *.cpp | ForEach-Object {
    clang-format -i $_.FullName
}
