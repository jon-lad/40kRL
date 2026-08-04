# MSBuild Path

MSBuild is NOT on the system PATH. Always use the full path when invoking MSBuild:

```
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe"
```

Do NOT use:
- `msbuild` (not on PATH)
- `C:\Program Files\Microsoft Visual Studio\2022\Community\...` (wrong version folder)

Example build commands:
```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" Tests/40kRL_Tests.vcxproj /p:Configuration=Debug /p:Platform=x64 /m /v:minimal
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" 40kRL.vcxproj /p:Configuration=Debug /p:Platform=x64 /m /v:minimal
```

Example test run (after build):
```powershell
& ".\x64\Debug\40kRL_Tests.exe" "[action-system]"
```
