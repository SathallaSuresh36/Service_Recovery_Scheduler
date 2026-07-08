#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build/cov"
REPORTS="$ROOT/build/reports"
GCOVR="${GCOVR:-$HOME/.local/bin/gcovr}"

rm -rf "$BUILD"
mkdir -p "$BUILD/tmp" "$REPORTS/tests" "$REPORTS/coverage"

export TMP="$BUILD/tmp"
export TEMP="$BUILD/tmp"

echo "==> Configuring (coverage)"
cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Debug -DSRS_ENABLE_COVERAGE=ON

echo "==> Building tests"
cmake --build "$BUILD" --target srs_tests

echo "==> Running tests"
XML="$REPORTS/tests/results.xml"
set +e
"$BUILD/unit_tests/srs_tests" --gtest_output="xml:$XML"
TEST_RC=$?
set -e

python3 - "$XML" "$REPORTS/tests/index.html" <<'PYEOF'
import sys, html, xml.etree.ElementTree as ET

xml_path, out_path = sys.argv[1], sys.argv[2]
root = ET.parse(xml_path).getroot()

def as_int(el, key):
    try:
        return int(el.get(key, "0"))
    except ValueError:
        return 0

total    = as_int(root, "tests")
failures = as_int(root, "failures")
errors   = as_int(root, "errors")
disabled = as_int(root, "disabled")
duration = root.get("time", "0")
passed   = total - failures - errors - disabled
ok       = (failures == 0 and errors == 0)

rows = []
for suite in root.findall("testsuite"):
    sname = suite.get("name", "")
    for case in suite.findall("testcase"):
        cname = case.get("name", "")
        ctime = case.get("time", "0")
        failure = case.find("failure")
        error = case.find("error")
        if failure is not None:
            status, detail = "FAIL", (failure.get("message") or failure.text or "")
        elif error is not None:
            status, detail = "ERROR", (error.get("message") or error.text or "")
        else:
            status, detail = "PASS", ""
        rows.append((sname, cname, status, ctime, detail))

def esc(s):
    return html.escape(str(s))

banner = "PASSED" if ok else "FAILED"
banner_class = "ok" if ok else "bad"

parts = []
parts.append("<!doctype html><html><head><meta charset='utf-8'>")
parts.append("<title>Unit Test Report</title><style>")
parts.append("body{font-family:Segoe UI,Arial,sans-serif;margin:2rem;color:#222}")
parts.append("h1{margin:0 0 .25rem}")
parts.append(".sub{color:#666;margin-bottom:1rem}")
parts.append(".banner{display:inline-block;padding:.4rem .9rem;border-radius:6px;color:#fff;font-weight:600}")
parts.append(".ok{background:#2e7d32}.bad{background:#c62828}")
parts.append("table{border-collapse:collapse;width:100%;margin-top:1rem}")
parts.append("th,td{border:1px solid #ddd;padding:.45rem .6rem;text-align:left;font-size:.92rem}")
parts.append("th{background:#f4f4f4}")
parts.append("tr.pass td.status{color:#2e7d32;font-weight:600}")
parts.append("tr.fail td.status,tr.error td.status{color:#c62828;font-weight:700}")
parts.append(".cards{display:flex;gap:1rem;margin-top:1rem}")
parts.append(".card{border:1px solid #ddd;border-radius:8px;padding:.7rem 1.1rem;min-width:90px}")
parts.append(".card .n{font-size:1.6rem;font-weight:700}.card .l{color:#666;font-size:.8rem}")
parts.append("pre{margin:.3rem 0 0;white-space:pre-wrap;color:#c62828;font-size:.82rem}")
parts.append("</style></head><body>")
parts.append("<h1>Unit Test Report</h1>")
parts.append("<div class='sub'>GoogleTest &middot; total time %ss</div>" % esc(duration))
parts.append("<span class='banner %s'>%s</span>" % (banner_class, banner))
parts.append("<div class='cards'>")
parts.append("<div class='card'><div class='n'>%d</div><div class='l'>Total</div></div>" % total)
parts.append("<div class='card'><div class='n'>%d</div><div class='l'>Passed</div></div>" % passed)
parts.append("<div class='card'><div class='n'>%d</div><div class='l'>Failed</div></div>" % failures)
parts.append("<div class='card'><div class='n'>%d</div><div class='l'>Errors</div></div>" % errors)
parts.append("<div class='card'><div class='n'>%d</div><div class='l'>Disabled</div></div>" % disabled)
parts.append("</div>")
parts.append("<table><thead><tr><th>Suite</th><th>Test</th><th>Status</th><th>Time (s)</th></tr></thead><tbody>")
for sname, cname, status, ctime, detail in rows:
    cls = {"PASS": "pass", "FAIL": "fail", "ERROR": "error"}[status]
    parts.append("<tr class='%s'><td>%s</td><td>%s</td><td class='status'>%s</td><td>%s</td></tr>"
                 % (cls, esc(sname), esc(cname), status, esc(ctime)))
    if detail:
        parts.append("<tr class='%s'><td colspan='4'><pre>%s</pre></td></tr>" % (cls, esc(detail)))
parts.append("</tbody></table></body></html>")

with open(out_path, "w", encoding="utf-8") as f:
    f.write("".join(parts))
print("Unit-test HTML report written to: %s" % out_path)
PYEOF

echo "==> Generating coverage"
"$GCOVR" \
    --root "$ROOT" \
    --object-directory "$BUILD" \
    --gcov-executable /usr/bin/gcov \
    --filter '(src|include)/.*' \
    --exclude '.*/_deps/.*' \
    --exclude-throw-branches \
    --exclude-unreachable-branches \
    --print-summary \
    --html --html-details \
    -o "$REPORTS/coverage/index.html"

echo
echo "==================== Reports ===================="
echo "  Unit tests : $REPORTS/tests/index.html"
echo "  Coverage   : $REPORTS/coverage/index.html"
echo "  JUnit XML  : $XML"
echo "================================================="

exit "$TEST_RC"
