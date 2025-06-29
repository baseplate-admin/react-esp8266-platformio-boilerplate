from pathlib import Path
from shutil import copytree, rmtree
from subprocess import check_output, Popen, PIPE, STDOUT, CalledProcessError
import os
import gzip

Import("env")


def gzipFile(file):
    # Skip files that are already compressed
    if file.endswith(".gz"):
        return

    with open(file, "rb") as f_in:
        with gzip.open(file + ".gz", "wb") as f_out:
            f_out.writelines(f_in)

    # Remove original file
    os.remove(file)


def flagExists(flag):
    build_flags = env.GetProjectOption("build_flags") or ""
    parsed_flags = env.ParseFlags(build_flags)
    for define in parsed_flags.get("CPPDEFINES", []):
        if define == flag or (isinstance(define, list) and define[0] == flag):
            return True
    return False


def buildWeb():
    os.chdir("interface")
    print("Building interface with npm")
    try:
        env.Execute("npm install --force")
        env.Execute("npm run build")
        buildPath = Path("dist")
        wwwPath = Path("../data")
        if wwwPath.exists() and wwwPath.is_dir():
            rmtree(wwwPath)
        if not flagExists("PROGMEM_WWW"):
            print("Copying interface to data directory")
            copytree(buildPath, wwwPath)
            for currentpath, folders, files in os.walk(wwwPath):
                for file in files:
                    if file.endswith(".html"):
                        continue
                    gzipFile(os.path.join(currentpath, file))
    finally:
        os.chdir("..")


if len(BUILD_TARGETS) == 0 or "upload" in BUILD_TARGETS:
    buildWeb()
else:
    print("Skipping build interface step for target(s): " + ", ".join(BUILD_TARGETS))
