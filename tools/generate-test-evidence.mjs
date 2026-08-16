#!/usr/bin/env node

import { spawn, spawnSync } from "node:child_process";
import crypto from "node:crypto";
import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import process from "node:process";
import { fileURLToPath } from "node:url";

const scriptPath = fileURLToPath(import.meta.url);
const scriptDir = path.dirname(scriptPath);
const repoRoot = path.resolve(scriptDir, "..");

function usage() {
  console.log(`Usage:
  node tools/generate-test-evidence.mjs [options] [-- vitest arguments]

Options:
  -o, --output FILE  Markdown output path.
                     Default: docs/test-evidence/test-evidence-<KST timestamp>.md
  -f, --force        Overwrite evidence files with the same name.
  -h, --help         Show this help.

Examples:
  npm run test:evidence
  npm run test:evidence -- --output docs/test-evidence/phase-1.md
  npm run test:evidence -- -- packages/score-domain/tests/numberPredicates.test.ts`);
}

function parseArguments(arguments_) {
  let output;
  let force = false;
  let index = 0;

  for (; index < arguments_.length; index += 1) {
    const argument = arguments_[index];

    if (argument === "--") {
      index += 1;
      break;
    }

    if (argument === "-h" || argument === "--help") {
      usage();
      process.exit(0);
    }

    if (argument === "-f" || argument === "--force") {
      force = true;
      continue;
    }

    if (argument === "-o" || argument === "--output") {
      output = arguments_[index + 1];
      if (!output) {
        throw new Error(`${argument} requires a file path`);
      }
      index += 1;
      continue;
    }

    throw new Error(`unknown option: ${argument}`);
  }

  return {
    output,
    force,
    vitestArguments: arguments_.slice(index),
  };
}

function formatKoreanTimestamp(date = new Date()) {
  const parts = new Intl.DateTimeFormat("en-CA", {
    timeZone: "Asia/Seoul",
    year: "numeric",
    month: "2-digit",
    day: "2-digit",
    hour: "2-digit",
    minute: "2-digit",
    second: "2-digit",
    hourCycle: "h23",
  }).formatToParts(date);
  const values = Object.fromEntries(parts.map(({ type, value }) => [type, value]));
  return `${values.year}-${values.month}-${values.day}T${values.hour}:${values.minute}:${values.second}+09:00`;
}

function createRunId(date = new Date()) {
  return formatKoreanTimestamp(date).replace(/[-:]/g, "").replace("T", "-").slice(0, 15);
}

function run(command, arguments_, options = {}) {
  const result = spawnSync(command, arguments_, {
    cwd: repoRoot,
    encoding: "utf8",
    windowsHide: true,
    ...options,
  });

  if (result.error) {
    return { status: 127, stdout: "", stderr: result.error.message };
  }

  return {
    status: result.status ?? 1,
    stdout: result.stdout?.trimEnd() ?? "",
    stderr: result.stderr?.trimEnd() ?? "",
  };
}

function requireCommand(command, versionArguments) {
  const result = run(command, versionArguments);
  if (result.status !== 0) {
    throw new Error(`required command not found or unusable: ${command}`);
  }
  return result.stdout || result.stderr;
}

function sha256(data) {
  return crypto.createHash("sha256").update(data).digest("hex");
}

function sha256File(filePath) {
  return sha256(fs.readFileSync(filePath));
}

function collectFiles(directory, predicate) {
  if (!fs.existsSync(directory)) {
    return [];
  }

  const files = [];
  for (const entry of fs.readdirSync(directory, { withFileTypes: true })) {
    const entryPath = path.join(directory, entry.name);
    if (entry.isDirectory()) {
      if (entry.name !== "dist" && entry.name !== "node_modules") {
        files.push(...collectFiles(entryPath, predicate));
      }
    } else if (entry.isFile() && predicate(entryPath)) {
      files.push(entryPath);
    }
  }
  return files;
}

function sourceFingerprint() {
  const rootFiles = [
    "package.json",
    "package-lock.json",
    "tsconfig.json",
    "tsconfig.base.json",
    "vitest.config.ts",
  ].map((file) => path.join(repoRoot, file));
  const packageFiles = collectFiles(path.join(repoRoot, "packages"), (file) =>
    file.endsWith(".ts") ||
    path.basename(file) === "package.json" ||
    /^tsconfig(?:\..+)?\.json$/.test(path.basename(file)),
  );
  const files = [...rootFiles, ...packageFiles]
    .filter((file) => fs.existsSync(file))
    .sort((left, right) => left.localeCompare(right));

  const hash = crypto.createHash("sha256");
  for (const file of files) {
    const relativePath = path.relative(repoRoot, file).split(path.sep).join("/");
    hash.update(relativePath);
    hash.update("\0");
    hash.update(fs.readFileSync(file));
    hash.update("\0");
  }
  return hash.digest("hex");
}

function relativeToRepo(filePath) {
  return path.relative(repoRoot, filePath).split(path.sep).join("/");
}

function shellDisplay(arguments_) {
  return arguments_.map((argument) => {
    if (/^[A-Za-z0-9_./:=@+-]+$/.test(argument)) {
      return argument;
    }
    return JSON.stringify(argument);
  }).join(" ");
}

function runTests(npmCommand, arguments_, logPath) {
  return new Promise((resolve) => {
    const logStream = fs.createWriteStream(logPath, { flags: "w" });
    const child = spawn(npmCommand, arguments_, {
      cwd: repoRoot,
      env: process.env,
      windowsHide: true,
      stdio: ["inherit", "pipe", "pipe"],
    });

    const copy = (chunk, destination) => {
      destination.write(chunk);
      logStream.write(chunk);
    };

    child.stdout.on("data", (chunk) => copy(chunk, process.stdout));
    child.stderr.on("data", (chunk) => copy(chunk, process.stderr));
    child.on("error", (error) => {
      const message = `Failed to start npm test: ${error.message}\n`;
      process.stderr.write(message);
      logStream.write(message);
    });
    child.on("close", (code, signal) => {
      logStream.end(() => resolve({ exitCode: code ?? 1, signal }));
    });
  });
}

function stripAnsi(value) {
  return String(value ?? "").replace(/[\u001B\u009B][[\]()#;?]*(?:(?:(?:[a-zA-Z\d]*(?:;[-a-zA-Z\d\/#&.:=?%@~_]+)*)?\u0007)|(?:(?:\d{1,4}(?:[;:]\d{0,4})*)?[\dA-PR-TZcf-nq-uy=><~]))/g, "");
}

function cell(value) {
  return stripAnsi(value).replace(/\|/g, "\\|").replace(/\r?\n/g, "<br>");
}

function code(value) {
  return String(value ?? "").replace(/`/g, "\\`");
}

function indented(value) {
  const cleaned = stripAnsi(value).trimEnd();
  return cleaned
    ? cleaned.split(/\r?\n/).map((line) => `    ${line}`).join("\n")
    : "    (No content)";
}

function readJsonReport(jsonPath) {
  try {
    return { report: JSON.parse(fs.readFileSync(jsonPath, "utf8")), error: null };
  } catch (error) {
    return {
      report: null,
      error: error instanceof Error ? error.message : String(error),
    };
  }
}

function renderEvidence(context) {
  const {
    runId, outputPath, jsonPath, logPath, startedAt, completedAt,
    durationSeconds, exitCode, commandDisplay, nodeVersion, npmVersion,
    osDescription, gitCommit, gitBranch, gitStatus, worktreeState,
    fingerprint, scriptHash,
  } = context;
  const { report, error: reportReadError } = readJsonReport(jsonPath);
  const passed = exitCode === 0 && report?.success === true;
  const overall = report ? (passed ? "PASS" : "FAIL") : "ERROR";
  const testResults = Array.isArray(report?.testResults) ? report.testResults : [];
  const assertions = testResults.flatMap((fileResult) =>
    (fileResult.assertionResults ?? []).map((assertion) => ({ fileResult, assertion })),
  );
  const failedAssertions = assertions.filter(({ assertion }) => assertion.status === "failed");
  const failedFilesWithoutAssertions = testResults.filter((fileResult) =>
    fileResult.status === "failed" &&
    !(fileResult.assertionResults ?? []).some((assertion) => assertion.status === "failed"),
  );
  const jsonHash = fs.existsSync(jsonPath) ? sha256File(jsonPath) : "not-generated";
  const logHash = sha256File(logPath);

  const fileRows = testResults.map((result) => {
    let hash = "unavailable";
    try {
      hash = sha256File(result.name);
    } catch {}
    return {
      relativeName: relativeToRepo(result.name),
      hash,
      status: result.status,
    };
  });

  const lines = [];
  lines.push("# Automated Test Results and Evidence", "");
  lines.push(`- Evidence ID: \`TEST-${code(runId)}\``);
  lines.push(`- Overall result: **${overall}**`);
  lines.push(`- Generated at: ${code(completedAt)} (Asia/Seoul)`, "");
  lines.push("## 1. Test Overview", "");
  lines.push("| Item | Details |", "| --- | --- |");
  lines.push("| Test objective | Run the registered Vitest test cases and verify that their expected results are satisfied. |");
  lines.push("| Pass criteria | PASS when the process exit code is 0 and the Vitest JSON `success` value is `true`; otherwise FAIL or ERROR. |");
  lines.push(`| Command | \`${code(commandDisplay)}\` |`);
  lines.push(`| Started at | ${cell(startedAt)} |`);
  lines.push(`| Completed at | ${cell(completedAt)} |`);
  lines.push(`| Duration | ${durationSeconds} seconds |`);
  lines.push(`| Exit code | ${exitCode} |`, "");
  lines.push("## 2. Execution Environment and Traceability", "");
  lines.push("| Item | Value |", "| --- | --- |");
  lines.push(`| Operating environment | ${cell(osDescription)} |`);
  lines.push(`| Node.js | ${cell(nodeVersion)} |`);
  lines.push(`| npm | ${cell(npmVersion)} |`);
  lines.push(`| Git branch | \`${code(gitBranch)}\` |`);
  lines.push(`| Git commit | \`${code(gitCommit)}\` |`);
  lines.push(`| Working tree | **${cell(worktreeState)}** |`);
  lines.push(`| Test source fingerprint (SHA-256) | \`${code(fingerprint)}\` |`);
  lines.push(`| Generator script SHA-256 | \`${code(scriptHash)}\` |`, "");

  if (report) {
    const failedFiles = testResults.filter((result) => result.status === "failed").length;
    const passedFiles = testResults.filter((result) => result.status === "passed").length;
    lines.push("## 3. Summary", "");
    lines.push("| Category | Total | PASS | FAIL | SKIP/TODO |", "| --- | ---: | ---: | ---: | ---: |");
    lines.push(`| Test files | ${testResults.length} | ${passedFiles} | ${failedFiles} | ${Math.max(0, testResults.length - passedFiles - failedFiles)} |`);
    lines.push(`| Test cases | ${report.numTotalTests ?? assertions.length} | ${report.numPassedTests ?? 0} | ${report.numFailedTests ?? 0} | ${(report.numPendingTests ?? 0) + (report.numTodoTests ?? 0)} |`, "");
    lines.push("## 4. Test Case Results", "");
    lines.push("| Test ID | Test file | Test condition and expected result | Result | Duration (ms) |", "| --- | --- | --- | --- | ---: |");

    assertions.forEach(({ fileResult, assertion }, index) => {
      const condition = [...(assertion.ancestorTitles ?? []), assertion.title].join(" › ");
      const status = assertion.status === "passed"
        ? "PASS"
        : assertion.status === "failed" ? "FAIL" : String(assertion.status).toUpperCase();
      const duration = Number.isFinite(assertion.duration) ? assertion.duration.toFixed(2) : "-";
      lines.push(`| TC-${String(index + 1).padStart(3, "0")} | \`${code(relativeToRepo(fileResult.name))}\` | ${cell(condition)} | **${status}** | ${duration} |`);
    });
    if (assertions.length === 0) {
      lines.push("| - | - | No test cases were collected. | **ERROR** | - |");
    }
    lines.push("", "## 5. Failure Details", "");

    if (failedAssertions.length === 0 && failedFilesWithoutAssertions.length === 0) {
      lines.push("No test cases failed.", "");
    } else {
      failedAssertions.forEach(({ fileResult, assertion }, index) => {
        const condition = [...(assertion.ancestorTitles ?? []), assertion.title].join(" › ");
        lines.push(`### ${index + 1}. ${condition}`, "");
        lines.push(`- File: \`${code(relativeToRepo(fileResult.name))}\``, "");
        lines.push(indented((assertion.failureMessages ?? []).join("\n\n")), "");
      });
      failedFilesWithoutAssertions.forEach((fileResult, index) => {
        lines.push(`### ${failedAssertions.length + index + 1}. Test File Execution Failed`, "");
        lines.push(`- File: \`${code(relativeToRepo(fileResult.name))}\``, "");
        lines.push(indented(fileResult.message || "The test cases could not be collected."), "");
      });
    }

    lines.push("## 6. Test File Integrity", "");
    lines.push("| Test file | Result | SHA-256 |", "| --- | --- | --- |");
    fileRows.forEach(({ relativeName, status, hash }) => {
      lines.push(`| \`${code(relativeName)}\` | ${cell(status.toUpperCase())} | \`${code(hash)}\` |`);
    });
    lines.push("");
  } else {
    lines.push("## 3. Result Generation Error", "");
    lines.push("The Vitest JSON report could not be read, so per-test results could not be generated.", "");
    lines.push(indented(reportReadError), "");
  }

  lines.push("## 7. Raw Evidence", "");
  if (report) {
    lines.push(`- [Original Vitest JSON](./${encodeURI(path.basename(jsonPath))}) — SHA-256: \`${code(jsonHash)}\``);
  } else {
    lines.push("- Original Vitest JSON — not generated");
  }
  lines.push(`- [Execution Console Log](./${encodeURI(path.basename(logPath))}) — SHA-256: \`${code(logHash)}\``, "");
  lines.push("### Git Status at Execution Time", "");
  lines.push(indented(gitStatus), "");
  lines.push("## 8. Verdict and Usage Notes", "");
  if (overall === "PASS") {
    lines.push("All executed test cases passed, so the overall result is **PASS**.");
  } else if (overall === "FAIL") {
    lines.push("At least one test case failed or the test process terminated abnormally, so the overall result is **FAIL**.");
  } else {
    lines.push("A structured test report could not be generated, so the overall result is **ERROR**.");
  }
  lines.push("");
  lines.push("This document summarizes the test run. The JSON and log files with the same base name are the raw evidence. If the working tree was dirty, the Git commit alone cannot fully reproduce the source used for the run; verify the source fingerprint and recorded Git status as well.", "");

  fs.writeFileSync(outputPath, `${lines.join("\n")}\n`, "utf8");
  return overall;
}

async function main() {
  const { output, force, vitestArguments } = parseArguments(process.argv.slice(2));
  const startedDate = new Date();
  const runId = createRunId(startedDate);
  const outputInput = output ?? `docs/test-evidence/test-evidence-${runId}.md`;
  const outputPath = path.resolve(repoRoot, outputInput);

  if (path.extname(outputPath).toLowerCase() !== ".md") {
    throw new Error("output file must use the .md extension");
  }

  const evidenceBase = outputPath.slice(0, -3);
  const jsonPath = `${evidenceBase}.json`;
  const logPath = `${evidenceBase}.log`;
  const evidenceFiles = [outputPath, jsonPath, logPath];

  if (!force) {
    const existingFile = evidenceFiles.find((file) => fs.existsSync(file));
    if (existingFile) {
      throw new Error(`evidence file already exists: ${existingFile}\nUse --force to overwrite it.`);
    }
  }

  const npmCommand = process.platform === "win32" ? "npm.cmd" : "npm";
  const nodeVersion = process.version;
  const npmVersion = requireCommand(npmCommand, ["--version"]);
  requireCommand("git", ["--version"]);
  fs.mkdirSync(path.dirname(outputPath), { recursive: true });

  const gitCommitResult = run("git", ["rev-parse", "HEAD"]);
  const gitBranchResult = run("git", ["branch", "--show-current"]);
  const gitStatusResult = run("git", ["status", "--short"]);
  const gitCommit = gitCommitResult.status === 0 ? gitCommitResult.stdout : "unavailable";
  const gitBranch = gitBranchResult.stdout || "detached HEAD";
  const gitStatus = gitStatusResult.status === 0 ? (gitStatusResult.stdout || "(clean)") : "unavailable";
  const worktreeState = gitStatus === "(clean)" ? "clean" : "dirty";
  const fingerprint = sourceFingerprint();
  const scriptHash = sha256File(scriptPath);
  const jsonDisplay = relativeToRepo(jsonPath);
  const npmArguments = [
    "test",
    "--",
    "--reporter=json",
    `--outputFile=${jsonPath}`,
    ...vitestArguments,
  ];
  const commandDisplay = shellDisplay([
    "npm", "test", "--", "--reporter=json", `--outputFile=${jsonDisplay}`,
    ...vitestArguments,
  ]);

  console.log(`Running: ${commandDisplay}`);
  const { exitCode, signal } = await runTests(npmCommand, npmArguments, logPath);
  const completedDate = new Date();
  const overall = renderEvidence({
    runId,
    outputPath,
    jsonPath,
    logPath,
    startedAt: formatKoreanTimestamp(startedDate),
    completedAt: formatKoreanTimestamp(completedDate),
    durationSeconds: Math.max(0, Math.round((completedDate - startedDate) / 1000)),
    exitCode,
    commandDisplay,
    nodeVersion,
    npmVersion,
    osDescription: `${os.type()} ${os.release()} ${os.arch()} (${os.platform()})`,
    gitCommit,
    gitBranch,
    gitStatus,
    worktreeState,
    fingerprint,
    scriptHash,
  });

  console.log(`Evidence document: ${relativeToRepo(outputPath)}`);
  console.log(`Raw JSON: ${relativeToRepo(jsonPath)}`);
  console.log(`Console log: ${relativeToRepo(logPath)}`);
  console.log(`Overall result: ${overall}`);
  if (signal) {
    console.error(`Test process signal: ${signal}`);
  }
  process.exitCode = exitCode;
}

main().catch((error) => {
  console.error(`error: ${error instanceof Error ? error.message : String(error)}`);
  process.exitCode = 2;
});
