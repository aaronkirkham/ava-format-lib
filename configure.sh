#/bin/bash

if [ ! -f "./premake5.exe" ]; then
  echo "Downloading build tools..."

  curl -L -sS "https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-windows.zip" -o "./premake.zip"

  unzip -q "./premake.zip"
  rm "./premake.zip"
fi

./premake5.exe --file=premake.lua vs2017
