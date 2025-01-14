# file: clang-format.ps1
Param(
    [Parameter(Mandatory = $false)]
    [String]$StartPath = "."
)

Get-ChildItem -Path $StartPath -Recurse -File `
    -Include *.h, *.hpp, *.c, *.cc, *.cpp `
| Where-Object { $_.FullName -notmatch "3rd-party" } `
| ForEach-Object {
    clang-format -i $_.FullName
}