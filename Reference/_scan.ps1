$path = 'c:\Users\jonat\Documents\Projects\40kRL\Reference\850108181-The-Liber-Imperium-1-6.txt'
# Look for chapter 1 heading and anything after chapter 13
Write-Output '--- Chapter 1 area (lines 200-660) headings ---'
Select-String -Path $path -Pattern '^\s*1\.[01]\s+[A-Za-z]|Liber Imperium|Core Rules' |
  Where-Object { $_.LineNumber -gt 200 -and $_.LineNumber -lt 660 } |
  ForEach-Object { '{0}: {1}' -f $_.LineNumber, $_.Line.Trim() }
Write-Output '--- After chapter 13 (line > 65763), any N.0 headings ---'
Select-String -Path $path -Pattern '^\s*\d{1,2}\.0\s+[A-Za-z].*$' |
  Where-Object { $_.LineNumber -gt 65763 -and $_.Line.Trim() -notmatch '-\s*\d+\s*$' } |
  ForEach-Object { '{0}: {1}' -f $_.LineNumber, $_.Line.Trim() }
Write-Output '--- Last 5 lines ---'
Get-Content $path -Tail 5
