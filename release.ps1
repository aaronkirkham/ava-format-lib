$version = Get-Content ".\include\AvaFormatLib.h" | Select-String "#define AVAFORMATLIB_VERSION_" | ForEach-Object{$_ -replace '\D+(\d+)', '$1'}

if (Test-Path -LiteralPath ".\bin\AvaFormatLib") {
  Remove-Item ".\bin\AvaFormatLib" -Recurse -Force
}

New-Item -ItemType Directory -Path ".\bin\AvaFormatLib" | Out-Null
New-Item -ItemType Directory -Path ".\bin\AvaFormatLib\lib" | Out-Null
Copy-Item ".\bin\Debug\AvaFormatLib.lib" -Destination ".\bin\AvaFormatLib\lib\AvaFormatLib_d.lib"
Copy-Item ".\bin\Release\AvaFormatLib.lib" -Destination ".\bin\AvaFormatLib\lib\AvaFormatLib.lib"
Copy-Item ".\include\" -Recurse -Destination ".\bin\AvaFormatLib\include\"

Compress-Archive -Path ".\bin\AvaFormatLib" -DestinationPath ".\bin\AvaFormatLib-$($version -join ".").zip" -Force

Remove-Item ".\bin\AvaFormatLib" -Recurse -Force