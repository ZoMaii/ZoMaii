:: 此文件为 windows 原生系统运行支持
:: Code Type: GBK2312
echo off
setlocal
title "[qcr] rmdir - windows"
@echo.

set date=%date:~0,4%%date:~5,2%%date:~8,2%
echo %0 正在运行中...
@echo.
echo ****************
set sourceFile=tmp.qcr
set tmpdir=runtime

mkdir ..\\.%tmpdir%
if exist %sourceFile% ( 
    for %%A in (%sourceFile%) do (
        if %%~zA==0 (
            echo 审查文件出了一些问题！请重新尝试！！！
            echo "[qcr][%time%](rmdir) > 复查文件参数时发生了错误，请检查 tmp.qcr 文件！" >> ..\\.runtime\%date%.log
        ) else (
            setlocal enabledelayedexpansion
            for /f "tokens=1-3 delims=," %%i in (%sourceFile%) do (
                set ddx=%%i

                echo 删除文件夹 !ddx!
                rmdir !ddx! /Q /S
            )
        )
    )

) else (
    echo 诶...你不应该跳过代码安全审查而单独打开此文件的~!
    echo "[qcr][%time%](rmdir) > 用户试图单独运行命令！已阻止无参数的运行！" >> ..\\.runtime\%date%.log
)
del tmp.qcr
pause
endlocal