# Test Project Source Files

When adding a new `.cpp` source file to `40kRL.vcxproj`, **always** also add it to `Tests/40kRL_Tests.vcxproj` in the "Game source files" ItemGroup (the one with the comment "all except main.cpp to avoid duplicate main()").

The test project links all game source files except `main.cpp`. If a new source file is not added to both projects, the test build will fail with unresolved external symbols (LNK2001).

Format:
```xml
<ClCompile Include="..\Source\NewFile.cpp" />
```

This applies to all agents: developer, qa-tester, and spec-task-execution.
