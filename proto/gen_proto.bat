@echo off
setlocal enabledelayedexpansion

echo ��ʼ����proto����...
echo.

FOR %%p in (*.proto) do (����
	set proto=!proto!%%p 
)

echo %proto%
protoc --cpp_out F:\cocos\SurvivalGame\Classes\Messages ./%proto%

::����protobuf��game-manage��̨
::xcopy *.proto E:\Project\game-server2\game-server\game-manage\src\main\webapp\assets\proto /s /h

echo.
echo ִ�����...
echo.
PAUSE 