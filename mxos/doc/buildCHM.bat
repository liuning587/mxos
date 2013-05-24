@echo off
REM    ============================================
REM    说明：
REM          使用doxygen生成chm文档批处理文件
REM    支持eclipse生成chm文档
REM    更新日期：2012-12-4
REM    版本：v1.1
REM    作者：刘宁
REM    时间：2012-3-4
REM    ============================================
REM    1. 执行清理
REM    ============================================
echo   1. 清空以前输出
if exist refman.chm del /f /q dproc.chm
if exist html del /f /q html\*.*
REM    ============================================
REM    2. 执行执行doxygen生成文档
REM    ============================================
echo   2. 生成文档
doxygen chm.doxyfile
REM    ============================================
REM    3. 将Doxygen输出文件编码从utf-8转换到gbk
REM    ============================================
echo   3. 将Doxygen输出文件编码从utf-8转换到gbk
set    path=%path%;%cd%
cd     html

echo   处理chm输入文件
REM    call utf82gbk.bat index.hhp
echo   将index.hhp从utf-8编码转换到gbk编码
for    %%f in (index.hhp) do copy %%f %%f.utf8
for    %%f in (index.hhp) do iconv -c -f utf-8 -t gbk %%f.utf8 > %%f

REM    call utf82gbk.bat index.hhc
echo   将index.hhc从utf-8编码转换到gbk编码
for    %%f in (index.hhc) do copy %%f %%f.utf8
for    %%f in (index.hhc) do iconv -c -f utf-8 -t gbk %%f.utf8 > %%f

REM    call utf82gbk.bat index.hhk
echo   将index.hhp从utf-8编码转换到gbk编码
for    %%f in (index.hhk) do copy %%f %%f.utf8
for    %%f in (index.hhk) do iconv -c -f utf-8 -t gbk %%f.utf8 > %%f

REM    call html-utf82gbk *.html
echo   将index.hhp从utf-8编码转换到gbk编码
for    %%f in (*.html) do copy %%f %%f.utf8
for    %%f in (*.html) do iconv -c -f utf-8 -t gbk %%f.utf8 > %%f
echo   将*.html中的charset从UTF-8改为gb2312
fr     *.html -fic:charset=UTF-8 -t:charset=gb2312

REM    ============================================
REM    4. 使用hhc.exe生成chm文档
REM    ============================================
REM    call makechm.bat
echo   4. 使用hhc.exe生成chm文档
"C:\Program Files\HTML Help Workshop\hhc.exe" index.hhp

if exist index.chm copy index.chm ..\refman.chm
del /f /q *.chm
cd ..\
REM    ============================================
REM    5. rebuild结束
REM    ============================================
if exist html del /f /q html
echo   5. rebuild结束
pause
