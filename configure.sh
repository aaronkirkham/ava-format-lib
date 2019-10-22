#/bin/bash

case "$OSTYPE" in
  linux*)             BINARY="./tools/premake5";      ACTION="gmake2";   ARCHIVE="linux.tar.gz" ;;
  darwin*)            BINARY="./tools/premake5";      ACTION="xcode4";   ARCHIVE="macosx.tar.gz" ;;
  win|msys|cygwin*)   BINARY="./tools/premake5.exe";  ACTION="vs2017";   ARCHIVE="windows.zip" ;;
  *)                  echo "Error! Unknown OS: $OSTYPE"; exit ;;
esac

if [ ! -f "$BINARY" ]; then
  echo "Downloading build tools..."

  mkdir -p "./tools"
  curl -L -sS "https://github.com/premake/premake-core/releases/download/v5.0.0-alpha14/premake-5.0.0-alpha14-$ARCHIVE" -o "./tools/$ARCHIVE"

  if [[ $ARCHIVE == *".zip" ]]; then
    unzip -q "./tools/$ARCHIVE" -d tools
  else
    tar -zxvf "./tools/$ARCHIVE" -C tools
  fi

  rm "./tools/$ARCHIVE"
fi

$BINARY --file=premake.lua $ACTION
