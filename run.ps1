$ErrorActionPreference = 'Stop'

$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Build = Join-Path $Root 'build'
New-Item -ItemType Directory -Force -Path $Build | Out-Null

function Require-Tool([string]$Name) {
    if (-not (Get-Command $Name -ErrorAction SilentlyContinue)) {
        throw "Missing required tool: $Name"
    }
}

Require-Tool 'nasm'
Require-Tool 'qemu-system-i386'

$UsingGnu = $false

if ((Get-Command 'i686-elf-g++' -ErrorAction SilentlyContinue) -and
    (Get-Command 'i686-elf-ld' -ErrorAction SilentlyContinue) -and
    (Get-Command 'i686-elf-objcopy' -ErrorAction SilentlyContinue)) {
    $UsingGnu = $true
    $Cxx = 'i686-elf-g++'
    $Ld = 'i686-elf-ld'
    $Objcopy = 'i686-elf-objcopy'
} elseif ((Get-Command 'clang++' -ErrorAction SilentlyContinue) -and
          (Get-Command 'ld.lld' -ErrorAction SilentlyContinue) -and
          (Get-Command 'llvm-objcopy' -ErrorAction SilentlyContinue)) {
    $UsingGnu = $false
    $Cxx = 'clang++'
    $Ld = 'ld.lld'
    $Objcopy = 'llvm-objcopy'
} else {
    throw "Missing required toolchain. Install either GNU i686-elf (i686-elf-g++, i686-elf-ld, i686-elf-objcopy) or LLVM (clang++, ld.lld, llvm-objcopy)."
}

$BootBin = Join-Path $Build 'boot.bin'
$KernelEntryObj = Join-Path $Build 'kernel_entry.o'
$ConsoleObj = Join-Path $Build 'console.o'
$StringObj = Join-Path $Build 'string.o'
$KmainObj = Join-Path $Build 'kmain.o'
$KernelElf = Join-Path $Build 'kernel.elf'
$KernelBin = Join-Path $Build 'kernel.bin'
$ImgPath = Join-Path $Build 'os.img'

$SectorSize = 512
$KernelSectors = 16
$KernelBytes = $KernelSectors * $SectorSize
$DiskBytes = ($KernelSectors + 1) * $SectorSize

& nasm -f bin (Join-Path $Root 'boot\boot.asm') -o $BootBin
& nasm -f elf32 (Join-Path $Root 'kernel\kernel_entry.asm') -o $KernelEntryObj

$CxxFlags = @(
    '-std=c++20',
    '-ffreestanding',
    '-fno-exceptions',
    '-fno-rtti',
    '-fno-stack-protector',
    '-m32',
    '-O2',
    '-Wall',
    '-Wextra'
)

if (-not $UsingGnu) {
    $CxxFlags = @('--target=i386-elf') + $CxxFlags
}

& $Cxx @CxxFlags -c (Join-Path $Root 'kernel\console.cpp') -o $ConsoleObj
& $Cxx @CxxFlags -c (Join-Path $Root 'kernel\string.cpp') -o $StringObj
& $Cxx @CxxFlags -c (Join-Path $Root 'kernel\kmain.cpp') -o $KmainObj
& $Cxx @CxxFlags -c (Join-Path $Root 'kernel\shell.cpp') -o (Join-Path $Build 'shell.o')

& $Ld -m elf_i386 -T (Join-Path $Root 'linker.ld') -o $KernelElf $KernelEntryObj $KmainObj $ConsoleObj $StringObj (Join-Path $Build 'shell.o')
& $Objcopy -O binary $KernelElf $KernelBin

$KernelData = [System.IO.File]::ReadAllBytes($KernelBin)
if ($KernelData.Length -gt $KernelBytes) {
    throw "Kernel too large: $($KernelData.Length) bytes (max $KernelBytes)"
}

$BootData = [System.IO.File]::ReadAllBytes($BootBin)
if ($BootData.Length -ne $SectorSize) {
    throw "Boot sector must be exactly $SectorSize bytes, got $($BootData.Length)"
}

$KernelPadded = New-Object byte[] $KernelBytes
[System.Array]::Copy($KernelData, $KernelPadded, $KernelData.Length)

$fs = [System.IO.File]::Open($ImgPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::ReadWrite, [System.IO.FileShare]::None)
try {
    $fs.SetLength($DiskBytes)

    $fs.Position = 0
    $fs.Write($BootData, 0, $BootData.Length)

    $fs.Position = $SectorSize
    $fs.Write($KernelPadded, 0, $KernelPadded.Length)
} finally {
    $fs.Dispose()
}

if (-not (Test-Path $ImgPath)) {
    throw "Image not found: $ImgPath"
}

Write-Host "Launching QEMU with image: $ImgPath"

& qemu-system-i386 @('-drive', "format=raw,file=$ImgPath", '-no-reboot', '-no-shutdown')
