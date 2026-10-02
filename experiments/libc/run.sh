#!/usr/bin/env bash
# 构建八个程序，检查链接信息，再在四种环境中运行。
set -euo pipefail
root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
output="$root/results"
if [[ $# -gt 0 ]]; then
    if [[ $# -ne 2 || $1 != --output ]]; then
        echo "用法：bash run.sh [--output 目录]" >&2
        exit 2
    fi
    output=$2
fi
command -v python3 >/dev/null
docker info >/dev/null
mkdir -p "$output"
output=$(cd -- "$output" && pwd)
work=$(mktemp -d "$output/work-XXXXXX")
container=
cleanup() {
    if [[ -n $container ]]; then docker rm -f "$container" >/dev/null; fi
    rm -rf -- "$work"
}
trap cleanup EXIT
raw="$work/raw"
mkdir -p "$raw/artifacts"

# Docker 从当前环境读取代理值，命令中只传变量名。
proxy_args=()
for name in http_proxy https_proxy no_proxy HTTP_PROXY HTTPS_PROXY NO_PROXY; do
    if [[ -n ${!name:-} ]]; then proxy_args+=(--build-arg "$name"); fi
done
build() {
    local dockerfile=$1 context=$2 target=$3
    echo "构建 $target …"
    if ! docker build "${proxy_args[@]}" -f "$dockerfile" --target "$target" \
        -t "learning-c-libc:$target" "$context" >"$work/build.log" 2>&1; then
        cat "$work/build.log" >&2
        return 1
    fi
}
run() {
    local target=$1
    shift
    docker run --rm --network none "learning-c-libc:$target" "$@"
}
copy_files() {
    local target=$1 source=$2 destination=$3
    mkdir -p "$destination"
    container=$(docker create "learning-c-libc:$target")
    docker cp "$container:$source/." "$destination/"
    docker rm "$container" >/dev/null
    container=
}

# 1. Debian 和 Alpine 各构建两个程序；Arch 用 gcc 和 musl-gcc 构建四个。
for target in debian-build alpine-build arch-libs arch-build; do
    build "$root/Dockerfile" "$root" "$target"
done
for origin in debian alpine arch; do
    directory="$raw/builders/$origin"
    mkdir -p "$directory"
    docker image inspect --format '{{.Id}}' "learning-c-libc:$origin-build" >"$directory/image_id"
    copy_files "$origin-build" /out "$raw/artifacts"
done
run debian-build sh -c 'gcc -dumpfullversion; dpkg-query -W gcc libc6 libc6-dev' >"$raw/builders/debian/versions"
run alpine-build sh -c 'gcc -dumpfullversion; apk list --installed musl gcc' >"$raw/builders/alpine/versions"
run arch-build sh -c 'gcc -dumpfullversion; pacman -Q gcc glibc musl' >"$raw/builders/arch/versions"
run arch-build sh -c 'cat "$(command -v musl-gcc)"' >"$raw/builders/arch/wrapper"
run arch-build cat /usr/lib/musl/lib/musl-gcc.specs >"$raw/builders/arch/specs"

# 2. 使用 readelf 和 nm 记录八个文件的链接信息。
build "$root/runtime.Dockerfile" "$raw" inspect
for binary in "$raw/artifacts/"*; do
    name=${binary##*/}
    directory="$raw/checks/$name"
    mkdir -p "$directory"
    run inspect readelf -SW "/programs/$name" >"$directory/sections"
    run inspect readelf -lW "/programs/$name" >"$directory/headers"
    run inspect readelf -dW "/programs/$name" >"$directory/dynamic"
    run inspect readelf --version-info "/programs/$name" >"$directory/versions"
    run inspect nm --defined-only "/programs/$name" >"$directory/symbols"
done

# 3. 四种运行镜像均放入八个文件，分别执行并保存退出码、标准输出和错误输出。
for env in debian alpine arch scratch; do
    build "$root/runtime.Dockerfile" "$raw" "$env-run"
    copy_files "$env-run" /programs "$raw/runtime-files/$env"
    for binary in "$raw/artifacts/"*; do
        name=${binary##*/}
        directory="$raw/runs/$env/$name"
        mkdir -p "$directory"
        code=0
        run "$env-run" "/programs/$name" >"$directory/stdout" 2>"$directory/stderr" || code=$?
        echo "$code" >"$directory/exit_code"
    done
done

# 4. Python 解析检查结果、核对文件和 32 个运行结果，生成表格报告。
python3 "$root/report.py" "$raw" "$output"
