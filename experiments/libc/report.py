#!/usr/bin/env python3
"""Read experiment files and generate the libc comparison report."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import re
import sys

OUTPUT = "Hello, C!\n40 + 2 = 42\n"
BUILDS = (("debian", "glibc", "gcc"), ("alpine", "musl", "gcc"),
          ("arch", "glibc", "gcc"), ("arch", "musl", "musl-gcc"))
ENVIRONMENTS = ("debian", "alpine", "arch", "scratch")

def table(headers, rows):
    return "\n".join(["| " + " | ".join(headers) + " |",
                      "| " + " | ".join("---" for _ in headers) + " |"] +
                     ["| " + " | ".join(map(str, row)) + " |" for row in rows])


def validate_runs(report):
    available = {"debian": {"glibc"}, "alpine": {"musl"},
                 "arch": {"glibc", "musl"}, "scratch": set()}
    mismatches = []
    for artifact in report["artifacts"]:
        for env in ENVIRONMENTS:
            expected = artifact["mode"] == "static" or artifact["libc"] in available[env]
            result = report["runs"][artifact["name"]][env]
            result["expected_success"] = expected
            if result["success"] != expected:
                mismatches.append(f"{artifact['name']} → {env}：预期{'成功' if expected else '失败'}，"
                                  f"实际{'成功' if result['success'] else '失败'}")
    return mismatches


def render(report):
    lines = ["# libc 实验报告", "", f"执行时间：{report['timestamp']}", "",
             "## 构建与链接检查", "",
             table(["二进制", "字节数", "动态链接器", "共享库依赖"],
                   [(a['name'], a['bytes'], a['interpreter'] or "无",
                     ", ".join(a['needed']) or "无") for a in report['artifacts']]),
             "", "## 运行结果：8 × 4", "",
             table(["二进制", *ENVIRONMENTS],
                   [(a['name'], *["成功" if report['runs'][a['name']][env]['success']
                                 else "失败" for env in ENVIRONMENTS])
                    for a in report['artifacts']]), "", "## 失败详情", ""]
    for name, results in report['runs'].items():
        for env, result in results.items():
            if not result['success']:
                lines += [f"- {name} → {env}（退出码 {result['exit_code']}）："
                          f"`{(result['stderr'] or result['stdout']).strip().replace(chr(10), ' / ')}`"]
    if report["mismatches"]:
        lines += ["", "## 与实验预期不同的结果", "",
                  *["- " + message for message in report["mismatches"]]]
    lines += ["", "## 构建环境与显式选择", ""]
    for origin, metadata in report['builders'].items():
        lines += [f"### {origin}", "", "```text", metadata['versions'].strip(), "```", ""]
    lines += ["### Arch 的 musl-gcc 包装脚本", "", "```sh",
              report['builders']['arch']['musl_gcc_wrapper'].strip(), "```", "",
              "验证：八个文件的链接类型与所选 libc 一致；四个运行镜像中均包含相同的八个文件。",
              f"运行验证：{report['verified_cases']} 个结果已逐格对照预期，{len(report['mismatches'])} 个不一致。", ""]
    return "\n".join(lines)


def hashes(directory):
    return {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in directory.iterdir()}


def collect(raw):
    report = {"timestamp": datetime.now(timezone.utc).isoformat(), "builders": {},
              "artifacts": [], "runs": {}}
    names = {f"{origin}-{libc}-{mode}" for origin, libc, _ in BUILDS
             for mode in ("dynamic", "static")}
    expected_hashes = hashes(raw / "artifacts")
    if set(expected_hashes) != names:
        raise ValueError("构建产物没有恰好包含预期的八个二进制。")
    for origin in ("debian", "alpine", "arch"):
        directory = raw / "builders" / origin
        metadata = {"versions": (directory / "versions").read_text(),
                    "image_id": (directory / "image_id").read_text().strip()}
        if origin == "arch":
            metadata["musl_gcc_wrapper"] = (directory / "wrapper").read_text()
            metadata["musl_gcc_specs"] = (directory / "specs").read_text()
        report["builders"][origin] = metadata
    for origin, libc, compiler in BUILDS:
        for mode in ("dynamic", "static"):
            name = f"{origin}-{libc}-{mode}"
            directory = raw / "checks" / name
            headers = (directory / "headers").read_text()
            dynamic = (directory / "dynamic").read_text()
            symbols = (directory / "symbols").read_text()
            match = re.search(r"Requesting program interpreter: ([^\]]+)", headers)
            interpreter = match.group(1) if match else None
            needed = re.findall(r"\(NEEDED\).*?\[([^\]]+)\]", dynamic)
            printf_defined = bool(re.search(r"\b[TW] printf$", symbols, re.MULTILINE))
            if mode == "dynamic":
                marker = "ld-musl" if libc == "musl" else "ld-linux"
                if not interpreter or marker not in interpreter or not needed:
                    raise ValueError(f"{name} 的动态链接信息与所选 libc 不符。")
            elif interpreter or needed or not printf_defined:
                raise ValueError(f"{name} 的静态链接检查未通过。")
            report["artifacts"].append({
                "name": name, "origin": origin, "libc": libc, "mode": mode,
                "compiler": compiler, "bytes": (raw / "artifacts" / name).stat().st_size,
                "sha256": expected_hashes[name], "interpreter": interpreter,
                "needed": needed, "printf_defined": printf_defined,
                "version_info": (directory / "versions").read_text(),
                "readelf_dynamic": dynamic,
                "compile_command": f"{compiler} -std=c17 -Wall -Wextra -Wpedantic -c /work/main.c -o /work/{libc}.o",
                "link_command": f"{compiler} {'-static ' if mode == 'static' else ''}/work/{libc}.o -o /out/{name}"})
    for env in ENVIRONMENTS:
        if hashes(raw / "runtime-files" / env) != expected_hashes:
            raise ValueError(f"{env} 镜像没有包含完全相同的八个产物。")
        for name in sorted(names):
            directory = raw / "runs" / env / name
            code = int((directory / "exit_code").read_text())
            stdout = (directory / "stdout").read_text()
            stderr = (directory / "stderr").read_text()
            if code == 125:
                raise ValueError(f"Docker 无法启动测试容器：{stderr}")
            report["runs"].setdefault(name, {})[env] = {
                "success": code == 0 and stdout == OUTPUT,
                "exit_code": code, "stdout": stdout, "stderr": stderr}
    report["mismatches"] = validate_runs(report)
    report["verified_cases"] = sum(len(values) for values in report["runs"].values())
    return report


def main():
    raw, output = map(Path, sys.argv[1:])
    try:
        report = collect(raw)
        markdown = render(report)
        (output / "report.json").write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
        (output / "report.md").write_text(markdown)
        print(markdown)
        print(f"报告已保存：{output / 'report.json'}")
        return bool(report["mismatches"])
    except (OSError, ValueError) as error:
        print(f"实验未完成：{error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
