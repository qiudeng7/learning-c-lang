#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
# 固定第三方源码，依赖和产物都留在当前 demo，不安装到系统目录。
onion_revision=de8ea938342b36c28024fd8393ebc27b8442a161
for tool in git cmake gcc make; do
    command -v "$tool" >/dev/null || { echo "Missing build tool: $tool" >&2; exit 1; }
done
mkdir -p .deps
if [[ ! -d .deps/onion-src/.git ]]; then
    git init .deps/onion-src
    git -C .deps/onion-src remote add origin https://github.com/davidmoreno/onion.git
fi
if ! git -C .deps/onion-src cat-file -e "${onion_revision}^{commit}" 2>/dev/null; then
    git -C .deps/onion-src fetch --depth 1 origin "$onion_revision"
fi
git -C .deps/onion-src checkout --detach "$onion_revision"
cmake -S .deps/onion-src -B .deps/onion-build \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_BUILD_TYPE=Release -DONION_USE_SSL=OFF -DONION_USE_PAM=OFF \
    -DONION_USE_PNG=OFF -DONION_USE_JPEG=OFF -DONION_USE_XML2=OFF \
    -DONION_USE_SYSTEMD=OFF -DONION_USE_SQLITE3=OFF -DONION_USE_REDIS=OFF \
    -DONION_USE_GC=OFF -DONION_USE_TESTS=OFF -DONION_EXAMPLES=OFF \
    -DONION_USE_BINDINGS_CPP=OFF
cmake --build .deps/onion-build --target onion_static --parallel 2
